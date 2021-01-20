#include "ProducerCallback.h"
#include "ProducerISendCallback.h"
#include "ProducerDefaultSendCallback.h"

ProducerCallback::ProducerCallback()
{
	ProducerISendCallback* sendCallback = new ProducerDefaultSendCallback();
	_sendCallback = sendCallback;
	_isDefaultCallback = true;
}

ProducerCallback::~ProducerCallback()
{
	if ( _isDefaultCallback )
	{
		if ( _sendCallback != NULL )
		{
			delete _sendCallback;
		}
		_sendCallback = NULL;
	}
}

void ProducerCallback::dr_cb (RdKafka::Message& message)
{
	switch (message.status())
	{
		case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
			_sendCallback->fail(message);
			break;

		case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
		case RdKafka::Message::MSG_STATUS_PERSISTED:
			_sendCallback->success(message);
			break;

		default:
			_sendCallback->fail(message);
			break;
	}
}

void ProducerCallback::setSendCallback(ProducerISendCallback* sendCallback)
{
	if ( _sendCallback != NULL ) 
	{
		delete _sendCallback;
		_sendCallback = NULL;
	}

	_sendCallback = sendCallback;
	_isDefaultCallback = false;
}
