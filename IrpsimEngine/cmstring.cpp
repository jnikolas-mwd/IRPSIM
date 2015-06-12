// string.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// string string class
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <values.h>
#include <iomanip>
//#include <assert.h>

//#include <fstream.h>
//static ofstream sdebug("string.deb");

const unsigned CM_HASH_SHIFT = 5;
int string::case_sensitive_flag = 0;
int string::skip_whitespace_flag = 0;

// This is the global null string representation, shared among all
// empty strings.  The space for it is in "nullref" which the
// loader will set to zero:
static long nullref[(sizeof(stringRef)+1)/sizeof(long) + 1];
static stringRef* const nullStringRef = (stringRef*)nullref;
/*
 * In what follows, npts_ is the length of the underlying representation
 * vector.  Hence, the capacity for a null terminated string held in this
 * vector is npts_-1.  The variable nchars_ is the length of the held
 * string, excluding the terminating null.
 *
 * The algorithms make no assumptions about whether internal strings
 * hold embedded nulls.  However, they do assume that any string
 * passed in as an argument that does not have a length count is null
 * terminated and therefore has no embedded nulls.
 *
 * The internal string is always null terminated.
 *
 */



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                             stringRef                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


/* static */ stringRef*
stringRef::getRep(size_t capacity, size_t nchar)
{
  if (capacity<nchar) capacity=nchar;
  stringRef* ret = (stringRef*)new wchar_t[capacity + sizeof(stringRef) + 1];
  ret->capacity_ = capacity;
  ret->Refs=0;
  (*ret)[ret->nchars_ = nchar] = 0; // Terminating null
  return ret;
}

// Find first occurrence of a character c:
size_t
stringRef::find(wchar_t c) const
{
  const wchar_t* f = wcschr(array(), c);
  return f ? (size_t)(f - array()): CM_NPOS;
}

// Find first occurrence of a character in wchr:
size_t
stringRef::find(const wchar_t* cs) const
{
//  PRECONDITION2(cs!=0, "stringRef::first(const wchar_t* cs)const: nil pointer");
  const wchar_t* f = wcspbrk(array(), cs);
  return f ? (size_t)(f - array()) : CM_NPOS;
}

inline static void mash(unsigned& hash, unsigned chars)
{
  hash = (chars ^
       ((hash << CM_HASH_SHIFT) |
		  (hash >> (BITSPERBYTE*sizeof(unsigned) - CM_HASH_SHIFT))));
}

/*
 * Return a case-sensitive hash value.
 */
unsigned
stringRef::hash() const
{
	unsigned i = length()*sizeof(wchar_t) / sizeof(unsigned);
  unsigned hv       = (unsigned)length(); // Mix in the string length.
  const unsigned* p = (const unsigned*)array();
  {
	 while (i--)
      mash(hv, *p++);			// XOR in the characters.
  }
  // XOR in any remaining characters:
  if ((i = length()*sizeof(wchar_t) % sizeof(unsigned)) != 0) {
    unsigned h = 0;
	const wchar_t* c = (const wchar_t*)p;
    while (i--) 
		h = ((h << BITSPERBYTE*sizeof(wchar_t)) | *c++);
    mash(hv, h);
  }
  return hv;
}

/*
 * Return a case-insensitive hash value.
 */
unsigned
stringRef::hashFoldCase() const
{
  unsigned hv = (unsigned)length();    // Mix in the string length.
  unsigned i  = hv;
  const wchar_t* p = (const wchar_t*)array();
  while (i--) {
	 mash(hv, towlower(*p));
	 ++p;
  }
  return hv;
}

// Find last occurrence of a character c
size_t
stringRef::last(wchar_t c) const
{
  const wchar_t* f = wcsrchr(array(), c);
  return f ? (size_t)(f - array()) : CM_NPOS;
}



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                              string                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


string::string()
{
  // make sure the null string rep has not been clobbered:
//  assert(nullStringRef->capacity()==0);
//  assert(nullStringRef->length()==0);
//  assert((*nullStringRef)[0]=='\0');
  pref_ = nullStringRef;
  pref_->AddReference();
}

// Copy constructor
string::string(const string& S)
{
	pref_ = S.pref_;
	pref_->AddReference();
}

string::string(size_t ic)
{
  pref_ = stringRef::getRep(ic, 0);
}

string::string(const wchar_t* cs)
{
//  PRECONDITION(cs!=0);
  //***TODO buffer size may not be calculated correctly moving from char to wchar_t
  size_t N = wcslen(cs);
  pref_ = stringRef::getRep(N, N);
  wmemcpy(pref_->array(), cs, N);
}

