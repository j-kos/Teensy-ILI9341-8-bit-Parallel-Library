/*

See rights and use declaration in License.h

This library has been modified for the Maple Mini.

Includes DMA transfers on DMA1 CH2 and CH3.

*/

#include <Adafruit_ILI9341_8bit.h>


void RD_STROBE() {RD_ACTIVE; RD_IDLE;}
void WR_STROBE() {WR_ACTIVE; WR_IDLE;}
void swap(int16_t a, int16_t b) {int16_t t = a; a = b; b = t;}
void write8special(uint8_t c) {
  GPIOC_PSOR = 0;
  GPIOC_PCOR = 0;
  for (uint8_t b = 0; b < 8; b++) {
	  if (bitRead(c, b)) {
		  GPIOC_PSOR |= (1 << b);
	  } else {
		  GPIOC_PCOR |= (1 << b);
	  }
  }

  //TFT_DATA->regs->ODR = ((TFT_DATA->regs->ODR & 0xFF00) | ((c) & 0x00FF));//FF00 is Binary 1111111100000000

  WR_STROBE();
}



// Constructor when using 8080 mode of control.

Adafruit_ILI9341_8bit_STM::Adafruit_ILI9341_8bit_STM(void)

: Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {

  

  //Set PB4 - PB7 as output

  //Note: CRH and CRL are both 32 bits wide

  //Each pin is represented by 4 bits 0x3 (hex) sets that pin to O/P

  pinMode(TFT_RD, OUTPUT);
  pinMode(TFT_WR, OUTPUT);
  pinMode(TFT_RS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);

    CS_IDLE; // Set all control bits to HIGH (idle)

    CD_DATA; // Signals are ACTIVE LOW

    WR_IDLE;

    RD_IDLE;

  if(TFT_RST) {

    //pinMode(TFT_RST, OUTPUT);

    //Set PB8 as output

    pinMode(TFT_RST, OUTPUT);

    digitalWriteFast(TFT_RST, HIGH);

  }

  //set up 8 bit parallel port to write mode.

  setWriteDataBus();

}



void Adafruit_ILI9341_8bit_STM::setWriteDataBus(void) {

  // set the pins to output mode

  // not required to mask and assign, because all pins of bus are set together

  PORTC_PCR0 = PORT_PCR_MUX(0x1);
  PORTC_PCR1 = PORT_PCR_MUX(0x1);
  PORTC_PCR2 = PORT_PCR_MUX(0x1);
  PORTC_PCR3 = PORT_PCR_MUX(0x1);
  PORTC_PCR4 = PORT_PCR_MUX(0x1);
  PORTC_PCR5 = PORT_PCR_MUX(0x1);
  PORTC_PCR6 = PORT_PCR_MUX(0x1);
  PORTC_PCR7 = PORT_PCR_MUX(0x1);

  //each pin is configured by four bits, and 0b0011 or 0x3 means output mode (same as pinmode())
  GPIOC_PDDR = 0b11111111;
}



void Adafruit_ILI9341_8bit_STM::setReadDataBus(void) {

  //set the pins to input mode

  // not required to mask and assign, because all pins of bus are set together

  PORTC_PCR0 = PORT_PCR_MUX(0x1);
  PORTC_PCR1 = PORT_PCR_MUX(0x1);
  PORTC_PCR2 = PORT_PCR_MUX(0x1);
  PORTC_PCR3 = PORT_PCR_MUX(0x1);
  PORTC_PCR4 = PORT_PCR_MUX(0x1);
  PORTC_PCR5 = PORT_PCR_MUX(0x1);
  PORTC_PCR6 = PORT_PCR_MUX(0x1);
  PORTC_PCR7 = PORT_PCR_MUX(0x1);
  
  GPIOC_PDDR = 0;

  //8 in hex is 0b1000, which means input, same as pinmode()

  // for (uint8_t i = 0; i <= 7; i++){

  //   pinMode(DPINS[i], INPUT);

  // }

}



