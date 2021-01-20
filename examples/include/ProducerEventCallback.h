#ifndef __PRODUCER_EVENT_CALLBACK_H__
#define __PRODUCER_EVENT_CALLBACK_H__

#include "kafka/rdkafkacpp.h"
#include <iostream>
#include <vector>
#include <cstdio>

// 글로벌 카프카 이벤트 콜백함수 class
// Error, Statistics, Logs 등의 이벤트 등을 전파하기위한 이벤트 콜백함수 인터페이스 이다.
class ProducerEventCallback : public  RdKafka::EventCb
{
	public:
		void event_cb(RdKafka::Event& error);

};

#endif 
