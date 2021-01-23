#include "KafkaConsumer.h"

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <sys/time.h>

bool KafkaConsumer::_setConsumerConfiguration(const std::map<std::string, std::string>& consumerConfigurationMap)
{
	std::string errorString;

	// mandatory producer configuration setting.
	if ( _globalConf->set("bootstrap.servers", _strBrokers.c_str(), errorString) != RdKafka::Conf::CONF_OK )
	{
		fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
		fprintf(stderr, "[ERR][%s][boostrap.servers config failed]\n", getNowTimeString().c_str());
		return false;
	}

	if ( _globalConf->set("group.id", _strConsumerId.c_str(), errorString) != RdKafka::Conf::CONF_OK )
	{
		printf("[ERR][%s][group.id config failed]\n", getNowTimeString().c_str());
		return false;
	}

	// custom producer configuration setting.
	if ( !consumerConfigurationMap.empty() )
	{
		std::map<std::string, std::string>::const_iterator ite;
		for( ite = consumerConfigurationMap.begin(); ite != consumerConfigurationMap.end(); ite++)
		{
			if ( _globalConf->set(ite->first, ite->second, errorString) != RdKafka::Conf::CONF_OK )
			{
				fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
				fprintf(stderr, "[ERR][%s][%s config failed]\n", getNowTimeString().c_str(), (ite->first).c_str());
				return false;
			}
		}
	}

	return true;
}

bool KafkaConsumer::initKafka(const std::map<std::string, std::string>& consumerConfigurationMap,
		std::string brokers, std::vector<std::string> topics, std::string consumerId, int pollPeriod)
{

	// broker 도메인
	_strBrokers = brokers;
	_strConsumerId = consumerId;
	_pollPeriod = pollPeriod;

	_globalConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

	std::string errorString;
	// Broker 설정
	fprintf(stderr, "[DEBUG][%s][strBrokers][%s]\n", getNowTimeString().c_str(), _strBrokers.c_str());

	if ( _setConsumerConfiguration(consumerConfigurationMap) == false )
	{
		fprintf(stderr, "[ERR][%s][Consumer Configuration Setting failed finally]\n", getNowTimeString().c_str()); 
		return false;
	}

	// Kafka Consumer Instance 생성
	consumer = RdKafka::KafkaConsumer::create(_globalConf, errorString);
	if ( !consumer )
	{
		fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
		fprintf(stderr, "[ERR][%s][Failed to create consumer]\n", getNowTimeString().c_str());
		return false;
	}
	else
	{	
		fprintf(stderr, "[SUCCESS][%s][Created consumer]\n", getNowTimeString().c_str());
	}

	// consumer start
	RdKafka::ErrorCode response = consumer->subscribe(topics);
	if ( response != RdKafka::ERR_NO_ERROR )
	{
		fprintf(stderr, "[ERR][%s][Failed to start RdKafka::Consumer]\n", getNowTimeString().c_str());
		return false;
	}

	return true;
}

KafkaConsumer::KafkaConsumer()
{
	isDebugMode = false;

	consumer = NULL;
	_globalConf = NULL;
}

KafkaConsumer::~KafkaConsumer()
{	
	// memory leak 방지
	RdKafka::wait_destroyed(5000);
}

void KafkaConsumer::closeKafka()
{
	if ( NULL == consumer && NULL == _globalConf )
	{
		fprintf(stderr, "[ERR][%s][FAIL closeKafka()][consumer, _globalConf are NULL]\n",
			getNowTimeString().c_str());
		return;
	}

	consumer->close();

	if ( consumer != NULL )
	{
		delete consumer;
		consumer = NULL;
	}

	if ( _globalConf != NULL )
	{
		delete _globalConf;
		_globalConf = NULL;
	}
}

void KafkaConsumer::setDebugMode(const bool& isDebugMode)
{
	this->isDebugMode = isDebugMode;
}


bool KafkaConsumer::consume(ReceivedMessage& receivedMessage)
{
	bool bResult = false;

	RdKafka::Message *message = consumer->consume(_pollPeriod);

	switch ( message->err() )
	{
		case RdKafka::ERR_NO_ERROR:
			bResult = true;
			break;
		case RdKafka::ERR__TIMED_OUT:
		case RdKafka::ERR__PARTITION_EOF:
		case RdKafka::ERR__UNKNOWN_TOPIC:
		case RdKafka::ERR__UNKNOWN_PARTITION:
		default:
			fprintf(stderr, "[ERR][%s][Failed to consume RdKafka::Consumer error Message : %s ]\n", 
					getNowTimeString().c_str(), message->errstr().c_str());
			delete message;
			bResult = false;
	}

	do
	{
		// consume 실패시, 실패 반환
		if ( bResult == false )
			break;

		receivedMessage.setKey(message->key()->c_str());

		std::map<std::string, std::string> receivedHeaders;
		RdKafka::Headers *headers = message->headers();
		if ( headers )
		{
			std::vector<RdKafka::Headers::Header> vHeaders = headers->get_all();
			for ( size_t i = 0 ; i < vHeaders.size(); i++)
			{
				const RdKafka::Headers::Header header = vHeaders[i];
				if ( header.value() != NULL )
				{
					receivedHeaders[header.key().c_str()] = (const char *)header.value();
				}
			}
		}
		receivedMessage.setHeaders(receivedHeaders);

		char receivedMsgBody[1024 * 1024];
		memset(receivedMsgBody, 0x00, sizeof(receivedMsgBody));
		strncpy(receivedMsgBody, 
				static_cast<const char *>(message->payload()), 
				static_cast<int>(message->len()));

		receivedMessage.setBody(receivedMsgBody);

		if ( message != NULL )
			delete message; // message delete 안하면, blocking 됨
	}
	while (false);

	return bResult;
}

void KafkaConsumer::setPollPeriod(int pollPeriod)
{
	_pollPeriod = pollPeriod;
}

// 현재시간 std::string으로 출력
std::string KafkaConsumer::getNowTimeString() const
{
	time_t now;
	struct tm* timestamp;
	now = time(NULL);
	timestamp= localtime(&now);
	char buf[30];
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
				              timestamp->tm_year + 1900, timestamp->tm_mon + 1, timestamp->tm_mday,
							                timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec);

	std::string strNowTime = buf;
	return strNowTime;
}


