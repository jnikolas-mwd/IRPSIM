// cmlib.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Library of utility functions
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "cmlib.h"
#include "notify.h"
#include <ctype.h>

//#include <fstream>
//wofstream sdebug(L"debug_cmlib.txt");

#if !defined (_CM_SOLARIS)
	#undef isdigit
#endif

static wchar_t* algops[] = { L"==", L"=", L"!=", L"<=", L">=", L"<", L">",
						L"||", L"|", L"&&" ,L"&", L"+", L"-",
						L"*", L"/", L"%", L"^", NULL};
/*
long _IRPFUNC round(double val)
{
	long l = (long)val;
	if (val>=0) return (val+0.5) >= l+1 ? l+1 : l;
	else return (val-0.5) <= l-1 ? l-1 : l;
}
*/

/***TODO: Implement newstring if necessary
wchar_t* newstring(const wchar_t* str)
{
	wchar_t* ret = new wchar_t[wcslen(str) + 1];
	wcscpy_s(ret, wcslen(str) + 1, str);
	return ret;
}
*/

int skipwhite(wchar_t*& ptr)
{
	while (*ptr==L' ' || *ptr==L'\t' || *ptr==L'\r' || *ptr==L'\n')
		ptr++;
	return *ptr;
}

int isnumber(const wchar_t* str)
{
	int ret = 1;
	wchar_t* ptr = (wchar_t*)str;
	int points=0;
	skipwhite(ptr);
	if (!*ptr) return 0;
	if (*ptr==L'+' || *ptr==L'-')
		skipwhite(++ptr);
	while (*ptr && ret) {
		if(*ptr==L'.') points++;
		if (!(iswdigit(*ptr) || *ptr==L'.'))
			ret = 0;
		ptr++;
	}
	if (ret)	ret = (points>1) ? 0 : 1;
	return ret;
}

int isnumber(const wchar_t ch)
{
	return (iswdigit(ch) || ch == L'.') ? 1 : 0;
}

int isvariablename(const wchar_t* str) // is this a legal variable name?
{
   if (!str || !*str)
   	return 0;
   if (!(iswalpha(*str) || (*str==L'_')))
   	return 0;
   int ch;
	for (int i=1;(ch=str[i])!=0;i++) {
		if (!(iswalpha(ch) || iswdigit(ch) || (ch==L'_') || (ch==L'.') || (ch==L'@')) )
      	return 0;
   }
   return 1;
}

int isalgop(const wchar_t* str)
{
	if (!str || !*str)
		return 0;

	int ret = 0;

	for (int i = 0; algops[i] != NULL && ret==0; i++) {
		if (!wcsncmp(str, algops[i], wcslen(algops[i])))
			ret = 1;
	}
	
	return ret;
}

int contains(const wchar_t* str, const wchar_t**list, int n, int case_sen)
{
	int ret = -1;
	int oldflag = CMString::set_case_sensitive(case_sen);
	CMString test(str);
	for (int i=0;i<n && list[i] && ret<0;i++)
		if (test==list[i])
			ret = i;
	CMString::set_case_sensitive(oldflag);
	return ret;
}

int _IRPFUNC writestringbinary(const CMString& s,wostream& os)
{
	size_t len = s.length();
	os.write((wchar_t*)&len, sizeof(size_t));
	os.write(s.c_str(),len);
	return len + sizeof(size_t);
}

int _IRPFUNC readstringbinary(CMString& s,wistream& is)
{
	wchar_t buffer[256];
	size_t len = 0;
	if (!is.fail() && !is.eof())
		is.read((wchar_t*)&len, sizeof(size_t));
	if (!is.fail() && !is.eof() && len)
		is.read(buffer,len);
	else
   		len=0;
	buffer[len]=0;
	s=buffer;
	return len + sizeof(size_t);
}


CMString _IRPFUNC readstringbinary(wistream& is)
{
	wchar_t buffer[256];
	size_t len = 0;
	if (!is.fail() && !is.eof())
		is.read((wchar_t*)&len, sizeof(size_t));
	if (!is.fail() && !is.eof() && len)
		is.read(buffer,len);
	else
   		len=0;
	buffer[len]=0;
	return CMString(buffer);
}

