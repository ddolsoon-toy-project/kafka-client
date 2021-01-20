#include "ProducerEventCallback.h"
#include "common/include/kafka/rdkafkacpp.h"

void ProducerEventCallback::event_cb(RdKafka::Event& event)
{
	switch (event.type())
	{
		case RdKafka::Event::EVENT_ERROR:
		if (event.fatal()) 
		{
			std::cerr << "FATAL ";
		}
		std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
		event.str() << std::endl;
		break;

		case RdKafka::Event::EVENT_STATS:
			std::cerr << "\"STATS\": " << event.str() << std::endl;
		break;

		case RdKafka::Event::EVENT_LOG:
			fprintf(stderr, "LOG-%i-%s: %s\n",
						event.severity(), event.fac().c_str(), event.str().c_str());
		break;

		default:
			std::cerr << "EVENT " << event.type() <<
					" (" << RdKafka::err2str(event.err()) << "): " <<
					event.str() << std::endl;
		break;
	}
}
