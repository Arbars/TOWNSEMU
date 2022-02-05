#include <windows.h>
#include <direct.h>
#include "filesys.h"



class FileSys::FindContext
{
public:
	HANDLE hFind=INVALID_HANDLE_VALUE;
	static void DirEntFromFd(DirectoryEntry &ent,WIN32_FIND_DATAA &fd);
	~FindContext();
	void Close(void);
};

FileSys::FindContext::~FindContext()
{
	Close();
}
void FileSys::FindContext::Close(void)
{
	if(INVALID_HANDLE_VALUE!=hFind)
	{
		FindClose(hFind);
		hFind=INVALID_HANDLE_VALUE;
	}
}


/* static */ void FileSys::FindContext::DirEntFromFd(DirectoryEntry &ent,WIN32_FIND_DATAA &fd)
{
	FILETIME localFileTime;
	FileTimeToLocalFileTime(&fd.ftLastWriteTime,&localFileTime);
	SYSTEMTIME sysTime;
	FileTimeToSystemTime(&localFileTime,&sysTime);

	ent.fName=fd.cFileName;
	ent.year=sysTime.wYear;
	ent.month=sysTime.wMonth;
	ent.day=sysTime.wDay;
	ent.hours=sysTime.wHour;
	ent.minutes=sysTime.wMinute;
	ent.seconds=sysTime.wSecond;
	if(0==(FILE_ATTRIBUTE_DIRECTORY&fd.dwFileAttributes))
	{
		ent.attr&=~ATTR_DIR;
		ent.length=((unsigned long long)fd.nFileSizeHigh<<32)+fd.nFileSizeLow;
	}
	else
	{
		ent.attr|=ATTR_DIR;
		ent.length=0;
	}
}


FileSys::FileSys()
{
	context=new FindContext;
}
FileSys::~FileSys()
{
	context->Close();
	delete context;
	context=nullptr;
}
FileSys::DirectoryEntry FileSys::FindFirst(std::string subDir)
{
	return FindFirst(subDir,context);
}
FileSys::DirectoryEntry FileSys::FindNext(void)
{
	return FindNext(context);
}



FileSys::FindContext *FileSys::CreateFindContext(void)
{
	return new FindContext;
}
void FileSys::DeleteFindContext(FindContext *find)
{
	delete find;
}
FileSys::DirectoryEntry FileSys::FindFirst(std::string subPath,FindContext *find)
{
	find->Close();

	auto path=hostPath;
	if(""!=subPath && "/"!=subPath && "\\"!=subPath)
	{
		if(""==path || (path.back()!='/' && path.back()!='\\'))
		{
			path.push_back('/');
		}
		path+=subPath;
	}
	if(""==path || (path.back()!='/' && path.back()!='\\'))
	{
		path.push_back('/');
	}
	path+="*.*";

	WIN32_FIND_DATAA fd;
	find->hFind=FindFirstFileA(path.c_str(),&fd);

	DirectoryEntry ent;
	if(INVALID_HANDLE_VALUE==find->hFind)
	{
		ent.endOfDir=true;
	}
	else
	{
		ent.endOfDir=false;
		find->DirEntFromFd(ent,fd);
	}
	return ent;
}
FileSys::DirectoryEntry FileSys::FindNext(FindContext *find)
{
	DirectoryEntry ent;
	if(INVALID_HANDLE_VALUE==find->hFind)
	{
		// Called without FindFirst
		ent.endOfDir=true;
	}
	else
	{
		WIN32_FIND_DATAA fd;
		if(TRUE!=FindNextFileA(find->hFind,&fd))
		{
			ent.endOfDir=true;
			find->Close();
		}
		else
		{
			ent.endOfDir=false;
			find->DirEntFromFd(ent,fd);
		}
	}
	return ent;
}



/* static */ std::string FileSys::Getcwd(void)
{
	char buf[1024];
	if(nullptr==_getcwd(buf,1023))
	{
		return "";
	}
	return buf;
}
/* static */ bool FileSys::Chdir(std::string str)
{
	if(0==_chdir(str.c_str()))
	{
		return true;
	}
	return false;
}
/* static */ bool FileSys::Mkdir(std::string str)
{
	if(0==_mkdir(str.c_str()))
	{
		return true;
	}
	return false;
}
