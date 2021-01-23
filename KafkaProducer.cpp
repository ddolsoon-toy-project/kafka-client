#include "KafkaProducer.h"

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <sys/time.h>

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

bool KafkaProducer::send(const KafkaMessage& message)
{
	// Kafka Message Header 생성
	RdKafka::Headers * headers = RdKafka::Headers::create();

	std::string messageId;
	if ( false == _createKafkaHeader(headers, message, _strUserAgent, messageId) )
	{
		fprintf(stderr, "[ERR][%s][MESSAGE HEADER ERROR][MESSAGE VALUE IS INVALID]\n", getNowTimeString().c_str());
		delete headers;
		return false;
	}

	// Kafka Message 전송
	RdKafka::ErrorCode errorCode = producer->produce(_strTopic, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
					const_cast<char *>(message.getContents().c_str()), message.getContents().size(),
					const_cast<char *>(message.getKey().c_str()), message.getKey().size(),
					(long)0,
					headers,
					NULL);

	if ( errorCode != RdKafka::ERR_NO_ERROR )
	{
		fprintf(stderr, "[FAIL][%s][KAFKA][MESSAGE ID][%s][[Produce Message Failed][topic:%s]\n", 
				getNowTimeString().c_str(), messageId.c_str(), _strTopic.c_str());
		delete headers;
		return false;
	}
	else
	{
		fprintf(stderr, "[SUCCESS][%s][KAFKA][MESSAGE ID][%s][Produce Message Success][topic:%s]\n", 
				getNowTimeString().c_str(), messageId.c_str(), _strTopic.c_str());
	}

	return true;
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

void KafkaProducer::setTopic(const std::string& topic)
{
	_strTopic = topic;
}

std::string KafkaProducer::getTopic() const
{
	return _strTopic;
}

bool KafkaProducer::_createKafkaHeader(RdKafka::Headers* headers, const KafkaMessage& message,
		const std::string& userAgent, std::string& messageId)
{
	// ms 단위 timestamp 측정
	struct timeval te;
	gettimeofday(&te, NULL);
	long long int timestamp = te.tv_sec * 1000LL + te.tv_usec/1000;

	// x-message-id
	std::string strMessageId = _getRandomUUID(); 
	headers->add(HEADERS_MESSAGE_ID, strMessageId);
	messageId = strMessageId;

	// x-timestamp
	char strTimestamp[20];
	sprintf(strTimestamp, "%lld", timestamp);
	headers->add(HEADERS_TIMESTAMP, strTimestamp);	// x-timestamp

	// x-command
	headers->add(HEADERS_COMMAND, message.getCommand());

	// x-message-version
	char strMsgVersion[5];
	sprintf(strMsgVersion, "%d", message.getVersion());
	headers->add(HEADERS_MESSAGE_VERSION, strMsgVersion);

	// x-content-type
	headers->add(HEADERS_CONTENT_TYPE, message.getContentType());	

	// x-user-agent
	headers->add(HEADERS_USER_AGENT, userAgent);
	
	return true;
}

std::string KafkaProducer::_getRandomUUID() const
{
	struct timeval time;
	gettimeofday(&time,NULL);
	unsigned int seed = (time.tv_sec * 1000000) + (time.tv_usec);

	char UUID[40];
	char szTemp[40] = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
	char szHex[18] = "0123456789ABCDEF-";
	int nLen = strlen (szTemp);

	for ( int i = 0; i < nLen + 1; i++ )
	{
        int nRandomIdx = rand_r(&seed) % 16;
		char c = ' ';

		switch (szTemp[i])
		{
			case 'x' :
				c = szHex [nRandomIdx];
				break;
			case 'y' :
				c = szHex [nRandomIdx & (0x03 | 0x08)];
				break;
			case '-' :
				c = '-';
				break;
			case '4' :
				c = '4';
				break;
		}
		UUID[i] = ( i < nLen ) ? c : 0x00;
	}

	return std::string(UUID);
}
