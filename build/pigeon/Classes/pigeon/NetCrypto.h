#ifndef __pigeon_netcrypto_h
#define __pigeon_netcrypto_h

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
//#include <openssl/rsa.h>
//#include <openssl/aes.h>
//#include <openssl/x509.h>
//#include <openssl/err.h>

#ifdef  __cplusplus
extern "C" {
#endif
    
# define RSA_F4  0x10001L
    typedef struct rsa_st RSA;
    typedef struct aes_key_st AES_KEY;
    
#ifdef  __cplusplus
}
#endif

#include "netbasic.h"

/******************************************************************************
 *  RSA encryption algorithm
 ******************************************************************************/
class RSACrypto
{
public:
    RSACrypto();
    ~RSACrypto();

    // Generate a new keypair.
    bool generateKeypair(int bits, unsigned int e);

    // Encode public key into byte array formatted as X.509 (public).
    uint8_t* encodePublicKey(size_t* keylen);
    // Decode public key from byte array formatted as X.509 (public).
    bool decodePublicKey(const uint8_t* key, size_t keylen);
    // Encode private key into byte array formatted as PKCS#1 (private).
    uint8_t* encodePrivateKey(size_t* keylen);
    // Decode private key from byte array formatted as PKCS#1 (private).
    bool decodePrivateKey(const uint8_t* key, size_t keylen);
    // Free the byte array of encoded public/private key.
    static void freeKeyBuffer(uint8_t* key);

    // Calculate the length of encrypted data.
    size_t calcEncryptedLength(size_t len) const;

    // Calculate the length of decrypted data.
    // Decrypted length depends on the encrypted content, this method returns the
    // maximum length of any possile encrypted content. The accurate length will be
    // returned by the 2nd argument (`outlen`) by decryption methods.
    size_t calcDecryptedLength(size_t len) const;

    // Encrypt/decrypt data.
    // NOTE These methods will NOT check the length of output buffer. To avoid
    // overflows, the size of the array pointed by `out` shall be long enough.
    bool encryptWithPrivateKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen);
    bool decryptWithPrivateKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen);
    bool encryptWithPublicKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen);
    bool decryptWithPublicKey(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen);

private:
    RSA* _keypair;
};

/******************************************************************************
 *  Abstraction of symmetric crypto algorithms
 ******************************************************************************/
class SymmCrypto
{
public:
    virtual ~SymmCrypto() {}

    virtual void setConfig(const void* config, size_t bytes) = 0;
    virtual size_t calcEncryptedLength(size_t len) const = 0;
    virtual size_t calcDecryptedLength(size_t len) const = 0;
    virtual bool encrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen) = 0;
    virtual bool decrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen) = 0;
};

/******************************************************************************
 *  Factory of symmetric crypto algorithms
 ******************************************************************************/
class SymmCryptoFactory
{
public:
    class CryptoCreator {
    public:
        virtual ~CryptoCreator() {}
        virtual SymmCrypto* create() = 0;
    };
    template<typename CCrypto>
    class Register : public CryptoCreator {
    public:
        Register(int typeValue, const char* name) {
            SymmCryptoFactory::getInstance()->registCrypto(typeValue, name, this);
        }
        SymmCrypto* create() {
            return new CCrypto();
        }
    };

public:
    static SymmCryptoFactory* getInstance();
    SymmCrypto* newCrypto(int cryptoType);
    void registCrypto(int cryptoType, const char* name, CryptoCreator* creator);
    int listCryptos(std::function<void(int, const char*)>);

private:
    SymmCryptoFactory();
    struct crypto_info {int type; std::string name;};
    std::vector<crypto_info> _supportedCryptos;
    std::map<int, CryptoCreator*> _cryptoDictionary;
};

#if !defined(REGISTER_SYMMCRYPTO_IMPL)
#   define REGISTER_SYMMCRYPTO(idsymbol, idvalue, name, clazz) \
    const int idsymbol = (idvalue);
#else
#   define REGISTER_SYMMCRYPTO(idsymbol, idvalue, name, clazz) \
    static SymmCryptoFactory::Register<clazz> __register_##clazz(idvalue, name);
#endif

/******************************************************************************
 *  AES encryption algorithm
 ******************************************************************************/
#define AES_CBC  1
#define AES_ECB  2
#define AES_CTR  3
#define AES_OCF  4
#define AES_CFB  5

#define AESKEY128  1
#define AESKEY192  2
#define AESKEY256  3

struct aescfg_t;

class AESCrypto: public SymmCrypto
{
public:
    AESCrypto();
    ~AESCrypto();
    virtual void setConfig(const void* config, size_t bytes) override;
    virtual size_t calcEncryptedLength(size_t len) const override;
    virtual size_t calcDecryptedLength(size_t len) const override;
    virtual bool encrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen) override;
    virtual bool decrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen) override;
private:
    AES_KEY* _encKey;
    AES_KEY* _decKey;
    struct aescfg_t* _config;
};

REGISTER_SYMMCRYPTO(NETCRYPTO_AES, 1, "aes", AESCrypto)

#endif
