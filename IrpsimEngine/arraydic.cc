#if !defined (_CM_COMPILE_INSTANTIATE)
#include "arraydic.h"
#endif

template<class TP> inline
CMArrayDictionary<TP>::CMArrayDictionary(unsigned short sz,unsigned short d) :
count(0),
size(0),
delta(d),
array(0),
issorted(0)
{
	if (sz)
		resize(sz);
}

template<class TP> inline 
CMArrayDictionary<TP>::~CMArrayDictionary()
{
	Reset(1);
}

template<class TP> inline  void
CMArrayDictionary<TP>::Reset(int freemem)
{
	reset(freemem,0);
}

template<class TP> inline  void
CMArrayDictionary<TP>::ResetAndDestroy(int freemem)
{
	reset(freemem,1);
}

template<class TP> inline  int
CMArrayDictionary<TP>::Add(TP* val)
{
	return addat(count,val);
}

template<class TP> inline  unsigned short
CMArrayDictionary<TP>::Count() const
{
	return count;
}

template<class TP> inline  TP*
CMArrayDictionary<TP>::operator [] (unsigned short n) const
{
	return At(n);
}

template<class TP> inline CMArrayDictionary<TP>&
CMArrayDictionary<TP>::operator = (const CMArrayDictionary<TP>& a)
{
	Reset(1);
	for (unsigned short i=0;i<a.Count();i++)
		addat(i,new TP(*a.At(i)));
	return *this;
}

template<class TP> inline int
CMArrayDictionary<TP>::operator == (const CMArrayDictionary<TP>& a)
{
	if (Count() != a.Count())
		return 0;
	for (unsigned short i=0;i<Count();i++)
		if (!(*At(i) == *a.At(i)))
			return 0;
	return 1;
}

template<class TP> inline void
CMArrayDictionary<TP>::reset(int freemem,int destroy)
{
	if (array) {
		if (destroy)
			for (unsigned short i=0;i<count;i++)
				delete array[i];
		if (freemem) {
			delete [] array;
			array = 0;
			size = 0;
		}
	}
	count = 0;
}

template<class TP> inline TP**
CMArrayDictionary<TP>::make_new_array(unsigned short& sz)
{
	TP** arr=0;
	while (sz) {
		if ((arr = new TP*[sz])!=0)
			break;
		sz--;
	}
	return arr;
}

template<class TP> inline unsigned short
CMArrayDictionary<TP>::resize(unsigned short sz)
{
	unsigned short maxsize = (unsigned short)(32768L/sizeof(TP*));
	if (sz > maxsize) sz = maxsize;

	TP** oldarray = array;
	array = make_new_array(sz);

	for (unsigned short i=0;array && i<size && i<sz;i++)
		array[i] = oldarray[i];

	delete [] oldarray;
	if (count>sz) count = sz;
	return (size = sz);
}


template<class TP> inline int
CMArrayDictionary<TP>::addat(unsigned short pos,TP* val)
{
	if (pos >= size)
		if (resize(pos+delta) < pos+delta)
			return 0;

	array[pos] = val;
	if (count <= pos) count = pos+1;
	issorted=0;
	return 1;
}


template<class TP> inline TP*
CMArrayDictionary<TP>::At(unsigned short pos) const
{
	return (pos<count) ? array[pos] : 0;
}

template<class TP> inline TP*
CMArrayDictionary<TP>::Find(const CMString& name,unsigned short* loc)
{
	if (loc) *loc=0;

	if (!count)
   	return 0;

	Sort();
	unsigned short bottom = 0;
	unsigned short top = count-1;

	while (top-bottom>4) {
		unsigned short test = (bottom + top) >> 1;
		if (name < At(test)->GetName())
			top = test;
		else
			bottom = test;
	}

	for (unsigned short i=bottom;i<=top;i++)
		if (name == At(i)->GetName()) {
			if (loc) *loc=i;
			return At(i);
		}
	return 0;
}

template<class TP> inline void
CMArrayDictionary<TP>::Detach(const CMString& name,int destroy)
{
	unsigned short loc;
	TP* val = Find(name,&loc);
	if (val==0)
		return;
	for (unsigned short i=loc;i<Count()-1;i++)
		addat(i,At(i+1));
	if (destroy)
		delete val;
	count--;
}

template<class TP> inline void
CMArrayDictionary<TP>::Sort()
{
	if (issorted || !Count())
		return;
   long p;
	for (p=1;p<Count();p<<=1);
	p>>=1;
	unsigned short val = (unsigned short)p;
	for (;p>0;p>>=1) {
		unsigned short q = val;
		unsigned short r = 0;
		unsigned short d = (unsigned short)p;
		while (1) {
			for (unsigned short i=0;i<Count()-d;i++) {
				if ( (i&p) == r) {
					if (*At(i+d) < *At(i)) {
						TP* temp = At(i);
						addat(i,At(i+d));
						addat(i+d,temp);
					}
				}
			}
			if (q==p)
				break;
			d = q-(unsigned short)p;
			q>>=1;
			r=(unsigned short)p;
		}
	}
	issorted=1;
}

