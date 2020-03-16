/* LICENSE>>
Copyright 2020 Soji Yamakawa (CaptainYS, http://www.ysflight.com)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<< LICENSE */
#include "ym2612.h"



void YM2612::State::PowerOn(void)
{
	Reset();
}
void YM2612::State::Reset(void)
{
	deviceTimeInNS=0;
	lastTickTimeInNS=0;
	for(auto &r : reg)
	{
		r=0;
	}
	for(auto &t : timerCounter)
	{
		t=0;
	}
	for(auto &b : timerUp)
	{
		b=false;
	}
}

////////////////////////////////////////////////////////////

YM2612::YM2612()
{
	PowerOn();
}
YM2612::~YM2612()
{
}
void YM2612::PowerOn(void)
{
	state.PowerOn();
}
void YM2612::Reset(void)
{
	state.Reset();
}
void YM2612::WriteRegister(unsigned int reg,unsigned int value)
{
	reg&=255;
	auto prev=state.reg[reg];
	state.reg[reg]=value;
	if(REG_TIMER_CONTROL==reg)
	{
		// [2] pp. 202 RESET bits will be cleared immediately after set.
		// .... I interpret it as RESET bits are always read to zero.
		state.reg[REG_TIMER_CONTROL]&=0xCF;

		// LOAD bits are mysterious.
		// [2] pp.201 tells that writing 1 to LOAD bit resets the counter and start counting.
		// Towns OS's behavior does not seem to be agree with it.
		// Timer A interrupt handler writes 0x3F to register 0x27.  If I implement as [2] pp.201,
		// Timer B counter is reset when Timer A is up, or vise versa.
		// Slower one of Timer A or B will never be up.
		//
		// There are some possibilities:
		// (1) The timer counter resets on the rising edge of LOAD.  When timer is up, LOAD will be cleared.
		//     Therefore, writing LOAD=1 when LOAD is already 1 does nothing.
		// (2) Towns OS writes (0118:[0727H])|0x15 for Timer A or (0118:[0727H])|0x2A for Timer B for resetting the counter.
		//     0118:[0727H] is a cached value (or read back) from YM2612 register 27H.  If LOAD bits are always zero when
		//     read, it won't reset the timer when resetting the other timer.
		// (3) The timer counter is reloaded on LOAD=1 only if the timer is up.
        //
		// There are some web sites such as:
		//   https://plutiedev.com/ym2612-registers#reg-27
		//   https://www.smspower.org/maxim/Documents/YM2612
		//   https://wiki.megadrive.org/index.php?title=YM2612_Registers
		// suggesting that LOAD bit means the timer is running.  If so, I guess (2) is unlikely.
		// With my elementary knowledge in FPGA programming, (1) looks to be more straight-forward.
		// Currently I go with (1).

		if(0==(prev&1) && 0!=(value&1)) // Load Timer A
		{
			unsigned int countHigh=state.reg[REG_TIMER_A_COUNT_HIGH];
			unsigned int countLow=state.reg[REG_TIMER_A_COUNT_LOW];
			auto count=(countHigh<<2)|(countLow&3);
			state.timerCounter[0]=count*TIMER_A_PER_TICK;
		}
		if(0==(prev&2) && 0!=(value&2)) // Load Timer B
		{
			state.timerCounter[1]=(unsigned int)(state.reg[REG_TIMER_B_COUNT])*TIMER_B_PER_TICK;
		}
		if(value&4) // Enable Timer A Flag
		{
		}
		if(value&8) // Enable Timer B Flag
		{
		}
		if(value&0x10) // Reset Timer A Flag
		{
			state.timerUp[0]=false;
		}
		if(value&0x20) // Reset Timer B Flag
		{
			state.timerUp[1]=false;
		}
	}
}
unsigned int YM2612::ReadRegister(unsigned int reg) const
{
	return state.reg[reg&255];
}
void YM2612::Run(unsigned long long int systemTimeInNS)
{
	if(0==state.deviceTimeInNS)
	{
		state.lastTickTimeInNS=systemTimeInNS;
		state.deviceTimeInNS=systemTimeInNS;
		return;
	}
	if(state.lastTickTimeInNS+TICK_DURATION_IN_NS<systemTimeInNS)
	{
		auto nTick=(systemTimeInNS-state.lastTickTimeInNS)/TICK_DURATION_IN_NS;
		state.lastTickTimeInNS+=nTick*TICK_DURATION_IN_NS;
		// See (1) in the above comment.
		if(0!=(state.reg[REG_TIMER_CONTROL]&0x01))
		{
			state.timerCounter[0]+=nTick;
			if(NTICK_TIMER_A<=state.timerCounter[0])
			{
				state.reg[REG_TIMER_CONTROL]&=(~0x01);
				if(0!=(state.reg[REG_TIMER_CONTROL]&0x04))
				{
					state.timerUp[0]=true;
				}
			}
		}
		if(0!=(state.reg[REG_TIMER_CONTROL]&0x02))
		{
			state.timerCounter[1]+=nTick;
			if(NTICK_TIMER_B<=state.timerCounter[1])
			{
				state.reg[REG_TIMER_CONTROL]&=(~0x02);
				if(0!=(state.reg[REG_TIMER_CONTROL]&0x08))
				{
					state.timerUp[1]=true;
				}
			}
		}
	}


	state.deviceTimeInNS=systemTimeInNS;
}
bool YM2612::TimerAUp(void) const
{
	return state.timerUp[0];
}
bool YM2612::TimerBUp(void) const
{
	return state.timerUp[1];
}
bool YM2612::TimerUp(unsigned int timerId) const
{
	switch(timerId&1)
	{
	default:
	case 0:
		return TimerAUp();
	case 1:
		return TimerBUp();
	}
}


