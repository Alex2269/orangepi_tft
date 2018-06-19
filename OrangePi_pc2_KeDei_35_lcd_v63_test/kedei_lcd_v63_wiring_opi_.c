#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>

#define LCD_SPI 1
#define LCD_CS 11
#define TOUCH_CS 0
#define LCD_WIDTH 480
#define LCD_HEIGHT 320

#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

volatile uint8_t color;
volatile uint8_t lcd_rotation;
volatile uint16_t lcd_h;
volatile uint16_t lcd_w;

void delayms(int ms)
{
  delay(ms);
}

int lcd_open(void)
{
  int r;
  r = wiringPiSetup();
  if (r<0)return -1;

  r = wiringPiSPISetup(LCD_SPI, 16000000);
  if(r<0)return -1;

  #if 0
  r = wiringPiSPISetup(TOUCH_CS, 1000000);
  if(r<0) return -1;

  #endif
  wiringPiSetup();
  pinMode(LCD_CS, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  return 0;
}

int lcd_close(void)
{
  return 0;
}

int spi_transmit(int devsel, uint8_t *data, int len)
{
  int ret = 0;
  digitalWrite(LCD_CS, HIGH);
  ret = wiringPiSPIDataRW(devsel, (unsigned char*)data, len);
  digitalWrite(LCD_CS, LOW);
  return ret;
}

void lcd_rst(void)
{
  uint8_t buff[3];
  buff[0] = 0x00;
  buff[1] = 0x01;
  buff[2] = 0x00;
  spi_transmit(LCD_SPI, &buff[0], sizeof(buff));
  delayms(50);

  buff[0] = 0x00;
  buff[1] = 0x00;
  buff[2] = 0x00;
  spi_transmit(LCD_SPI, &buff[0], sizeof(buff));
  delayms(100);

  buff[0] = 0x00;
  buff[1] = 0x01;
  buff[2] = 0x00;
  spi_transmit(LCD_SPI, &buff[0], sizeof(buff));
  delayms(50);
}

void lcd_cmd(uint8_t cmd)
{
  uint8_t b1[3];
  b1[0] = 0x11;
  b1[1] = 0x00;
  b1[2] = cmd;
  spi_transmit(LCD_SPI, &b1[0], sizeof(b1));
}

void lcd_data(uint8_t dat)
{
  uint8_t b1[3];
  b1[0] = 0x15;
  b1[1] = 0x00;
  b1[2] = dat;
  spi_transmit(LCD_SPI, &b1[0], sizeof(b1));
}

void lcd_color(uint16_t col)
{
  uint8_t b1[3];
  b1[0] = 0x15;
  b1[1] = col>>8;
  b1[2] = col&0xFF;
  spi_transmit(LCD_SPI, &b1[0], sizeof(b1));
}

void lcd_colorRGB(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t b1[3];
  uint16_t col = ((r<<8) & 0xF800) | ((g<<3) & 0x07E0) | ((b>>3) & 0x001F);
  b1[0] = 0x15;
  b1[1] = col>>8;
  b1[2] = col&0x00FF;
  spi_transmit(LCD_SPI, &b1[0], sizeof(b1));
}

void lcd_setrotation(uint8_t m)
{
  lcd_cmd(0x36);
  if(m==0)
  {
    lcd_data(0x40|0x08);
    lcd_w = LCD_HEIGHT;
    lcd_h = LCD_WIDTH;
  }
  if(m==1)
  {
    lcd_data(0x20|0x08);
    lcd_w = LCD_WIDTH;
    lcd_h = LCD_HEIGHT;
  }
  if(m==2)
  {
    lcd_data(0x80|0x08);
    lcd_w = LCD_HEIGHT;
    lcd_h = LCD_WIDTH;
  }
  if(m==3)
  {
    lcd_data(0x80|0x40|0x20|0x08);
    lcd_w = LCD_WIDTH;
    lcd_h = LCD_HEIGHT;
  }
}

void lcd_init(void)
{
  lcd_rst();
  lcd_cmd(0x00);
  delayms(10);
  lcd_cmd(0xFF); lcd_cmd(0xFF);
  delayms(10);
  lcd_cmd(0xFF); lcd_cmd(0xFF); lcd_cmd(0xFF); lcd_cmd(0xFF);
  delayms(15);
  lcd_cmd(0x11);
  delayms(150);
  lcd_cmd(0xB0); lcd_data(0x00);

  lcd_data(0x00); lcd_data(0x01); lcd_data(0x00); lcd_data(0x43);

  lcd_cmd(0xC6); lcd_data(0x00);
  lcd_cmd(0xC8); lcd_data(0x03); lcd_data(0x03); lcd_data(0x13); lcd_data(0x5C);

  lcd_cmd(0x35); lcd_data(0x00);
  lcd_cmd(0x36); lcd_data(0x60);
  lcd_cmd(0x3A); lcd_data(0x55);
  lcd_cmd(0xD0); lcd_data(0x07); lcd_data(0x07); lcd_data(0x1D); lcd_data(0x03);

  lcd_cmd(0x21); // INVON 0x21
  lcd_setrotation(1);
  lcd_cmd(0x29); // Display ON
}

void lcd_setframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  lcd_cmd(0x2A);
  lcd_data(x>>8); lcd_data(x&0xFF); lcd_data(((w+x)-1)>>8); lcd_data(((w+x)-1)&0xFF);
  lcd_cmd(0x2B);
  lcd_data(y>>8); lcd_data(y&0xFF); lcd_data(((h+y)-1)>>8); lcd_data(((h+y)-1)&0xFF);
  lcd_cmd(0x2C);
}