string::string(const wchar_t* cs, size_t N)
{
//  PRECONDITION(cs!=0);
  pref_ = stringRef::getRep(N, N);
  wmemcpy(pref_->array(), cs, N);
}

string::string(wchar_t c)
{
  pref_ = stringRef::getRep(getInitialCapacity(), 1);
  (*pref_)[0] = c;
}

string::string(wchar_t c, size_t N)
{
  pref_ = stringRef::getRep(N, N);
  while (N--) (*pref_)[N] = c;
}

string::string(const CMSubString& substr)
{
  size_t len = substr.is_null() ? 0 : substr.length();
  pref_ = stringRef::getRep(adjustCapacity(len), len);
  wmemcpy(pref_->array(), substr.array(), len);
}

string::~string()
{
//  if (pref_->RemoveReference() == 0) delete pref_;
  pref_->RemoveReference();
}

string&
string::operator=(const wchar_t* cs)
{
//  PRECONDITION(cs!=0);
  return replace(0, length(), cs, wcslen(cs));
}

// Assignment:  
string&
string::operator=(const string& str)
{
  str.pref_->AddReference();
  if (pref_->RemoveReference() == 0) delete [] pref_;
  pref_ = str.pref_;
  return *this;
}

/********************** Member Functions *************************/

string&
string::append(wchar_t c, size_t rep)
{
  size_t tot;
  cow(tot = length() + rep);
  wchar_t* p = pref_->array() + length();
  while (rep--)
    *p++ = c;

  (*pref_)[pref_->nchars_ = tot] = L'\0';

  return *this;
}

// Change the string capacity, returning the new capacity
size_t
string::capacity(size_t nc)
{
  if (nc > length() && nc != capacity())
    clone(nc);

//  RWPOSTCONDITION(capacity() >= length());
  return capacity();
}

// String comparisons
int
string::compareTo(const wchar_t* cs2) const
{
  const wchar_t* cs1 = c_str();
  size_t len = length();
  size_t i = 0;
  if (case_sensitive_flag) {
	 for (; cs2[i]; i++) {
		if (i == len) return -1;
		if (cs1[i] != cs2[i]) return ((cs1[i] > cs2[i]) ? 1 : -1);
	 }
  } else {                  // ignore case
	 for (; cs2[i]; i++) {
		if (i == len) return -1;
		wchar_t c1 = towlower(cs1[i]);
		wchar_t c2 = towlower(cs2[i]);
		if (c1 != c2) return ((c1 > c2)? 1 : -1);
	 }
  }
  return (i < len) ? 1 : 0;
}

int
string::compareTo(const string& str) const
{
  const wchar_t* s1 = c_str();
  const wchar_t* s2 = str.c_str();
  size_t len = str.length();
  if (length() < len) len = length();
  if (case_sensitive_flag) {
	 int result = wmemcmp(s1, s2, len);
	 if (result != 0) return result;
  }
  else {
	 size_t i = 0;
	 for (; i < len; ++i) {
		wchar_t c1 = towlower(s1[i]);
		wchar_t c2 = towlower(s2[i]);
		if (c1 != c2) return ((c1 > c2)? 1 : -1);
	 }
  }
  // strings are equal up to the length of the shorter one.
  if (length() == str.length()) return 0;
  return (length() > str.length())? 1 : -1;
}


string
string::copy() const
{
  string temp(*this);	// Has increased reference count
  temp.clone();			// Distinct copy
  return temp;
}

unsigned
string::hash() const
{
  return (case_sensitive_flag) ? pref_->hash() : pref_->hashFoldCase();
}


static int
cmMemiEqual(const wchar_t* p, const wchar_t* q, size_t N)
{
  while (N--)
  {
	  if (towlower(*p) != towlower(*q))
      return FALSE;
	 p++; q++;
  }
  return TRUE;
}

