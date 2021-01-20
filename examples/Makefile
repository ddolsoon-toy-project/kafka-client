CXX=g++
LIBS=-Llib/ -lKafkaClient -lrdkafka++ -lrdkafka -ldl -lpthread -lssl -lsasl2 -lcurl -lcrypto -lz -lzstd
INCLUDES=-Iinclude/

all: producer_examples 

SRCS=./producer_examples.cpp

OBJS=$(SRCS:.cpp=.o)

%.o : %.cpp 
	@printf "\n\033[0;40;33m ****** Build $@ ($^) Start    ****** \033[0m\n";
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^
	@printf "\033[0;40;33m ****** Build $@ ($^) Complete ****** \033[0m\n";

producer_examples: producer_examples.o
	@printf "\n\n\n\033[0;40;33m ****** Build $@($^) Start       ****** \033[0m\n";
	$(CXX) -o $@ $^ $(LIBS)
	@printf "\033[0;40;33m ****** Build $@($^) Complete    ****** \033[0m\n";

clean:
	rm -rf producer_examples *.o 
