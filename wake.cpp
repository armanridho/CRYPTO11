#include "wake.h"
#include <assert.h>

inline word32 WAKE::M(word32 x, word32 y)
{
    return ((x+y)>>8) ^ t[byte(x+y)];
}

inline void WAKE::enc(word32 &V)
{
    V = V^r6 ;
    r3 = M(r3, V);
    r4 = M(r4, r3);
    r5 = M(r5, r4);
    r6 = M(r6, r5);
}

inline void WAKE::dec(word32 &V)
{
    r3 = M(r3, V);
    V = V^r6 ;
    r4 = M(r4, r3);
    r5 = M(r5, r4);
    r6 = M(r6, r5);
}

void WAKE::genkey(word32 k0, word32 k1, word32 k2, word32 k3)
{
    long x, z;
    int p ;
    static long tt[10]= {
        0x726a8f3bL,                                 // table
        0xe69a3b5cL,
        0xd3c71fe5L,
        0xab3c73d2L,
        0x4d3a8eb3L,
        0x0396d6e8L,
        0x3d4c2f7aL,
        0x9ee27cf3L, } ;
    t[0] = k0;
    t[1] = k1;
    t[2] = k2;
    t[3] = k3;
    for (p=4 ; p<256 ; p++)
    {
      x=t[p-4]+t[p-1] ;                        // fill t
      t[p]= (x>>3) ^ tt[byte(x&7)] ;
    }

    for (p=0 ; p<23 ; p++)
        t[p]+=t[p+89] ;           // mix first entries
    x=t[33] ; z=t[59] | 0x01000001L ;
    z=z&0xff7fffffL ;
    for (p=0 ; p<256 ; p++) {       //change top byte to
      x=(x&0xff7fffffL)+z ;          // a permutation etc
      t[p]=(t[p] & 0x00ffffffL) ^ x ; }

    t[256]=t[0] ;
    byte y=byte(x);
    for (p=0 ; p<256 ; p++) {     // further change perm.
      t[p]=t[y=byte(t[p^y]^y)] ;  // and other digits
      t[y]=t[p+1] ;  }
}

WAKEEncryption::WAKEEncryption(const byte *key, BufferedTransformation *outQueue)
    : Filter(outQueue), inbuf(INBUFMAX), inbufSize(0)
{
    r3 = ((word32)key[0] << 24) | ((word32)key[1] << 16) | ((word32)key[2] << 8) | (word32)key[3];
    r4 = ((word32)key[4] << 24) | ((word32)key[5] << 16) | ((word32)key[6] << 8) | (word32)key[7];
    r5 = ((word32)key[8] << 24) | ((word32)key[9] << 16) | ((word32)key[10] << 8) | (word32)key[11];
    r6 = ((word32)key[12] << 24) | ((word32)key[13] << 16) | ((word32)key[14] << 8) | (word32)key[15];
    genkey(r3, r4, r5, r6);
}

void WAKEEncryption::ProcessInbuf()
{
    assert((inbufSize % 4) == 0);

#ifdef LITTLE_ENDIAN
    byteReverse((word32 *)inbuf.ptr, (word32 *)inbuf.ptr, inbufSize);
#endif

    word32 *ptr = (word32 *)inbuf.ptr;
    byte *const end = (byte *)inbuf+inbufSize;

    while (ptr!=(word32 *)end)
        enc(*ptr++);

#ifdef LITTLE_ENDIAN
    byteReverse((word32 *)inbuf.ptr, (word32 *)inbuf.ptr, inbufSize);
#endif

    outQueue->Put(inbuf, inbufSize);
    inbufSize=0;
}

void WAKEEncryption::Put(const byte *inString, unsigned int length)
{
    while (length)
    {
        if (inbufSize==INBUFMAX)
            ProcessInbuf();
        unsigned int l = min(length, INBUFMAX-inbufSize);
        memcpy(inbuf+inbufSize, inString, l);
        inString+=l;
        length-=l;
        inbufSize+=l;
    }
}

void WAKEEncryption::InputFinished()
{
    if (inbufSize == INBUFMAX)
        ProcessInbuf();
    // pad to next multiple of 4
    memset(inbuf+inbufSize, 4-(inbufSize%4), 4-(inbufSize%4));
    inbufSize += 4-(inbufSize%4);
    ProcessInbuf();
}

void WAKEDecryption::ProcessInbuf()
{
    assert((inbufSize % 4) == 0);

#ifdef LITTLE_ENDIAN
    byteReverse((word32 *)inbuf.ptr, (word32 *)inbuf.ptr, inbufSize);
#endif

    word32 *ptr = (word32 *)inbuf.ptr;
    byte *const end = (byte *)inbuf+inbufSize;

    while (ptr!=(word32 *)end)
        dec(*ptr++);

#ifdef LITTLE_ENDIAN
    byteReverse((word32 *)inbuf.ptr, (word32 *)inbuf.ptr, inbufSize);
#endif

    if (lastBlock)
    {
        if (inbuf[inbufSize-1] > 4) inbuf[inbufSize-1]=0;
        outQueue->Put(inbuf, inbufSize-inbuf[inbufSize-1]);
    }
    else
        outQueue->Put(inbuf, inbufSize);

    inbufSize=0;
}

void WAKEDecryption::InputFinished()
{
    lastBlock = TRUE;
    ProcessInbuf();
}

