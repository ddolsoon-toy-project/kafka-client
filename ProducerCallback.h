#ifndef __PRODUCER_CALLBACK_H__
#define __PRODUCER_CALLBACK_H__

#include "kafka/rdkafkacpp.h"

namespace RdKafka
{
	class DeliveryReportCb;
	class Message;
}

#include <iostream>
#include <vector>
#include <cstdio>
#include "ProducerISendCallback.h"

//글로벌 카프카 콜백함수 class
class ProducerCallback : public  RdKafka::DeliveryReportCb
{
	public:
		ProducerCallback();
		virtual ~ProducerCallback();
		virtual void dr_cb (RdKafka::Message& message);
		void setSendCallback(ProducerISendCallback* sendCallback);

	private:
		ProducerISendCallback* _sendCallback;
		bool _isDefaultCallback;
};

#endif
