#include <iostream>
#include <vector>
#include <map>
#include "KafkaProducer.h"
#include "KafkaConsumer.h"

int main (int argc, char * argv[])
{
	KafkaProducer kafkaProducer;
	std::map<std::string, std::string> configMap;
	configMap.insert(std::pair<std::string, std::string>("enable.idempotence", "true"));
	configMap.insert(std::pair<std::string, std::string>("acks", "all"));
	configMap.insert(std::pair<std::string, std::string>("max.in.flight.requests.per.connection", "1"));
	configMap.insert(std::pair<std::string, std::string>("compression.type", "lz4"));

	///////////////////////////////////////////////////
	// 1. Kafka Producer 초기화
	///////////////////////////////////////////////////
	if ( false == kafkaProducer.initKafka(configMap, "172.31.14.25:9091", "test-agent", NULL, 1000) )
	{
		printf("\n\ninitKafka 실패 \n");
		return 0;
	}
	kafkaProducer.setTopic("ppp_thumb");

	KafkaMessage message;
	message.setKey("testKey");
	message.setCommand("CREATE_THUMBNAIL");
	message.setVersion(1);
	message.setContentType("application/json");
	message.setContents("ddolsoon  contents222222");

	kafkaProducer.send(message);

	printf("\n시작\n");

	kafkaProducer.setDebugMode(true); // (optional)

	kafkaProducer.closeKafka();

	///////////////////////////////////////////////////
	// 1. Kafka Consumer 초기화
	///////////////////////////////////////////////////
	KafkaConsumer kafkaConsumer;
	std::map<std::string, std::string> configConsumeMap;
	std::vector<std::string> topics;
	topics.push_back("ppp_thumb");
	configConsumeMap.insert(std::pair<std::string, std::string>("auto.offset.reset", "latest"));

	if ( false == kafkaConsumer.initKafka(configConsumeMap, "172.31.14.25:9091", topics, "test-consumer", 5000) )
	{
		printf("\n\ninitKafka 실패 \n");
		return 0;
	}
	ReceivedMessage recvMsg;

	do
	{
		bool bResult = false;
		bResult = kafkaConsumer.consume(recvMsg);
		if ( bResult == false )
			break;
		printf("받았다key %s \n", recvMsg.getKey().c_str());
		std::map<std::string, std::string>::iterator it;
		std::map<std::string, std::string> headers =  recvMsg.getHeaders();
		for (it = headers.begin(); it != headers.end(); it++ )
		{
			printf("받았다 header : %s:%s \n", it->first.c_str(), it->second.c_str());
		}
		printf("받았다 body.... %s \n", recvMsg.getBody().c_str());
	}
	while ( true );

	kafkaConsumer.setDebugMode(true); // (optional)
	kafkaConsumer.closeKafka();

	return 0;
}
