// cmstring.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMString string class
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

class _IRPCLASS CMString;
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
//                             CMStringRef                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

/*
 * This is the dynamically allocated part of a RWCString.
 * It maintains a reference count.
 * There are no public member functions.
 */

class _IRPCLASS CMStringRef : public CMReference
{

  static CMStringRef*	getRep(size_t capac, size_t nchar);

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

friend class _IRPCLASS CMString;
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
  CMSubString&	operator=(const CMString&);	// Assignment to CMString
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
  CMSubString(const CMString & s, size_t start, size_t len);

  CMString*  	str_;		// Referenced CMString
  size_t	begin_;		// Index of starting character
  size_t	extent_;	// Length of CMSubString

  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const CMSubString& s2);
  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const CMString& s2);
  friend BOOL _IRPFUNC operator==(const CMSubString& s1, const wchar_t* s2);
  friend class _IRPCLASS CMString;
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                              CMString                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


class _IRPCLASS CMString
{
	static int case_sensitive_flag;
	static int skip_whitespace_flag;

public:

  enum stripType {Leading = 0x1, Trailing = 0x2, Both = 0x3};

  CMString();			// Null CMString
  CMString(size_t ic);	// Suggested capacity
  CMString(const CMString& S);	// Copy constructor
  CMString(const wchar_t * a);		// Copy to embedded null
  CMString(const wchar_t * a, size_t N);	// Copy past any embedded nulls
  CMString(wchar_t);
  CMString(wchar_t, size_t N);

  CMString(const CMSubString& SS);

  ~CMString();

  static int set_case_sensitive(int flag);
  static int skip_whitespace(int flag);
  static int is_case_sensitive() {return case_sensitive_flag;}

  // Assignment:
  CMString&	operator=(const wchar_t*);		// Replace CMString
  CMString&	operator=(const CMString&);	// Replace CMString
  CMString&	operator+=(const wchar_t*);	// Append CMString.
  CMString&	operator+=(const CMString& s);
  CMString&	operator+=(wchar_t c);
  CMString&	operator+=(int d);
  CMString&	operator+=(long d);
  CMString&	operator+=(double f);

  // Indexing operators:
  CMSubString	operator()(size_t start, size_t len);		// Sub-string operator
  CMSubString	substring(const wchar_t* pat, size_t start = 0);

  wchar_t		operator[](size_t) const;
  const CMSubString	substr(size_t start) const;
  const CMSubString	substr(size_t start, size_t len) const;
  const CMSubString	substring(const wchar_t* pat, size_t start = 0) const;	// Match the RE

		// Non-static member functions:
  CMString&	append(const wchar_t* cs);
  CMString&	append(const wchar_t* cs, size_t N);
  CMString&	append(const CMString& s);
  CMString&	append(const CMString& s, size_t N);
  CMString&	append(wchar_t c, size_t rep = 1);	// Append c rep times
  size_t	binaryStoreSize() const		{return length()+sizeof(size_t);}
  size_t	capacity() const		{return pref_->capacity();}
  size_t	capacity(size_t N);
  int		collate(const wchar_t* cs) const	{ return pref_->collate(cs); }
  int		collate(const CMString& st) const;
  int		compareTo(const wchar_t* cs) const;
  int		compareTo(const CMString& st) const;
  BOOL	contains(const wchar_t* pat) const;
  BOOL	contains(const CMString& pat) const;
  CMString    	copy() const;
  const wchar_t*  	c_str() const { return pref_->array(); }
  size_t	find(wchar_t c) const			{ return pref_->find(c); }
  size_t	find(const wchar_t* cs) const		{ return pref_->find(cs); }
  size_t	find(const CMString& c) const {return find(c_str());}
  unsigned	hash() const;
  size_t	index(const wchar_t* pat, size_t i = 0)
				const;
  size_t	index(const CMString& s, size_t i=0)
				const;
  size_t	index(const wchar_t* pat, size_t patlen, size_t i) const;
  size_t	index(const CMString& s, size_t patlen, size_t i) const;
  CMString&	insert(size_t pos, const wchar_t*);
  CMString&	insert(size_t pos, const wchar_t*, size_t extent);
  CMString&	insert(size_t pos, const CMString&);
  CMString&	insert(size_t pos, const CMString&, size_t extent);
  BOOL	isAscii() const;
  BOOL	is_null() const				{return pref_->nchars_ == 0;}
  size_t	last(char c) const			{return pref_->last(c);}
  size_t  	length() const				{return pref_->nchars_;}
  //size_t	mbLength() const;	// multibyte length, or CM_NPOS on error
  CMString&	prepend(const wchar_t*);			// Prepend a character CMString
  CMString&	prepend(const wchar_t* cs, size_t N);
  CMString&	prepend(const CMString& s);
  CMString&	prepend(const CMString& s, size_t N);
  CMString&	prepend(wchar_t c, size_t rep = 1);	// Prepend c rep times
  wistream&	readFile(wistream&);			// Read to EOF or null character.
  wistream&	read_line(wistream&);	// Read to EOF or newline.
  wistream&	readString(wistream&);			// Read to EOF or null character.
  wistream&	readToDelim(wistream&, wchar_t delim);	// Read to EOF or delimitor.
  wistream&	readToken(wistream&);			// Read separated by white space.
  CMString&	remove(size_t pos);			// Remove pos to end of CMString
  CMString&	remove(size_t pos, size_t n);		// Remove n chars starting at pos
  CMString&	replace(size_t pos, size_t n, const wchar_t*);
  CMString&	replace(size_t pos, size_t n, const wchar_t*, size_t);
  CMString&	replace(size_t pos, size_t n, const CMString&);
  CMString&	replace(size_t pos, size_t n, const CMString&, size_t);
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
	CMString(const wchar_t* a1, size_t N1, const wchar_t* a2, size_t N2);
  void			cow();				// Do copy on write as needed
  void			cow(size_t nc);			// Do copy on write as needed
  static size_t		adjustCapacity(size_t nc);

private:

