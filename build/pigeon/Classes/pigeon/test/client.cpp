#include <stdio.h>
#include <stdlib.h>

#include "SessionActor.h"
#include "SessionSelector.h"
#include "Serializer.h"
#include "Deserializer.h"
#include "NetCrypto.h"
#include "RPCTransport.h"
#include "NetController.h"

# define DEFBITS 1024

void testSerializer()
{
    const char* msg = "hello, world";

    Serializer serializer(128);
    serializer.writeInt8(-1);
    serializer.writeInt16(1);
    serializer.writeInt32(0x01020304);
    serializer.writeUint8(0xff);
    serializer.writeBytes(msg, strlen(msg));
    printf("serialized data, length=%ld\n", serializer.length());
    hexdump(serializer.buffer(), serializer.length(), NULL);

    int error = 0;
    int8_t value1 = 0;
    int16_t value2 = 0;
    int32_t value3 = 0;
    uint8_t value4 = 0;
    // int8_t value4 = 0;
    Deserializer deserializer(serializer.buffer(), serializer.length());
    // Deserializer deserializer(serializer.buffer(), 4);
    deserializer.readInt8(&value1);
    error = deserializer.readInt16(&value2);
    if (error) { printf("error %d\n", error); }
    error = deserializer.readInt32(&value3);
    if (error) { printf("error %d\n", error); }
    error = deserializer.readUint8(&value4);
    // error = deserializer.readInt8(&value4);
    if (error) { printf("error %d\n", error); }
    printf("values: %d, %d, 0x%08x, %u\n", value1, value2, value3, value4);
    // char msg2[100] = {0};
    // deserializer.readBytes(msg2, deserializer.available());
    // printf("msg: %s\n", msg2);
    std::string hello;
    deserializer.readString(hello, deserializer.available());
    printf("msg: %s\n", hello.c_str());
}

void testCryptoRSA()
{
    RSACrypto rsa1;
    rsa1.generateKeypair(DEFBITS, RSA_F4);

    size_t publen = 0;
    uint8_t* pubkey = rsa1.encodePublicKey(&publen);
    printf("PublicKeyBuff, Len=%ld\n", publen);
    hexdump(pubkey, publen, NULL);

    size_t prilen = 0;
    uint8_t* prikey = rsa1.encodePrivateKey(&prilen);
    printf("PrivateKeyBuff, Len=%ld\n", publen);
    hexdump(prikey, prilen, NULL);

    char msg[DEFBITS>>3], encrypted[DEFBITS>>3];
    strcpy(msg, "hello, world");
    size_t encrypt_len = 0;
    rsa1.encryptWithPrivateKey(
        (unsigned char*)encrypted,
        &encrypt_len,
        (unsigned char*)msg,
        strlen(msg));
    printf("encrypted data, Len=%ld:\n", encrypt_len);
    hexdump(encrypted, encrypt_len, NULL);

    RSACrypto rsa2;
    // rsa2.decodePublicKey(pubkey, publen);
    rsa2.decodePrivateKey(prikey, prilen);

    char decrypted[DEFBITS>>3];
    memset(decrypted, -1, sizeof(decrypted));
    size_t decrypted_len = 0;
    if (!rsa2.decryptWithPublicKey(
        //!rsa2.decryptWithPrivateKey(
        (unsigned char*)decrypted,
        &decrypted_len,
        (unsigned char*)encrypted,
        encrypt_len)) {
        printf("decrypt failed\n");
    } else {
        printf("decrypted data (%ld):\n", sizeof(decrypted));
        hexdump(decrypted, sizeof(decrypted), NULL);
        decrypted[decrypted_len] = 0;
        printf("decrypted (%ld): %s\n", decrypted_len, decrypted);
    }

    rsa1.freeKeyBuffer(pubkey);
    rsa1.freeKeyBuffer(prikey);
}

