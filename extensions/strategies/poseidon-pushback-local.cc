#define ArraySize(x)  (sizeof(x) / sizeof(x[0]))

#include "poseidon-pushback-local.h"

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

NS_LOG_COMPONENT_DEFINE("DEBUG");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
PoseidonPushbackLocal<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if( Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ())  << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackLocal::OutputPitSize, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
PoseidonPushbackLocal<Parent>::OutputSatisfied ()
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
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackLocal::OutputSatisfied, this);
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
PoseidonPushbackLocal<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{   
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> ConsumerData));
	    
	    ConsumerData = 0;
	}
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackLocal::OutputData, this);
}

/// get the index of a specific id
template<class Parent>
int
PoseidonPushbackLocal<Parent>::IdToIndex (uint32_t id)
{
	bool found = false;
	int idIndex = 35505;

	for(int i = 0; i < ArraySize(FaceID) && found == false; i++)
	{
		if(FaceID[i] == id)
		{
			idIndex = i;
			found = true;
		}
	}

    return idIndex;
}

/// calculate the p-value for one specific face
template<class Parent>
uint32_t
PoseidonPushbackLocal<Parent>::IntervalpSize (int index)
{
    	int entries = 0;
	bool found = false;

 	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (FaceID[index]);

        for(Ptr<pit::Entry> pitEntry = this -> m_pit->Begin ();
            pitEntry != this -> m_pit->End ();
            pitEntry = this -> m_pit->Next (pitEntry))
        {
            ns3::ndn::pit::Entry::in_container inContainer = pitEntry->GetIncoming ();
            for(ns3::ndn::pit::Entry::in_container::iterator initerator = inContainer.begin ();
                initerator != inContainer.end ();
                initerator ++)
            {
                if(face == initerator -> m_face)
                {
                    entries++;
                }
            }
        }
        
    	pSizeL = entries * 100;
    return pSizeL;

}

/// calculate the omega-value for one specific face
template<class Parent>
uint32_t
PoseidonPushbackLocal<Parent>::IntervaloSize (int index)
{
  Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	
	// the o(mega)Size will be calculated
	int currentincomeData = dataInt2[index] - dataInt1[index];
	int currentincomeInterest = interestInt2[index] - interestInt1[index];

	if(currentincomeData == 0)
	{
		currentincomeData = 1;
	}

    	omegaSize = currentincomeInterest/currentincomeData;
	
    return omegaSize;

}

template<class Parent>
LogComponent PoseidonPushbackLocal<Parent>::g_log = LogComponent (PoseidonPushbackLocal<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
PoseidonPushbackLocal<Parent>::GetTypeId (void)
{
  static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PoseidonPushbackLocal").c_str ())
    .SetGroupName ("Ndn")
    .template SetParent <super> ()
    .template AddConstructor <PoseidonPushbackLocal> ()
    ;
  return tid;
}

template<class Parent>
void
PoseidonPushbackLocal<Parent>::DoDispose ()
{  
  super::DoDispose ();
}

template<class Parent>
std::string
PoseidonPushbackLocal<Parent>::GetLogName ()
{
  return super::GetLogName ()+".PoseidonPushbackLocal";
}

template<class Parent>
void
PoseidonPushbackLocal<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),
                                          Seconds (r.GetValue ()), &PoseidonPushbackLocal<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
PoseidonPushbackLocal<Parent>::AddFace (Ptr<Face> face)
{
 	FaceID[cIterator] = face -> GetId ();

 	LogComponentEnable("DEBUG",LOG_INFO);
 	cIterator++;
	
 	super::AddFace (face);
}

