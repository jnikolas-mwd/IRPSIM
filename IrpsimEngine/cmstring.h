// string.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"
#include <string.h>
#include <iostream>

using namespace std;

class _IRPCLASS string;
class _IRPCLASS CMSubString;

const size_t CM_NPOS = size_t(-1);

class _IRPCLASS CMReference
{
protected:
	 unsigned short Refs;    // Number of references to this block
public:
	 enum CMReferenceFlag { STATIC_INIT };
	 CMReference(int initRef = 0) : Refs((unsigned short)initRef-1) { }
	 CMReference(CMReferenceFlag) {}
	 void _RTLENTRY AddReference() { Refs++; }
	 unsigned short _RTLENTRY References() { return Refs+1; }
	 unsigned short _RTLENTRY RemoveReference() { return Refs--; }
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                             stringRef                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

/*
 * This is the dynamically allocated part of a RWCString.
 * It maintains a reference count.
 * There are no public member functions.
 */

class _IRPCLASS stringRef : public CMReference
{

  static stringRef*	getRep(size_t capac, size_t nchar);

  size_t	length   () const {return nchars_;}
  size_t	capacity () const {return capacity_;}
  wchar_t*	array() const {return (wchar_t*)(this+1);}

  wchar_t&		operator[](size_t i)       {return ((wchar_t*)(this+1))[i];}
  wchar_t		operator[](size_t i) const { return ((wchar_t*)(this + 1))[i]; }

  size_t	find    (wchar_t) const;
  size_t	find    (const wchar_t*) const;
  unsigned	hash     (           ) const;
  unsigned	hashFoldCase (       ) const;
  size_t	last     (wchar_t       ) const;

  int		collate(const wchar_t*) const;

  size_t	nchars_;	// String length (excluding terminating null)
  size_t	capacity_;	// Max string length (excluding null)

friend class _IRPCLASS string;
friend class _IRPCLASS CMSubString;
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                             CMSubString                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

/*
 * The CMSubString class allows selected elements to be addressed.
 * There are no public constructors.
 */

class _IRPCLASS CMSubString
{
public:
  CMSubString(const CMSubString& SP)
	 : str_(SP.str_), begin_(SP.begin_), extent_(SP.extent_) {;}

  CMSubString&	operator=(const wchar_t*);		// Assignment to wchar_t*
  CMSubString&	operator=(const string&);	// Assignment to string
  wchar_t  	operator[](size_t i) const;	// Index with bounds checking
  const wchar_t* array() const;
  size_t	length() const		{return extent_;}
  size_t	start() const		{return begin_;}
  void		to_lower();		// Convert self to lower-case
  void		to_upper();		// Convert self to upper-case

  // For detecting null substrings:
  BOOL	is_null() const		{return begin_==CM_NPOS;}

private:

  // NB: the only constructor is private:
  CMSubString(const string & s, size_t start, size_t len);

  string*  	str_;		// Referenced string
  size_t	begin_;		// Index of starting character
  size_t	extent_;	// Length of CMSubString

  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const CMSubString& s2);
  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const string& s2);
  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const wchar_t* s2);
  friend class _IRPCLASS string;
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                              string                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


class _IRPCLASS string
{
	static int case_sensitive_flag;
	static int skip_whitespace_flag;

public:

  enum stripType {Leading = 0x1, Trailing = 0x2, Both = 0x3};

  string();			// Null string
  string(size_t ic);	// Suggested capacity
  string(const string& S);	// Copy constructor
  string(const wchar_t * a);		// Copy to embedded null
  string(const wchar_t * a, size_t N);	// Copy past any embedded nulls
  string(wchar_t);
  string(wchar_t, size_t N);

  string(const CMSubString& SS);

  ~string();

  static int set_case_sensitive(int flag);
  static int skip_whitespace(int flag);
  static int is_case_sensitive() {return case_sensitive_flag;}

  // Assignment:
  string&	operator=(const wchar_t*);		// Replace string
  string&	operator=(const string&);	// Replace string
  string&	operator+=(const wchar_t*);	// Append string.
  string&	operator+=(const string& s);
  string&	operator+=(wchar_t c);
  string&	operator+=(int d);
  string&	operator+=(long d);
  string&	operator+=(double f);

