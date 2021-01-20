#include "KafkaMessage.h"

std::string KafkaMessage::getKey() const
{
	return _strKey;
}

void KafkaMessage::setKey(std::string strKey)
{
	_strKey = strKey;
}

std::string KafkaMessage::getCommand() const
{
	return _strCommand;
}

void KafkaMessage::setCommand(std::string strCommand)
{
	_strCommand = strCommand;
}

int KafkaMessage::getVersion() const 
{
	return _nVersion;
}

void KafkaMessage::setVersion(int nVersion)
{
	_nVersion = nVersion;
}

std::string KafkaMessage::getContentType() const
{
	return _strContentType;
}

void KafkaMessage::setContentType(std::string strContentType)
{
	_strContentType = strContentType;
}

std::string KafkaMessage::getContents() const
{
	return _strContents;
}

void KafkaMessage::setContents(std::string strContents)
{
	_strContents = strContents;
}

std::string KafkaMessage::getUserAgent() const
{
	return _strUserAgent;
}

void KafkaMessage::setUserAgent(std::string strUserAgent)
{
	_strUserAgent = strUserAgent;
}


