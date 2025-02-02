#ifndef DES_H
#define DES_H

#include "cryptlib.h"

class DES : public BlockTransformation
{
public:
    DES(const byte *userKey, CipherDir);
    ~DES();

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock)
        {DES::ProcessBlock(inoutBlock, inoutBlock);}

    enum {KEYLENGTH=8, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
    static inline void GETBLOCK(const byte *block, word32 &left, word32 &right);
    static inline void PUTBLOCK(byte *block, word32 left, word32 right);
    static const word32 Spbox[8][64];

    word32 *const k;
};

class DESEncryption : public DES
{
public:
    DESEncryption(const byte * userKey)
	: DES (userKey, ENCRYPTION) {}
};

class DESDecryption : public DES
{
public:
    DESDecryption(const byte * userKey)
	: DES (userKey, DECRYPTION) {}
};

class DES_EDE_Encryption : public BlockTransformation
{
public:
    DES_EDE_Encryption(const byte * userKey)
        : e(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES e;
    DES d;
};

class DES_EDE_Decryption : public BlockTransformation
{
public:
    DES_EDE_Decryption(const byte * userKey)
        : d(userKey, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES d;
    DES e;
};

#endif

