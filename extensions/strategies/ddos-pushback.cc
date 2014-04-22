#define ArraySize(x)  (sizeof(x) / sizeof(x[0]))

#include "ddos-pushback.h"

#include <ns3/point-to-point-module.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndn-l3-protocol.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/ndn-name.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

NS_LOG_COMPONENT_DEFINE("DEBUG_DDOS");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
DDoSPushback<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);
	

	if( Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  	NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ())  << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &DDoSPushback::OutputPitSize, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
DDoSPushback<Parent>::OutputSatisfied ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{
	    if(ConsumerInterests == 0)
	    {
	      ConsumerInterests = 1;
	    }
	    
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> ConsumerData/ConsumerInterests));
	}
	
	Simulator::Schedule (Seconds (0.5), &DDoSPushback::OutputSatisfied, this);
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
DDoSPushback<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);
	
	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{   
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> ConsumerData));
	    
	    ConsumerData = 0;
	}
	
	Simulator::Schedule (Seconds (0.5), &DDoSPushback::OutputData, this);
}

/// checks whether a prefix is already blacklisted
template<class Parent>
bool
DDoSPushback<Parent>::PrefixOnEvilPrefixes (std::string prefix)
{
	bool in = false, found = false;
	
  	for (int i = 0; i < ArraySize(evilPrefixes) && found == false; i ++)
  	{
		if(prefix == evilPrefixes[i])
		{
			in = true;
			found = true;
		}
  	}

    return in;
}

/// get the index of a specific FaceID
template<class Parent>
int
DDoSPushback<Parent>::IdToIndex (uint32_t id)
{
	bool found = false;
	int idIndex = 35505;

	for(int i = 0; i < ArraySize(faceid) && found == false; i++)
	{
		if(faceid[i] == id)
		{
			idIndex = i;
			found = true;
		}
	}

    return idIndex;
}

/// get the index of an evil prefix
template<class Parent>
int
DDoSPushback<Parent>::EvilPrefixToIndex (std::string prefix)
{
	bool found = false;
	int prefixIndex = 35505;

	for(int i = 0; i < ArraySize(evilPrefixes) && found == false; i++)
	{
		if(evilPrefixes[i] == prefix)
		{
			prefixIndex = i;
			found = true;
		}
	}

    return prefixIndex;
}

/// calculate the size of every prefix in the PIT
template<class Parent>
void
DDoSPushback<Parent>::GetPrefixSize()
{
  	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3 -> GetFace(0);
	
	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  
	  for (int i = 0; i < ArraySize(prefixSize); i ++)
	  {
		prefixSize[i] = 0;
		prefix[i] = "";
	  }

	  for(Ptr<pit::Entry> pitEntry = this -> m_pit ->Begin ();
	      pitEntry != this -> m_pit ->End ();
	      pitEntry = this -> m_pit ->Next (pitEntry))
	  {
		std::string currentPrefix = pitEntry -> GetPrefix ().toUri ().substr(0,21);
		if(currentPrefix.substr(20,1) != "/")
		{
			currentPrefix = currentPrefix.substr(0,22);
		}

		auto p = std::find(prefix,prefix+ArraySize(prefix),currentPrefix);
		int index = p-prefix;

		auto q = std::find(prefix,prefix+ArraySize(prefix),"");
		int free = q-prefix;

		if(p >= prefix + ArraySize(prefix))
		{
			prefix[free] = currentPrefix;
			prefixSize[free]++;
			exactEntry[free] = pitEntry -> GetPrefix ();
		}
		else
		{
			prefixSize[index]++;
			exactEntry[index] = pitEntry -> GetPrefix ();
		}
	  }
	}
}

template<class Parent>
LogComponent DDoSPushback<Parent>::g_log = LogComponent (DDoSPushback<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
DDoSPushback<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::DDoSPushback").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <DDoSPushback> ()
    	;
     return tid;
}

template<class Parent>
void
DDoSPushback<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
DDoSPushback<Parent>::GetLogName ()
{
     return super::GetLogName ()+".DDoSPushback";
}

template<class Parent>
void
DDoSPushback<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),
                                          Seconds (r.GetValue ()), &DDoSPushback<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
DDoSPushback<Parent>::AddFace (Ptr<Face> face)
{
 	faceid[faceIterator] = face -> GetId();
 	LogComponentEnable("DEBUG_DDOS",LOG_INFO);

 	super::AddFace (face);
 	faceIterator++;
}