void Adafruit_ILI9341_8bit_STM::write8(uint8_t c) {



  //retain values of A8-A15, and update A0-A7

  CS_ACTIVE;

  //BRR or BSRR avoid read, mask write cycle time

  //BSRR is 32 bits wide. 1's in the most significant 16 bits signify pins to reset (clear)

  // 1's in least significant 16 bits signify pins to set high. 0's mean 'do nothing'

  GPIOC_PSOR = 0;
  GPIOC_PCOR = 0;
  for (uint8_t b = 0; b < 8; b++) {
	  if (bitRead(c, b)) {
		  GPIOC_PSOR |= (1 << b);
	  } else {
		  GPIOC_PCOR |= (1 << b);
	  }
  }

  //TFT_DATA->regs->ODR = ((TFT_DATA->regs->ODR & 0xFF00) | ((c) & 0x00FF));//FF00 is Binary 1111111100000000

  WR_STROBE();

  CS_IDLE;

  //delayMicroseconds(50); //used to observe patterns

}



void Adafruit_ILI9341_8bit_STM::writecommand(uint8_t c) {

  CD_COMMAND;



  write8(c);

}





void Adafruit_ILI9341_8bit_STM::writedata(uint8_t c) {

  CD_DATA;



  write8(c);

}



// Rather than a bazillion writecommand() and writedata() calls, screen

// initialization commands and arguments are organized in these tables

// stored in PROGMEM.  The table may look bulky, but that's mostly the

// formatting -- storage-wise this is hundreds of bytes more compact

// than the equivalent code.  Companion function follows.

#define DELAY 0x80





// Companion code to the above tables.  Reads and issues

// a series of LCD commands stored in PROGMEM byte array.

void Adafruit_ILI9341_8bit_STM::commandList(uint8_t *addr) {



  uint8_t  numCommands, numArgs;

  uint16_t ms;



  numCommands = pgm_read_byte(addr++);   // Number of commands to follow

  while (numCommands--) {                // For each command...

    writecommand(pgm_read_byte(addr++)); //   Read, issue command

    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow

    ms       = numArgs & DELAY;          //   If hibit set, delay follows args

    numArgs &= ~DELAY;                   //   Mask out delay bit

    while (numArgs--) {                  //   For each argument...

      writedata(pgm_read_byte(addr++));  //     Read, issue argument

    }



    if (ms) {

      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)

      if (ms == 255) ms = 500;    // If 255, delay for 500 ms

      delay(ms);

    }

  }

}





