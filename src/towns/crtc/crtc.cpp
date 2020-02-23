#include <iostream>
#include <algorithm>

#include "cpputil.h"
#include "crtc.h"
#include "towns.h"
#include "townsdef.h"



void TownsCRTC::AnalogPalette::Reset(void)
{
	codeLatch=0;
	for(int i=0; i<2; ++i)
	{
		plt16[i][ 0].Set(  0,  0,  0);
		plt16[i][ 1].Set(  0,  0,128);
		plt16[i][ 2].Set(128,  0,  0);
		plt16[i][ 3].Set(128,  0,128);
		plt16[i][ 4].Set(  0,128,  0);
		plt16[i][ 5].Set(  0,128,128);
		plt16[i][ 6].Set(128,128,  0);
		plt16[i][ 7].Set(128,128,128);
		plt16[i][ 8].Set(  0,  0,  0);
		plt16[i][ 9].Set(  0,  0,255);
		plt16[i][10].Set(255,  0,  0);
		plt16[i][11].Set(255,  0,255);
		plt16[i][12].Set(  0,255,  0);
		plt16[i][13].Set(  0,255,255);
		plt16[i][14].Set(255,255,  0);
		plt16[i][15].Set(255,255,255);
	}

	for(int i=0; i<256; ++i)
	{
		plt256[i].Set(255,255,255);
	}
}

void TownsCRTC::AnalogPalette::Set16(unsigned int page,unsigned int component,unsigned char v)
{
	v=v&0xF0;
	v|=(v>>4);
	plt16[page][codeLatch&0x0F].v[component]=v;
}
void TownsCRTC::AnalogPalette::Set256(unsigned int component,unsigned char v)
{
	plt256[codeLatch].v[component]=v;
}
void TownsCRTC::AnalogPalette::SetRed(unsigned char v,unsigned int PLT)
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		Set16(0,0,v);
		break;
	case 2: // 16-color paletter Layer 1
		Set16(1,0,v);
		break;
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		Set256(0,v);
		break;
	}
}
void TownsCRTC::AnalogPalette::SetGreen(unsigned char v,unsigned int PLT)
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		Set16(0,1,v);
		break;
	case 2: // 16-color paletter Layer 1
		Set16(1,1,v);
		break;
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		Set256(1,v);
		break;
	}
}
void TownsCRTC::AnalogPalette::SetBlue(unsigned char v,unsigned int PLT)
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		Set16(0,2,v);
		break;
	case 2: // 16-color paletter Layer 1
		Set16(1,2,v);
		break;
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		Set256(2,v);
		break;
	}
}

unsigned char TownsCRTC::AnalogPalette::Get16(unsigned int page,unsigned int component) const
{
	return plt16[page][codeLatch&0x0F][component];
}
unsigned char TownsCRTC::AnalogPalette::Get256(unsigned int component) const
{
	return plt256[codeLatch][component];
}
unsigned char TownsCRTC::AnalogPalette::GetRed(unsigned int PLT) const
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		return Get16(0,0);
	case 2: // 16-color paletter Layer 1
		return Get16(1,0);
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		return Get256(0);
	}
	return 0;
}
unsigned char TownsCRTC::AnalogPalette::GetGreen(unsigned int PLT) const
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		return Get16(0,1);
	case 2: // 16-color paletter Layer 1
		return Get16(1,1);
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		return Get256(1);
	}
	return 0;
}
unsigned char TownsCRTC::AnalogPalette::GetBlue(unsigned int PLT) const
{
	switch(PLT)
	{
	case 0: // 16-color paletter Layer 0
		return Get16(0,2);
	case 2: // 16-color paletter Layer 1
		return Get16(1,2);
	case 1: // 256-color paletter
	case 3: // 256-color paletter
		return Get256(2);
	}
	return 0;
}


////////////////////////////////////////////////////////////