  void			clone();          // Make self a distinct copy
  void			clone(size_t nc); // Make self a distinct copy w. capacity nc

  static size_t	initialCapac;		// Initial allocation Capacity
  static size_t	resizeInc;		// Resizing increment
  static size_t	freeboard;		// Max empty space before reclaim

  CMStringRef*		pref_;		// Pointer to ref. counted data

friend CMString _IRPFUNC operator+(const CMString& s1, const CMString& s2);
friend CMString _IRPFUNC operator+(const CMString& s, const wchar_t* cs);
friend CMString _IRPFUNC operator+(const wchar_t* cs, const CMString& s);
friend BOOL _IRPFUNC operator==(const CMString& s1, const CMString& s2);
friend BOOL _IRPFUNC operator==(const CMString& s1, const wchar_t* s2);
friend class _IRPCLASS CMSubString;
friend class _IRPCLASS CMStringRef;

};

// Related global functions:
_IRPFUNC wistream& operator>>(wistream& str, CMString& cstr);
_IRPFUNC wostream& operator<<(wostream& str, const CMString& cstr);

CMString _IRPFUNC to_lower(const CMString&);	// Return lower-case version of argument.
CMString _IRPFUNC to_upper(const CMString&);	// Return upper-case version of argument.
inline    unsigned cmhash(const CMString& s) { return s.hash(); }
inline    unsigned cmhash(const CMString* s) { return s->hash(); }
#ifndef RW_NO_LOCALE
CMString _IRPFUNC strXForm(const CMString&);	// strxfrm() interface
#endif


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                               Inlines                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

inline int CMString::set_case_sensitive(int flag)
{int oldflag=case_sensitive_flag;case_sensitive_flag=flag;return oldflag;}

inline int CMString::skip_whitespace(int flag)
{int oldflag=skip_whitespace_flag;skip_whitespace_flag=flag;return oldflag;}

inline void CMString::cow()
{ if (pref_->References() > 1) clone(); }

inline void CMString::cow(size_t nc)
{ if (pref_->References() > 1  || capacity() < nc) clone(nc); }

inline CMString& CMString::append(const wchar_t* cs)
{ return replace(length(), 0, cs, wcslen(cs)); }

inline CMString& CMString::append(const wchar_t* cs, size_t N)
{ return replace(length(), 0, cs, N); }

inline CMString& CMString::append(const CMString& s)
{ return replace(length(), 0, s.c_str(), s.length()); }

inline CMString& CMString::append(const CMString& s, size_t N)
{ return replace(length(), 0, s.c_str(),cmmin(N, s.length())); }

inline CMString& CMString::operator+=(const wchar_t* cs)
{ return append(cs, wcslen(cs)); }

inline CMString& CMString::operator+=(const CMString& s)
{ return append(s.c_str(),s.length()); }

inline CMString& CMString::operator+=(wchar_t c)
{ return append(&c,1);}

#ifndef RW_NO_LOCALE
inline int CMString::collate(const CMString& st) const
{ return pref_->collate(st.c_str()); }
#endif

inline BOOL CMString::contains(const CMString& pat) const
{ return index(pat.c_str(), pat.length(), (size_t)0) != CM_NPOS; }

inline BOOL CMString::contains(const wchar_t* s) const
{ return index(s, wcslen(s), (size_t)0) != CM_NPOS; }

inline size_t CMString::index(const wchar_t* s, size_t i) const
{ return index(s, wcslen(s), i); }

inline size_t CMString::index(const CMString& s, size_t i) const
{ return index(s.c_str(), s.length(), i); }

inline size_t CMString::index(const CMString& pat, size_t patlen, size_t i) const
{ return index(pat.c_str(), patlen, i); }

inline CMString& CMString::insert(size_t pos, const wchar_t* cs)
{ return replace(pos, 0, cs, wcslen(cs)); }

inline CMString& CMString::insert(size_t pos, const wchar_t* cs, size_t N)
{ return replace(pos, 0, cs, N); }

