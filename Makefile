CXX=g++
EXCLUDE_FILE_NAME=jsoncpp
CXXFLAGS=-fPIC -Wall $(DEFINES)
LIBS=-lrdkafka++ -lrdkafka -ldl -lpthread -lssl -lsasl2 -lrt -lcurl -lzstd -Lcommon/lib
INCLUDES=-Icommon/include/

LDSO=$(CXX) -shared -rdynamic -fPIC

SRCS=./KafkaMessage.cpp \
	 ./KafkaProducer.cpp \
	 ./ProducerCallback.cpp \
	 ./ProducerDefaultSendCallback.cpp \
	 ./ProducerEventCallback.cpp

ifeq  ($(unit), on)
CXXFLAGS += -fprofile-arcs -ftest-coverage -fno-exceptions
LDSO += -fprofile-arcs -ftest-coverage -fno-exceptions
endif

ifeq ($(debug), on)
CXXFLAGS += -g
endif

OBJS=$(SRCS:.cpp=.o)

TARGET=libKafkaClient.so libKafkaClient.a

all: $(TARGET)
	@cp libKafkaClient.a examples/lib/
	@cp libKafkaClient.so examples/lib/

%.o : %.cpp
	@printf "\n\033[0;40;33m ****** Build $@ ($<) Start    ****** \033[0m\n";
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<
	@printf "\033[0;40;33m ****** Build $@ ($<) Complete ****** \033[0m\n";

libKafkaClient.so: $(OBJS) 
	@printf "\n\n\n\033[0;40;33m ****** Build $@ Start       ****** \033[0m\n";
	$(LDSO) $(OBJS) $(LIBS) -o $@
	@printf "\033[0;40;33m ****** Build $@ Complete    ****** \033[0m\n";
	@cp KafkaMessage.h examples/include
	@cp KafkaProducer.h examples/include
	@cp ProducerCallback.h examples/include
	@cp ProducerEventCallback.h examples/include
	@cp ProducerDefaultSendCallback.h examples/include
	@cp ProducerISendCallback.h examples/include
	@cp common/include/kafka/rdkafkacpp.h examples/include/kafka

libKafkaClient.a: $(OBJS)
	ar -x common/lib/librdkafka++.a
	ar -x common/lib/librdkafka.a
	ar srcv libKafkaClient.a *.o

clean:
	@printf "\033[0;40;33m ****** Make Clean Start    ****** \033[0m\n";
	rm -f $(TARGET) $(OBJS) *.o *.gcno *.gcda
	@printf "\033[0;40;33m ****** Make Clean Complete ****** \033[0m\n";

install:
	@printf "\n\n\n\033[0;40;33m ****** Build Install $@ Start       ****** \033[0m\n";
	@mkdir -p builds
	@mkdir -p builds/lib/
	@mkdir -p builds/include/
	@cp libKafkaClient.a builds/lib/
	@cp libKafkaClient.so builds/lib/
	@printf "\033[0;40;33m ****** Build Install $@ Complete    ****** \033[0m\n";
