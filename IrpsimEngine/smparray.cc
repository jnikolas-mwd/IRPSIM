#if !defined (_CM_COMPILE_INSTANTIATE)
#include <cm/smparray.h>
#endif

template<class TP> inline
CMPSmallArray<TP>::CMPSmallArray(unsigned short sz,unsigned short d) :
count(0),
size(0),
delta(d),
array(0)
{
	if (sz)
		Resize(sz);
}

template<class TP> inline
CMPSmallArray<TP>::~CMPSmallArray()
{
	Reset(1);
}

template<class TP> inline  void
CMPSmallArray<TP>::Reset(int freemem)
{
	reset(freemem,0);
}

template<class TP> inline  void
CMPSmallArray<TP>::ResetAndDestroy(int freemem)
{
	reset(freemem,1);
}

template<class TP> inline  int
CMPSmallArray<TP>::Add(TP* val)
{
	return AddAt(count,val);
}

template<class TP> inline  unsigned short
CMPSmallArray<TP>::Count() const
{
	return count;
}

template<class TP> inline  TP*
CMPSmallArray<TP>::operator [] (unsigned short n) const
{
	return At(n);
}

template<class TP> inline  TP**
CMPSmallArray<TP>::Array() const
{
	return array;
}

/*
template<class TP> inline CMPSmallArray<TP>&
CMPSmallArray<TP>::operator = (const CMPSmallArray<TP>& a)
{
	Reset(1);
	for (unsigned short i=0;i<a.Count();i++)
		AddAt(i,new TP(*a.At(i)));
	return *this;
}
*/

template<class TP> inline int
CMPSmallArray<TP>::operator == (const CMPSmallArray<TP>& a) const
{
	if (Count() != a.Count())
		return 0;
	for (unsigned short i=0;i<Count();i++) {
		if (At(i)==0 && a.At(i)!=0)
      	return 0;
		else if (At(i)!=0 && a.At(i)==0)
      	return 0;
      else if (At(i)==0 && a.At(i)==0)
      	continue;
		else if (!(*At(i) == *a.At(i)))
			return 0;
   }
	return 1;
}

template<class TP> inline void
CMPSmallArray<TP>::reset(int freemem,int destroy)
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
CMPSmallArray<TP>::make_new_array(unsigned short& sz)
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
CMPSmallArray<TP>::Resize(unsigned short sz)
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

template<class TP> inline void
CMPSmallArray<TP>::Exchange(unsigned short i,unsigned short j)
{
	TP* temp = At(i);
	AddAt(i,At(j));
	AddAt(j,temp);
}

template<class TP> inline int
CMPSmallArray<TP>::AddAt(unsigned short pos,TP* val)
{
	if (pos >= size)
		if (Resize(pos+delta) < pos+delta)
			return 0;

	array[pos] = val;
	if (count <= pos) count = pos+1;
	return 1;
}

template<class TP> inline int
CMPSmallArray<TP>::InsertAt(unsigned short pos,TP* val)
{
	if (pos>=Count())
		return AddAt(pos,val);

	for (unsigned short i=Count();i>pos;i--)
		AddAt(i,At(i-1));

	array[pos] = val;

	return 1;
}

template<class TP> inline TP*
CMPSmallArray<TP>::At(unsigned short pos) const
{
	return (pos < count) ? array[pos] : 0;
}

template<class TP> inline int
CMPSmallArray<TP>::Contains(const TP& val,unsigned short* loc)
{
	for (unsigned short i=0;i<Count();i++)
		if (At(i)!=0 && *At(i) == val) {
			if (loc) *loc=i;
			return 1;
		}
	return 0;
}

template<class TP> inline void
CMPSmallArray<TP>::Detach(const TP* val,int destroy)
{
	unsigned short loc;
	if (Contains(*val,&loc))
		DetachAt(loc,destroy);
}

template<class TP> inline void
CMPSmallArray<TP>::Detach(const TP& val,int destroy)
{
	unsigned short loc;
	if (Contains(val,&loc))
		DetachAt(loc,destroy);
}

