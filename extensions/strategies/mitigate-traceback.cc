#define ArraySize(x)  (sizeof(x) / sizeof(x[0]))

#include "mitigate-traceback.h"

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

NS_LOG_COMPONENT_DEFINE("DEBUG_TRACEBACK");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
MitigateTraceback<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if( Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ())  << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &MitigateTraceback::OutputPitSize, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
MitigateTraceback<Parent>::OutputSatisfied ()
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
	
	Simulator::Schedule (Seconds (0.5), &MitigateTraceback::OutputSatisfied, this);
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
MitigateTraceback<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{   
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> ConsumerData));
	    
	    ConsumerData = 0;
	}
	
	Simulator::Schedule (Seconds (0.5), &MitigateTraceback::OutputData, this);
}

/// calculate the values (second interval)
template<class Parent>
void
MitigateTraceback<Parent>::CalculateRate ()
{  
  	pitSizeInterval2 = pitSizeInterval1;
}

/// get the index of a specific FaceID
template<class Parent>
int
MitigateTraceback<Parent>::IdToIndex (uint32_t id)
{
	bool found = false;
	int idIndex = 0;

	for(int i = 0; i < ArraySize(faceID) && found == false; i++)
	{
		if(faceID[i] == id)
		{
			idIndex = i;
			found = true;
		}
	}

     return idIndex;
}

/// return the topology-number of a router
template<class Parent>
int
MitigateTraceback<Parent>::NameToIndex (std::string name)
{
      int nid = boost::lexical_cast<int>(name.erase(0,6));
      
     return nid;
}

template<class Parent>
LogComponent MitigateTraceback<Parent>::g_log = LogComponent (MitigateTraceback<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
MitigateTraceback<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::MitigateTraceback").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <MitigateTraceback> ()
    	;
    return tid;
}

template<class Parent>
void
MitigateTraceback<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
MitigateTraceback<Parent>::GetLogName ()
{
     return super::GetLogName ()+".MitigateTraceback";
}

template<class Parent>
void
MitigateTraceback<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this-> m_pit != 0 && this-> m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),
                                          Seconds (r.GetValue ()), &MitigateTraceback<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
MitigateTraceback<Parent>::AddFace (Ptr<Face> face)
{
 	faceID[iterator] = face -> GetId();
 	LogComponentEnable("DEBUG_TRACEBACK",LOG_INFO);
 	super::AddFace (face);
 	iterator++;
}

/// check whether the incoming data-packet is a spoofed data-packet  (forward it) or a normal data-packet
template<class Parent>
void
MitigateTraceback<Parent>::OnData (Ptr<Face> face,
                                   Ptr<Data> data)
{	
	uint32_t spoofed = 200735, normal = 0;

	// output only
	if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	{
	  ConsumerData++;
	}
	
	// normal data will be forwarded directly
	if(data -> GetSignature () == normal)
	{
		super::OnData (face, data);
	}
	
	if(data -> GetSignature () == spoofed)
	{
		if (Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer")==0 || 
		    Names::FindName (face -> GetNode ()).compare (0, 13, "evil-Consumer")==0)
		{
			super::OnData (face,data);
		}

		else if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router")==0)
		{
			if(edge[NameToIndex(Names::FindName (face -> GetNode ()))])
			{
				last[IdToIndex(face -> GetId ())] = true;
			}

			super::OnData (face, data);
		}
	}
}

/// if the current router is the last one - all incoming interests will be dropped
template<class Parent>
void
MitigateTraceback<Parent>::OnInterest (Ptr<Face> face,
                                      Ptr<Interest> interest)
{
  	 if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	 {
	    ConsumerInterests++;
	 }
	 
	if(last[IdToIndex(face -> GetId ())]){}

	else
	{
		super::OnInterest (face, interest);
	}
}

/// if the values of a face reaches the thresholds -> send a spoofed packet
template<class Parent>
void
MitigateTraceback<Parent>::AnnounceLimits ()
{
  	Ptr<Face> face = this -> template GetObject<L3Protocol> () -> GetFace (0);

	if(first)
	{
	    	for(int i = 0; i < 6; i++)
		{
		  edge[NameToIndex(edgeRouter[i])] = true;
		}
		first = false;
		//OutputPitSize();
		//OutputSatisfied();
		OutputData();
	}

	//Calculate the new values
    	pitSizeInterval1 = this->m_pit -> GetSize ();
	Simulator::Schedule (Seconds (0.5), &MitigateTraceback::CalculateRate, this);	
    
	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  // if the condition is true -> every Pitentry of the biggest Consumer will get a spoofed packet
	  if(pitSizeInterval1 > pitThreshold || ((pitSizeInterval2-pitSizeInterval1)/2) > pitRateThreshold)
	  {
  		for (Ptr<pit::Entry> entry = this -> m_pit ->Begin ();
       		     entry != this -> m_pit ->End ();
       		     entry = this -> m_pit ->Next (entry))
    		{
			ns3::ndn::pit::Entry::in_container inContainer = entry -> GetIncoming ();

			for(ns3::ndn::pit::Entry::in_container::iterator inIterator = inContainer.begin ();
                	    inIterator != inContainer.end ();
                	    inIterator ++)
            		{
				Name evilName = entry -> GetInterest() -> GetName();
                		evilFace =  inIterator -> m_face;

				Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
				data->SetName (Create<Name> (evilName));
            			data->SetFreshness (Seconds(0));
            			data->SetSignature (200735); 
           			bool ok = evilFace->SendData (data);
	    			if(ok)
	    			{
					//NS_LOG_INFO("Spoofed Data sent at:" << Simulator::Now ().ToDouble(Time::S));
	    			}

			}

			if(Names::FindName (face -> GetNode ()).compare(0,6,"Router") == 0)
			{
				if(edge[NameToIndex(Names::FindName (face -> GetNode ()))])
				{
					last[IdToIndex(evilFace -> GetId ())] = true;
				}
			}
	 	}   		
	  }
	}

    	Simulator::Schedule (Seconds (1.0), &MitigateTraceback::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::MitigateTraceback::PerOutFaceLimits
template class PerOutFaceLimits< MitigateTraceback<BestRoute> >;
typedef PerOutFaceLimits< MitigateTraceback<BestRoute> > PerOutFaceLimitsMitigateTracebackBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsMitigateTracebackBestRoute);


} // namespace fw
} // namespace ndn
} // namespace ns3
