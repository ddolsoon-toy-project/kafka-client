/**
* @file KafkaMessage.cpp
* @brief 카프카 메세지
* @author minseong6329@naver.com
* @date $Date: 2021-01-21
*/

#ifndef __KAFKA_MESSAGE_H__
#define __KAFKA_MESSAGE_H__

#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <cstring>

//글로벌 카프카 메세지 포멧을 위한 class
class KafkaMessage 
{
	private:
		/**
		 * @brief 전송할 메세지의 순서보장의 기준이 되는 key
		 */
		std::string _strKey;
		/**
		 * @brief 전송할 메세지의 command
		 */
		std::string _strCommand;
		/**
		 * @brief 전송할 메세지의 version 
		 */
		int _nVersion;
		/**
		 * @brief 전송할 메세지의 content-type
		 */ 
		std::string _strContentType;
		/**
		 * @brief 전송할 메세지의 body
		 */
		std::string _strContents;

	public:
		static const int MIN_VERSION = 1;

		KafkaMessage()
		{
		}
		~KafkaMessage()
		{
		}

		std::string getKey() const;
		void setKey(std::string strKey);

		std::string getCommand() const;
		void setCommand(std::string strCommand);
	
		int getVersion() const;
		void setVersion(int nVersion);

		std::string getContentType() const;
		void setContentType(std::string strContentType);

		std::string getContents() const;
		void setContents(std::string strContents);
};

class ReceivedMessage
{
	private:
		/**
		 * @brief 전송받은 메세지의 순서보장의 기준이 되는 key
		 */
		std::string _strKey;

		/**
		 * @brief 전송받은 메시지의 headers
		 */
		std::map<std::string, std::string> _headers;

		/**
		 * @brief 전송할 메세지의 body
		 */
		std::string _strBody;

	public:
		ReceivedMessage()
		{
		}
		~ReceivedMessage()
		{
		}

		std::string getKey() const;
		void setKey(std::string strKey);

		std::map<std::string, std::string> getHeaders() const;
		void setHeaders(std::map<std::string, std::string> headers);

		std::string getBody() const;
		void setBody(std::string strBody);


};


#endif 
