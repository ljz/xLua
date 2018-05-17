CC = g++
CFLAGS = -std=c++0x
INCLUDES = -I./ -I../external/openssl/include -I../external/snappy/include
LIBPATHS = -L../external/openssl/lib -L../external/snappy/lib/Mac
LDFLAGS = ${LIBPATHS} -lcrypto -lsnappy

CFLAGS += -DDEBUG=1

PROGS = client server

COMM_HEADERS = netbasic.h SessionSelector.h SessionActor.h \
			Serializer.h Deserializer.h NetCrypto.h \
			RPCTransport.h TimeCorrector.h \
			NetActivity.h NetController.h
COMM_SRCS = SessionSelector.cpp SessionClient.cpp SessionServer.cpp SessionListener.cpp \
			Serializer.cpp Deserializer.cpp MessageProtocol.cpp \
			utils.cpp RSACrypto.cpp SymmCryptoFactory.cpp AESCrypto.cpp \
			RPCTransport.cpp TimeCorrector.cpp \
			NetActivity.cpp NetController.cpp

COMM_OBJS = $(COMM_SRCS:.cpp=.o)

CLIENT_SRCS = test/client.cpp
SERVER_SRCS = test/server.cpp

.PHONY: clean

all: ${PROGS}

# this is a suffix replacement rule for building .o's from .cpp's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .cpp file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

client: test/client.o ${COMM_OBJS}
	${CC} ${LDFLAGS} -o $@ test/client.o ${COMM_OBJS}

server: test/server.o ${COMM_OBJS}
	${CC} ${LDFLAGS} -o $@ test/server.o ${COMM_OBJS}

clean:
	rm -f ${PROGS} *.o *.d

# generate dependencies automatically
SRCS = ${COMM_SRCS} ${CLIENT_SRCS} ${SERVER_SRCS}
dependency: ${SRCS} ${COMM_HEADERS}
	${CC} -MM $(CFLAGS) $(INCLUDES) ${SRCS} > $@
include dependency

# dependency: ${SRCS} ${COMM_HEADERS}
# 	makedepend $(INCLUDES) $^

# %.d: %.cpp
# 	${CC} -MM $(CFLAGS) $(INCLUDES) $< > $@
# include $(COMM_SRCS:.cpp=.d)
# include $(CLIENT_SRCS:.cpp=.d)
# include $(SERVER_SRCS:.cpp=.d)