// Pattern Matching:
size_t
string::index(const wchar_t* pattern,	// Pattern to search for
		 size_t plen,		// Length of pattern
		 size_t startIndex) const	// Starting index from which to start
{
//  PRECONDITION2(pattern!=0,"string::index(const wchar_t*, size_t, caseCompare) const: nil pointer");
  
  size_t slen = length();
  if (slen < startIndex + plen) return CM_NPOS;
  if (plen == 0) return startIndex;
  slen -= startIndex + plen;
  const wchar_t* sp = c_str() + startIndex;
  if (case_sensitive_flag) {
	 wchar_t first = *pattern;
	 for (size_t i = 0; i <= slen; ++i)
		if (sp[i] == first && wmemcmp(sp+i+1, pattern+1, plen-1) == 0)
		  return i + startIndex;
  } else {
	  int first = towlower(*pattern);
	 for (size_t i = 0; i <= slen; ++i)
	 if (towlower(sp[i]) == first &&
	  cmMemiEqual(sp+i+1, pattern+1, plen-1))
	return i + startIndex;
  }
  return CM_NPOS;

}

// Prepend characters to self:
string&
string::prepend(wchar_t c, size_t rep)
{
  size_t tot = length() + rep;	// Final string length

  // Check for shared representation or insufficient capacity:
  if ( pref_->References() > 1 || capacity() < tot )
  {
    stringRef* temp = stringRef::getRep(adjustCapacity(tot), tot);
    wmemcpy(temp->array()+rep, c_str(), length());
	 if (pref_->RemoveReference() == 0) delete [] pref_;
    pref_ = temp;
  }
  else
  {
    wmemmove(pref_->array()+rep, c_str(), length());
    (*pref_)[pref_->nchars_ = tot] = L'\0';
  }

  wchar_t* p = pref_->array();
  while (rep--)
    *p++ = c;

  return *this;
}


// Remove at most n1 characters from self beginning at pos,
// and replace them with the first n2 characters of cs.
string&
string::replace(size_t pos, size_t n1, const wchar_t* cs, size_t n2)
{
  n1 = cmmin(n1, length()-pos);
  if (!cs) n2 = 0;

  size_t tot = length()-n1+n2;	// Final string length
  size_t rem = length()-n1-pos;	// Length of remnant at end of string

  // Check for shared representation, insufficient capacity,
  // excess waste, or overlapping copy:
  if ( pref_->References() > 1
       || capacity() < tot
       || capacity() - tot > getMaxWaste()
       || (cs && (cs >= c_str() && cs < c_str()+length())) )
  {
    stringRef* temp = stringRef::getRep(adjustCapacity(tot), tot);
	 if (pos) wmemcpy(temp->array(), c_str(), pos);
	 if (n2 ) wmemcpy(temp->array()+pos, cs, n2);
	 if (rem) wmemcpy(temp->array()+pos+n2, c_str()+pos+n1, rem);
	 if (pref_->RemoveReference() == 0) delete [] pref_;
	 pref_ = temp;
  }
  else
  {
	 if (rem) wmemmove(pref_->array()+pos+n2, c_str()+pos+n1, rem);
	 if (n2 ) wmemcpy (pref_->array()+pos   , cs, n2);
	 (*pref_)[pref_->nchars_ = tot] = 0;	// Add terminating null
  }
  return *this;
}


// Truncate or add blanks as necessary
void
string::resize(size_t N)
{
  if (N < length())
	 remove(N);			// Shrank; truncate the string
  else
    append(L' ', N-length());	// Grew or stayed the same

}

// Return a substring of self stripped at beginning and/or end

CMSubString string::strip(

#ifndef RW_GLOBAL_ENUMS
  string::
#endif
             stripType st,
  wchar_t c)
{
  size_t start = 0;		// Index of first character
  size_t end = length();	// One beyond last character
  const wchar_t* direct = c_str();	// Avoid a dereference w dumb compiler

//  assert((int)st != 0);
  if (st & Leading)
	 while (start < end && direct[start] == c)
		++start;
  if (st & Trailing)
    while (start < end && direct[end-1] == c)
      --end;
  if (end == start) start = end = CM_NPOS;  // make the null substring
  return CMSubString(*this, start, end-start);
}


// Change self to lower-case
void
string::to_lower()
{
  cow();
  register size_t N = length();
  register wchar_t* p = pref_->array();
  while ( N-- ) { *p = towlower(*p); p++;}
}

// Change self to upper case
void
string::to_upper()
{
  cow();
  register size_t N = length();
  register wchar_t* p = pref_->array();
  while ( N-- ) { *p = towupper(*p); p++;}
}

// append integer converted to string
string&
string:: operator+=(int d)
{
   wchar_t buffer[64];
   swprintf_s(buffer,L"%d",d);
   return append(buffer,wcslen(buffer));
}

// append long converted to string
string&
string:: operator+=(long d)
{
   wchar_t buffer[64];
   swprintf_s(buffer,L"%ld",d);
   return append(buffer,wcslen(buffer));
}