void TownsCRTC::State::Reset(void)
{
	unsigned int defCRTCReg[32]=
	{
		0x0040,0x0320,0x0000,0x0000,0x035F,0x0000,0x0010,0x0000,0x036F,0x009C,0x031C,0x009C,0x031C,0x0040,0x0360,0x0040,
		0x0360,0x0000,0x009C,0x0000,0x0050,0x0000,0x009C,0x0000,0x0050,0x004A,0x0001,0x0000,0x003F,0x0003,0x0000,0x0150,
	};
	for(int i=0; i<32; ++i)
	{
		crtcReg[i]=defCRTCReg[i];
	}
	crtcAddrLatch=0;

	unsigned char defSifter[4]={0x15,0x08,0,0};
	for(int i=0; i<4; ++i)
	{
		sifter[i]=defSifter[i];
	}
	sifterAddrLatch=0;

	for(auto &d : mxVideoOutCtrl)
	{
		d=0;
	}
	mxVideoOutCtrlAddrLatch=0;

	palette.Reset();
}

TownsCRTC::ScreenModeCache::ScreenModeCache()
{
	MakeFMRCompatible();
}

void TownsCRTC::ScreenModeCache::MakeFMRCompatible(void)
{
	numLayers=2;

	layer[0].VRAMAddr=0;
	layer[0].bitsPerPixel=4;
	layer[0].sizeOnMonitor=Vec2i::Make(640,400);
	layer[0].bytesPerLine=320;

	layer[1].VRAMAddr=0x40000;
	layer[1].bitsPerPixel=4;
	layer[1].sizeOnMonitor=Vec2i::Make(640,400);
	layer[1].bytesPerLine=512;
}

////////////////////////////////////////////////////////////


TownsCRTC::TownsCRTC(class FMTowns *ptr)
{
	townsPtr=ptr;
	state.mxVideoOutCtrl.resize(0x10000);
	state.Reset();

	CLKSELtoFreq[0]=28636; // 28636KHz
	CLKSELtoFreq[1]=24545; // 24545KHz
	CLKSELtoFreq[2]=25175; // 25175KHz
	CLKSELtoFreq[3]=21052; // 21052KHz

	// Tentatively
	cached=true;
}


// Let's say 60 frames per sec.
// 1 frame takes 16.7ms.
// Horizontal Scan frequency is say 31KHz.
// 1 line takes 0.032ms.
// 480 lines take 15.36ms.
// Then, VSYNC should be 1.34ms long.
// Will take screenmode into account eventually.
// Also should take HSYNC into account.
bool TownsCRTC::InVSYNC(const unsigned long long int townsTime) const
{
	auto intoFrame=townsTime%16700000;
	return  (15360000<intoFrame);
}
bool TownsCRTC::InHSYNC(const unsigned long long int townsTime) const
{
	auto intoFrame=townsTime%16700000;
	if(intoFrame<1536000)
	{
		auto intoLine=intoFrame%32000;
		return (30000<intoLine);
	}
	return false;
}

bool TownsCRTC::InSinglePageMode(void) const
{
	return (0==(state.sifter[0]&0x10));
}

