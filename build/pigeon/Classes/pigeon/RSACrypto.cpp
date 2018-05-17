#include "NetCrypto.h"
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#if DEBUG_PIGEON
#define LOG_CRYPTO_ERR() {\
    char err[512];\
    ERR_load_crypto_strings();\
    ERR_error_string(ERR_get_error(), err);\
    NLOG(NL_ERROR, "[crypto error] %s\n", err);\
}
#else
#define LOG_CRYPTO_ERR()
#endif

#define FREE_KEYPAIR(keypair) if (keypair) { RSA_free(keypair); keypair = NULL; }

RSACrypto::RSACrypto()
{
    _keypair = NULL;
}

RSACrypto::~RSACrypto()
{
    FREE_KEYPAIR(_keypair)
}

bool RSACrypto::generateKeypair(int bits, unsigned int e)
{
    FREE_KEYPAIR(_keypair)

    RSA* rsa = RSA_new();
    BIGNUM *bn = BN_new();
    if (BN_set_word(bn, e) && RSA_generate_key_ex(rsa, bits, bn, NULL)) {
        _keypair = rsa;
    }
    if (bn) {
        BN_free(bn);
    }
    if (rsa && _keypair==NULL) {
        RSA_free(rsa);
    }

    return true;
}

// Encode/Decode public/private key.
// @see http://linux.die.net/man/3/i2d_rsapublickey

uint8_t* RSACrypto::encodePublicKey(size_t* keylen)
{
    if (_keypair==NULL) {
        return NULL;
    }
    uint8_t* pubkey = NULL;
    int len = i2d_RSA_PUBKEY(_keypair, &pubkey);
    if (keylen!=NULL && pubkey!=NULL) {
        *keylen = len;
    }
    return pubkey;
}

bool RSACrypto::decodePublicKey(const uint8_t* key, size_t keylen)
{
    FREE_KEYPAIR(_keypair)
    if (key==NULL) { return false; }
    RSA* rsa = d2i_RSA_PUBKEY(NULL, &key, keylen);
    if (rsa==NULL) {
        return false;
    }
    _keypair = rsa;
    return true;
}

uint8_t* RSACrypto::encodePrivateKey(size_t* keylen)
{
    if (_keypair==NULL) {
        return NULL;
    }
    uint8_t* pubkey = NULL;
    int len = i2d_RSAPrivateKey(_keypair, &pubkey);
    if (keylen!=NULL && pubkey!=NULL) {
        *keylen = len;
    }
    return pubkey;
}

bool RSACrypto::decodePrivateKey(const uint8_t* key, size_t keylen)
{
    FREE_KEYPAIR(_keypair)
    if (key==NULL) { return false; }
    RSA* rsa = d2i_RSAPrivateKey(NULL, &key, keylen);
    if (rsa==NULL) {
        return false;
    }
    _keypair = rsa;
    return true;
}

void RSACrypto::freeKeyBuffer(uint8_t* key)
{
    if (key) {
        OPENSSL_free(key);
    }
}

size_t RSACrypto::calcEncryptedLength(size_t len) const
{
    if (_keypair==NULL) {
        return 0;
    }
    int modulus = RSA_size(_keypair);
    int blockSize = modulus-11; // RSA_PKCS1_PADDING
    return ((len+blockSize-1)/blockSize)*modulus;
}

size_t RSACrypto::calcDecryptedLength(size_t len) const
{
    if (_keypair==NULL) {
        return 0;
    }
    int modulus = RSA_size(_keypair);
    int blockSize = modulus-11; // RSA_PKCS1_PADDING
    return (len/modulus)*blockSize;
}

bool RSACrypto::encryptWithPrivateKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (_keypair==NULL || out==NULL || in==NULL) {
        return false;
    }
    if (1!=RSA_check_key(_keypair)) {
        // `_keypair` doesn't contain private key
        return false;
    }
    int totalLength = 0;
    uint8_t* pdest = out;
    const uint8_t* psrc = in;
    size_t remain = inlen;
    int modulus = RSA_size(_keypair);
    int blockSize = modulus-11;
    while (remain>0) {
        int len0 = remain>blockSize ? blockSize : (int)remain;
        remain -= len0;
        int len = RSA_private_encrypt(len0, psrc, pdest, _keypair, RSA_PKCS1_PADDING);
        if (len<0) {
            LOG_CRYPTO_ERR()
            return false;
        }
        psrc += len0;
        pdest += len;
        totalLength += len;
    }
    if (outlen) { *outlen = totalLength; }
    return true;
}

bool RSACrypto::decryptWithPrivateKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (_keypair==NULL || out==NULL || in==NULL) {
        return false;
    }
    if (1!=RSA_check_key(_keypair)) {
        // `_keypair` doesn't contain private key
        return false;
    }
    int totalLength = 0;
    uint8_t* pdest = out;
    const uint8_t* psrc = in;
    size_t remain = inlen;
    int modulus = RSA_size(_keypair);
    while (remain>=modulus) {
        remain -= modulus;
        int len = RSA_private_decrypt(modulus, psrc, pdest, _keypair, RSA_PKCS1_PADDING);
        if (len<0) {
            LOG_CRYPTO_ERR()
            return false;
        }
        psrc += modulus;
        pdest += len;
        totalLength += len;
    }
    if (outlen) { *outlen = totalLength; }
    return true;
}

bool RSACrypto::encryptWithPublicKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (_keypair==NULL || out==NULL || in==NULL) {
        return false;
    }
    int totalLength = 0;
    uint8_t* pdest = out;
    const uint8_t* psrc = in;
    size_t remain = inlen;
    int modulus = RSA_size(_keypair);
    int blockSize = modulus-11;
    while (remain>0) {
        int len0 = remain>blockSize ? blockSize : (int)remain;
        remain -= len0;
        int len = RSA_public_encrypt(len0, psrc, pdest, _keypair, RSA_PKCS1_PADDING);
        if (len<0) {
            LOG_CRYPTO_ERR()
            return false;
        }
        psrc += len0;
        pdest += len;
        totalLength += len;
    }
    if (outlen) { *outlen = totalLength; }
    return true;
}

bool RSACrypto::decryptWithPublicKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (_keypair==NULL || out==NULL || in==NULL) {
        return false;
    }
    int totalLength = 0;
    uint8_t* pdest = out;
    const uint8_t* psrc = in;
    size_t remain = inlen;
    int modulus = RSA_size(_keypair);
    while (remain>=modulus) {
        remain -= modulus;
        int len = RSA_public_decrypt(modulus, psrc, pdest, _keypair, RSA_PKCS1_PADDING);
        if (len<0) {
            LOG_CRYPTO_ERR()
            return false;
        }
        psrc += modulus;
        pdest += len;
        totalLength += len;
    }
    if (outlen) { *outlen = totalLength; }
    return true;
}
