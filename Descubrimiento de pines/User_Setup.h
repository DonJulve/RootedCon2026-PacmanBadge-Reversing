#define ST7735_DRIVER
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// --- EL TIPO DE PANTALLA ---
// Si al probar esto los colores salen invertidos (ej. el azul se ve naranja), 
// cambia BLACKTAB por GREENTAB o REDTAB160
#define ST7735_BLACKTAB  

// --- TUS PINES HACKEADOS ---
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC   16
#define TFT_RST   4
#define TFT_BL   21

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define SPI_FREQUENCY  27000000
