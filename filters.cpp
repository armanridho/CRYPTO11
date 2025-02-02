#include <memory.h>
#include <assert.h>

#include "filters.h"
#include "misc.h"

void Filter::Detach(BufferedTransformation *newOut)
{
    outQueue->Close();
    outQueue->TransferTo(*newOut);
    delete outQueue;
    outQueue=newOut;
}

void Filter::Attach(BufferedTransformation *newOut)
{
    if (outQueue->Attachable())
        outQueue->Attach(newOut);
    else
        Detach(newOut);
}

BlockFilterBase::BlockFilterBase(BlockTransformation &c,
                                 BufferedTransformation *outQ)
    : cipher(c), S(cipher.BlockSize()), inBuf(SecAlloc(byte, S)), Filter(outQ)
{
    inBufSize=0;
}

BlockFilterBase::~BlockFilterBase()
{
    SecFree(inBuf, S);
}

void BlockFilterBase::ProcessBuf()
{
    cipher.ProcessBlock(inBuf);
    outQueue->Put(inBuf, S);
    inBufSize=0;
}

void BlockFilterBase::Put(const byte *inString, unsigned int length)
{
    while (length--)
        BlockFilterBase::Put(*inString++);
}

void BlockEncryptionFilter::InputFinished()
{
    if (inBufSize == S)
        ProcessBuf();
    // pad last block
    memset(inBuf+inBufSize, S-inBufSize, S-inBufSize);
    ProcessBuf();
}

void BlockDecryptionFilter::InputFinished()
{
    cipher.ProcessBlock(inBuf);
    if (inBuf[S-1] > S)
        inBuf[S-1] = 0;     // something's wrong with the padding
    outQueue->Put(inBuf, S-inBuf[S-1]);
    inBufSize=0;
}

void StreamCipherFilter::Put(const byte *inString, unsigned int length)
{
    SecByteBlock temp(length);
    cipher.ProcessString(temp, inString, length);
    outQueue->Put(temp, length);
}

void HashFilter::InputFinished()
{
    SecByteBlock buf(hash.DigestSize());
    hash.Final(buf);
    outQueue->Put(buf, hash.DigestSize());
}

BufferedTransformation *Insert(const byte *in, unsigned int length, BufferedTransformation *outQueue)
{
    outQueue->Put(in, length);
    return outQueue;
}

unsigned int Extract(Source *source, byte *out, unsigned int length)
{
    while (source->MaxRetrieveable() < length && source->Pump(1));
    return source->Get(out, length);
}