// append double converted to string
string&
string::operator+=(double f)
{
   wchar_t buffer[64];
   swprintf_s(buffer,L"%.12g",f);
   return append(buffer,wcslen(buffer));
}

/********************** Protected functions ***********************/

// Special constructor to initialize with the concatenation of a1 and a2:
string::string(const wchar_t* a1, size_t N1, const wchar_t* a2, size_t N2)
{
  if (!a1) N1=0;
  if (!a2) N2=0;
  size_t tot = N1+N2;
  pref_ = stringRef::getRep(adjustCapacity(tot), tot);
  wmemcpy(pref_->array(),    a1, N1);
  wmemcpy(pref_->array()+N1, a2, N2);
}

// Calculate a nice capacity greater than or equal to nc
/* static */ size_t
string::adjustCapacity(size_t nc)
{
  size_t ic = getInitialCapacity();
  if (nc<=ic) return ic;
  size_t rs = getResizeIncrement();
  return (nc - ic + rs - 1) / rs * rs + ic;
}

// Make self a distinct copy
void
string::clone()
{
  stringRef* temp = stringRef::getRep(length(), length());
  wmemcpy(temp->array(), c_str(), length());
  if (pref_->RemoveReference() == 0) delete [] pref_;
  pref_ = temp;
}

// Make self a distinct copy with capacity nc.
void
string::clone(size_t nc)
{
  stringRef* temp = stringRef::getRep(nc, length());
  wmemcpy(temp->array(), c_str(), length());
  if (pref_->RemoveReference() == 0) delete [] pref_;
  pref_ = temp;
}



/****************** Related global functions ***********************/

BOOL
operator==(const string& s1, const wchar_t* s2)
{
  return ((s1.length() == wcslen(s2)) && !s1.compareTo(s2)); 
}

// Return a lower-case version of str:
string
to_lower(const string& str)
{
  register size_t N = str.length();
  string temp((wchar_t)0, N);
  register const wchar_t* uc = str.c_str();
  register       wchar_t* lc = (wchar_t*)temp.c_str();
  // Guard against tolower() being a macro:
  while( N-- ) { *lc++ = towlower(*uc); uc++; }
  return temp;
}

// Return an upper-case version of str:
string
to_upper(const string& str)
{
  register size_t N = str.length();
  string temp((wchar_t)0, N);
  register const wchar_t* uc = str.c_str();
  register       wchar_t* lc = (wchar_t*)temp.c_str();
  // Guard against toupper() being a macro:
  while( N-- ) { *lc++ = towupper(*uc); uc++; }
  return temp;
}

string
operator+(const string& s, const wchar_t* cs)
{
  // Use the special concatenation constructor:
  return string(s.c_str(), s.length(), cs, wcslen(cs));
}            

string
operator+(const wchar_t* cs, const string& s)
{
  // Use the special concatenation constructor:
  return string(cs, wcslen(cs), s.c_str(), s.length());
}