void testCryptoAES()
{
    SymmCrypto* crypto = SymmCryptoFactory::getInstance()->newCrypto(NETCRYPTO_AES);
    if (crypto) {
        // struct {
        //     aescfg_cbc_t _;
        //     uint8_t keybuffer[128/8];
        // } cfg;

        // cfg._.mode = AES_CBC;
        // cfg._.keytype = AESKEY128;
        // RAND_bytes(cfg._.iv, AES_BLOCK_SIZE);
        // RAND_bytes(cfg.keybuffer, sizeof(cfg.keybuffer));

        uint8_t cfg[] = {
            0x01, 0x01, 0x00, 0x00, 0xae, 0x54, 0xe8, 0xdb,
            0x05, 0xea, 0x87, 0x3e, 0x19, 0xf6, 0xec, 0x8b,
            0x05, 0x47, 0x9f, 0x1f, 0x04, 0x9b, 0x65, 0x8a,
            0x8d, 0x93, 0xf2, 0xe5, 0x59, 0x98, 0x7b, 0x42,
            0x99, 0xaf, 0xff, 0x32
        };

        // uint8_t cfg[] = {
        //     0x01, 0x01, 0x00, 0x00, '1', '2', '3', '4',
        //     '5', '6', '7', '8', '9', '0', '1', '2',
        //     '3', '4', '5', '6', '1', '2', '3', '4',
        //     '5', '6', '7', '8', '9', '0', '1', '2',
        //     '3', '4', '5', '6'
        // };

        crypto->setConfig(cfg, sizeof(cfg));

        // unsigned char enc_out[AES_BLOCK_SIZE];
        const size_t inlen = 16;
        unsigned char* aes_input = new unsigned char[inlen];
        memset(aes_input, 'X', inlen);

        printf("aes_input (%lu):\n", inlen);
        hexdump(aes_input, inlen, NULL);

        const size_t enclen = crypto->calcEncryptedLength(inlen);
        unsigned char* enc_out = new unsigned char[enclen];
        crypto->encrypt(enc_out, NULL, aes_input, inlen);

        printf("enc_out (%lu):\n", enclen);
        hexdump(enc_out, enclen, NULL);

        const size_t declen = crypto->calcDecryptedLength(enclen);
        unsigned char* dec_out = new unsigned char[declen];
        crypto->decrypt(dec_out, NULL, enc_out, enclen);

        printf("dec_out (%lu):\n", declen);
        hexdump(dec_out, declen, NULL);

        delete[] aes_input;
        delete[] enc_out;
        delete[] dec_out;
        delete crypto;
    }
}

void testClientInMainThread()
{
    const char* msgs[] = {
        "{\"rid\":\"1_1\", \"opCode\": 90001}",
        "{\"rid\":\"1_2\", \"opCode\": 90002}",
        "{\"rid\":\"1_3\", \"opCode\": 90003}",
    };
    // int msgcnt = sizeof(msgs)/sizeof(msgs[0]);

    SessionSelector selector;

    SessionClient client;
    client.connect("172.16.149.78", 1234, 3000);
    client.setHeartbeatInterval(10*1000);
    client.setHeartbeatTimeout(30*1000);

    selector.addSessionActor(&client);

    RPCTransport rpc;
    rpc.attachSession(&client);
    // rpc.setListener(NULL);

    long interval = 5*1000;
    long step = 1000*1000/interval;

    int cnt = 0;
    printf("in looping...\n");

    while (true) {
        cnt++;
        if (cnt % (step/10) == 0) {
            printf("."); fflush(stdout);
        }

        struct timeval begin;
        gettimeofday(&begin, NULL);

        rpc.flush();
        selector.select();
        rpc.receive();

        struct timeval now;
        gettimeofday(&now, NULL);
        long elapsed = (long)difftimeval(&now, &begin);
        if (elapsed<interval) {
            usleep((int)(interval-elapsed));
        }

        if (cnt % (5*step) == 0) {
            int idx = 0;
            rpc.request(
                90001,
                RPCTransport::kComprSnappy,
                msgs[idx],
                strlen(msgs[idx]),
                [](int req, int err, const std::string& resp) -> void {
                    printf("responded: %s\n", resp.c_str());
                });
        }

        // if (cnt % (5*step) == 0) {
        //     printf("<<< %s\n", msgs[idx]);
        //     gettimeofday(&sendtime, NULL);
        //     client.send(msgs[idx], strlen(msgs[idx]));
        //     idx++;
        //     if (idx>=msgcnt) {
        //         idx = 0;
        //     }
        // }

        // while (client.available() > 0) {
        //     size_t bytes = 0;
        //     void* buffer = client.receive(&bytes);
        //     if (buffer==NULL) {
        //         break;
        //     }
        //     std::string str((char*)buffer, bytes);
        //     client.freeReceived(buffer);

        //     long resptime = (long)difftimeval(&now, &sendtime);
        //     printf(">>> (%ldms) %s\n", resptime, str.c_str());
        // }

        // if (cnt > step*20) {
        //     client.close();
        // }

        // if (client.isClosed()) {
        //     break;
        // }

        // if (++cnt > (15*step)) {
        //     cnt = 0;
        //     printf("reconnect\n");
        //     client.reconnect();
        // }
    }
}