void Adafruit_ILI9341_8bit_STM::begin(void) {



  // toggle RST low to reset


    digitalWrite(TFT_RST, HIGH);

    delay(5);

    digitalWrite(TFT_RST, LOW);

    delay(20);

    digitalWrite(TFT_RST, HIGH);

    delay(150);


  



  writecommand(0xEF);

  writedata(0x03);

  writedata(0x80);

  writedata(0x02);



  writecommand(0xCF);

  writedata(0x00);

  writedata(0XC1);

  writedata(0X30);



  writecommand(0xED);

  writedata(0x64);

  writedata(0x03);

  writedata(0X12);

  writedata(0X81);



  writecommand(0xE8);

  writedata(0x85);

  writedata(0x00);

  writedata(0x78);



  writecommand(0xCB);

  writedata(0x39);

  writedata(0x2C);

  writedata(0x00);

  writedata(0x34);

  writedata(0x02);



  writecommand(0xF7);

  writedata(0x20);



  writecommand(0xEA);

  writedata(0x00);

  writedata(0x00);



  writecommand(ILI9341_PWCTR1);    //Power control

  writedata(0x23);   //VRH[5:0]



  writecommand(ILI9341_PWCTR2);    //Power control

  writedata(0x10);   //SAP[2:0];BT[3:0]



  writecommand(ILI9341_VMCTR1);    //VCM control

  writedata(0x3e); //�Աȶȵ���

  writedata(0x28);



  writecommand(ILI9341_VMCTR2);    //VCM control2

  writedata(0x86);  //--



  writecommand(ILI9341_MADCTL);    // Memory Access Control

  writedata(0x48);



  writecommand(ILI9341_PIXFMT);

  writedata(0x55);



  writecommand(ILI9341_FRMCTR1);

  writedata(0x00);

  writedata(0x18);



  writecommand(ILI9341_DFUNCTR);    // Display Function Control

  writedata(0x08);

  writedata(0x82);

  writedata(0x27);



  writecommand(0xF2);    // 3Gamma Function Disable

  writedata(0x00);



  writecommand(ILI9341_GAMMASET);    //Gamma curve selected

  writedata(0x01);



  writecommand(ILI9341_GMCTRP1);    //Set Gamma

  writedata(0x0F);

  writedata(0x31);

  writedata(0x2B);

  writedata(0x0C);

  writedata(0x0E);

  writedata(0x08);

  writedata(0x4E);

  writedata(0xF1);

  writedata(0x37);

  writedata(0x07);

  writedata(0x10);

  writedata(0x03);

  writedata(0x0E);

  writedata(0x09);

  writedata(0x00);



  writecommand(ILI9341_GMCTRN1);    //Set Gamma

  writedata(0x00);

  writedata(0x0E);

  writedata(0x14);

  writedata(0x03);

  writedata(0x11);

  writedata(0x07);

  writedata(0x31);

  writedata(0xC1);

  writedata(0x48);

  writedata(0x08);

  writedata(0x0F);

  writedata(0x0C);

  writedata(0x31);

  writedata(0x36);

  writedata(0x0F);



  writecommand(ILI9341_INVOFF); //Invert Off

  delay(120);

  writecommand(ILI9341_SLPOUT);    //Exit Sleep

  delay(120);

  writecommand(ILI9341_DISPON);    //Display on



}





void Adafruit_ILI9341_8bit_STM::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,

                                        uint16_t y1) {								

  CS_ACTIVE;

  CD_COMMAND;

  write8special(ILI9341_CASET); // Column addr set

  CD_DATA;

  write8special(x0 >> 8);

  write8special(x0 & 0xFF);     // XSTART

  write8special(x1 >> 8);

  write8special(x1 & 0xFF);     // XEND



  CD_COMMAND;



  write8special(ILI9341_PASET); // Row addr set

  CD_DATA;

  write8special(y0 >> 8);

  write8special(y0);     // YSTART

  write8special(y1 >> 8);

  write8special(y1);     // YEND

  CD_COMMAND;

  write8special(ILI9341_RAMWR); // write to RAM



}





void Adafruit_ILI9341_8bit_STM::pushColor(uint16_t color) {

  writedata(color >> 8);

  writedata(color);

}



void Adafruit_ILI9341_8bit_STM::drawPixel(int16_t x, int16_t y, uint16_t color) {



  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;



  setAddrWindow(x, y, x + 1, y + 1);

  CD_DATA;

  write8special(color >> 8);

  write8special(color);

  CS_IDLE;



}





void Adafruit_ILI9341_8bit_STM::drawFastVLine(int16_t x, int16_t y, int16_t h,

                                        uint16_t color) {



  // Rudimentary clipping

  if ((x >= _width) || (y >= _height || h < 1)) return;



  if ((y + h - 1) >= _height)

    h = _height - y;

  if (h < 2 ) {

	drawPixel(x, y, color);

	return;

  }



  //  if (hwSPI) spi_begin();

  setAddrWindow(x, y, x, y + h - 1);

  CD_DATA;

  uint8_t hi = color >> 8, lo = color;

  while (h--) {

    write8special(hi);

    write8special(lo);

  }

  CS_IDLE;

}