string
operator+(const string& s1, const string& s2)
{
  // Use the special concatenation constructor:
  return string(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}

/******************** Static Member Functions **********************/

#if !defined(__DLL__) || !defined(__WIN16__)

/*
 * Code for non 16-bit Windows DLL versions of the libary.  Otherwise,
 * the instance data must be retrieved from the instance manager.
 */

// Static member variable initialization:
size_t		string::initialCapac     = 15;
size_t		string::resizeInc        = 16;
size_t		string::freeboard        = 15;

size_t
string::initialCapacity(size_t ic)
{
  size_t ret = initialCapac;
  initialCapac = ic;
  return ret;
}

size_t
string::resizeIncrement(size_t ri)
{
  size_t ret = resizeInc;
  resizeInc = ri;
  return ret;
}

size_t
string::maxWaste(size_t mw)
{  
  size_t ret = freeboard;
  freeboard = mw;
  return ret;
}

#endif


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                             CMSubString                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

/*
 * A zero lengthed substring is legal.  It can start
 * at any character.  It is considered to be "pointing"
 * to just before the character.
 *
 * A "null" substring is a zero lengthed substring that
 * starts with the nonsense index CM_NPOS.  It can
 * be detected with the member function is_null().
 */

// Private constructor 
CMSubString::CMSubString(const string & str, size_t start, size_t nextent)
: str_((string*)&str),
  begin_(start),
  extent_(nextent)
{
}

// Sub-string operator
CMSubString 
string::operator()(size_t start, size_t len) 
{
  return CMSubString(*this, start, len);
}

/*
 * Returns a substring matching "pattern", or the null substring 
 * if there is no such match.  It would be nice if this could be yet another 
 * overloaded version of operator(), but this would result in a type
 * conversion ambiguity with operator(size_t, size_t).
 */
CMSubString
string::substring(const wchar_t* pattern, size_t startIndex)
{
//  PRECONDITION2(pattern!=0,"string::substring(const wchar_t*, size_t): null pointer");
  size_t i = index(pattern, startIndex);
  return CMSubString(*this, i, i == CM_NPOS ? 0 : wcslen(pattern));
}

const CMSubString
string::substr(size_t start) const
{
  return CMSubString(*this, start, length());
}

const CMSubString
string::substr(size_t start, size_t len) const
{
  return CMSubString(*this, start, len);
}

const CMSubString
string::substring(const wchar_t* pattern, size_t startIndex) const
{
  size_t i = index(pattern, startIndex);
  return CMSubString(*this, i, i == CM_NPOS ? 0 : wcslen(pattern));
}

CMSubString&
CMSubString::operator=(const string& str)
{
  if( !is_null() )
    str_->replace(begin_, extent_, str.c_str(), str.length());

  return *this;
}

CMSubString&
CMSubString::operator=(const wchar_t* cs)
{
  if (!is_null() )
	 str_->replace(begin_, extent_, cs, wcslen(cs));

  return *this;
}

BOOL
operator==(const CMSubString& ss, const wchar_t* cs)
{
	if (!cs) return 0;
	if ( ss.is_null() ) return *cs ==L'\0'; // Two null strings compare equal
	const wchar_t* data = ss.str_->c_str() + ss.begin_;
	size_t i = 0;
	if (string::is_case_sensitive()) {
		for (;cs[i];i++)
			if (cs[i] != data[i] || i == ss.extent_) return FALSE;
	}
	else {
		for (;cs[i];i++)
			if (towlower(cs[i]) != towlower(data[i]) || i == ss.extent_) return FALSE;
	}
	return (i == ss.extent_);
}

BOOL
operator==(const CMSubString& ss, const string& s)
{
	if (ss.is_null()) return s.is_null(); // Two null strings compare equal.
	if (ss.extent_ != s.length()) return FALSE;
	if (string::is_case_sensitive()) {
		return !wmemcmp(ss.str_->c_str() + ss.begin_, s.c_str(), ss.extent_);
	}
	else {
		const wchar_t* data1 = ss.str_->c_str() + ss.begin_;
		const wchar_t* data2 = s.c_str();
		for (unsigned i=0;i<ss.extent_;i++)
			if (towlower(data1[i]) != towlower(data2[i])) return FALSE;
	}
	return TRUE;
}

BOOL
operator==(const CMSubString& s1, const CMSubString& s2)
{
	if (s1.is_null()) return s2.is_null();
	if (s1.extent_ != s2.extent_) return FALSE;
	if (string::is_case_sensitive()) {
		return !wmemcmp(s1.str_->c_str()+s1.begin_, s2.str_->c_str()+s2.begin_, s1.extent_);
	}
	else {
		const wchar_t* data1 = s1.str_->c_str() + s1.begin_;
		const wchar_t* data2 = s2.str_->c_str() + s2.begin_;
		for (unsigned i=0;i<s1.extent_;i++)
			if (towlower(data1[i]) != towlower(data2[i])) return FALSE;
	}
	return TRUE;
}

// Convert self to lower-case
void
CMSubString::to_lower()
{
  if(!is_null())
  {				// Ignore null substrings
     str_->cow();
	 register wchar_t* p = (wchar_t*)(str_->c_str() + begin_); // Cast away constness
	  size_t N = extent_;
	  while( N-- ) { *p = towlower(*p); p++;}
  }
}

// Convert self to upper-case
void
CMSubString::to_upper()
{
  if(!is_null())
  {				// Ignore null substrings
     str_->cow();
	 register wchar_t* p = (wchar_t*)(str_->c_str() + begin_); // Cast away constness
     size_t N = extent_;
     while( N-- ) { *p = towupper(*p); p++;}
  }
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                           LOCALE RELATED                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//***TODO: Find a lib function to do this
BOOL
string::isAscii() const
{
  const wchar_t* cp = c_str();
  for (size_t i = 0; i < length(); ++i)
	 if (cp[i] & ~0x7F)
      return FALSE;
  return TRUE;
}

#ifndef RW_NO_LOCALE

// String collation
int
stringRef::collate(const wchar_t* cstr) const
{
  // Cast to "wchar_t*" necessary for Sun cfront:
  return ::wcscoll(array(), cstr);
}

string
strXForm(const string& cstr)
{
  // Get the size required to transform the string;
  // cast to "wchar_t*" necessary for Sun cfront:
	size_t N = ::wcsxfrm(NULL, (wchar_t*)cstr.c_str(), 0);

  string temp((wchar_t)0, N);

  // Return null string in case of failure:
  if (::wcsxfrm((wchar_t*)temp.c_str(), (wchar_t*)cstr.c_str(), N) >= N)
	return string();

  return temp;
}


//***TODO: Add mbLength if necessary
/*
size_t
string::mbLength() const 
{
  const wchar_t* cp = c_str();
  size_t i = 0;
  size_t len = 0;
  mblen((const wchar_t*)0, MB_CUR_MAX);  // clear static state (bleah!)
  while (i < length() && cp[i]) {
    int l = mblen(cp+i, MB_CUR_MAX);
    if (l <= 0) return CM_NPOS;
    i += l;
    ++len;
  }
  if (i > length()) return CM_NPOS; // incomplete last char
  return len;
}
*/

// STREAM I/O

// Replace self with the contents of strm, stopping at an EOF.
wistream&
string::readFile(wistream& strm)
{
  cow(getResizeIncrement());
  (*pref_)[pref_->nchars_ = 0] = L'\0';		// Abandon old data

  while(1)
  {
	strm.read(pref_->array()+length(), capacity()-length());
    pref_->nchars_ += (size_t)strm.gcount();

    if (!strm.good())
      break;			// EOF encountered

    // If we got here, the read must have stopped because
    // the buffer was going to overflow.  Resize and keep
    // going.
    capacity(length() + getResizeIncrement());
  }

  (*pref_)[length()] = L'\0';			// Add null terminator

  if (capacity()-length() > getMaxWaste())
    capacity(adjustCapacity(capacity()));

  return strm;
}

wistream&
string::read_line(wistream& strm)
{
  if (skip_whitespace_flag)
	 strm >> ws;

  return readToDelim(strm, L'\n');
}

wistream&
string::readString(wistream& strm)
{
  return readToDelim(strm, L'\0');
}


/*
 * Read up to an EOF, or a delimiting character, whichever comes
 * first.  The delimiter is not stored in the string,
 * but is removed from the input stream.  
 *
 * Because we don't know how big a string to expect, we first read
 * as much as we can and then, if the EOF or null hasn't been
 * encountered, do a resize and keep reading.
 */

wistream&
string::readToDelim(wistream& strm, wchar_t delim)
{
  cow(getResizeIncrement());
  (*pref_)[pref_->nchars_ = 0] = L'\0';		// Abandon old data

  while (1)
  {
	 strm.get(pref_->array()+length(),	// Address of next byte
		  capacity()-length(),	// Space available
		  delim);			// Delimiter
	 pref_->nchars_ += (size_t)strm.gcount();
	 if (!strm.good()) break;		// Check for EOF or stream failure
	 int p = strm.peek();
	 if (p == delim)			// Check for delimiter
	 {
		strm.get();			// eat the delimiter.
		break;
	 }
	 // Delimiter not seen.  Resize and keep going:
	 capacity(length() + getResizeIncrement());
  }

  (*pref_)[length()] = L'\0';		// Add null terminator

  if (capacity()-length() > getMaxWaste())
	 capacity(adjustCapacity(capacity()));

  return strm;
}

wistream&
string::readToken(wistream& strm)
{
  cow(getResizeIncrement());
  (*pref_)[pref_->nchars_ = 0] = L'\0';		// Abandon old data

  strm >> ws;					// Eat whitespace

  wchar_t c;
  while (strm.get(c).good() && !iswspace(c))
  {
	 // Check for overflow:
	 if (length() == capacity())
		capacity(length() + getResizeIncrement());
	 (*pref_)[pref_->nchars_++] = c;
  }

  (*pref_)[length()] = L'\0';			// Add null terminator

  if (capacity()-length() > getMaxWaste())
	 capacity(adjustCapacity(capacity()));

  return strm;
}

// GLOBAL I/O FUNCTIONS


wistream&
operator>>(wistream& strm, string& s)
{
  return s.readToken(strm);
}

wostream&
operator<<(wostream& os, const string& s)
{
	return os << ((wchar_t*)s.c_str());
//	return os.write((wchar_t*)s.c_str(), s.length());
}

#endif
