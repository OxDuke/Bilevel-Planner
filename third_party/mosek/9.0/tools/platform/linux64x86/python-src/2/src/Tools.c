#include <string.h>
#include <stdint.h>
#include <math.h>

void mosek_4fusion_4Utils_4Tools_4arraycopy___3II_3III(int32_t * src, size_t src_len, int32_t srcoffset, int32_t * tgt, size_t tgt_len, int32_t tgtoffset, int32_t size);
void mosek_4fusion_4Utils_4Tools_4arraycopy___3JI_3JII(int64_t * src, size_t src_len, int32_t srcoffset, int64_t * tgt, size_t tgt_len, int32_t tgtoffset, int32_t size);
void mosek_4fusion_4Utils_4Tools_4arraycopy___3DI_3DII(double  * src, size_t src_len, int32_t srcoffset, double  * tgt, size_t tgt_len, int32_t tgtoffset, int32_t size);
void mosek_4fusion_4Utils_4Tools_4arraycopy___3IJ_3IJJ(int32_t * src, size_t src_len, int64_t srcoffset, int32_t * tgt, size_t tgt_len, int64_t tgtoffset, int64_t size);
void mosek_4fusion_4Utils_4Tools_4arraycopy___3JJ_3JJJ(int64_t * src, size_t src_len, int64_t srcoffset, int64_t * tgt, size_t tgt_len, int64_t tgtoffset, int64_t size);
void mosek_4fusion_4Utils_4Tools_4arraycopy___3DJ_3DJJ(double  * src, size_t src_len, int64_t srcoffset, double  * tgt, size_t tgt_len, int64_t tgtoffset, int64_t size);
double mosek_4fusion_4Utils_4Tools_4sqrt__D(double v);
/*
// _ => _4
// ; => _2
// [ => _3
// . => _4
// , => _5
*/

double mosek_4fusion_4Utils_4Tools_4sqrt__D(double v)
{
  return sqrt(v);
}

/*
public static method arraycopy void
(   src [int32],
    srcoffset int32,
    tgt [int32],
    tgtoffset int32,
    size int32); */
void mosek_4fusion_4Utils_4Tools_4arraycopy___3II_3III(int32_t * src, size_t src_len, int32_t srcoffset, int32_t * tgt, size_t tgt_len, int32_t tgtoffset, int32_t size)
{
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
}

/*
public static method arraycopy void
(   src [int64],
    srcoffset int32,
    tgt [int64],
    tgtoffset int32,
    size int32); */
void mosek_4fusion_4Utils_4Tools_4arraycopy___3JI_3JII(int64_t * src, size_t src_len, int32_t srcoffset, int64_t * tgt, size_t tgt_len, int32_t tgtoffset, int32_t size)
{
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
}

/*
public static method arraycopy void
(   src [double],
    srcoffset int32,
    tgt [double],
    tgtoffset int32,
    size int32);*/
void mosek_4fusion_4Utils_4Tools_4arraycopy___3DI_3DII(double * src, size_t src_len, int32_t srcoffset, double * tgt , size_t tgt_len, int32_t tgtoffset, int32_t size)
{
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
}

/*
public static method arraycopy void
(   src [int32],
    srcoffset int64,
    tgt [int32],
    tgtoffset int64,
    size int64); */
void mosek_4fusion_4Utils_4Tools_4arraycopy___3IJ_3IJJ(int32_t * src, size_t src_len, int64_t srcoffset, int32_t * tgt, size_t tgt_len, int64_t tgtoffset, int64_t size)
{
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
}

/*
public static method arraycopy void
(   src [int64],
    srcoffset int64,
    tgt [int64],
    tgtoffset int64,
    size int64);*/
void mosek_4fusion_4Utils_4Tools_4arraycopy___3JJ_3JJJ(int64_t * src, size_t src_len, int64_t srcoffset, int64_t * tgt, size_t tgt_len, int64_t tgtoffset, int64_t size)
{
  /*
  int i;
  printf("array_copy(src,%lld,tgt,%lld,%lld)\n",srcoffset,tgtoffset,size);
  printf("src : "); for (i = 0; i < size; ++i) printf(" %lld",src[i+srcoffset]); printf("\n");
  */
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
/*  printf("tgt : "); for (i = 0; i < size; ++i) printf(" %lld",tgt[i+tgtoffset]); printf("\n");*/
}

/*
public static method arraycopy void
(   src [double],
    srcoffset int64,
    tgt [double],
    tgtoffset int64,
    size int64);*/
void mosek_4fusion_4Utils_4Tools_4arraycopy___3DJ_3DJJ(double * src, size_t src_len, int64_t srcoffset, double * tgt , size_t tgt_len, int64_t tgtoffset, int64_t size)
{
  memcpy(tgt+tgtoffset, src+srcoffset, size*sizeof(*src));
}
