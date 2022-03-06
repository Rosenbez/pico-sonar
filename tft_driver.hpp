#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

// driver https://cdn-shop.adafruit.com/datasheets/ILI9340.pdf
// screen: https://cdn-shop.adafruit.com/datasheets/TM022HDH26_V1.0.pdf
// display breakout: https://www.adafruit.com/product/1480

// ADAFRTUIT DEFS
#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

// enters low power sleep mode. display stops, but memory commands
// still work. Takes 120ms for sleep out to finish.
// Display powers on in sleep mode, must power off.
#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition

// Mem Access control - send one byte:
// MY, MX, MV, ML, BGR, MH, 0, 0
// Row order, column order, row-column exchange, vert refresh order, bl/gr/rd toggle, horiz refresh order
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1  0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC
///  ADAFRUIT cmds

// memory write command. Send as command, and send pixel data after.
// resets column/page pointers to start.
#define MEMWRT 0x2C
#define MEMREAD 0x2E


class TFTDriver {
public:

    // Default constructor, uses c/s=gpio25 and c/dx=24
    // uses hardware spi0 pins.
    TFTDriver(int chip_select=25, int tft_data_cmd_x=24) {
        _cs = chip_select;
        _tft_dcx = tft_data_cmd_x;

        spi = spi0;
    }

    // Set the data/cmd pin to command (low)
    void set_command() {
        gpio_put(_tft_dcx, 0);
    }
    
    // Set data/cmd pin to data (high)
    void set_data() {
        gpio_put(_tft_dcx, 1);
    }
    
    void send_command(uint8_t command_byte) {
        set_command();
        spi_write_blocking(spi, &command_byte, 1);
    }
    
    void send_data(uint8_t* data_buff, int num_bytes) {
        set_data();
        spi_write_blocking(spi, data_buff, num_bytes);
    }
    
    void colset(uint16_t start_column, uint16_t end_column) {
        send_command(ILI9341_CASET);

        uint8_t colstart[] = {
            start_column >> 8, start_column & 0xFF 
        };
        uint8_t colend[] = {
            end_column >> 8, end_column & 0xFF
        };
    
        set_data();
        spi_write_blocking(spi, colstart, 2);
        spi_write_blocking(spi, colend, 2);
    }
    
    void paset(uint16_t start_row, uint16_t end_row) {
        send_command(ILI9341_PASET);

        uint8_t rowstart[] = {
            start_row >> 8, start_row & 0xff
        };
        uint8_t rowend[] = {
            end_row >> 8, end_row & 0xff
        };
    
        set_data();
        spi_write_blocking(spi, rowstart, 2);
        spi_write_blocking(spi, rowend, 2);
    
    }

    /// Fill screen with rbg color.
    // Color values between 0-63
    void fill_screen(uint8_t red, uint8_t green, uint8_t blue) {
        colset(0, width - 1);
        paset(0, height - 1);

        uint8_t pxbuf[3] = {blue << 2, green << 2, red << 2};

        send_command(MEMWRT);

        set_data();
        int total_pixs = width * height;
        for (int i = 0; i < total_pixs; i++) {
            spi_write_blocking(spi, pxbuf, 3);
        }
    }

    void write_pixel(uint8_t *color, uint16_t x, uint16_t y, uint8_t sz=1) {

        colset(x, x + sz - 1);
        paset(y, y + sz -1 );

        send_command(MEMWRT);
        //printf("writing color: %d, %d, %d\n", color[2], color[1], color[0]);

        for (int i = 0; i < (sz * sz); i++) {
            //puts("writepx");
            send_data(color, 3);
        }

    }


