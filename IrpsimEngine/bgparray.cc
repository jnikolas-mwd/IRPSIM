#if !defined (_CM_COMPILE_INSTANTIATE)
#include <cm/bgparray.h>
#endif

template<class TP> inline
CMPArrayNode<TP>::CMPArrayNode(long ndx,long n,int lvl) :
startindex(ndx),
level(lvl),
size(0),
array(0),
next(0),
child(0)
{
	Resize(n);
}

template<class TP> inline
CMPArrayNode<TP>::~CMPArrayNode()
{
	if (array) delete [] array;
}

template<class TP> inline long
CMPArrayNode<TP>::Resize(long n)
{
	if (n<=size)
   	return size;
	TP** oldarray = array;
    long oldsize = size;
	long bytesize = n*sizeof(TP*);
	if (bytesize>32768L) bytesize = 32768L;
	size = bytesize/sizeof(TP*);
	while (size) {
		if ((array = new TP*[size])!=0)
			break;
		size--;
	}
	if (oldarray && array) {
   	for (long i=0;i<oldsize&&i<size;i++)
      	array[i] = oldarray[i];
      delete [] oldarray;
   }
   return size;
}

template<class TP> inline void
CMPArrayNode<TP>::Destroy(long n)
{
	for (long i=0;i<n&&i<size;i++)
   	if (array[i]!=0) delete array[i];
}

template<class TP> inline int
CMPArrayNode<TP>::AddAt(long pos,TP* val)
{
	long index = pos - startindex;
   if (index < (long)size) {
   	array[index] = val;
      return 1;
   }
   return 0;
}

template<class TP> inline TP*
CMPArrayNode<TP>::At(long n) const
{
	long index = n - startindex;
   return (index < (long)size) ? array[index] : 0;
}

template<class TP> inline
CMPBigArray<TP>::CMPBigArray(long sz,long d,int tlength) :
count(0),
root(0),
left(0),
delta(d),
size(0)
{
	travlength = (tlength>1) ? tlength : 4;
   if (sz) Resize(sz);
}

template<class TP> inline
CMPBigArray<TP>::~CMPBigArray()
{
	reset(1,0);
}

template<class TP> inline  void
CMPBigArray<TP>::Reset(int freemem)
{
	reset(freemem,0);
}

template<class TP> inline  void
CMPBigArray<TP>::ResetAndDestroy(int freemem)
{
	reset(freemem,1);
}

template<class TP> inline void
CMPBigArray<TP>::reset(int freemem,int destroy)
{
	if (destroy) {
		CMPArrayNode<TP>* node = left;
      long n = count;
      while (node) {
      	node->Destroy(n);
         n-=node->size;
         node=node->next;
      }
	}
	if (freemem) {
		CMPArrayNode<TP>* node = root;
		while (node) {
			CMPArrayNode<TP>* child = node->child;
			while (node) {
				CMPArrayNode<TP>* next = node->next;
            delete node;
            node=next;
         }
         node=child;
      }
      root=left=0;
      size=0;
   }
	count = 0;
}

template<class TP> inline long
CMPBigArray<TP>::Count() const
{
	return count;
}

template<class TP> inline CMPBigArray<TP>&
CMPBigArray<TP>::operator = (const CMPBigArray<TP>& a)
{
	Reset(1);
	for (long i=0;i<a.Count();i++)
		AddAt(i,new TP(*a.At(i)));
	return *this;
}

