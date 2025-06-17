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

#ifndef G1OJS_Tiny_Si5351_CLK0_H_
#define G1OJS_Tiny_Si5351_CLK0_H_

#include "Arduino.h"
#include "Wire.h"

#define G1OJS_SI5351_CLK0_VERSION "1.0.1"


class G1OJS_Tiny_Si5351_CLK0{
  public:
	void set_freq_Hz(uint32_t fout_Hz);
  private:
        void I2CFlexiWrite(uint8_t reg, uint8_t b0, 
            bool include_b1_to_b7 = false, 
            uint8_t b1 = 0, uint8_t b2 = 0, uint8_t b3 = 0,
            uint8_t b4 = 0, uint8_t b5 = 0, uint8_t b6 = 0, uint8_t b7 = 0);
};

#endif
