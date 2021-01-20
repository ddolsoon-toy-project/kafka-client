/**
* @file KafkaProducer.h
* @brief 카프카 Producer Class
* @author minseong6329@naver.com
* @date $Date: 2021-01-21
*/

#ifndef __KAFKA_PRODUCER_H__
#define __KAFKA_PRODUCER_H__

#include "ProducerCallback.h"
#include "ProducerEventCallback.h"
#include "kafka/rdkafkacpp.h"

#include <vector>
#include <map>

#define KAFKA_MESSAGE_VERSION			1

static const char* const HEADERS_MESSAGE_ID					= "x-message-id";
static const char* const HEADERS_TIMESTAMP					= "x-timestamp";
static const char* const HEADERS_COMMAND					= "x-command";
static const char* const HEADERS_MESSAGE_VERSION			= "x-message-version";
static const char* const HEADERS_CONTENT_TYPE				= "x-content-type";
static const char* const HEADERS_USER_AGENT					= "x-user-agent";

class EventPoll
{
	public:
		EventPoll()
		{
			producer = NULL;
			bRun = false;
			timeOut = 0;
		}
		RdKafka::Producer* producer;
		bool bRun;
		int timeOut;
	public:
		void eventPoll() const;
};

class KafkaProducer 
{
	public:
		KafkaProducer();
		virtual ~KafkaProducer();

		bool initKafka(const std::map<std::string, std::string>& producerConfigurationMap,
				std::string brokers, std::string userAgent, ProducerISendCallback* const producerCallback, 
				int nTimeOut);

		// Queue에 저장된 Callback에 대한 메세지를 꺼내오는 함수 ( 별도의 쓰레드로 Polling 할것 )
		bool pollEvent(const int& nTimeOut) const;

		// Kafka 종료 함수
		void closeKafka();

		// true 시, 보낸 카프카 메세지에 대한 Header/Body를 stderr에 출력 ( default : false )
		void setDebugMode(const bool& isDebugMode);

		std::string getTopic() const;
		void setTopic(const std::string& topic);

		std::string _strBrokers;
		std::string _strUserAgent;
		std::string _strTopic;

	protected:
		std::string getNowTimeString() const;

		RdKafka::Producer* producer;
		bool isDebugMode;

	private:
		// UserAgent Setter
		void _setUserAgent(const std::string& strUserAgent);

		// Producer Configuration Setter
		bool _setProducerConfiguration(const std::map<std::string, std::string>& producerConfigurationMap,
				ProducerISendCallback* const producerCallback);

		// EventPolling Thread
		static void* eventPollingThread(void* const pTimeOut);

		bool _bRunPollingThread;
		EventPoll _eventPoll;
		pthread_t _pollingThreadId;

		RdKafka::Conf* _globalConf;

		ProducerEventCallback _producerEventCallback;
		ProducerCallback _producerCallback;
};

#endif 