unsigned int TownsCRTC::GetBaseClockFreq(void) const
{
	auto CLKSEL=state.crtcReg[REG_CR1]&3;
	static const unsigned int freqTable[4]=
	{
		28636300,
		24545400,
		25175000,
		21052500,
	};
	return freqTable[CLKSEL];
}
unsigned int TownsCRTC::GetBaseClockScaler(void) const
{
	auto SCSEL=state.crtcReg[REG_CR1]&0x0C;
	return (SCSEL>>1)+2;
}
Vec2i TownsCRTC::GetPageZoom(unsigned char page) const
{
	Vec2i zoom;
	auto pageZoom=(state.crtcReg[REG_ZOOM]>>(8*page));
	zoom.x()=(( pageZoom    &15)+1);
	zoom.y()=(((pageZoom>>4)&15)+1);
	return zoom;
}
Vec2i TownsCRTC::GetPageOriginOnMonitor(unsigned char page) const
{
	int x0,y0;
	switch(CLKSEL())
	{
	case 0:
		x0=(state.crtcReg[REG_HDS0]-0x129)>>1;
		y0=(state.crtcReg[REG_VDS0]-0x2a)>>1; // I'm not sure if I should divide by 2.  Will need experiments.
		break;
	case 1:
		x0=(state.crtcReg[REG_HDS0]-0xe7)>>1;
		y0=(state.crtcReg[REG_VDS0]-0x2a)>>1; // I'm not sure if I should divide by 2.  Will need experiments.
		break;
	case 2:
		x0=(state.crtcReg[REG_HDS0]-0x8a);
		y0=(state.crtcReg[REG_VDS0]-0x46);
		break;
	case 3:
		x0=(state.crtcReg[REG_HDS0]-0x9c);
		y0=(state.crtcReg[REG_VDS0]-0x40);
		break;
	default:
		x0=0;
		y0=0;
		break;
	}
	return Vec2i::Make(x0,y0);
}
Vec2i TownsCRTC::GetPageSizeOnMonitor(unsigned char page) const
{
	auto KHz=GetHorizontalFrequency();
	auto wid=state.crtcReg[REG_HDE0+page*2]-state.crtcReg[REG_HDS0+page*2];
	auto hei=state.crtcReg[REG_VDE0+page*2]-state.crtcReg[REG_VDS0+page*2];
	if(15==KHz)
	{
		wid/=2;
	}
	if(0==state.crtcReg[REG_FO0+4*page])
	{
		hei/=2;
	}
	return Vec2i::Make(wid,hei);
}
Vec2i TownsCRTC::GetPageVRAMCoverageSize1X(unsigned char page) const
{
	auto wid=state.crtcReg[REG_HDE0+page*2]-state.crtcReg[REG_HDS0+page*2];
	auto hei=state.crtcReg[REG_VDE0+page*2]-state.crtcReg[REG_VDS0+page*2];
	auto FO=state.crtcReg[REG_FO0+4*page];
	if(0==FO)
	{
		hei/=2;
	}
	return Vec2i::Make(wid,hei);
}
unsigned int TownsCRTC::GetPageBitsPerPixel(unsigned char page) const
{
	const unsigned int CL=(state.crtcReg[REG_CR0]>>(page*2))&3;
	if(true==InSinglePageMode())
	{
		if(2==CL)
		{
			return 16;
		}
		else if(3==CL)
		{
			return 256;
		}
	}
	else
	{
		if(1==CL)
		{
			return 16;
		}
		else if(3==CL)
		{
			return 4;
		}
	}
	std::cout << __FUNCTION__ << std::endl;
	std::cout << "Unknown color setting." << std::endl;
	return 4; // What else can I do?
}
unsigned int TownsCRTC::GetPageVRAMAddressOffset(unsigned char page) const
{
	// [2] pp. 145
	auto FA0=state.crtcReg[REG_FA0+page*4];
	switch(GetPageBitsPerPixel(page))
	{
	case 4:
		return FA0*4;  // 8 pixels for 1 count.
	case 8:
		return FA0*8;  // 8 pixels for 1 count.
	case 16:
		return (InSinglePageMode() ? FA0*8 : FA0*4); // 4 pixels or 2 pixels depending on the single-page or 2-page mode.
	}
	return 0;
}
unsigned int TownsCRTC::GetPriorityPage(void) const
{
	return state.sifter[1]&1;
}
unsigned int TownsCRTC::GetPageBytesPerLine(unsigned char page) const
{
	auto FOx=state.crtcReg[REG_FO0+page*4];
	auto LOx=state.crtcReg[REG_LO0+page*4];
	auto numBytes=(LOx-FOx)*4;
	if(true==InSinglePageMode())
	{
		numBytes*=2;
	}
	return numBytes;
}
void TownsCRTC::MakePageLayerInfo(Layer &layer,unsigned char page) const
{
	page&=1;
	layer.bitsPerPixel=GetPageBitsPerPixel(page);
	layer.originOnMonitor=GetPageOriginOnMonitor(page);
	layer.sizeOnMonitor=GetPageSizeOnMonitor(page);
	layer.VRAMCoverage1X=GetPageVRAMCoverageSize1X(page);
	layer.zoom=GetPageZoom(page);
	layer.VRAMAddr=0x40000*page;
	layer.VRAMOffset=GetPageVRAMAddressOffset(page);
	layer.bytesPerLine=GetPageBytesPerLine(page);
}

