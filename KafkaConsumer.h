/**
* @file KafkaConsumer.h
* @brief 카프카 Consumer Class
* @author minseong6329@naver.com
* @date $Date: 2021-01-21
*/

#ifndef __KAFKA_CONSUMER_H__
#define __KAFKA_CONSUMER_H__

#include "kafka/rdkafkacpp.h"
#include "KafkaMessage.h"

#include <vector>
#include <map>

class KafkaConsumer
{
	public:
		KafkaConsumer();
		virtual ~KafkaConsumer();

		bool initKafka(const std::map<std::string, std::string>& consumerConfigurationMap,
				std::string brokers, std::vector<std::string> topics, std::string consumerId, int pollPeriod);

		// Kafka 종료 함수
		void closeKafka();

		// true 시, 보낸 카프카 메세지에 대한 Header/Body를 stderr에 출력 ( default : false )
		void setDebugMode(const bool& isDebugMode);

		bool consume(ReceivedMessage& receivedMessage);

		void setPollPeriod(int pollPeriod);

		std::string _strBrokers;
		std::string _strConsumerId;
		std::vector<std::string> _topics;

	protected:
		std::string getNowTimeString() const;

		RdKafka::KafkaConsumer* consumer;
		bool isDebugMode;

	private:
		int _pollPeriod;

		// Consumer Configuration Setter
		bool _setConsumerConfiguration(const std::map<std::string, std::string>& consumerConfigurationMap);

		RdKafka::Conf* _globalConf;
};

#endif 