    void init() {
        puts("Running ILI9340 Startup Sequence!");
        float mhz = 50;
        spi_init(spi0, mhz * 1000000);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    
        gpio_init(_cs);
        gpio_set_dir(_cs, 1);
        gpio_put(_cs, 0);
    
        gpio_init(_tft_dcx);
        gpio_set_dir(_tft_dcx, 1);
        gpio_put(_tft_dcx, 0);
    
   
        send_command(ILI9341_SWRESET);
        sleep_ms(200);
    
        uint8_t cmd1 = 0xEF;
        uint8_t args1[] = {0x03, 0x80, 0x02};
        send_command(cmd1);
        send_data(args1, 3);
    
        uint8_t cmd2 = 0xCF;
        uint8_t args2[] = {0x00, 0xC1, 0x30};
        send_command(cmd2);
        send_data(args2, 3);
    
        uint8_t cmd3 = 0xED; 
        uint8_t args3[] = {0x64, 0x03, 0x12, 0x81};
        send_command(cmd3);
        send_data(args3, 4);
    
        uint8_t cmd4 = 0xE8;
        uint8_t args4[] = { 0x85, 0x00, 0x78};
        send_command(cmd4);
        send_data(args4, 3);
    
        uint8_t cmd5 = 0xCB;
        uint8_t args5[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
        send_command(cmd5);
        send_data(args5, 5);
    
        uint8_t cmd6 = 0xF7;
        uint8_t args6[] = {0x20};
        send_command(cmd6);
        send_data(args6, 1);
    
        uint8_t cmd7 = 0xEA;
        uint8_t args7[] = {0x00, 0x00};
        send_command(cmd7);
        send_data(args7, 2);
    
    
        send_command(ILI9341_PWCTR1);
        uint8_t args_pwctr1[] = {0x23};  // Power control VRH[5:0]
        send_data(args_pwctr1, 1);
        
        send_command(ILI9341_PWCTR2);
        uint8_t args_pwctr2[] = {0x10};   // Power control SAP[2:0];BT[3:0]
        send_data(args_pwctr2, 1);
    
        send_command(ILI9341_VMCTR1);
        uint8_t args_vmctr1[] = {0x3e, 0x28};   // VCM control
        send_data(args_vmctr1, 2);
        
        send_command(ILI9341_VMCTR2);
        uint8_t args_vmctr2[] = {0x86};  // VCM control2
        send_data(args_vmctr2, 1);
    
        // Mem Access control - send one byte:
        // MY, MX, MV, ML, BGR, MH, 0, 0
        // Row order, column order, row-column exchange, vert refresh order, bl/gr/rd toggle, horiz refresh order
        send_command(ILI9341_MADCTL);
        uint8_t args_madctl[] = {0xe0};   // Memory Access Control
        send_data(args_madctl, 1);
    
        send_command(ILI9341_VSCRSADD);
        uint8_t args_vscrsadd[] = {0x00};    // Vertical scroll zero
        send_data(args_vscrsadd, 1);
    
        send_command(ILI9341_PIXFMT);
        uint8_t args_pixfmt[] = {0x66}; // pixelformat! maybe change
        send_data(args_pixfmt, 1);
        
        send_command(ILI9341_FRMCTR1);
        uint8_t args_fmrctr1[] = {0x00, 0x18};
        send_data(args_fmrctr1, 2);
        
        send_command(ILI9341_DFUNCTR);
        uint8_t args_dfunctr[] = {0x08, 0x82, 0x27}; // Display Function Control
        send_data(args_dfunctr, 3);
    
        uint8_t cmd_gammfun = 0xF2;
        uint8_t args_gammfun[] = {0x00};   // 3Gamma Function Disable
        send_command(cmd_gammfun);
        send_data(args_gammfun, 1);
    
        send_command(ILI9341_GAMMASET);
        uint8_t args_gammaset[] = {0x01};  // Gamma curve selected
        send_data(args_gammaset, 1);
    
        send_command(ILI9341_GMCTRP1);
        uint8_t args_gmctrp1[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
            0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
        send_data(args_gmctrp1, 15);
    
        send_command(ILI9341_GMCTRN1);
        uint8_t args_gmctrn1[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
            0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};
        send_data(args_gmctrn1, 15);
    
        send_command(ILI9341_SLPOUT);               // Exit Sleep
        sleep_ms(150);
        send_command(ILI9341_DISPON);                // Display on
        sleep_ms(150);
    }


    int _cs, _tft_dcx;
    int width = 320;
    int height = 240;
    spi_inst_t *spi;
};