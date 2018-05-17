#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "SessionActor.h"
#include "SessionSelector.h"
#include "Serializer.h"
#include "Deserializer.h"
#include "NetCrypto.h"

//#include <snappy.h>
//void testSnappy()
//{
//    std::string original = "hello, world";
//    std::string compressed;
//    snappy::Compress(original.c_str(), original.length(), &compressed);
//    hexdump(compressed.c_str(), compressed.length(), NULL);
//    std::string uncompressed;
//    snappy::Uncompress(compressed.c_str(), compressed.length(), &uncompressed);
//    printf("uncompressed: %s\n", uncompressed.c_str());
//}

int main(int argc, char** argv)
{
    printf("server is running...\n");
    return 0;
}