template<class TP> inline int
CMPBigArray<TP>::operator == (const CMPBigArray<TP>& a) const
{
	if (Count() != a.Count())
		return 0;
	for (long i=0;i<Count();i++) {
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
CMPBigArray<TP>::Exchange(long i,long j)
{
	TP* temp = At(i);
	AddAt(i,At(j));
	AddAt(j,temp);
}

template<class TP> inline int
CMPBigArray<TP>::Add(TP* val)
{
	return AddAt(count,val);
}

template<class TP> inline int
CMPBigArray<TP>::AddAt(long pos,TP* val)
{
	if (pos >= size)
   	if (Resize(pos+delta) < pos+delta)
			return 0;
   if (pos>=count) count=pos+1;
   return seek(pos)->AddAt(pos,val);
}

template<class TP> inline TP*
CMPBigArray<TP>::At(long pos) const
{
   return ((count>0) ? seek(pos)->At(pos) : 0);
}

template<class TP> inline  TP*
CMPBigArray<TP>::operator [] (long n) const
{
	return At(n);
}

template<class TP> inline void
CMPBigArray<TP>::Detach(const TP* val,int destroy)
{
	long loc;
	if (Contains(*val,&loc))
		DetachAt(loc,destroy);
}

template<class TP> inline void
CMPBigArray<TP>::Detach(const TP& val,int destroy)
{
	long loc;
	if (Contains(val,&loc))
		DetachAt(loc,destroy);
}

template<class TP> inline void
CMPBigArray<TP>::DetachAt(long loc,int destroy)
{
	if (loc<0 || loc>=count)
   	return;

   CMPArrayNode<TP>* node = seek(loc);
   TP* val = node->At(loc);

	long i = loc - node->startindex;
   long n=loc;
   while (node) {
   	for (;i<node->size&&n<count-1;i++,n++) {
			if (i==node->size-1)
         	node->array[i] = (node->next && node->next->size) ? node->next->array[0] : 0;
         else
         	node->array[i] = node->array[i+1];
		}
		node = node->next;
      i=0;
   }

   if (destroy && val) delete val;

   count--;
}

template<class TP> inline CMPArrayNode<TP>*
CMPBigArray<TP>::seek(long pos) const
{
	if (!root) return 0;
	CMPArrayNode<TP>* node = root;
   while (node->child) {
		node = node->child;
      while (node->next && node->next->startindex<=pos)
         node = node->next;
   }
   return node;
}

template<class TP> inline int
CMPBigArray<TP>::Contains(const TP& val,long* loc)
{
   CMPArrayNode<TP>* node = left;
   long n=0;
   while (node) {
   	for (long i=0;i<node->size&&n<count;i++,n++) {
			if (node->array[i]!=0 && *node->array[i] == val) {
         	if (loc) *loc = node->startindex + i;
            return 1;
         }
      }
		node = node->next;
   }
   return 0;
}

template<class TP> inline long
CMPBigArray<TP>::Resize(long sz)
{
	if (sz<=size)
   	return size;
	CMPArrayNode<TP>* rightmost = seek(size);
	if (rightmost) {
		long oldsize = rightmost->size;
	   rightmost->Resize(rightmost->size + (sz-size));
   	size += (rightmost->size-oldsize);
   }
   while (size < sz)
   	make_new_leaf(0,sz-size);
   return size;
}

template<class TP> inline CMPArrayNode<TP>*
CMPBigArray<TP>::make_new_leaf(int level,long sz)
{
   CMPArrayNode<TP> *node,*newnode,*rightmost;

	if (left==0) {
   	  root = left = new CMPArrayNode<TP>(0,sz,0);
      size += root->size;
      return root;
   }

	if (root->level == level) {
   	node = new CMPArrayNode<TP>(0,0,level+1);
      node->child=root;
		root=node;
   }

	long peers=2;
	rightmost = root;
   while (level != rightmost->level && rightmost->child)
	  	rightmost=rightmost->child;
	while (rightmost->next) {
   	peers++;
	  	rightmost=rightmost->next;
   }

   newnode = new CMPArrayNode<TP>(size,level==0 ? sz : 0,level);
   rightmost->next = newnode;

   if ((peers%travlength) == 1) {
	   node = make_new_leaf(level+1,0);
      node->child = newnode;
   }

   if (level==0) size += newnode->size;
	return newnode;
}

template<class TP> inline
CMPSBigArray<TP>::CMPSBigArray(long sz,long d,int tlength) :
CMPBigArray<TP>(sz,d,tlength)
{
}

template<class TP> inline void
CMPSBigArray<TP>::Sort()
{
	if (!Count()) return;
	sort_call(0,Count()-1);
}

template<class TP> inline void
CMPSBigArray<TP>::sort_call(long p,long r)
{
	static const long selectionSortSize = 5;
   TP* key;

	if( r - p <= selectionSortSize )  {
		for( long j = p + 1; j <= r; j++ ) {
   		key = At(j);
			long i;
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
	   long pivotIndex = (p+r) >> 1;

		Exchange( pivotIndex, p );

		long i = p - 1, j = r + 1;
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

