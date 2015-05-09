#if !defined (_CM_COMPILE_INSTANTIATE)
#include <cm/bgvarray.h>
#endif

template<class T> T
CMVArrayNode<T>::dummy;

template<class T> inline
CMVArrayNode<T>::CMVArrayNode(long ndx,long n,int lvl) :
startindex(ndx),
level(lvl),
size(0),
array(0),
next(0),
child(0)
{
	Resize(n);
}

template<class T> inline
CMVArrayNode<T>::~CMVArrayNode()
{
	if (array) delete [] array;
}


template<class T> inline long
CMVArrayNode<T>::Resize(long n)
{
	if (n<=size)
   	return size;
	T* oldarray = array;
    long oldsize = size;
	long bytesize = n*sizeof(T);
	if (bytesize>32768L) bytesize = 32768L;
	size = bytesize/sizeof(T);
	while (size) {
		if ((array = new T[size])!=0)
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

template<class T> inline int
CMVArrayNode<T>::AddAt(long pos,const T& val)
{
	long index = pos - startindex;
   if (index < (long)size) {
   	array[index] = val;
      return 1;
   }
   return 0;
}

template<class T> inline T&
CMVArrayNode<T>::At(long n) const
{
	long index = n - startindex;
   return (index < (long)size) ? array[index] : dummy;
}

template<class T> inline
CMVBigArray<T>::CMVBigArray(long sz,long d,int tlength) :
count(0),
root(0),
left(0),
delta(d),
size(0)
{
	travlength = (tlength>1) ? tlength : 4;
   if (sz) Resize(sz);
}

template<class T> inline
CMVBigArray<T>::~CMVBigArray()
{
	Reset(1);
}

template<class T> inline long
CMVBigArray<T>::Count() const
{
	return count;
}

template<class T> inline CMVBigArray<T>&
CMVBigArray<T>::operator = (const CMVBigArray<T>& a)
{
	Reset(1);
	for (long i=0;i<a.Count();i++)
		AddAt(i,a.At(i));
	return *this;
}

template<class T> inline int
CMVBigArray<T>::operator == (const CMVBigArray<T>& a) const
{
	if (Count() != a.Count())
		return 0;
	for (long i=0;i<Count();i++)
		if (!(At(i) == a.At(i)))
			return 0;
	return 1;
}

template<class T> inline void
CMVBigArray<T>::Reset(int freemem)
{
	count = 0;
	if (freemem) {
		CMVArrayNode<T>* node = root;
		while (node) {
			CMVArrayNode<T>* child = node->child;
			while (node) {
				CMVArrayNode<T>* next = node->next;
            delete node;
            node=next;
         }
         node=child;
      }
      root=left=0;
      size=0;
   }
}

template<class T> inline void
CMVBigArray<T>::Exchange(long i,long j)
{
	T temp = At(i);
	At(i) = At(j);
	At(j) = temp;
}

template<class T> inline int
CMVBigArray<T>::Add(const T& val)
{
	return AddAt(count,val);
}

template<class T> inline int
CMVBigArray<T>::AddAt(long pos,const T& val)
{
	if (pos >= size)
	if (Resize(pos+delta) < (long)pos+delta)
			return 0;
	if (pos>=count) count=pos+1;
	return seek(pos)->AddAt(pos,val);
}

template<class T> inline T&
CMVBigArray<T>::At(long pos) const
{
   return seek(pos)->At(pos);
}

template<class T> inline  T&
CMVBigArray<T>::operator [] (long n) const
{
	return At(n);
}

template<class T> inline void
CMVBigArray<T>::Detach(const T& val)
{
	long loc;
	if (Contains(val,&loc))
		DetachAt(loc);
}

template<class T> inline void
CMVBigArray<T>::DetachAt(long loc)
{
	if (loc<0 || loc>=count)
   	return;

   CMVArrayNode<T>* node = seek(loc);
	long i = loc - node->startindex;

	long n=loc;
   while (node) {
   	for (;i<node->size&&n<count-1;i++,n++) {
			if (i==node->size-1) {
         	if (node->next && node->next->size)
            	node->array[i] = node->next->array[0];
         }
         else
         	node->array[i] = node->array[i+1];
		}
		node = node->next;
      i=0;
   }

   count--;
}

template<class T> inline CMVArrayNode<T>*
CMVBigArray<T>::seek(long pos) const
{
	if (!root) return 0;
	CMVArrayNode<T>* node = root;
   while (node->child) {
		node = node->child;
      while (node->next && node->next->startindex<=pos)
         node = node->next;
   }
   return node;
}

template<class T> inline int
CMVBigArray<T>::Contains(const T& val,long* loc)
{
   CMVArrayNode<T>* node = left;
	long n=0;
   while (node) {
   	for (long i=0;i<node->size&&n<count;i++,n++) {
			if (node->array[i] == val) {
         	if (loc) *loc = node->startindex + i;
            return 1;
         }
      }
		node = node->next;
   }
   return 0;
}

template<class T> inline long
CMVBigArray<T>::Resize(long sz)
{
	if (sz<=size)
   	return size;
	CMVArrayNode<T>* rightmost = seek(size);
	if (rightmost) {
		long oldsize = rightmost->size;
	   rightmost->Resize(rightmost->size + (sz-size));
   	size += (rightmost->size-oldsize);
   }
   while (size < sz)
   	make_new_leaf(0,sz-size);
   return size;
}

template<class T> inline CMVArrayNode<T>*
CMVBigArray<T>::make_new_leaf(int level,long sz)
{
   CMVArrayNode<T> *node,*newnode,*rightmost;

	if (left==0) {
   	root = left = new CMVArrayNode<T>(0,sz,0);
      size += root->size;
      return root;
   }

	if (root->level == level) {
   	node = new CMVArrayNode<T>(0,0,level+1);
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

   newnode = new CMVArrayNode<T>(size,level==0 ? sz : 0,level);
   rightmost->next = newnode;

   if ((peers%travlength) == 1) {
	   node = make_new_leaf(level+1,0);
      node->child = newnode;
   }

   if (level==0) size += newnode->size;
	return newnode;
}

template<class T> inline
CMVSBigArray<T>::CMVSBigArray(long sz,long d,int tlength) :
CMVBigArray<T>(sz,d,tlength)
{
}

template<class T> inline void
CMVSBigArray<T>::Sort()
{
	if (!Count()) return;
	sort_call(0,Count()-1);
}

template<class T> inline void
CMVSBigArray<T>::sort_call(long p,long r)
{
	static const long selectionSortSize = 5;
   T key;

	if( r - p <= selectionSortSize )  {
		for( long j = p + 1; j <= r; j++ ) {
   		key = At(j);
			long i;
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

	   long pivotIndex = (p+r) >> 1;

		Exchange( pivotIndex, p );

		long i = p - 1, j = r + 1;
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



