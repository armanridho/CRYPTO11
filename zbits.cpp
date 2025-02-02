#include "zbits.h"
#include <assert.h>

#ifdef DEBUG
ulg bits_sent;   /* bit length of the compressed data */
#endif

// #define putbyte(b) outQ.Put(b)

BitOutput::BitOutput(BufferedTransformation &outQ)
    : outQ(outQ)
{
   bitbuff = 0;
   boffset = 0;
#ifdef DEBUG
   bits_sent = 0L;
#endif
}

void BitOutput::send_bits(unsigned int value, int length) /* Send a value on a given number of bits. */
{
#ifdef DEBUG
   Tracevv((stderr," l %2d v %4x ", length, value));
   Assert(length > 0 && length <= 15, "invalid length");
   Assert(boffset < 8, "bad offset");
   bits_sent += (ulg)length;
#endif
   bitbuff |= value << boffset;
   if ((boffset += length) >= 8) {
      outQ.Put(bitbuff);
      value >>= length - (boffset -= 8);
      if (boffset >= 8) {
         boffset -= 8;
         outQ.Put(value);
         value >>= 8;
      }
      bitbuff = value;
   }
}

/* Write out any remaining bits in an incomplete byte. */
void BitOutput::bi_windup()
{
   assert(boffset < 8);
   if (boffset) {
      outQ.Put(bitbuff);
      boffset = 0;
      bitbuff = 0;
#ifdef DEBUG
      bits_sent = (bits_sent+7) & ~7;
#endif
   }
}

void BitOutput::bi_putsh(word16 x)
{
    outQ.Put((byte)x);
    outQ.Put(byte(x>>8));
}

/* Copy a stored block to the zip file, storing first the length and its
   one's complement if requested. */
void BitOutput::copy_block(byte *buf, unsigned int len, int header)
{
   /* align on byte boundary */
   bi_windup();

   if (header) {
        bi_putsh(len);
        bi_putsh(~len);
#ifdef DEBUG
      bits_sent += 2*16;
#endif
   }
   outQ.Put(buf, len);
#ifdef DEBUG
   bits_sent += (ulg)len<<3;
#endif
}