void Adafruit_ILI9341_8bit_STM::drawFastHLine(int16_t x, int16_t y, int16_t w,

                                        uint16_t color) {



  

  // Rudimentary clipping

  if ((x >= _width) || (y >= _height || w < 1)) return;

  if ((x + w - 1) >= _width)  w = _width - x;

  if (w < 2 ) {

	drawPixel(x, y, color);

	return;

  }



//  if (hwSPI) spi_begin();

  setAddrWindow(x, y, x + w - 1, y);

  CD_DATA;

	uint8_t hi = color >> 8, lo = color;

  while (w--) {

    write8special(hi);

    write8special(lo);

  }

  CS_IDLE;

}



void Adafruit_ILI9341_8bit_STM::fillScreen(uint16_t color) {



  fillRect(0, 0,  _width, _height, color);

}



// fill a rectangle

void Adafruit_ILI9341_8bit_STM::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,

                                   uint16_t color) {



  // rudimentary clipping (drawChar w/big text requires this)

  if ((x >= _width) || (y >= _height || h < 1 || w < 1)) return;

  if ((x + w - 1) >= _width)  w = _width  - x;

  if ((y + h - 1) >= _height) h = _height - y;

  if (w == 1 && h == 1) {

    drawPixel(x, y, color);

    return;

  }

  

  setAddrWindow(x, y, x + w - 1, y + h - 1);



  CD_DATA;

  uint8_t hi = color >> 8, lo = color;

  for(y=h; y>0; y--) {

    for(x=w; x>0; x--){

      write8special(hi);

      write8special(lo);

    }

  }

  CS_IDLE;



}



/*

* Draw lines faster by calculating straight sections and drawing them with fastVline and fastHline.

*/

#if defined (__STM32F1__)

void Adafruit_ILI9341_8bit_STM::drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color)

{

	if ((y0 < 0 && y1 <0) || (y0 > _height && y1 > _height)) return;

	if ((x0 < 0 && x1 <0) || (x0 > _width && x1 > _width)) return;

	if (x0 < 0) x0 = 0;

	if (x1 < 0) x1 = 0;

	if (y0 < 0) y0 = 0;

	if (y1 < 0) y1 = 0;



	if (y0 == y1) {

		if (x1 > x0) {

			drawFastHLine(x0, y0, x1 - x0 + 1, color);

		}

		else if (x1 < x0) {

			drawFastHLine(x1, y0, x0 - x1 + 1, color);

		}

		else { //x0==x1 and y0==y1

			drawPixel(x0, y0, color);

		}

		return;

	}

	else if (x0 == x1) {

		if (y1 > y0) {

			drawFastVLine(x0, y0, y1 - y0 + 1, color);

		}

		else {

			drawFastVLine(x0, y1, y0 - y1 + 1, color);

		}

		return;

	}



	bool steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep) {

		swap();();(x0, y0);

		swap();();(x1, y1);

	}

	if (x0 > x1) {

		swap();();(x0, x1);

		swap();();(y0, y1);

	}



	int16_t dx, dy;

	dx = x1 - x0;

	dy = abs(y1 - y0);



	int16_t err = dx / 2;

	int16_t ystep;



	if (y0 < y1) {

		ystep = 1;

	}

	else {

		ystep = -1;

	}



	int16_t xbegin = x0;

	lineBuffer[0] = color;

	//*csport &= ~cspinmask;

	if (steep) {

		for (; x0 <= x1; x0++) {

			err -= dy;

			if (err < 0) {

				int16_t len = x0 - xbegin;

				if (len) {

					drawFastVLine (y0, xbegin, len + 1, color);

					//writeVLine_cont_noCS_noFill(y0, xbegin, len + 1);

				}

				else {

					drawPixel(y0, x0, color);

					//writePixel_cont_noCS(y0, x0, color);

				}

				xbegin = x0 + 1;

				y0 += ystep;

				err += dx;

			}

		}

		if (x0 > xbegin + 1) {

			//writeVLine_cont_noCS_noFill(y0, xbegin, x0 - xbegin);

			drawFastVLine(y0, xbegin, x0 - xbegin, color);

		}



	}

	else {

		for (; x0 <= x1; x0++) {

			err -= dy;

			if (err < 0) {

				int16_t len = x0 - xbegin;

				if (len) {

					drawFastHLine(xbegin, y0, len + 1, color);

					//writeHLine_cont_noCS_noFill(xbegin, y0, len + 1);

				}

				else {

					drawPixel(x0, y0, color);

					//writePixel_cont_noCS(x0, y0, color);

				}

				xbegin = x0 + 1;

				y0 += ystep;

				err += dx;

			}

		}

		if (x0 > xbegin + 1) {

			//writeHLine_cont_noCS_noFill(xbegin, y0, x0 - xbegin);

			drawFastHLine(xbegin, y0, x0 - xbegin, color);

		}

	}

	//*csport |= cspinmask;

}