  // Indexing operators:
  CMSubString	operator()(size_t start, size_t len);		// Sub-string operator
  CMSubString	substring(const wchar_t* pat, size_t start = 0);

  wchar_t		operator[](size_t) const;
  const CMSubString	substr(size_t start) const;
  const CMSubString	substr(size_t start, size_t len) const;
  const CMSubString	substring(const wchar_t* pat, size_t start = 0) const;	// Match the RE

		// Non-static member functions:
  string&	append(const wchar_t* cs);
  string&	append(const wchar_t* cs, size_t N);
  string&	append(const string& s);
  string&	append(const string& s, size_t N);
  string&	append(wchar_t c, size_t rep = 1);	// Append c rep times
  size_t	binaryStoreSize() const		{return length()+sizeof(size_t);}
  size_t	capacity() const		{return pref_->capacity();}
  size_t	capacity(size_t N);
  int		collate(const wchar_t* cs) const	{ return pref_->collate(cs); }
  int		collate(const string& st) const;
  int		compareTo(const wchar_t* cs) const;
  int		compareTo(const string& st) const;
  BOOL	contains(const wchar_t* pat) const;
  BOOL	contains(const string& pat) const;
  string    	copy() const;
  const wchar_t*  	c_str() const { return pref_->array(); }
  size_t	find(wchar_t c) const			{ return pref_->find(c); }
  size_t	find(const wchar_t* cs) const		{ return pref_->find(cs); }
  size_t	find(const string& c) const {return find(c_str());}
  unsigned	hash() const;
  size_t	index(const wchar_t* pat, size_t i = 0)
				const;
  size_t	index(const string& s, size_t i=0)
				const;
  size_t	index(const wchar_t* pat, size_t patlen, size_t i) const;
  size_t	index(const string& s, size_t patlen, size_t i) const;
  string&	insert(size_t pos, const wchar_t*);
  string&	insert(size_t pos, const wchar_t*, size_t extent);
  string&	insert(size_t pos, const string&);
  string&	insert(size_t pos, const string&, size_t extent);
  BOOL	isAscii() const;
  BOOL	is_null() const				{return pref_->nchars_ == 0;}
  size_t	last(char c) const			{return pref_->last(c);}
  size_t  	length() const				{return pref_->nchars_;}
  //size_t	mbLength() const;	// multibyte length, or CM_NPOS on error
  string&	prepend(const wchar_t*);			// Prepend a character string
  string&	prepend(const wchar_t* cs, size_t N);
  string&	prepend(const string& s);
  string&	prepend(const string& s, size_t N);
  string&	prepend(wchar_t c, size_t rep = 1);	// Prepend c rep times
  wistream&	readFile(wistream&);			// Read to EOF or null character.
  wistream&	read_line(wistream&);	// Read to EOF or newline.
  wistream&	readString(wistream&);			// Read to EOF or null character.
  wistream&	readToDelim(wistream&, wchar_t delim);	// Read to EOF or delimiter.
  wistream&	readToken(wistream&);			// Read separated by white space.
  string&	remove(size_t pos);			// Remove pos to end of string
  string&	remove(size_t pos, size_t n);		// Remove n chars starting at pos
  string&	replace(size_t pos, size_t n, const wchar_t*);
  string&	replace(size_t pos, size_t n, const wchar_t*, size_t);
  string&	replace(size_t pos, size_t n, const string&);
  string&	replace(size_t pos, size_t n, const string&, size_t);
  void		resize(size_t N);	 		// Truncate or add blanks as necessary.
  CMSubString	strip(stripType s, wchar_t c=L' ');
  void		to_lower();				// Change self to lower-case
  void		to_upper();				// Change self to upper-case

  // Static member functions:
  static size_t		initialCapacity(size_t ic = 15);	// Initial allocation Capacity
  static size_t		maxWaste(size_t mw = 15);		// Max empty space before reclaim
  static size_t		resizeIncrement(size_t ri = 16);	// Resizing increment
  static size_t		getInitialCapacity()	{return initialCapac;}
  static size_t		getResizeIncrement()	{return resizeInc;}
  static size_t		getMaxWaste()		{return freeboard;}

protected:

