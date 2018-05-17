#include "NetCrypto.h"
#include <openssl/aes.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#define AES_KEY_COMMON_FIELDS \
uint8_t mode;    /* CBC/ECB/CTR/OCF/CFB */ \
uint8_t keytype; /* 128bits/192bits/256bits */ \
uint16_t reserved;

struct aescfg_t {
    AES_KEY_COMMON_FIELDS
};

struct aescfg_cbc_t {
    AES_KEY_COMMON_FIELDS
    uint8_t iv[AES_BLOCK_SIZE];
    uint8_t key[0];
};


static size_t calcKeybits(uint8_t keytype) {
    if (keytype==AESKEY128) {
        return 128;
    } else if (keytype==AESKEY192) {
        return 192;
    } else if (keytype==AESKEY256) {
        return 256;
    } else {
        return 0;
    }
}

AESCrypto::AESCrypto()
{
    _config = NULL;
    _encKey = (AES_KEY*)::malloc(sizeof(AES_KEY));
    _decKey = (AES_KEY*)::malloc(sizeof(AES_KEY));
}

AESCrypto::~AESCrypto()
{
    if (_encKey) {
        ::free(_encKey);
        _encKey = NULL;
    }
    if (_decKey) {
        ::free(_decKey);
        _decKey = NULL;
    }
    if (_config) {
        ::free(_config);
        _config = NULL;
    }
}

void AESCrypto::setConfig(const void* config, size_t bytes)
{
#if DEBUG_PIGEON
    NLOG(NL_INFO, "[AES] set user config(%lu):\n", bytes);
    hexdump(config, bytes, NULL);
#endif

    if (_config) {
        ::free(_config); _config = NULL;
    }

    aescfg_t cfg;
    memcpy(&cfg, config, sizeof(aescfg_t));

    size_t keybits = calcKeybits(cfg.keytype);
    if (keybits==0) {
        NLOG(NL_ERROR, "[AES] Unkown keytype (%d).\n", cfg.keytype);
        return;
    }

    if (cfg.mode==AES_CBC) {
        size_t keybytes = (keybits>>3);
        size_t cfgBytes = sizeof(aescfg_cbc_t) + keybytes;
        if (bytes!=cfgBytes) {
            NLOG(NL_ERROR, "[AES] Invalid AES(CBC) configuration structure.\n");
            return;
        }
        aescfg_cbc_t* cbccfg = (aescfg_cbc_t*)::malloc(cfgBytes);
        memcpy(cbccfg, config, bytes);
        _config = (aescfg_t*)cbccfg;
        AES_set_encrypt_key(cbccfg->key, (int)keybits, _encKey);
        AES_set_decrypt_key(cbccfg->key, (int)keybits, _decKey);
    } else if (cfg.mode==AES_ECB) {
        NLOG(NL_ERROR, "[AES] AES(ECB) is NOT supported currently.\n");
    } else if (cfg.mode==AES_CTR) {
        NLOG(NL_ERROR, "[AES] AES(CTR) is NOT supported currently.\n");
    } else if (cfg.mode==AES_OCF) {
        NLOG(NL_ERROR, "[AES] AES(OCF) is NOT supported currently.\n");
    } else if (cfg.mode==AES_CFB) {
        NLOG(NL_ERROR, "[AES] AES(CFB) is NOT supported currently.\n");
    } else {
        NLOG(NL_ERROR, "[AES] Unkown AES encryption mode: %d\n", cfg.mode);
    }
}

size_t AESCrypto::calcEncryptedLength(size_t len) const
{
    return ((len + AES_BLOCK_SIZE-1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
}

size_t AESCrypto::calcDecryptedLength(size_t len) const
{
    return (len / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
}

bool AESCrypto::encrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (!_config) {
        return false;
    }
    bool success = false;
    if (_config->mode==AES_CBC) {
        uint8_t iv[AES_BLOCK_SIZE];
        memcpy(iv, ((aescfg_cbc_t*)_config)->iv, sizeof(iv));
        size_t taillen = inlen % AES_BLOCK_SIZE;
        if (taillen == 0) {
            AES_cbc_encrypt(in, out, inlen, _encKey, iv, AES_ENCRYPT);
        } else {
            uint8_t tail[AES_BLOCK_SIZE];
            size_t nontaillen = inlen-taillen;
            AES_cbc_encrypt(in, out, nontaillen, _encKey, iv, AES_ENCRYPT);
            memset(tail, 0, AES_BLOCK_SIZE);
            memcpy(tail, in+nontaillen, taillen);
            AES_cbc_encrypt(tail, out+nontaillen, AES_BLOCK_SIZE, _encKey, iv, AES_ENCRYPT);
        }
        success = true;
    } else {
        NLOG(NL_ERROR, "[AES] Unsupported AES encryption mode: %d\n", _config->mode);
        success = false;
    }
    if (outlen) {
        *outlen = calcEncryptedLength(inlen);
    }
    return success;
}

bool AESCrypto::decrypt(uint8_t* out, size_t* outlen, const uint8_t* in, size_t inlen)
{
    if (!_config) {
        return false;
    }
    bool success = false;
    if (_config->mode==AES_CBC) {
        uint8_t iv[AES_BLOCK_SIZE];
        memcpy(iv, ((aescfg_cbc_t*)_config)->iv, sizeof(iv));
        AES_cbc_encrypt(in, out, inlen, _decKey, iv, AES_DECRYPT);
        success = true;
    } else {
        NLOG(NL_ERROR, "[AES] Unsupported AES encryption mode: %d\n", _config->mode);
        success = false;
    }
    if (outlen) {
        // AES can't calculate the exact length original data.
        *outlen = 0;
    }
    return success;
}