void testClientInWorkThread()
{
    const char* msgs[] = {
        "\0\3" "1_1{\"rid\":\"1_1\", \"opCode\": 90001}",
        "\0\3" "1_2{\"rid\":\"1_2\", \"opCode\": 90002}",
        "\0\3" "1_3{\"rid\":\"1_3\", \"opCode\": 90003}",
    };
//    int msgcnt = sizeof(msgs)/sizeof(msgs[0]);

    NetController net;
    RPCClient* client = net.createClient();
    client->setHeartbeatInterval(10*1000);
    client->setHeartbeatTimeout(30*1000);
    
    bool connected = client->connect("0::ffff:172.16.30.162", 8888, 3000);
    if (!connected) {
        printf("failed to connect to the server\n");
    } else {
        printf("connected...\n");
    }

    int interval = 5*1000;
    int step = 1000*1000/interval;
    int cnt = 0;
    while (connected) {
        cnt++;
        if (cnt % (step/10) == 0) {
            printf("."); fflush(stdout);
        }

        struct timeval begin;
        gettimeofday(&begin, NULL);

        if (cnt % (5*step) == 0) {
            int idx = 0;
            printf("request 90001\n");
            hexdump(msgs[idx], 1+strlen(msgs[idx]+1), NULL);
            int result = client->request(
                90001,
                RPCTransport::kComprSnappy,
                msgs[idx],
                1+strlen(msgs[idx]+1),
                [](int req, int err, const std::string& resp) -> void {
                    printf("responded: %s\n", resp.c_str());
                });
            if (result<0) {
                printf("request error\n");
            } else {
                printf("request number: %d\n", result);
            }
        }
        
//        if (cnt%step==0) {
//            printf("[STAT] %ld/%ld, %ld, %ld\n", client->getAvgReqDelay(),
//                   client->getMaxReqDelay(),
//                   client->remoteTimestamp(),
//                   client->remoteTimeMillis());
//        }

        struct timeval now;
        gettimeofday(&now, NULL);
        int elapsed = (int)difftimeval(&now, &begin);
        if (elapsed<interval) {
            usleep(interval-elapsed);
        }
        
//        if (cnt > step*20) {
//            client->close();
//            break;
//        }
    }

    net.destroyActivity(client);
    printf("All done!\n");
}

const char* afstr(int family) {
    static char familystr[32];
#define AFCASE(af) case af: strcpy(familystr, #af); break;
    switch (family) {
            AFCASE(AF_INET);
            AFCASE(AF_INET6);
        default:
            familystr[sprintf(familystr, "%d", family)] = 0;
            break;
    }
    return familystr;
}

void testGetaddrinfo()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    const char* node = "0::ffff:172.16.30.165";
    const char* service = "1234";
//    char svrname[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_DEFAULT;
    // hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(node, service, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    int sfd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            // fprintf(stderr, "create socket failed\n");
            continue;
        }
        
        printf("family: %s\n"
               "socktype: %d\n"
               "addr: %s\n"
               "\n"
               , afstr(rp->ai_family)
               , rp->ai_socktype
               , ""// inet_ntop(rp->ai_family, rp->ai_addr, svrname, INET6_ADDRSTRLEN)
               );
        
        s = ::connect(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s<0) {
            int err = errno;
            fprintf(stderr, "connect failed (%d)\n", err);
        }
        
        ::close(sfd);
    }
    
    freeaddrinfo(result);
}

int main(int argc, char** argv)
{
    printf("client is running...\n");

    // testGetaddrinfo();
    // testSerializer();

    // testRSA();

    // testCryptoRSA();
    // testCryptoAES();

    // testClientInMainThread();
     testClientInWorkThread();

    return 0;
}
