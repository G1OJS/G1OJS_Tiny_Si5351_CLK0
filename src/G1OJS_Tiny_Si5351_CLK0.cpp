// MIT License
//
// Copyright (c) 2025 Alan Robinson G1OJS
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

//=====================
// Minimal library containing only one public function; to set the frequency of the Si5351
//
// Code is minimised by accepting the following limitations
//  - CLK0 only
//  - No checks for output frequencies above 150MHz or below 100MHz
//  - V1.0.0 tested (functional only) only between 128.7 and 146.7 MHz
//  - Correction factor, Crystal frequency and Crystal load capacitance for your specific Si5351 unit are all hard coded below (no functions to set them)
//  - Calculation and register setting is done in one single function (no registers are pre-set for speed)
//  - The Si5351 is assumed to be initialised (no waiting -> no need for an IC2 read function)
//
//  References cited in G1OJS_Tiny_Si5351_CLK0.cpp:
//   AN619 application note at https://www.skyworksinc.com/-/media/Skyworks/SL/documents/public/application-notes/AN619.pdf
//   Si5351A-B Data Sheet https://d1ehax0mqsd4rz.cloudfront.net/-/media/SkyWorks/SL/documents/public/data-sheets/Si5351-B.pdf
//  Great resource for understanding:
//   RfZero(TM) pages https://rfzero.net/tutorials/si5351a/
//=====================

#include "G1OJS_Tiny_Si5351_CLK0.h"

#include <stdint.h>
#include "Arduino.h"
#include "Wire.h"

#define CorrFact 0.999658117	        // Correction factor ( = fout_Hz / freq measured when CorrFact = 1.0)
#define i2c_bus_address 0x60		// address of Si5351 on the i2c bus


