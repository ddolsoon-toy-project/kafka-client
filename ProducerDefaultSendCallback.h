#ifndef __PRODUCER_DEFAULT_SEND_CALLBACK_H__
#define __PRODUCER_DEFAULT_SEND_CALLBACK_H__

#include "kafka/rdkafkacpp.h"
#include "ProducerISendCallback.h"

namespace RdKafka
{
	class Message;
}

#include <iostream>
#include <vector>
#include <cstdio>

class ProducerDefaultSendCallback : public ProducerISendCallback
{
	public:
		ProducerDefaultSendCallback()
		{
		}
		~ProducerDefaultSendCallback()
		{
		}

		void success (RdKafka::Message& message);
		void fail (RdKafka::Message& message);
};

#endif