/* virtual */ void TownsCRTC::IOWriteByte(unsigned int ioport,unsigned int data)
{
	switch(ioport)
	{
	case TOWNSIO_ANALOGPALETTE_CODE://=  0xFD90,
		state.palette.codeLatch=data;
		break;
	case TOWNSIO_ANALOGPALETTE_BLUE://=  0xFD92,
		state.palette.SetBlue(data,(state.sifter[1]>>4)&3);
		break;
	case TOWNSIO_ANALOGPALETTE_RED://=   0xFD94,
		state.palette.SetRed(data,(state.sifter[1]>>4)&3);
		break;
	case TOWNSIO_ANALOGPALETTE_GREEN://= 0xFD96,
		state.palette.SetGreen(data,(state.sifter[1]>>4)&3);
		break;


	case TOWNSIO_CRTC_ADDRESS://             0x440,
		state.crtcAddrLatch=data&0x1f;
		break;
	case TOWNSIO_CRTC_DATA_LOW://            0x442,
		state.crtcReg[state.crtcAddrLatch]&=0xff00;
		state.crtcReg[state.crtcAddrLatch]|=(data&0xff);
		break;
	case TOWNSIO_CRTC_DATA_HIGH://           0x443,
		state.crtcReg[state.crtcAddrLatch]&=0x00ff;
		state.crtcReg[state.crtcAddrLatch]|=((data&0xff)<<8);
		break;


	case TOWNSIO_VIDEO_OUT_CTRL_ADDRESS://=   0x448,
		state.sifterAddrLatch=(data&3);
		break;
	case TOWNSIO_VIDEO_OUT_CTRL_DATA://=      0x44A,
		state.sifter[state.sifterAddrLatch]=data;
		break;


	case TOWNSIO_MX_HIRES://            0x470,
	case TOWNSIO_MX_VRAMSIZE://         0x471,
		break; // No write access;

	case TOWNSIO_MX_IMGOUT_ADDR_LOW://  0x472,
		state.mxVideoOutCtrlAddrLatch=((state.mxVideoOutCtrlAddrLatch&0xff00)|(data&0xff));
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_HIGH:// 0x473,
		state.mxVideoOutCtrlAddrLatch=((state.mxVideoOutCtrlAddrLatch&0x00ff)|((data<<8)&0xff));
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D0://   0x474,
		std::cout << "MX-VIDOUTCONTROL8[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch) << "H]=" << cpputil::Ubtox(data) << "H" << std::endl;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch]=data;
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D1://   0x475,
		if(state.mxVideoOutCtrlAddrLatch+1<state.mxVideoOutCtrl.size())
		{
			std::cout << "MX-VIDOUTCONTROL8[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch+1) << "H]=" << cpputil::Ubtox(data) << "H" << std::endl;
			state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+1]=data;
		}
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D2://   0x476,
		if(state.mxVideoOutCtrlAddrLatch+2<state.mxVideoOutCtrl.size())
		{
			std::cout << "MX-VIDOUTCONTROL8[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch+2) << "H]=" << cpputil::Ubtox(data) << "H" << std::endl;
			state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+2]=data;
		}
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D3://   0x477,
		if(state.mxVideoOutCtrlAddrLatch+3<state.mxVideoOutCtrl.size())
		{
			std::cout << "MX-VIDOUTCONTROL8[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch+3) << "H]=" << cpputil::Ubtox(data) << "H" << std::endl;
			state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+3]=data;
		}
		break;
	}
}

/* virtual */ void TownsCRTC::IOWriteWord(unsigned int ioport,unsigned int data)
{
	switch(ioport)
	{
	case TOWNSIO_CRTC_ADDRESS://             0x440,
		state.crtcAddrLatch=data&0x1f;
		break;
	case TOWNSIO_CRTC_DATA_LOW://            0x442,
		state.crtcReg[state.crtcAddrLatch]=(data&0xfff);
		break;
	case TOWNSIO_CRTC_DATA_HIGH://           0x443,
		break;

	case TOWNSIO_VIDEO_OUT_CTRL_ADDRESS://=   0x448,
		state.sifterAddrLatch=(data&3);
		break;
	case TOWNSIO_VIDEO_OUT_CTRL_DATA://=      0x44A,
		state.sifter[state.sifterAddrLatch]=data;
		break;

	case TOWNSIO_MX_HIRES://            0x470,
	case TOWNSIO_MX_VRAMSIZE://         0x471,
		break; // No write access;

	case TOWNSIO_MX_IMGOUT_ADDR_LOW://  0x472,
		state.mxVideoOutCtrlAddrLatch=(data&0xffff);
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D0://   0x474,
		std::cout << "MX-VIDOUTCONTROL16[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch) << "H]=" << cpputil::Ustox(data) << "H" << std::endl;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch]=data;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+1]=(data>>8)&255;
		break;
	default:
		Device::IOWriteWord(ioport,data); // Let it write twice.
		break;
	}
}