  // Special concatenation constructor:
	string(const wchar_t* a1, size_t N1, const wchar_t* a2, size_t N2);
  void			cow();				// Do copy on write as needed
  void			cow(size_t nc);			// Do copy on write as needed
  static size_t		adjustCapacity(size_t nc);

private:

  void			clone();          // Make self a distinct copy
  void			clone(size_t nc); // Make self a distinct copy w. capacity nc

  static size_t	initialCapac;		// Initial allocation Capacity
  static size_t	resizeInc;		// Resizing increment
  static size_t	freeboard;		// Max empty space before reclaim

  stringRef*		pref_;		// Pointer to ref. counted data

friend string _IRPFUNC operator+(const string& s1, const string& s2);
friend string _IRPFUNC operator+(const string& s, const wchar_t* cs);
friend string _IRPFUNC operator+(const wchar_t* cs, const string& s);
friend BOOL _IRPFUNC operator==(const string& s1, const string& s2);
friend BOOL _IRPFUNC operator==(const string& s1, const wchar_t* s2);
friend class _IRPCLASS CMSubString;
friend class _IRPCLASS stringRef;

};

// Related global functions:
_IRPFUNC wistream& operator>>(wistream& str, string& cstr);
_IRPFUNC wostream& operator<<(wostream& str, const string& cstr);

string _IRPFUNC to_lower(const string&);	// Return lower-case version of argument.
string _IRPFUNC to_upper(const string&);	// Return upper-case version of argument.
inline    unsigned cmhash(const string& s) { return s.hash(); }
inline    unsigned cmhash(const string* s) { return s->hash(); }
#ifndef RW_NO_LOCALE
string _IRPFUNC strXForm(const string&);	// strxfrm() interface
#endif


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                               Inlines                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

inline int string::set_case_sensitive(int flag)
{int oldflag=case_sensitive_flag;case_sensitive_flag=flag;return oldflag;}

inline int string::skip_whitespace(int flag)
{int oldflag=skip_whitespace_flag;skip_whitespace_flag=flag;return oldflag;}

inline void string::cow()
{ if (pref_->References() > 1) clone(); }

inline void string::cow(size_t nc)
{ if (pref_->References() > 1  || capacity() < nc) clone(nc); }

inline string& string::append(const wchar_t* cs)
{ return replace(length(), 0, cs, wcslen(cs)); }

inline string& string::append(const wchar_t* cs, size_t N)
{ return replace(length(), 0, cs, N); }

inline string& string::append(const string& s)
{ return replace(length(), 0, s.c_str(), s.length()); }

inline string& string::append(const string& s, size_t N)
{ return replace(length(), 0, s.c_str(),cmmin(N, s.length())); }

inline string& string::operator+=(const wchar_t* cs)
{ return append(cs, wcslen(cs)); }

inline string& string::operator+=(const string& s)
{ return append(s.c_str(),s.length()); }

inline string& string::operator+=(wchar_t c)
{ return append(&c,1);}

#ifndef RW_NO_LOCALE
inline int string::collate(const string& st) const
{ return pref_->collate(st.c_str()); }
#endif

inline BOOL string::contains(const string& pat) const
{ return index(pat.c_str(), pat.length(), (size_t)0) != CM_NPOS; }

inline BOOL string::contains(const wchar_t* s) const
{ return index(s, wcslen(s), (size_t)0) != CM_NPOS; }

inline size_t string::index(const wchar_t* s, size_t i) const
{ return index(s, wcslen(s), i); }

inline size_t string::index(const string& s, size_t i) const
{ return index(s.c_str(), s.length(), i); }

inline size_t string::index(const string& pat, size_t patlen, size_t i) const
{ return index(pat.c_str(), patlen, i); }

inline string& string::insert(size_t pos, const wchar_t* cs)
{ return replace(pos, 0, cs, wcslen(cs)); }

inline string& string::insert(size_t pos, const wchar_t* cs, size_t N)
{ return replace(pos, 0, cs, N); }

inline string& string::insert(size_t pos, const string& cstr)
{ return replace(pos, 0, cstr.c_str(), cstr.length()); }

inline string& string::insert(size_t pos, const string& cstr, size_t N)
{ return replace(pos, 0, cstr.c_str(), cmmin(N, cstr.length())); }

