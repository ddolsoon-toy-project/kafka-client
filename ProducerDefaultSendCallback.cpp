#include "ProducerDefaultSendCallback.h"
#include "kafka/rdkafkacpp.h"

using namespace RdKafka;

static const char* const HEADERS_MESSAGE_ID = "x-message-id";

void ProducerDefaultSendCallback::success(RdKafka::Message& message)
{
	std::vector<RdKafka::Headers::Header> vHeaders =  message.headers()->get(HEADERS_MESSAGE_ID);
	if ( vHeaders.size() != 0 ) 
	{
		fprintf(stderr, "===========================[ Kafka Produce Success ]=========================== \n");
		fprintf(stderr, "message id : %s\n", (const char*)vHeaders[0].value());

		if ( message.key() ) 
		{
			fprintf(stderr, "message key : %s\n", (const char*)(message.key()->c_str()));
		}
	}
}

void ProducerDefaultSendCallback::fail(RdKafka::Message& message)
{
	std::vector<RdKafka::Headers::Header> vHeaders =  message.headers()->get(HEADERS_MESSAGE_ID);
	if ( vHeaders.size() != 0 ) 
	{
		fprintf(stderr, "===========================[ Kafka Produce Fail ]=========================== \n");
		fprintf(stderr, "message id : %s\n", (const char*)vHeaders[0].value());
	}
}