/// check whether the incoming data-packet is a report or a normal data-packet
template<class Parent>
void
DDoSPushback<Parent>::OnData (Ptr<Face> face,
                              Ptr<Data> data)
{
	uint32_t report = 239057, normal = 0;
	bool foundPrefix = false;
	uint32_t csSize = this -> m_contentStore -> GetSize();

	if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	{
	  ConsumerData++;
	}
	// normal data will be forwarded directly
	if(data -> GetSignature () == normal)
	{
		super::OnData (face, data);
	}
	
	// if a report reaches the router - the transmitted prefix and the report will be forwarded over the corresponding face 
	if(data -> GetSignature () == report && Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
		size_t cprefixSize = data -> GetName ().toUri().size();
		std::string cprefix = data -> GetName ().toUri().substr(7,cprefixSize-6);
		cprefix = cprefix + "/";

		for(Ptr<pit::Entry> pitEntry = this -> m_pit -> Begin ();
            	    (pitEntry != this -> m_pit -> End ()) && (foundPrefix == false);
            	    pitEntry = this -> m_pit -> Next (pitEntry))
        	{
			std::string prefix = pitEntry -> GetPrefix ().toUri().substr(0,21);
			if(prefix.substr(20,1) != "/")
			{
				prefix = pitEntry -> GetPrefix ().toUri().substr(0,22);
			}

			if(prefix == cprefix)
			{
				foundPrefix = true;
				ns3::ndn::pit::Entry::in_container inContainerPrefix = pitEntry -> GetIncoming ();
				evilFace = inContainerPrefix.begin () -> m_face;
				
				evilPrefixes[EvilPrefixToIndex("")] = prefix;
				
				//NS_LOG_INFO("transmitted face blacklisted");
			}
		}

		if(foundPrefix)
		{
			Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
			data->SetName (Create<Name> ("/Report" + cprefix));
			data->SetFreshness (Seconds(0));
        		data->SetSignature (239057); 

        		bool ok = evilFace->SendData (data);
			if(ok)
			{
				//NS_LOG_INFO("second NodeName: " << Names::FindName (evilFace -> GetNode ()));
				//NS_LOG_INFO("second report sent at: " << Simulator::Now ().ToDouble(Time::S));
	    		}
		}
	}
}

/// evil-interests are not forwarded
template<class Parent>
void
DDoSPushback<Parent>::OnInterest (Ptr<Face> face,
                                  Ptr<Interest> interest)
{
	 if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	 {
	    ConsumerInterests++;
	 }
  
	std::string interestPrefix =  interest -> GetName ().toUri ().substr(0,21);

	if(interestPrefix.substr(20,1) != "/")
	{
		interestPrefix = interest -> GetName ().toUri ().substr(0,22);
	}
	
	if(EvilPrefixToIndex(interestPrefix) != 35505)
	{
			//NS_LOG_INFO("INTEREST NOT FORWARDED!");
	}
	else
	{
        		super::OnInterest (face, interest);
	}
	
}

/// check whether a prefix reaches the threshold -> blacklist & report to next router as a datapacket
template<class Parent>
void
DDoSPushback<Parent>::AnnounceLimits ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3 -> GetFace(0);
	
	GetPrefixSize();
	
	if(first)
	{
	    for(int i=0;i<ArraySize(prefixThreshold);i++)
	    {
		prefixThreshold[i] = 20;
	    }
	    
	    OutputPitSize();
	    //OutputData();
	    
	    first = false;
	}
	
	for(int i = 0; i < ArraySize(prefixSize); i++)
	{
		if(prefixSize[i] > prefixThreshold[i] && Names::FindName (face -> GetNode ()).compare(0, 6, "Router") == 0 )
		{			
			Ptr<pit::Entry> currentPitEntry = this -> m_pit -> Find(exactEntry[i]);
			ns3::ndn::pit::Entry::in_container inContainer = currentPitEntry -> GetIncoming ();
			evilPrefixFace = inContainer.begin () -> m_face;
			evilPrefixes[i] = prefix[i];

			Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
			data->SetName (Create<Name> ("/Report" + prefix[i]));
        		data->SetTimestamp (Simulator::Now ());
			data->SetFreshness (Seconds(0));
        		data->SetSignature (239057); 
			bool ok = evilPrefixFace->SendData (data);
			if(ok)
			{
				//NS_LOG_INFO("first NodeName: " << Names::FindName (evilPrefixFace -> GetNode ()));
				//NS_LOG_INFO(prefix[i]);
				//NS_LOG_INFO("first report sent at: " << Simulator::Now ().ToDouble(Time::S) << "\n");
			}
		}
	}

    Simulator::Schedule (Seconds (0.1), &DDoSPushback::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3

#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::DDoSPushback::PerOutFaceLimits
template class PerOutFaceLimits< DDoSPushback<BestRoute> >;
typedef PerOutFaceLimits< DDoSPushback<BestRoute> > PerOutFaceLimitsDDoSPushbackBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsDDoSPushbackBestRoute);

} // namespace fw
} // namespace ndn
} // namespace ns3