inline string& string::prepend(const wchar_t* cs)
{ return replace(0, 0, cs, wcslen(cs)); }

inline string& string::prepend(const wchar_t* cs, size_t N)
{ return replace(0, 0, cs, N); }

inline string& string::prepend(const string& s)
{ return replace(0, 0, s.c_str(), s.length()); }

inline string& string::prepend(const string& s, size_t N)
{ return replace(0, 0, s.c_str(), cmmin(N, s.length())); }

inline string& string::remove(size_t pos)
{ return replace(pos, length()-pos, 0, 0); }

inline string& string::remove(size_t pos, size_t n)
{ return replace(pos, n, 0, 0); }

inline string& string::replace(size_t pos, size_t n, const wchar_t* cs)
{ return replace(pos, n, cs, wcslen(cs)); }

inline string& string::replace(size_t pos, size_t n, const string& cstr)
{ return replace(pos, n, cstr.c_str(), cstr.length()); }

inline string& string::replace(size_t pos, size_t n1, const string& cstr, size_t n2)
{ return replace(pos, n1, cstr.c_str(), cmmin(cstr.length(),n2)); }


inline wchar_t string::operator[](size_t i) const
{ return (*pref_)[i]; }

inline const wchar_t* CMSubString::array() const
{ return str_->c_str() + begin_; }

// Access to elements of sub-string with bounds checking

inline wchar_t CMSubString::operator[](size_t i) const
{return (*str_->pref_)[begin_+i]; }

// String Logical operators:
inline BOOL	operator==(const string& s1, const string& s2)
				  { return ((s1.length() == s2.length()) && !s1.compareTo(s2)); }
inline BOOL	operator!=(const string& s1, const string& s2)
				  { return !(s1 == s2); }
inline BOOL	operator< (const string& s1, const string& s2)
				  { return s1.compareTo(s2)< 0;}
inline BOOL	operator> (const string& s1, const string& s2)
				  { return s1.compareTo(s2)> 0;}
inline BOOL	operator<=(const string& s1, const string& s2)
				  { return s1.compareTo(s2)<=0;}
inline BOOL	operator>=(const string& s1, const string& s2)
				  { return s1.compareTo(s2)>=0;}

//     BOOL	operator==(const string& s1, const wchar_t* s2);
inline BOOL	operator!=(const string& s1, const wchar_t* s2)
				  { return !(s1 == s2); }
inline BOOL	operator< (const string& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)< 0; }
inline BOOL	operator> (const string& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)> 0; }
inline BOOL	operator<=(const string& s1, const wchar_t* s2)
                                  { return s1.compareTo(s2)<=0; }
inline BOOL	operator>=(const string& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)>=0; }

inline BOOL	operator==(const wchar_t* s1, const string& s2)
				  { return (s2 == s1); }
inline BOOL	operator!=(const wchar_t* s1, const string& s2)
				  { return !(s2 == s1); }
inline BOOL	operator< (const wchar_t* s1, const string& s2)
				  { return s2.compareTo(s1)> 0; }
inline BOOL	operator> (const wchar_t* s1, const string& s2)
				  { return s2.compareTo(s1)< 0; }
inline BOOL	operator<=(const wchar_t* s1, const string& s2)
											 { return s2.compareTo(s1)>=0; }
inline BOOL	operator>=(const wchar_t* s1, const string& s2)
				  { return s2.compareTo(s1)<=0; }

// SubString Logical operators:
//     BOOL operator==(const CMSubString& s1, const CMSubString& s2);
//     BOOL operator==(const CMSubString& s1, const wchar_t* s2);
//     BOOL operator==(const CMSubString& s1, const string& s2);
inline BOOL operator==(const string& s1,    const CMSubString& s2)
				{ return (s2 == s1); }
inline BOOL operator==(const wchar_t* s1, const CMSubString& s2)
				{ return (s2 == s1); }
inline BOOL operator!=(const CMSubString& s1, const wchar_t* s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const CMSubString& s1, const string& s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const CMSubString& s1, const CMSubString& s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const string& s1,    const CMSubString& s2)
				{ return !(s2 == s1); }
inline BOOL operator!=(const wchar_t* s1, const CMSubString& s2)
				{ return !(s2 == s1); }


