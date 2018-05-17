#include <stdio.h>
#include <stdlib.h>

#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

# define DEFBITS 1024

void testRandBytes()
{
    // #include <openssl/rand.h>
    // int RAND_bytes(unsigned char *buf, int num);
    // int RAND_pseudo_bytes(unsigned char *buf, int num);
    uint8_t iv[AES_BLOCK_SIZE];
    RAND_pseudo_bytes(iv, AES_BLOCK_SIZE);
    hexdump(iv, AES_BLOCK_SIZE, NULL);
    
    RAND_bytes(iv, AES_BLOCK_SIZE);
    hexdump(iv, AES_BLOCK_SIZE, NULL);
}

void testRSA()
{
    int num = DEFBITS;
    unsigned int f4 = RSA_F4;
    unsigned char* pubkey = NULL;
    BIGNUM *bn = BN_new();
    RSA *rsa = RSA_new();
    RSA* rsa2 = NULL;

    do {
        if (!BN_set_word(bn, f4) || !RSA_generate_key_ex(rsa, num, bn, NULL)) {
            printf("failed to generate rsa key pair\n");
            break;
        }

        if (bn) {
            BN_free(bn); bn = NULL;
        }

        // int pubkeyLen = i2d_RSAPublicKey(rsa, &pubkey);
        int pubkeyLen = i2d_RSA_PUBKEY(rsa, &pubkey);

        printf("PublicKeyBuff, Len=%d\n", pubkeyLen);
        hexdump(pubkey, pubkeyLen, NULL);

        char msg[DEFBITS>>3], encrypted[DEFBITS>>3];
        strcpy(msg, "hello, world");

        int encrypt_len;
        if((encrypt_len = RSA_private_encrypt(
            (int)strlen(msg),
            (unsigned char*)msg,
            (unsigned char*)encrypted,
            rsa,
            RSA_PKCS1_PADDING)) == -1) {
            char err[512];
            ERR_load_crypto_strings();
            ERR_error_string(ERR_get_error(), err);
            printf("encrypt error: %s\n", err);
            break;
        }
        printf("encrypted data, Len=%d:\n", encrypt_len);
        hexdump(encrypted, encrypt_len, NULL);

        const unsigned char* pubkey2 = pubkey;
        rsa2 = d2i_RSA_PUBKEY(NULL, &pubkey2, pubkeyLen);
        if (rsa2==NULL) {
            ERR_print_errors_fp(stderr);
        }
        char decrypted[DEFBITS>>3];
        int xxx = 0;
        // char decrypted[13];
        int decrypted_len = 0;
        if ((decrypted_len = RSA_public_decrypt(
            encrypt_len,
            (unsigned char*)encrypted,
            (unsigned char*)decrypted,
            rsa2,
            RSA_PKCS1_PADDING)) == -1) {
            char err[512];
            ERR_load_crypto_strings();
            ERR_error_string(ERR_get_error(), err);
            printf("encrypt error: %s\n", err);
            break;
        }
        decrypted[decrypted_len] = 0;
        printf("decrypted (%d): %s\n", decrypted_len, decrypted);
        printf("%d\n", xxx);

    } while (0);

    if (bn) {
        BN_free(bn); bn = NULL;
    }
    if (rsa) {
        RSA_free(rsa); rsa = NULL;
    }
    if (rsa2) {
        RSA_free(rsa2); rsa2 = NULL;
    }
    if (pubkey) {
        OPENSSL_free(pubkey); pubkey = NULL;
    }
}
