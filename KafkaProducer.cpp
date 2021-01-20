#include "KafkaProducer.h"

#include <iostream>
#include <algorithm>
#include <cstdio>

bool KafkaProducer::_setProducerConfiguration(const std::map<std::string, std::string>& producerConfigurationMap,
	ProducerISendCallback* const producerCallback)
{
	std::string errorString;

	// mandatory producer configuration setting.
	if ( _globalConf->set("bootstrap.servers", _strBrokers.c_str(), errorString) != RdKafka::Conf::CONF_OK )
	{
		fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
		fprintf(stderr, "[ERR][%s][boostrap.servers config failed]\n", getNowTimeString().c_str());
		return false;
	}

	if ( _globalConf->set("event_cb", &_producerEventCallback, errorString) != RdKafka::Conf::CONF_OK )
	{
		fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
		fprintf(stderr, "[ERR][%s][event_cb config failed]\n", getNowTimeString().c_str());
		return false;
	}

	if ( producerCallback == NULL )
	{
		if ( _globalConf->set("dr_cb", &_producerCallback, errorString) != RdKafka::Conf::CONF_OK )
		{
			fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
			fprintf(stderr, "[ERR][%s][dr_cb config failed]\n", getNowTimeString().c_str());
			return false;
		}
	}
	else
	{
		_producerCallback.setSendCallback(producerCallback);
		if ( _globalConf->set("dr_cb", &_producerCallback, errorString) != RdKafka::Conf::CONF_OK )
		{
			fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
			fprintf(stderr, "[ERR][%s][dr_cb config failed]\n", getNowTimeString().c_str());
			return false;
		}
	}

	// custom producer configuration setting.
	if ( !producerConfigurationMap.empty() )
	{
		std::map<std::string, std::string>::const_iterator ite;
		for( ite = producerConfigurationMap.begin(); ite != producerConfigurationMap.end(); ite++)
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

bool KafkaProducer::initKafka(const std::map<std::string, std::string>& producerConfigurationMap, 
		std::string brokers ,std::string userAgent, ProducerISendCallback* const producerCallback, 
		int nTimeOut)
{

	// broker 도메인
	_strBrokers = brokers;

	// userAgent 세팅
	_setUserAgent(userAgent);

	_globalConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

	std::string errorString;
	// Broker 설정
	fprintf(stderr, "[DEBUG][%s][strBrokers][%s]\n", getNowTimeString().c_str(), _strBrokers.c_str());

	if ( _setProducerConfiguration(producerConfigurationMap, producerCallback) == false )
	{
		fprintf(stderr, "[ERR][%s][Producer Configuration Setting failed finally]\n", getNowTimeString().c_str()); 
		return false;
	}

	// Kafka Producer Instance 생성
	producer = RdKafka::Producer::create(_globalConf, errorString);
	if ( !producer )
	{
		fprintf(stderr, "[ERR][%s][errorString][%s]\n", getNowTimeString().c_str(), errorString.c_str());
		fprintf(stderr, "[ERR][%s][Failed to create producer]\n", getNowTimeString().c_str());
		return false;
	}
	else
	{	
		fprintf(stderr, "[SUCCESS][%s][Created producer]\n", getNowTimeString().c_str());
	}
	
	// Event Polling Thread 생성
	_eventPoll.bRun = true;
	_eventPoll.producer = producer;
	_eventPoll.timeOut = nTimeOut;

	if ( pthread_create(&_pollingThreadId, NULL, eventPollingThread, (void*)&_eventPoll) < 0 )
	{
		fprintf(stderr, "[ERR][%s][Faild to create Event Polling Thread]\n", getNowTimeString().c_str());	
		return false;
	}

	return true;
}

bool KafkaProducer::pollEvent(const int& nTimeOut) const
{
	if ( producer != NULL )
	{
		producer->poll(nTimeOut);
		return true;
	}
	
	return false;
}

KafkaProducer::KafkaProducer()
{
	isDebugMode = false;

	producer = NULL;
	_globalConf = NULL;
}

KafkaProducer::~KafkaProducer()
{	
	// memory leak 방지
	RdKafka::wait_destroyed(5000);
}

void KafkaProducer::closeKafka()
{
	if ( NULL == producer && NULL == _globalConf )
	{
		fprintf(stderr, "[ERR][%s][FAIL closeKafka()][producer, _globalConf are NULL]\n",
			getNowTimeString().c_str());
		return;
	}

	// 메세지를 다 보낼때 까지 대기
	while ( producer != NULL && producer->outq_len() > 0 )
		producer->poll(500);

	if ( _eventPoll.bRun )
	{
		_eventPoll.bRun = false;

		if ( 0 != _pollingThreadId )
		{
			pthread_join(_pollingThreadId, NULL); // polling Thread 종료되길 기다림.
		}
	}

	if ( producer != NULL )
	{
		delete producer;
		producer = NULL;
	}

	if ( _globalConf != NULL )
	{
		delete _globalConf;
		_globalConf = NULL;
	}
}

void KafkaProducer::setDebugMode(const bool& isDebugMode)
{
	this->isDebugMode = isDebugMode;
}


// 현재시간 std::string으로 출력
std::string KafkaProducer::getNowTimeString() const
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

void KafkaProducer::_setUserAgent(const std::string& strUserAgent)
{
	_strUserAgent = strUserAgent;
}

// EventPolling Thread
void* KafkaProducer::eventPollingThread(void* eventPoll)
{
	EventPoll* producerEventPoll = (EventPoll*)eventPoll;
	producerEventPoll->eventPoll();

	return NULL;
}

void EventPoll::eventPoll() const
{
	while ( bRun )
	{
		// Event를 수신하기 위해서, 주기적으로 Polling
		producer->poll(timeOut);
	}

	pthread_exit(NULL);

}