/* virtual */ void TownsCRTC::IOWriteDword(unsigned int ioport,unsigned int data)
{
	switch(ioport)
	{
	case TOWNSIO_MX_IMGOUT_ADDR_D0://   0x474,
		std::cout << "MX-VIDOUTCONTROL32[" << cpputil::Ustox(state.mxVideoOutCtrlAddrLatch) << "H]=" << cpputil::Uitox(data) << "H" << std::endl;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch]=data;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+1]=(data>>8)&255;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+2]=(data>>16)&255;
		state.mxVideoOutCtrl[state.mxVideoOutCtrlAddrLatch+3]=(data>>24)&255;
		break;
	default:
		Device::IOWriteWord(ioport,data); // Let it write 4 times.
		break;
	}
}

/* virtual */ unsigned int TownsCRTC::IOReadByte(unsigned int ioport)
{
	unsigned char data=0xff;
	switch(ioport)
	{
	case TOWNSIO_ANALOGPALETTE_CODE://=  0xFD90,
		data=state.palette.codeLatch;
		break;
	case TOWNSIO_ANALOGPALETTE_BLUE://=  0xFD92,
		data=state.palette.GetBlue((state.sifter[1]>>4)&3);
		break;
	case TOWNSIO_ANALOGPALETTE_RED://=   0xFD94,
		data=state.palette.GetRed((state.sifter[1]>>4)&3);
		break;
	case TOWNSIO_ANALOGPALETTE_GREEN://= 0xFD96,
		data=state.palette.GetGreen((state.sifter[1]>>4)&3);
		break;


	case TOWNSIO_CRTC_ADDRESS://             0x440,
		break;
	case TOWNSIO_CRTC_DATA_LOW://            0x442,
		// It is supposed to be write-only, but 
		//   1 bit "START" and high-byte of FR can be read.  Why not make all readable then.
		data=state.crtcReg[state.crtcAddrLatch]&0xff;
		break;

	case TOWNSIO_VIDEO_OUT_CTRL_ADDRESS://=   0x448,  Supposed to be write-only
		data=state.sifterAddrLatch;
		break;
	case TOWNSIO_VIDEO_OUT_CTRL_DATA://=      0x44A,  Supposed to be write-only
		data=state.sifter[state.sifterAddrLatch];
		break;

	case TOWNSIO_CRTC_DATA_HIGH://           0x443,
		if(REG_FR==state.crtcAddrLatch)
		{
			const auto VSYNC=InVSYNC(townsPtr->state.townsTime);
			const auto HSYNC=InHSYNC(townsPtr->state.townsTime);
			const auto DSPTV0=!VSYNC;
			const auto DSPTV1=DSPTV0;
			const auto DSPTH0=!HSYNC;
			const auto DSPTH1=DSPTH0;
			const bool FIELD=false;   // What's FIELD?
			const bool VIN=false;
			data= (true==VIN ?    0x01 : 0)
			     |(true==HSYNC  ? 0x02 : 0)
			     |(true==VSYNC  ? 0x04 : 0)
			     |(true==FIELD  ? 0x08 : 0)
			     |(true==DSPTH0 ? 0x10 : 0)
			     |(true==DSPTH1 ? 0x20 : 0)
			     |(true==DSPTV0 ? 0x40 : 0)
			     |(true==DSPTV1 ? 0x80 : 0);
		}
		else
		{
			data=(state.crtcReg[state.crtcAddrLatch]>>8)&0xff;
		}
		break;

	case TOWNSIO_MX_HIRES://            0x470,
		data=(TOWNSTYPE_2_MX<=townsPtr->townsType ? 0x7F : 0x80); // [2] pp. 831
		break;

	case TOWNSIO_MX_VRAMSIZE://         0x471,
		data=(TOWNSTYPE_2_MX<=townsPtr->townsType ? 0x01 : 0x00); // [2] pp. 831
		break;

	case TOWNSIO_MX_IMGOUT_ADDR_LOW://  0x472,
		data=(TOWNSTYPE_2_MX<=townsPtr->townsType ? (state.mxVideoOutCtrlAddrLatch&255) : 0xff);
		break;

	case TOWNSIO_MX_IMGOUT_ADDR_HIGH:// 0x473,
		data=(TOWNSTYPE_2_MX<=townsPtr->townsType ? ((state.mxVideoOutCtrlAddrLatch>>8)&255) : 0xff);
		break;

	case TOWNSIO_MX_IMGOUT_ADDR_D0://   0x474,
		switch(state.mxVideoOutCtrlAddrLatch)
		{
		case 0x0004:
			data=0;
			break;
		}
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D1://   0x475,
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D2://   0x476,
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D3://   0x477,
		break;

	case TOWNSIO_HSYNC_VSYNC:
		data= (true==InVSYNC(townsPtr->state.townsTime) ? 1 : 0)
		     |(true==InHSYNC(townsPtr->state.townsTime) ? 2 : 0);
		break;
	}
	return data;
}