#include "cpputil.h"

std::vector <std::string> YM2612::GetStatusText(void) const
{
	std::vector <std::string> text;
	std::string empty;

	text.push_back(empty);
	text.back()="YM2612";

	text.push_back(empty);
	text.back()="TimerA Up=";
	text.back().push_back(cpputil::BoolToChar(TimerAUp()));
	text.back()+="  Count Preset=";
	text.back()+=cpputil::Ustox((state.reg[REG_TIMER_A_COUNT_HIGH]<<2)|(state.reg[REG_TIMER_A_COUNT_LOW]&3));
	text.back()+="  Internal Count/Threshold=";
	text.back()+=cpputil::Uitox(state.timerCounter[0]&0xFFFFFFFF)+"/"+cpputil::Uitox(NTICK_TIMER_A);



	text.push_back(empty);
	text.back()+="TimerB Up=";
	text.back().push_back(cpputil::BoolToChar(TimerBUp()));
	text.back()+="  Count Preset=";
	text.back()+=cpputil::Ustox(state.reg[REG_TIMER_B_COUNT]);
	text.back()+="  Internal Count/Threshold=";
	text.back()+=cpputil::Uitox(state.timerCounter[1]&0xFFFFFFFF)+"/"+cpputil::Uitox(NTICK_TIMER_B);

	text.push_back(empty);
	text.back()="Timer Control(Reg ";
	text.back()+=cpputil::Ubtox(REG_TIMER_CONTROL);
	text.back()+=")=";
	text.back()+=cpputil::Ubtox(state.reg[REG_TIMER_CONTROL]);
	text.back()+=" MODE:"+cpputil::Ubtox((state.reg[REG_TIMER_CONTROL]>>6)&3);
	text.back()+=" RST:"+cpputil::Ubtox((state.reg[REG_TIMER_CONTROL]>>4)&3);
	text.back()+=" ENA:"+cpputil::Ubtox((state.reg[REG_TIMER_CONTROL]>>2)&3);
	text.back()+=" LOAD:"+cpputil::Ubtox(state.reg[REG_TIMER_CONTROL]&3);


	return text;
}
