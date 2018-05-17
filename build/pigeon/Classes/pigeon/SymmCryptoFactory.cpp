#define REGISTER_SYMMCRYPTO_IMPL 1
#include "NetCrypto.h"

SymmCryptoFactory::SymmCryptoFactory()
{

}

SymmCryptoFactory* SymmCryptoFactory::getInstance()
{
    static SymmCryptoFactory __instance;
    return &__instance;
}

SymmCrypto* SymmCryptoFactory::newCrypto(int cryptoType)
{
    std::map<int, CryptoCreator*>::iterator it = _cryptoDictionary.find(cryptoType);
    if (it!=_cryptoDictionary.end()) {
        return it->second->create();
    }
    return NULL;
}

void SymmCryptoFactory::registCrypto(int cryptoType, const char* name, CryptoCreator* creator)
{
    // NLOG(NL_INFO, "Register SYMMCRYPTO: %d, %s\n", cryptoType, name);
    if (creator==NULL) {
        return;
    }
    if (_cryptoDictionary.find(cryptoType)!=_cryptoDictionary.end()) {
        // reduplicative crypto types
        return;
    }
    _cryptoDictionary[cryptoType] = creator;
    crypto_info info;
    info.type = cryptoType;
    info.name = name;
    _supportedCryptos.insert(_supportedCryptos.end(), info);
}

int SymmCryptoFactory::listCryptos(std::function<void(int, const char*)> visitor)
{
    int count = 0;
    std::vector<crypto_info>::iterator it = _supportedCryptos.begin();
    for (; it!=_supportedCryptos.end(); ++it) {
        count++;
        visitor(it->type, it->name.c_str());
    }
    return count;
}