/// update the data-values for the current face
template<class Parent>
void
    PoseidonPushbackLocal<Parent>::OnData (Ptr<Face> face,
					    Ptr<Data> data)
{
  	if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	{
	  ConsumerData++;
	}
	
	int odIndex = IdToIndex(face -> GetId ());
	incomeData[odIndex]++;

	if(measureData[odIndex])
	{
		double currentTime = Simulator::Now ().ToDouble (Time::S);

		//set the beginning for the interval
		if(firstMeasureData[odIndex])
		{
			dataInt1[odIndex] = incomeData[odIndex];
			startData = Simulator::Now ().ToDouble (Time::S);
			firstMeasureData[odIndex] = false;
		}

		// if the end of the interval is reached -> set the second value
		if(currentTime > (startData + 0.06))
		{
			dataInt2[odIndex] = incomeData[odIndex];
			measureData[odIndex] = false;
			//firstMeasureData[odIndex] = true;
		}
	}

	if( Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0 && data -> GetSignature () == 0)
	{
		incomeDataRouter++;
	}

  	if (data -> GetSignature () == 0)
  	{
      		super::OnData (face, data);
  	}
}

/// update the interest-values for the current face
template<class Parent>
void
PoseidonPushbackLocal<Parent>::OnInterest (Ptr<Face> face,
                                      Ptr<Interest> interest)
{
    	 if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	 {
	    ConsumerInterests++;
	 }
	 
	int oiIndex = IdToIndex(face -> GetId ());
	incomeInterest[oiIndex]++;
	
	if(first)
	{
	  for (int i = 0; i < ArraySize(peThreshold); i ++)
	  {
		firstMeasureInterest[i] = true;
		firstMeasureData[i] = true;
		measureData[i] = true;
		measureInterests[i] = true;
		omegaThreshold[i] = 3;
		peThreshold[i] = 2000;
	  }
      
	  first = false;
	}

	if(measureInterests[oiIndex])
	{
		double currentTime = Simulator::Now ().ToDouble (Time::S);

		//set the beginning for the interval
		if(firstMeasureInterest[oiIndex])
		{
			interestInt1[oiIndex] = incomeInterest[oiIndex];
			pSizeInt1[oiIndex] = IntervalpSize(oiIndex);
			startInterest = Simulator::Now ().ToDouble (Time::S);
			firstMeasureInterest[oiIndex] = false;
		}

		// if the end of the interval is reached -> set the second value
		if(currentTime > (startInterest + 0.06))
		{
			pSizeInt2[oiIndex] = IntervalpSize(oiIndex);
			interestInt2[oiIndex] = incomeInterest[oiIndex];
			measureInterests[oiIndex] = false;
			firstMeasureInterest[oiIndex] = true;
		}

	}

    	if(limitedFace[oiIndex]){}
    	else
    	{
		super::OnInterest (face, interest);

    	} 
}

/// check whether the values of every face exceed the limits -> decrease the limits 
template<class Parent>
void
PoseidonPushbackLocal<Parent>::AnnounceLimits ()
{
    Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();

    if(firstAnn)
    {
       OutputPitSize();
       //OutputSatisfied();
       //OutputData();
       firstAnn = false;
    }
      
    if(Names::FindName (l3 -> GetFace (0) -> GetNode ()).compare(0, 6, "Router") == 0)
    {

      // for every face the current values will be calculated
      for (uint32_t i = 0;  i < ArraySize(FaceID); i ++)
      {
	int currentIndex = i;
	measureData[currentIndex] = true;
	measureInterests[currentIndex] = true;

        int currentomegaSize = IntervaloSize(currentIndex);
	int currentpSize =  pSizeInt2[currentIndex] - pSizeInt1[currentIndex];
		  
	if(currentpSize < 0)
	{
	  currentpSize = 0;
	}
	
	// if the values exceed the thresholds -> face will be limited	
        if(currentomegaSize > omegaThreshold[currentIndex] && currentpSize > peThreshold[currentIndex])
        {
		omegaThreshold[currentIndex] = omegaThreshold[currentIndex] / scale;
		peThreshold[currentIndex] = peThreshold[currentIndex] / scale;
		limitedFace[currentIndex] = true;
        }
      }
    }

    Simulator::Schedule (Seconds (0.06), &PoseidonPushbackLocal::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::PoseidonPushbackLocal::PerOutFaceLimiterators
template class PerOutFaceLimits< PoseidonPushbackLocal<BestRoute> >;
typedef PerOutFaceLimits< PoseidonPushbackLocal<BestRoute> > PerOutFaceLimitsPoseidonPushbackLocalBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPoseidonPushbackLocalBestRoute);


} // namespace fw
} // namespace ndn
} // namespace ns3
