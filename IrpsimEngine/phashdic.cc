#if !defined (_CM_COMPILE_INSTANTIATE)
#include <cm/phashdic.h>
#endif

template<class TP>
CMPHashDictionary<TP>::CMPHashDictionary(unsigned short sz,int flag) :
arraysize(sz),
csflag(flag)
{
	array = new CMPSmallArray<TP> [sz];
}

template<class TP>
CMPHashDictionary<TP>::~CMPHashDictionary()
{
	delete [] array;
}

template<class TP> long
CMPHashDictionary<TP>::Count()
{
	long ret = 0;
	for (unsigned short i=0;i<arraysize;i++)
		ret += array[i].Count();
	return ret;
}

template<class TP> void
CMPHashDictionary<TP>::Add(TP* val)
{
	if (!val)
		return;

	CMString s(val->GetName());
	int flag = CMString::set_case_sensitive(csflag);
	unsigned short n = s.hash() % arraysize;
	array[n].Add(val);
	CMString::set_case_sensitive(flag);
}

template<class TP> TP*
CMPHashDictionary<TP>::Find(const CMString& val)
{
	if (!val.length())
		return (TP*)0;
	TP* ret = 0;
	int flag = CMString::set_case_sensitive(csflag);
	unsigned short n = val.hash() % arraysize;
	for (unsigned short i=0;i<array[n].Count()&&ret==0;i++)
		if (val == array[n].At(i)->GetName())
			ret = array[n].At(i);
	CMString::set_case_sensitive(flag);
	return ret;
}

template<class TP> void
CMPHashDictionary<TP>::Reset(int destroy)
{
	for (unsigned short i=0;i<arraysize;i++) {
		if (destroy)	array[i].ResetAndDestroy(1);
		else        	array[i].Reset(1);
	}
}

template<class TP> int
CMPHashDictionary<TP>::Detach(const CMString& val,int destroy)
{
	int flag = CMString::set_case_sensitive(csflag);
   int ret = 0;
	unsigned short n = val.hash() % arraysize;
	for (unsigned short i=0;i<array[n].Count();i++) {
		if (val == array[n].At(i)->GetName()) {
			array[n].DetachAt(i,destroy);
         ret=1;
			break;
		}
	}
	CMString::set_case_sensitive(flag);
   return ret;
}

template<class TP>
CMPHashDictionaryIterator<TP>::CMPHashDictionaryIterator(const CMPHashDictionary<TP>* d) :
dictionary(d),
which(0),
pos(0)
{
}

template<class TP>  void
CMPHashDictionaryIterator<TP>::Reset()
{
	which=pos=0;
}

template<class TP> TP*
CMPHashDictionaryIterator<TP>::operator ()()
{
	while (dictionary && which < dictionary->arraysize) {
		if (pos < dictionary->array[which].Count())
			return dictionary->array[which].At(pos++);
		pos=0;
		which++;
	}
	return 0;
}
