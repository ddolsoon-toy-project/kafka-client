#ifndef __PRODUCER_I_SEND_CALLBACK_H__
#define __PRODUCER_I_SEND_CALLBACK_H__

#include "kafka/rdkafkacpp.h"

namespace RdKafka
{
	class Message;
}

#include <iostream>
#include <vector>
#include <cstdio>

class ProducerISendCallback
{
	public:
		ProducerISendCallback() { }
		virtual ~ProducerISendCallback( ) {}
		virtual void success (RdKafka::Message& message) = 0;
		virtual void fail (RdKafka::Message& message) = 0;
};

#endif
