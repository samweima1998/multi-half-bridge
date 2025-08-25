// spi_ping.c
#include <bcm2835.h>
#include <stdio.h>

int main(void) {
    if (!bcm2835_init()) { printf("bcm2835_init failed\n"); return 1; }
    if (!bcm2835_spi_begin()) { printf("bcm2835_spi_begin failed\n"); return 1; }

    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048); // ~122 kHz

    char tx[3] = {0xAA, 0x55, 0xFF};
    char rx[3] = {0};
    bcm2835_spi_transfernb(tx, rx, 3);

    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}
