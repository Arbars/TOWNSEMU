#include <iostream>

#include "cpputil.h"
#include "crtc.h"
#include "towns.h"
#include "townsdef.h"



void TownsCRTC::State::Reset(void)
{
	for(auto &d : mxVideoOutCtrl)
	{
		d=0;
	}
	mxVideoOutCtrlAddrLatch=0;
}


////////////////////////////////////////////////////////////


TownsCRTC::TownsCRTC(class FMTowns *ptr)
{
	townsPtr=ptr;
	state.mxVideoOutCtrl.resize(0x10000);
	state.Reset();
}


/* virtual */ void TownsCRTC::IOWriteByte(unsigned int ioport,unsigned int data)
{
	switch(ioport)
	{
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
	switch(ioport)
	{
	case TOWNSIO_MX_HIRES://            0x470,
		return (TOWNSTYPE_2_MX<=townsPtr->townsType ? 0x7F : 0x80); // [2] pp. 831

	case TOWNSIO_MX_VRAMSIZE://         0x471,
		break;

	case TOWNSIO_MX_IMGOUT_ADDR_LOW://  0x472,
		return (TOWNSTYPE_2_MX<=townsPtr->townsType ? (state.mxVideoOutCtrlAddrLatch&255) : 0xff);

	case TOWNSIO_MX_IMGOUT_ADDR_HIGH:// 0x473,
		return (TOWNSTYPE_2_MX<=townsPtr->townsType ? ((state.mxVideoOutCtrlAddrLatch>>8)&255) : 0xff);

	case TOWNSIO_MX_IMGOUT_ADDR_D0://   0x474,
		switch(state.mxVideoOutCtrlAddrLatch)
		{
		case 0x0004:
			return 0;
		}
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D1://   0x475,
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D2://   0x476,
		break;
	case TOWNSIO_MX_IMGOUT_ADDR_D3://   0x477,
		break;
	}
	return 0xff;
}

/* virtual */ void TownsCRTC::Reset(void)
{
	state.Reset();
}
