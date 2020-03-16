/* LICENSE>>
Copyright 2020 Soji Yamakawa (CaptainYS, http://www.ysflight.com)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<< LICENSE */
#ifndef FDC_IS_INCLUDED
#define FDC_IS_INCLUDED
/* { */

#include <vector>
#include <string>
#include "device.h"
#include "d77.h"



class TownsFDC : public Device
{
public:
	enum
	{
		IMGFILE_RAW,
		IMGFILE_D77
	};
	enum
	{
		NUM_DRIVES=4,
		RESTORE_TIME=20000,     // In Nano Seconds.  Just arbitrary.  Need to make it real.
		SEEK_TIME=20000,        // In Nano Seconds.  Just arbitrary.  Need to make it real.
		STEP_TIME=10000,
		SECTOR_READ_WRITE_TIME=5000,  // In Nano Seconds.  Just arbitrary.  Need to make it real.
		ADDRMARK_READ_TIME=5000,
	};
	class ImageFile
	{
	public:
		int fileType;
		std::string fName;
		D77File d77;
		void SaveIfModified(void);
	};
	ImageFile imgFile[NUM_DRIVES];

	class State
	{
	public:
		class Drive
		{
		public:
			int trackPos;      // Actual head location.
			int trackReg;      // Value in track register 0202H
			int sectorReg;     // Value in sector register 0x04H
			int dataReg;       // Value in data register 0x06H

			int lastSeekDir;   // For STEP command.
			int imgFileNum;    // Pointer to imgFile.
			int diskIndex;     // Disk Index in imgFile[imgFileNum]

			bool motor;
		};

		Drive drive[NUM_DRIVES];
		bool driveSwitch;  // [2] pp.258
		bool busy;
		bool MODEB,HISPD;  // [2] pp.258, pp.809
		bool INUSE;
		unsigned int side; // Is side common for all drives?  Or Per drive?
		bool CLKSEL,DDEN,IRQMSK;

		unsigned int driveSelectBit;
		unsigned int lastCmd;
		unsigned int lastStatus;

		bool recordType,recordNotFound,CRCError,lostData;
		unsigned int addrMarkReadCount;

		long long int scheduleTime;

		void Reset(void);
	};

	class FMTowns *townsPtr;
	class TownsPIC *PICPtr;
	class TownsDMAC *DMACPtr;

	State state;

	bool debugBreakOnCommandWrite;

	virtual const char *DeviceName(void) const{return "FDC";}

	TownsFDC(class FMTowns *townsPtr,class TownsPIC *PICPtr,class TownsDMAC *dmacPtr);

	bool LoadRawBinary(unsigned int driveNum,const char fName[],bool verbose=true);
	D77File::D77Disk *GetDriveDisk(int driveNum);
	const D77File::D77Disk *GetDriveDisk(int driveNum) const;
	ImageFile *GetDriveImageFile(int driveNum);
	const ImageFile *GetDriveImageFile(int driveNum) const;

	void SendCommand(unsigned int data);

	unsigned int CommandToCommandType(unsigned int cmd) const;
	unsigned char MakeUpStatus(unsigned int cmd) const;
	unsigned int DriveSelect(void) const;

	/*! Turns off BUSY flag.  Also if IRQ is not masked it raises IRR flag of PIC.
	*/
	void MakeReady(void);

	bool DriveReady(void) const;
	bool WriteProtected(void) const;
	bool SeekError(void) const;
	bool CRCError(void) const;
	bool IndexHole(void) const;
	bool RecordType(void) const;
	bool RecordNotFound(void) const;
	bool LostData(void) const;
	bool DataRequest(void) const;
	bool WriteFault(void) const;

	virtual void RunScheduledTask(unsigned long long int townsTime);
	virtual void IOWriteByte(unsigned int ioport,unsigned int data);
	virtual unsigned int IOReadByte(unsigned int ioport);

	virtual void Reset(void);

	std::vector <std::string> GetStatusText(void) const;
};


/* } */
#endif