void lcd_fillframe(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t col)
{
  int span=h*w;
  int q;
  lcd_setframe(x,y,w,h);
  for(q=0;q<span;q++)
  {
    lcd_color(col);
  }
}

void lcd_fill(uint16_t col)
{
  lcd_fillframe(0, 0, lcd_w, lcd_h, col);
}

void lcd_fillframeRGB(uint16_t x, uint16_t y,
uint16_t w, uint16_t h,
uint8_t r, uint8_t g, uint8_t b)
{
  int span=h*w;
  lcd_setframe(x,y,w,h);
  int q;
  for(q=0;q<span;q++)
  {
    lcd_colorRGB(r, g, b);
  }
}

void lcd_fillRGB(uint8_t r, uint8_t g, uint8_t b)
{
  lcd_fillframeRGB(0, 0, lcd_w, lcd_h, r, g, b);
}

void lcd_img(char *fname, uint16_t x, uint16_t y)
{
  uint8_t buf[54];
  uint16_t p, c;
  uint32_t isize, ioffset, iwidth, iheight, ibpp, fpos, rowbytes;
  FILE *f = fopen(fname, "rb");
  if (f != NULL)
  {
    fseek(f, 0L, SEEK_SET);
    fread(buf, 30, 1, f);
    isize = buf[2] + (buf[3]<<8) + (buf[4]<<16) + (buf[5]<<24);
    ioffset = buf[10] + (buf[11]<<8) + (buf[12]<<16) + (buf[13]<<24);
    iwidth = buf[18] + (buf[19]<<8) + (buf[20]<<16) + (buf[21]<<24);
    iheight = buf[22] + (buf[23]<<8) + (buf[24]<<16) + (buf[25]<<24);
    ibpp = buf[28] + (buf[29]<<8);
    printf("\n\n");
    printf("File Size: %u\nOffset: %u\nWidth: %u\nHeight: %u\nBPP: %u\n\n",isize,ioffset,iwidth,iheight,ibpp);
    lcd_setframe(x,y,iwidth,iheight);
    rowbytes=(iwidth*3) + 4-((iwidth*3)%4);
    for (p=iheight-1;p>0;p--)
    {
      fpos = ioffset+(p*rowbytes);
      fseek(f, fpos, SEEK_SET);
      for (c=0;c<iwidth;c++)
      {
        fread(buf, 3, 1, f);
        lcd_colorRGB(buf[2], buf[1], buf[0]);
      }
    }
    fclose(f);
  }
}

int main(int argc,char *argv[])
{
  if(lcd_open())
  {
    printf("error!\n");
    exit(-1);
  }
  lcd_init();

  lcd_fill(0x0);
  lcd_img("kedei_lcd_v50_pi.bmp", 50, 5);
  delayms(500);
  lcd_fillRGB(0xFF, 0x00, 0x00);
  lcd_fillRGB(0x00, 0xFF, 0x00);
  lcd_fillRGB(0xFF, 0xFF, 0x00);
  lcd_fillRGB(0x00, 0x00, 0xFF);
  lcd_fillRGB(0xFF, 0x00, 0xFF);
  lcd_fillRGB(0x00, 0xFF, 0xFF);
  lcd_fillRGB(0xFF, 0xFF, 0xFF);
  lcd_fillRGB(0x00, 0x00, 0x00);
  lcd_img("kedei_lcd_v50_pi.bmp", 50, 5);
  delayms(500);
  lcd_close();
}