/* virtual */ void TownsCRTC::Reset(void)
{
	state.Reset();
}

Vec2i TownsCRTC::GetRenderSize(void) const
{
	return Vec2i::Make(640,480);
}

std::vector <std::string> TownsCRTC::GetStatusText(void) const
{
	std::vector <std::string> text;
	std::string empty;

	text.push_back(empty);
	text.back()="Registers:";
	for(int i=0; i<sizeof(state.crtcReg)/sizeof(state.crtcReg[0]); ++i)
	{
		if(0==i%16)
		{
			text.push_back(empty);
			text.back()+="REG";
			text.back()+=cpputil::Ubtox(i);
			text.back()+=":";
		}
		text.back()+=" ";
		text.back()+=cpputil::Ustox(state.crtcReg[i]);
	}

	text.push_back(empty);
	text.back()="Address Latch: ";
	text.back()+=cpputil::Uitox(state.crtcAddrLatch)+"H";

	const unsigned int CL[2]=
	{
		(unsigned int)( state.crtcReg[REG_CR0]&3),
		(unsigned int)((state.crtcReg[REG_CR0]>>2)&3),
	};

	text.push_back(empty);
	text.back()="CL0:"+cpputil::Itoa(CL[0])+"  CL1:"+cpputil::Itoa(CL[1]);

	if(true==InSinglePageMode())
	{
		text.push_back(empty);
		text.back()="Single-Page Mode.  ";

		auto pageStat0=GetPageStatusText(0);
		text.insert(text.end(),pageStat0.begin(),pageStat0.end());
	}
	else
	{
		text.push_back(empty);
		text.back()="2-Page Mode.  ";

		auto pageStat0=GetPageStatusText(0);
		text.insert(text.end(),pageStat0.begin(),pageStat0.end());

		auto pageStat1=GetPageStatusText(1);
		text.insert(text.end(),pageStat1.begin(),pageStat1.end());
	}

	

	return text;
}

std::vector <std::string> TownsCRTC::GetPageStatusText(int page) const
{
	std::vector <std::string> text;
	std::string empty;

	text.push_back(empty);
	text.back()="Page "+cpputil::Itoa(page);

	text.push_back(empty);
	auto topLeft=GetPageOriginOnMonitor(page);
	text.back()+="Top-Left:("+cpputil::Itoa(topLeft.x())+","+cpputil::Itoa(topLeft.y())+")  ";
	auto dim=GetPageSizeOnMonitor(page);
	text.back()+="Display Size:("+cpputil::Itoa(dim.x())+","+cpputil::Itoa(dim.y())+")";

	text.push_back(empty);
	text.back()+=cpputil::Itoa(1<<GetPageBitsPerPixel(page))+" colors";

	return text;
}