template<class TP> inline void
CMPSmallArray<TP>::DetachAt(unsigned short loc,int destroy)
{
	if (loc >= Count()) return;
	TP* val = At(loc);
	for (unsigned short i=loc;i<Count()-1;i++)
		AddAt(i,At(i+1));
	if (destroy && val)
		delete val;
	count--;
}


template<class TP> inline
CMPSSmallArray<TP>::CMPSSmallArray(unsigned short sz,unsigned short d) :
CMPSmallArray<TP>(sz,d)
{
}

template<class TP> inline void
CMPSSmallArray<TP>::Sort()
{
	if (!Count()) return;
   sort_call(0,Count()-1);
}

template<class TP> inline void
CMPSSmallArray<TP>::sort_call(unsigned short p,unsigned short r)
{
	static const unsigned short selectionSortSize = 5;
   TP* key;

	if( r - p <= selectionSortSize )  {
		for( unsigned short j = p + 1; j <= r; j++ ) {
   		key = At(j);
			unsigned short i;
			for( i = j - 1; *key < *At(i); i-- ) {
				AddAt(i+1,At(i));
				if( i == p ) {
					i--;
					break;
				}
			}
			AddAt(i+1,key);
		}
	}
	else {

	   unsigned short pivotIndex = (unsigned short)(((long)p+r) >> 1);

		Exchange( pivotIndex, p );

		unsigned short i = p - 1, j = r + 1;
		key = At(p);

		// Partition the elements.
		while (1) {
			while( *key < *At(--j) );
			while( *At(++i) < *key );
			if( i < j )
				Exchange( i, j );
			else
				break;
		}
		sort_call( p, j );
		sort_call( j+1, r );
   }
}

template<class TP> inline unsigned short
CMPSSmallArray<TP>::get_insert_position(const TP& val,int& entryexists) const
{
	entryexists = 0;
   unsigned short i;

	if (!Count())	return 0;

	unsigned short bottom = 0;
	unsigned short top = Count()-1;

	while (top-bottom>4) {
		unsigned short test = (bottom + top) >> 1;
		if (val < *At(test))
			top = test;
		else
			bottom = test;
	}

	for (i=bottom;i<=top;i++) {
		if (At(i)!=0 && val==*At(i)) {
			entryexists=1;
			return i;
		}
		else if (At(i)!=0 && val<*At(i))
			break;
	}
	return i;
}

template<class TP> inline int
CMPSSmallArray<TP>::Find(const TP& val,unsigned short* loc)
{
	if (loc) *loc = 0;
	int ret;
	unsigned short pos = get_insert_position(val,ret);
	if (loc && ret)
		*loc = pos;
	return ret;
}

template<class TP> inline int
CMPSSmallArray<TP>::Insert(TP* val,int duplicate_allowed)
{
	int entryexists;
	unsigned short pos = get_insert_position(*val,entryexists);
	if (!duplicate_allowed && entryexists)
		return 1;
	return InsertAt(pos,val);
}

template<class TP> inline int
CMPSSmallArray<TP>::Insert(const TP& val,int duplicate_allowed)
{
	int entryexists;
	unsigned short pos = get_insert_position(val,entryexists);
	if (!duplicate_allowed && entryexists)
		return 1;
	return InsertAt(pos,new TP(val));
}

/*
template<class TP> inline void
CMPSSmallArray<TP>::Sort()
{
	if (!Count())
   	return;

	unsigned short n=Count()-1,sorted=0;

	while (!sorted && n > 0) {
		sorted = 1;
		for (int i=0;i<n;i++) {
			if (*At(i+1) < *At(i)) {
				TP* temp = At(i);
				AddAt(i,At(i+1));
				AddAt(i+1,temp);
				sorted = 0;
			}
		}
		n--;
	}
}

template<class TP> inline void
CMPSSmallArray<TP>::BSort()
{
   long p;
	if (!Count())
		return;
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