inline CMString& CMString::insert(size_t pos, const CMString& cstr)
{ return replace(pos, 0, cstr.c_str(), cstr.length()); }

inline CMString& CMString::insert(size_t pos, const CMString& cstr, size_t N)
{ return replace(pos, 0, cstr.c_str(), cmmin(N, cstr.length())); }

inline CMString& CMString::prepend(const wchar_t* cs)
{ return replace(0, 0, cs, wcslen(cs)); }

inline CMString& CMString::prepend(const wchar_t* cs, size_t N)
{ return replace(0, 0, cs, N); }

inline CMString& CMString::prepend(const CMString& s)
{ return replace(0, 0, s.c_str(), s.length()); }

inline CMString& CMString::prepend(const CMString& s, size_t N)
{ return replace(0, 0, s.c_str(), cmmin(N, s.length())); }

inline CMString& CMString::remove(size_t pos)
{ return replace(pos, length()-pos, 0, 0); }

inline CMString& CMString::remove(size_t pos, size_t n)
{ return replace(pos, n, 0, 0); }

inline CMString& CMString::replace(size_t pos, size_t n, const wchar_t* cs)
{ return replace(pos, n, cs, wcslen(cs)); }

inline CMString& CMString::replace(size_t pos, size_t n, const CMString& cstr)
{ return replace(pos, n, cstr.c_str(), cstr.length()); }

inline CMString& CMString::replace(size_t pos, size_t n1, const CMString& cstr, size_t n2)
{ return replace(pos, n1, cstr.c_str(), cmmin(cstr.length(),n2)); }


inline wchar_t CMString::operator[](size_t i) const
{ return (*pref_)[i]; }

inline const wchar_t* CMSubString::array() const
{ return str_->c_str() + begin_; }

// Access to elements of sub-CMString with bounds checking

inline wchar_t CMSubString::operator[](size_t i) const
{return (*str_->pref_)[begin_+i]; }

// String Logical operators:
inline BOOL	operator==(const CMString& s1, const CMString& s2)
				  { return ((s1.length() == s2.length()) && !s1.compareTo(s2)); }
inline BOOL	operator!=(const CMString& s1, const CMString& s2)
				  { return !(s1 == s2); }
inline BOOL	operator< (const CMString& s1, const CMString& s2)
				  { return s1.compareTo(s2)< 0;}
inline BOOL	operator> (const CMString& s1, const CMString& s2)
				  { return s1.compareTo(s2)> 0;}
inline BOOL	operator<=(const CMString& s1, const CMString& s2)
				  { return s1.compareTo(s2)<=0;}
inline BOOL	operator>=(const CMString& s1, const CMString& s2)
				  { return s1.compareTo(s2)>=0;}

//     BOOL	operator==(const CMString& s1, const wchar_t* s2);
inline BOOL	operator!=(const CMString& s1, const wchar_t* s2)
				  { return !(s1 == s2); }
inline BOOL	operator< (const CMString& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)< 0; }
inline BOOL	operator> (const CMString& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)> 0; }
inline BOOL	operator<=(const CMString& s1, const wchar_t* s2)
                                  { return s1.compareTo(s2)<=0; }
inline BOOL	operator>=(const CMString& s1, const wchar_t* s2)
				  { return s1.compareTo(s2)>=0; }

inline BOOL	operator==(const wchar_t* s1, const CMString& s2)
				  { return (s2 == s1); }
inline BOOL	operator!=(const wchar_t* s1, const CMString& s2)
				  { return !(s2 == s1); }
inline BOOL	operator< (const wchar_t* s1, const CMString& s2)
				  { return s2.compareTo(s1)> 0; }
inline BOOL	operator> (const wchar_t* s1, const CMString& s2)
				  { return s2.compareTo(s1)< 0; }
inline BOOL	operator<=(const wchar_t* s1, const CMString& s2)
											 { return s2.compareTo(s1)>=0; }
inline BOOL	operator>=(const wchar_t* s1, const CMString& s2)
				  { return s2.compareTo(s1)<=0; }

// SubString Logical operators:
//     BOOL operator==(const CMSubString& s1, const CMSubString& s2);
//     BOOL operator==(const CMSubString& s1, const wchar_t* s2);
//     BOOL operator==(const CMSubString& s1, const CMString& s2);
inline BOOL operator==(const CMString& s1,    const CMSubString& s2)
				{ return (s2 == s1); }
inline BOOL operator==(const wchar_t* s1, const CMSubString& s2)
				{ return (s2 == s1); }
inline BOOL operator!=(const CMSubString& s1, const wchar_t* s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const CMSubString& s1, const CMString& s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const CMSubString& s1, const CMSubString& s2)
				{ return !(s1 == s2); }
inline BOOL operator!=(const CMString& s1,    const CMSubString& s2)
				{ return !(s2 == s1); }
inline BOOL operator!=(const wchar_t* s1, const CMSubString& s2)
				{ return !(s2 == s1); }


