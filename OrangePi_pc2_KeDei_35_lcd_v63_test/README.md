KeDei 6.3 SPI TFT Display Test Program



#### compile
cd ~/OrangePi_KeDei_35_lcd_v63  
make

#### execute !
./kedei_lcd_v63_pi_wiringpi

``` cpp
Enable SPI:
edit file /boot/armbianEnv.txt

verbosity=1
console=both
overlay_prefix=sun50i-h5
overlays=spi-spidev
param_spidev_spi_bus=1
param_spidev_spi_cs=0
param_spidev_max_freq=100000000
rootdev=UUID=dff839d5-7c67-464f-8866-c0029c7ec486
rootfstype=ext4
usbstoragequirks=0x2537:0x1066:u,0x2537:0x1068:u

```



