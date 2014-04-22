#define ArraySize(x)  (sizeof(x) / sizeof(x[0]))

#include "poseidon-resource-allocation.h"

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

NS_LOG_COMPONENT_DEFINE("DEBUG_RESOURCEALLOCATION");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
PoseidonResourceAllocation<Parent>::OutputPITSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	// output only (router)
	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
		NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &PoseidonResourceAllocation::OutputPITSize, this);
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
PoseidonResourceAllocation<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	// output only (router)
	if( Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ()) << "\t" << ConsumerData);	
	}
	
	ConsumerData = 0;
	
	Simulator::Schedule (Seconds (0.5), &PoseidonResourceAllocation::OutputData, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
PoseidonResourceAllocation<Parent>::OutputSatisfied ()
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
	
	Simulator::Schedule (Seconds (0.5), &PoseidonResourceAllocation::OutputSatisfied, this);
}

template<class Parent>
LogComponent PoseidonResourceAllocation<Parent>::g_log = LogComponent (PoseidonResourceAllocation<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
PoseidonResourceAllocation<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PoseidonResourceAllocation").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <PoseidonResourceAllocation> ()
    	;
   return tid;
}

template<class Parent>
void
PoseidonResourceAllocation<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
PoseidonResourceAllocation<Parent>::GetLogName ()
{
   return super::GetLogName ()+".PoseidonResourceAllocation";
}

template<class Parent>
void
PoseidonResourceAllocation<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),
                                          Seconds (r.GetValue ()), &PoseidonResourceAllocation<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
PoseidonResourceAllocation<Parent>::AddFace (Ptr<Face> face)
{
 	LogComponentEnable("DEBUG_RESOURCEALLOCATION",LOG_INFO);
 	super::AddFace (face);
}

/// for output only
template<class Parent>
void
    PoseidonResourceAllocation<Parent>::OnData (Ptr<Face> face,
                                      		 Ptr<Data> data)
{
  if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
  {
    ConsumerData++;
  }
    super::OnData (face, data);

}

/// for output only
template<class Parent>
void
    PoseidonResourceAllocation<Parent>::DidSendOutInterest (Ptr< Face > inFace, 
							     Ptr< Face > outFace,
							     Ptr< const Interest > interest, 
							     Ptr< pit::Entry > pitEntry)
{
  if(Names::FindName (inFace -> GetNode ()).compare (5, 8, "Consumer") == 0)
  {
    ConsumerInterests++;
  }
    super::DidSendOutInterest (inFace,outFace,interest,pitEntry);
}

/// if the Pit-size is greater than the limit-value -> drop the current interest else forward it
template<class Parent>
void
PoseidonResourceAllocation<Parent>::OnInterest (Ptr<Face> face,
                                      Ptr<Interest> interest)
{
  if(this->m_pit -> GetSize() >= 1200 * 1)
  {}
  else
  {
    super::OnInterest (face, interest);
  }

}

/// for output only
template<class Parent>
void
PoseidonResourceAllocation<Parent>::AnnounceLimits ()
{
    Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
    Ptr<Face> face = l3->GetFace (0);
    
    if(first)
    {
    //OutputPITSize ();
    //OutputSatisfied();
    OutputData();
    first=false;
    }
    
    Simulator::Schedule (Seconds (1.0), &PoseidonResourceAllocation::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::PoseidonResourceAllocation::PerOutFaceLimits
template class PerOutFaceLimits< PoseidonResourceAllocation<BestRoute> >;
typedef PerOutFaceLimits< PoseidonResourceAllocation<BestRoute> > PerOutFaceLimitsPoseidonResourceAllocationBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPoseidonResourceAllocationBestRoute);


} // namespace fw
} // namespace ndn
} // namespace ns3