void G1OJS_Tiny_Si5351_CLK0::set_freq_Hz(uint32_t fout_Hz) { // set frequency fout_Hz (CLK0 only)

  // Follows data sheet Figure 10. I2C Programming Procedure

  // Figure 10 Box 1: Disable Outputs   
  // ----------------------------------------------
    I2CFlexiWrite(24, 0x00);   // CLK3–0 Disable State
    I2CFlexiWrite(3, 0xFF);    // Disable all CLK output drivers

  // Figure 10 Box 2: Power-down all output drivers
  // ----------------------------------------------
    I2CFlexiWrite(16,0x80,true,0x80,0x80,0x80,0x80,0x80,0x80,0x80);

  // Figure 10 Box 3
  // ----------------------------------------------
    // Register 2 bits are 7:SYS_INIT_MASK, 6:LOL_B_MASK, 5:LOL_A _MASK, 4:LOS_CLKINMASK, 3:LOS_XTALMASK, 2:0 Reserved
    // This is the Interrupt Status Mask Register and we don't need to touch the default values (all masked i.e. disabled) 
    // unless we want to poll the status register

  // Figure 10 Box 4: Build and write our specific configuration (see AN619 sections 2, 3 and 4) 
  // ----------------------------------------------

    // Paragraph 3.1.1
    I2CFlexiWrite(15, 0x00);   // Para 3.1.1: Select the XTAL input as the reference clock for PLLA and PLLB
    I2CFlexiWrite(183, 0x2);   // Para 3.1.1: Set reference load C: 6 pF = 0x12, 8 pF = 0x92, 10 pF = 0xD2

    // Paragraph 2 "The device consists of two PLLs—PLLA and PLLB. Each PLL generates an intermediate
    // VCO frequency in the range of 600 to 900 MHz using a Feedback Multisynth" 
    // Knowing that our XTAL is 25 MHz, we can know from the first equation of paragraph 3.2 that 
    // the Feedback Multisynth setting a+b/c must be between 600/25 and 900/25, i.e. 24 to 36
    // The first equation of section 2 likewise constrains Output_Multisynth x R to between 4.09 and 6.99
    // Given that Output_Multisynth must be 4, 6, 8+b/c (paragraph 2.1.1 note 1), we can only conclude
    // that Output_Multisynth = 6 (a = 6, b = 0, c= don't care but >=1 and < 2^18) and R = 1

    // We can write the Output Multisynth registers and Output Divider setting directly from this conclusion
    // by looking at the register map and noting that (from above):
    // MS0_P1 = 128 x 6 + 0 - 512 = 256
    // MS0_P2 = 128 x 0 - 1 x 0  = 0
    // MS0_P3 = 1 (the lowest value that "c" can be) or in fact, as b=0, any integer represented in 18 bits
    // R0_DIV = 0 (see description of register 44 on page 34 of AN619)

    I2CFlexiWrite(42, 0, true, 1,0,1,0,0,0,0);

    // From above and the first equations in section 2, we know that 
    // fout_Hz = 25000000 x Feedback_Multisynth / 6, i.e. Feedback_Multisynth = fout_Hz * 6/25000000
    // The Feedback Multisynth setting is (paragraph 3.2) MSNA = a+b/c = fout_Hz * 6/25000000, 
    // and we need to calculate a, b, c for MS0A, not forgetting that we need to apply the correction factor to fout_Hz first
    double MSNA = fout_Hz * CorrFact * 6.0 / 25000000.0;
    uint32_t MSNAa = MSNA;
    #define MSNAc  1048575UL 	// largest allowed value for greatest precision
    uint32_t MSNAb = (double)(MSNA - MSNAa) * (double)MSNAc;      

    // Now we can calculate MSNA_P1, MSNA_P2, MSNA_P3 
    uint32_t MSNA_P1 = 128 * MSNAa + 128 * MSNAb / MSNAc - 512;
    uint32_t MSNA_P2 = 128 * MSNAb - MSNAc * (128 * MSNAb / MSNAc);
    uint32_t MSNA_P3 = MSNAc;
    
    // write the Feedback Multisynth registers
    I2CFlexiWrite(26,
	(uint8_t) ((MSNA_P3>>8) & 0xFF), 		// Reg 26 = MSNA_P3[15:8]
        true,						// write 7 more registers
    	(uint8_t) (MSNA_P3 & 0xFF), 			// Reg 27 = MSNA_P3[7:0] 
    	(uint8_t) ((MSNA_P1>>16) & 0x03), 		// Reg 28 = XXXXXXMSNA_P1[17:16]
    	(uint8_t) ((MSNA_P1>>8) & 0xFF),		// Reg 29 = MSNA_P1[15:8]  
    	(uint8_t) (MSNA_P1 & 0xFF),			// Reg 30 = MSNA_P1[7:0] 
    	(uint8_t) ((MSNA_P3>>12) & 0xF0) 
               + (uint8_t) ((MSNA_P2>>16) & 0x0F), 	// Reg 31 = MSNA_P3[19:16]MSNA_P2[19:16]
    	(uint8_t) ((MSNA_P2>>8) & 0xFF),		// Reg 32 = MSNA_P2[15:8]
    	(uint8_t) (MSNA_P2 & 0xFF)			// Reg 33 = MSNA_P2[7:0]
        );
    
    // power up clock 0, select PLLA etc
    I2CFlexiWrite(16, 0x4F);            // CLK0, PLLA, MS0 operates in integer mode, Output Clock 0 is not inverted, Select MultiSynth 0 as the source for CLK0 and 8 mA drive

    // Figure 10 Box 5: Reset PLLA (we are not using PLLB)
    delayMicroseconds(500);  			// Allow registers to settle before resetting the PLL
    I2CFlexiWrite(177, 0x20);  		// Reset the PLL

    // Figure 10 Box 6: Enable clock 1 output
    I2CFlexiWrite(3, 0x00);            	// Output Enable Control - just clock 0 (note that ClockBuilder uses 0 for all 8 bits)

  }

// Helper function I2CFlexiWrite writes one byte to the specified register,
// and optionally a further seven bytes to the following sequential registers
void G1OJS_Tiny_Si5351_CLK0::I2CFlexiWrite(uint8_t reg, uint8_t b0, 
            bool include_b1_to_b7 = false, 
            uint8_t b1 = 0, uint8_t b2 = 0, uint8_t b3 = 0,
            uint8_t b4 = 0, uint8_t b5 = 0, uint8_t b6 = 0, uint8_t b7 = 0)
  {
    Wire.beginTransmission(i2c_bus_address);
    Wire.write(reg);
    Wire.write(b0);
    if(include_b1_to_b7) {
      Wire.write(b1); Wire.write(b2); Wire.write(b3); Wire.write(b4);
      Wire.write(b5); Wire.write(b6); Wire.write(b7);
    }
    Wire.endTransmission();
}