#endif







// Pass 8-bit (each) R,G,B, get back 16-bit packed color

uint16_t Adafruit_ILI9341_8bit_STM::color565(uint8_t r, uint8_t g, uint8_t b) {

  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

}





#define MADCTL_MY  0x80

#define MADCTL_MX  0x40

#define MADCTL_MV  0x20

#define MADCTL_ML  0x10

#define MADCTL_RGB 0x00

#define MADCTL_BGR 0x08

#define MADCTL_MH  0x04



void Adafruit_ILI9341_8bit_STM::setRotation(uint8_t m) {



  writecommand(ILI9341_MADCTL);

  rotation = m % 4; // can't be higher than 3

  switch (rotation) {

    case 0:

      writedata(MADCTL_MX | MADCTL_BGR);

      _width  = ILI9341_TFTWIDTH;

      _height = ILI9341_TFTHEIGHT;

      break;

    case 1:

      writedata(MADCTL_MV | MADCTL_BGR);

      _width  = ILI9341_TFTHEIGHT;

      _height = ILI9341_TFTWIDTH;

      break;

    case 2:

      writedata(MADCTL_MY | MADCTL_BGR);

      _width  = ILI9341_TFTWIDTH;

      _height = ILI9341_TFTHEIGHT;

      break;

    case 3:

      writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);

      _width  = ILI9341_TFTHEIGHT;

      _height = ILI9341_TFTWIDTH;

      break;

  }

}





void Adafruit_ILI9341_8bit_STM::invertDisplay(boolean i) {

  writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);

}





////////// stuff not actively being used, but kept for posterity





uint8_t Adafruit_ILI9341_8bit_STM::read8(void){

  RD_ACTIVE;

  delay(5);

  uint8_t temp = 0;
  if(digitalReadFast(15)) {temp |= (1 << 0);}
  if(digitalReadFast(22)) {temp |= (1 << 1);}
  if(digitalReadFast(23)) {temp |= (1 << 2);}
  if(digitalReadFast(9)) {temp |= (1 << 3);}
  if(digitalReadFast(10)) {temp |= (1 << 4);}
  if(digitalReadFast(13)) {temp |= (1 << 5);}
  if(digitalReadFast(11)) {temp |= (1 << 6);}
  if(digitalReadFast(12)) {temp |= (1 << 7);}
  RD_IDLE;

  delay(5);

  return temp;

}



uint8_t Adafruit_ILI9341_8bit_STM::readcommand8(uint8_t c) {

  writecommand(c);

  CS_ACTIVE;

  CD_DATA;

  setReadDataBus();

  delay(5);

  //single dummy data

  uint8_t data = read8();

  //real data

  data = read8();

  setWriteDataBus();

  CS_IDLE;

  return data;

}



 uint32_t Adafruit_ILI9341_8bit_STM::readID(void) {

 writecommand(ILI9341_RDDID);

 CS_ACTIVE;

 CD_DATA;

 setReadDataBus();

 uint32_t r = read8();

 r <<= 8;

 r |= read8();

 r <<= 8;

 r |= read8();

 r <<= 8;

 r |= read8();

 setWriteDataBus();

 CS_IDLE;

 return r;

 }