int _IRPFUNC stringbinarylength(const CMString& s)
{
	return sizeof(size_t)+s.length();
}

//***TODO: Fix stripends so that it calls iswhitespace instead of 
CMString stripends(const CMString& s)
{
	CMString ret;
   unsigned i,j;
	if (!s.length())
		return ret;
	for (i=0;i<s.length()&&s[i]<=L' ';i++);
	for (j=s.length()-1;j>=i&&s[j]<=L' ';j--);
	if (j>=i)
		ret = s.substr(i,j-i+1);
	return ret;
}

int sign(double val)
{
	if (val<0) return -1;
	if (val>0) return 1;
	return 0;
}

CMString getfileinfo(const CMString& file)
{
	return CMString();
}

CMString strippath(const CMString& file)
{
	CMString ret = file;
	size_t index;
	while ((index=ret.find(L"/\\"))!=CM_NPOS)	ret.remove(0,index+1);
	return ret;
}

CMString extractpath(const CMString& file)
{
	size_t last = file.last(L'\\');
   if (last == CM_NPOS) last = file.last(L'/');
	if (last==CM_NPOS)
   	return CMString(L".\\");
   return CMString(file.substr(0,last+1));
}

#if defined (WIN32)
//#include <windows.h>
/*
CMString createtempfile(const wchar_t* prefix, const wchar_t* path)
{
	wchar_t buffer[256];
	const wchar_t* ptr = path;
	if (path==0) {
   	GetTempPath(256,(LPWSTR)buffer);
      ptr = buffer;
   }
	GetTempFileName((LPWSTR)ptr, (LPWSTR)prefix, 0, (LPWSTR)buffer);
	return CMString(buffer);
}
*/

int removefile(const CMString& filename)
{
	return 1;
	//return DeleteFile((LPWSTR)filename.c_str());
}

/*
CMString getfullpathname(const wchar_t* name)
{
	wchar_t buffer[256];
	LPTSTR pFilePart;
	int nRet=-1;
	CMString strRet;

	DWORD dwRslt = GetFullPathName((LPWSTR)name, 256, (LPWSTR)buffer, &pFilePart);

	if (!dwRslt) {
		CMString err(L"Unable to get path of ");
		err += name;
		CMError::ReportError(err);
	}
	else
		strRet = buffer;

	return strRet;
}
*/

CMString getrelativepath(const wchar_t* master, const wchar_t* slave)
{
	int i;
	for (i=0;master[i] && slave[i] && (towupper(master[i])==towupper(slave[i]));i++);
	// if the files are on different drives, return full slave path
	if (wcsrchr(slave+i,':'))
		return CMString(slave);
	// retreat until we are at a directory break
	for (;i>0 && !(slave[i-1]=='\\' || slave[i-1]=='/');i--);
	int nDescents=0;
	const wchar_t* ptr = master + i;
	while (ptr=wcspbrk(ptr,L"/\\")) {
		nDescents++;
		ptr+=1;
	}
	CMString path;
	if (!nDescents)
		path = L".\\";
	else for (;nDescents>0;nDescents--)
		path += L"..\\";
	
	path += (slave+i);
	return path;
}

CMString getabsolutepath(const wchar_t* master, const wchar_t* slave)
{
	if (wcschr(slave, ':'))
		return CMString(slave);

	CMString path = extractpath(master);

	return path + slave;
}

#else
#include <dir.h>
#include <io.h>

CMString createtempfile(const wchar_t* prefix,const wchar_t* path)
{
	wchar_t buffer[256];
   wcscpy(buffer,path ? path : L".\\");
   wcscat(buffer,prefix);
   wcscat(buffer,L"XX");
  return CMString(mktemp(buffer));
}

int removefile(const CMString& filename)
{
   return unlink(filename.c_str()) ? 0 : 1;
}

	
CMString getfullpathname(const wchar_t* name)
{
	return CMString(name);
}

CMString getrelativepath(const wchar_t* master,const wchar_t* slave)
{
	return CMString(slave);
}

#endif




