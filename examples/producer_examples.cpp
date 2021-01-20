#include <iostream>
#include <vector>
#include "KafkaProducer.h"

std::map<std::string, std::string>* setProducerConfigurationMap()
{
    std::map<std::string, std::string>* configMap = new std::map<std::string, std::string>();
	return configMap;
}

int main (int argc, char * argv[])
{
	KafkaProducer kafkaProducer;
	std::map<std::string, std::string>* configMap = setProducerConfigurationMap();

	///////////////////////////////////////////////////
	// 1. Kafka Producer 초기화
	///////////////////////////////////////////////////
	if ( false == kafkaProducer.initKafka(*configMap, "127.0.0.1:9091,127.0.0.1:9092,127.0.0.1:9093", "test-agent", NULL, 1000) )
	{
		printf("\n\ninitKafka 실패 \n");
		return 0;
	}

	printf("\n시작\n");

	kafkaProducer.setDebugMode(true); // (optional)

	kafkaProducer.closeKafka();

	delete configMap;

	return 0;
}
