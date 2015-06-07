#include <OpenCLInvertexIndex.h>

/**
 * WordMapInvertedIndex::InvertedIndexMap
 */
const char* WordMapInvertedIndex::InvertedIndexMap::at(cl_uint i) const
{
    return (const char*)data() + word_capacity()*i;
}
const char* WordMapInvertedIndex::InvertedIndexMap::operator[](cl_uint i) const
{
    return (const char*)data() + word_capacity()*i;
}