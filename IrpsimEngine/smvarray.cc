#if !defined (_CM_COMPILE_INSTANTIATE)
#include "smvarray.h"
#endif

template<class T> inline
CMVSmallArray<T>::CMVSmallArray(unsigned short sz,unsigned short d) :
count(0),
size(0),
delta(d),
array(0)
{
	if (sz)
		Resize(sz);
}

template<class T> inline
CMVSmallArray<T>::~CMVSmallArray()
{
	Reset(1);
}

template<class T> inline int
CMVSmallArray<T>::Add(const T& val)
{
	return AddAt(count,val);
}

template<class T> inline unsigned short
CMVSmallArray<T>::Count() const
{
	return count;
}

template<class T> inline T&
CMVSmallArray<T>::operator [] (unsigned short n) const
{
	return At(n);
}

template<class T> inline T*
CMVSmallArray<T>::Array() const
{
	return array;
}

template<class T> inline CMVSmallArray<T>&
CMVSmallArray<T>::operator = (const CMVSmallArray<T>& a)
{
	Reset(1);
	for (unsigned short i=0;i<a.Count();i++)
		AddAt(i,a.At(i));
	return *this;
}

template<class T> inline int
CMVSmallArray<T>::operator == (const CMVSmallArray<T>& a) const
{
	if (Count() != a.Count())
		return 0;
	for (unsigned short i=0;i<Count();i++)
		if (!(At(i) == a.At(i)))
			return 0;
	return 1;
}

template<class T> inline void
CMVSmallArray<T>::Reset(int freemem)
{
	count = 0;
	if (freemem && array) {
		delete [] array;
		array = 0;
		size = 0;
	}
}

template<class T> inline T*
CMVSmallArray<T>::make_new_array(unsigned short& sz)
{
	T* arr = 0;
	while (sz) {
		if ((arr = new T[sz])!=0)
			break;
		sz--;
	}
	return arr;
}

template<class T> inline unsigned short
CMVSmallArray<T>::Resize(unsigned short sz)
{
	unsigned short maxsize = (unsigned short)(32768L/sizeof(T));
	if (sz > maxsize) sz = maxsize;

	T* oldarray = array;
	array = make_new_array(sz);

	for (unsigned short i=0;array && i<size && i<sz;i++)
		array[i] = oldarray[i];

	delete [] oldarray;

	if (count>sz) count=sz;
	return (size = sz);
}

template<class T> inline void
CMVSmallArray<T>::Exchange(unsigned short i,unsigned short j)
{
	T temp = At(i);
	At(i) = At(j);
	At(j) = temp;
}

template<class T> inline int
CMVSmallArray<T>::AddAt(unsigned short pos,const T& val)
{
	if (pos >= size)
		if (Resize(pos+delta) < pos+delta)
			return 0;

	array[pos] = val;
	if (count <= pos) count = pos+1;
	return 1;
}

template<class T> inline int
CMVSmallArray<T>::InsertAt(unsigned short pos,const T& val)
{
	if (pos>=Count())
		return AddAt(pos,val);

	for (unsigned short i=Count();i>pos;i--)
		AddAt(i,At(i-1));

	array[pos] = val;

	return 1;
}

template<class T> inline T&
CMVSmallArray<T>::At(unsigned short pos) const
{
	static T t;
	return (pos < count) ? array[pos] : t;
}

template<class T> inline int
CMVSmallArray<T>::Contains(const T& val,unsigned short* loc)
{
	for (unsigned short i=0;i<Count();i++)
		if (At(i) == val) {
			if (loc) *loc=i;
			return 1;
		}
	return 0;
}

template<class T> inline void
CMVSmallArray<T>::Detach(const T& val)
{
	unsigned short loc;
	if (Contains(val,&loc))
		DetachAt(loc);
}

template<class T> inline void
CMVSmallArray<T>::DetachAt(unsigned short loc)
{
	if (loc >= Count()) return;
	for (unsigned short i=loc;i<Count()-1;i++)
		AddAt(i,At(i+1));
	count--;
}

template<class T> inline
CMVSSmallArray<T>::CMVSSmallArray(unsigned short sz,unsigned short d) :
CMVSmallArray<T>(sz,d)
{
}

template<class T> inline void
CMVSSmallArray<T>::Sort()
{
	if (!Count()) return;
	sort_call(0,Count()-1);
}

template<class T> inline void
CMVSSmallArray<T>::sort_call(unsigned short p,unsigned short r)
{
	static const unsigned short selectionSortSize = 5;
   T key;

	if( r - p <= selectionSortSize )  {
		for( unsigned short j = p + 1; j <= r; j++ ) {
   		key = At(j);
			unsigned short i;
			for( i = j - 1; key < At(i); i-- ) {
				At(i+1) = At(i);
				if( i == p ) {
					i--;
					break;
				}
			}
			At(i+1) = key;
		}
	}
	else {

	   unsigned short pivotIndex = ((long)p+r) >> 1;

		Exchange( pivotIndex, p );

		unsigned short i = p - 1, j = r + 1;
		key = At(p);

		// Partition the elements.
		while (1) {
			while( key < At(--j) );
			while( At(++i) < key );
			if( i < j )
				Exchange( i, j );
			else
				break;
		}
		sort_call( p, j );
		sort_call( j+1, r );
   }
}

template<class T> inline unsigned short
CMVSSmallArray<T>::get_insert_position(const T& val,int& entryexists) const
{
	entryexists = 0;

	if (!Count()) return 0;

	unsigned short bottom = 0;
	unsigned short top = Count()-1;

	while (top-bottom>4) {
		unsigned short test = (bottom+top) >> 1;
		if (val < At(test))
			top = test;
		else
			bottom = test;
	}

	unsigned short i;
	for (i=bottom;i<=top;i++) {
		if (val==At(i)) {
			entryexists=1;
			break;
		}
		else if (val<At(i))
			break;
	}

	return i;
}

template<class T> inline int
CMVSSmallArray<T>::Find(const T& val,unsigned short* loc)
{
	if (loc) *loc = 0;
	int ret;
	unsigned short pos = get_insert_position(val,ret);
	if (loc && ret)
		*loc = pos;
	return ret;
}

template<class T> inline int
CMVSSmallArray<T>::Insert(const T& val,int duplicate_allowed)
{
	int entryexists;
	unsigned short pos = get_insert_position(val,entryexists);
	if (!duplicate_allowed && entryexists)
		return 1;
	return InsertAt(pos,val);
}

/*
template<class T> inline void
CMVSSmallArray<T>::Sort()
{
	if (!Count())
		return;

	unsigned short n=Count()-1,sorted=0;

	while (!sorted && n > 0) {
		sorted = 1;
		for (int i=0;i<n;i++) {
			if (At(i+1) < At(i)) {
				T temp = At(i);
				At(i) = At(i+1);
				At(i+1) = temp;
				sorted = 0;
			}
		}
		n--;
	}
}

template<class T> inline void
CMVSSmallArray<T>::BSort()
{
	if (!Count())
		return;
	for (long p=1;p<Count();p<<=1);
	p>>=1;
	unsigned short val = (unsigned short)p;
	for (;p>0;p>>=1) {
		unsigned short q = val;
		unsigned short r = 0;
		unsigned short d = (unsigned short)p;
		while (1) {
			for (unsigned short i=0;i<Count()-d;i++) {
				if ( (i&p) == r) {
					if (At(i+d) < At(i)) {
						T temp = At(i);
						AddAt(i,At(i+d));
						AddAt(i+d,temp);
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
}

*/

