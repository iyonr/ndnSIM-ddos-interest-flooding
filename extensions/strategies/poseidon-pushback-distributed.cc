/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2014 Freie Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Samir Al-Sheikh <samir.al-sheikh@fu-berlin.de>
 */


#define ArraySize(x)  (sizeof(x) / sizeof(x[0]))

#include "poseidon-pushback-distributed.h"

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

NS_LOG_COMPONENT_DEFINE("DEBUG_DISTRIBUTED");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
PoseidonPushbackDistributed<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if( Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ())  << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackDistributed::OutputPitSize, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
PoseidonPushbackDistributed<Parent>::OutputSatisfied ()
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
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackDistributed::OutputSatisfied, this);
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
PoseidonPushbackDistributed<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{   
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> ConsumerData));
	    
	    ConsumerData = 0;
	}
	
	Simulator::Schedule (Seconds (0.5), &PoseidonPushbackDistributed::OutputData, this);
}

/// get the index of a specific id
template<class Parent>
int
PoseidonPushbackDistributed<Parent>::IdToIndex (uint32_t id)
{
	bool found = false;
	int idIndex = 35505;

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

/// calculate the p-value for one specific face
template<class Parent>
uint32_t
PoseidonPushbackDistributed<Parent>::IntervalpSize (int index)
{
    	int entries = 0;
	bool found = false;

 	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (faceID[index]);

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

    	pSize = entries * 100;

    return pSize;

}

/// calculate the omega-value for one specific face
template<class Parent>
uint32_t
PoseidonPushbackDistributed<Parent>::IntervaloSize (int index)
{	
	int currentincomeData = incomeDataInt2[index] - incomeDataInt1[index];
	int currentincomeInterest = incomeInterestInt2[index] - incomeInterestInt1[index];

	if(currentincomeData == 0)
	{
		currentincomeData = 1;
	}

    	oSize = currentincomeInterest/currentincomeData;
	
    return oSize;

}

template<class Parent>
LogComponent PoseidonPushbackDistributed<Parent>::g_log = LogComponent (PoseidonPushbackDistributed<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
PoseidonPushbackDistributed<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::PoseidonPushbackDistributed").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <PoseidonPushbackDistributed> ()
    	;
   return tid;
}

template<class Parent>
void
PoseidonPushbackDistributed<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
PoseidonPushbackDistributed<Parent>::GetLogName ()
{
   return super::GetLogName ()+".PoseidonPushbackDistributed";
}

template<class Parent>
void
PoseidonPushbackDistributed<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),
                                          Seconds (r.GetValue ()), &PoseidonPushbackDistributed<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
PoseidonPushbackDistributed<Parent>::AddFace (Ptr<Face> face)
{
 	faceID[it] = face -> GetId();

 	LogComponentEnable("DEBUG_DISTRIBUTED",LOG_INFO);

 	super::AddFace (face);
 	it++;
}

/// if an alert packet arrives on a router and the last alert is 60ms ago -> thresholds will be decreased
/// update the data-values for the current face
template<class Parent>
void
    PoseidonPushbackDistributed<Parent>::OnData (Ptr<Face> face,
                                      		 Ptr<Data> data)
{
    	if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	{
	  ConsumerData++;
	}
	
	int odIndex = IdToIndex(face -> GetId ());
	  

  	if (data -> GetSignature () == 12345 && (lastAlert < Simulator::Now ().ToDouble (Time::S) - wait_time)
	    && Names::FindName (face -> GetNode ()).compare(0 ,6 ,"Router") == 0)
  	{

			for (uint32_t i = 0; i < ArraySize(faceID); i ++)
			{
			  int currentIndex = i;	
			  
			  if(IntervaloSize(currentIndex) > 3)
			  {
			    oThreshold[currentIndex] = oThreshold[currentIndex] * 0.2;
			    pThreshold[currentIndex] = pThreshold[currentIndex] * 0.2;
			  }
			  
			  else if(2.5 < IntervaloSize(currentIndex) <= 3)
			  {
			    oThreshold[currentIndex] = oThreshold[currentIndex] * 0.4;
			    pThreshold[currentIndex] = pThreshold[currentIndex] * 0.4;
			  }
			  
			  else if(2.0 < IntervaloSize(currentIndex) <= 2.5)
			  {
			    oThreshold[currentIndex] = oThreshold[currentIndex] * 0.6;
			    pThreshold[currentIndex] = pThreshold[currentIndex] * 0.6;
			  }
			  
			  else if(1.5 <= IntervaloSize(currentIndex) <= 2)
			  {
			    oThreshold[currentIndex] = oThreshold[currentIndex] * 0.8;
			    pThreshold[currentIndex] = pThreshold[currentIndex] * 0.8;
			  }
			  
			}

			lastincAlert[odIndex] = Simulator::Now ().ToDouble (Time::S);
  	}
		
  	if (data -> GetSignature () == 12345 && lastAlert >= Seconds(Simulator::Now ().ToDouble (Time::S)) - wait_time)
	{
		// drop the alert
	}
    
  	if (data -> GetSignature () == 0)
  	{
		int nodIndex = IdToIndex(face -> GetId ());

		incomingData[nodIndex]++;

		if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
		{
			incomingDataRouter[boost::lexical_cast<int>( Names::FindName (face -> GetNode ()).substr(6,1) )]++;
		}
	
		if(measureDataDis[nodIndex])
		{
			double currentTime = Simulator::Now ().ToDouble (Time::S);

			//set the beginning for the interval
			if(firstMeasureDataDis[nodIndex])
			{
				incomeDataInt1[nodIndex] = incomingData[nodIndex];
				startDataDis = Simulator::Now ().ToDouble (Time::S);
				firstMeasureDataDis[nodIndex] = false;
			}

			// if the end of the interval is reached -> set the second value
			if(currentTime > (startDataDis + 0.06))
			{
				incomeDataInt2[nodIndex] = incomingData[nodIndex];
				measureDataDis[nodIndex] = false;
			}
		}

      		super::OnData (face, data);
  	}
}

/// update the interest-values for the current face
template<class Parent>
void
PoseidonPushbackDistributed<Parent>::OnInterest (Ptr<Face> face,
                                      Ptr<Interest> interest)
{

      	 if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
	 {
	    ConsumerInterests++;
	 }
	 
	int oiIndex = IdToIndex(face -> GetId ());
	incomingInterest[oiIndex]++;
	
	if ((lastincAlert[oiIndex] < (Seconds(Simulator::Now ().ToDouble (Time::S) - wait_time))) && incAlert[oiIndex] == true)
	{
	    if(oThreshold[oiIndex] > 3)
	    {
	      oThreshold[oiIndex] = 3;
	    }
	    
	    if(pThreshold[oiIndex] > 5000)
	    {
	      pThreshold[oiIndex] = 5000;
	    }
	    
	    if(oThreshold[oiIndex] == 3 && pThreshold[oiIndex] == 5000)
	    {
	      limited[oiIndex] = false;
	    }
	    if(oThreshold[oiIndex] <= 3 && pThreshold[oiIndex] <= 5000)
	    {
		oThreshold[oiIndex] = oThreshold[oiIndex] + (oThreshold[oiIndex]/8);
		pThreshold[oiIndex] = pThreshold[oiIndex] + (pThreshold[oiIndex]/8);
	    }
	}
	
	if(firstInterest)
	{
	  for (int i = 0; i < ArraySize(pThreshold); i ++)
	  {
		pThreshold[i] = 3000;
		oThreshold[i] = 3;
		measureDataDis[i] = true;
		measureInterestsDis[i] = true;
		firstMeasureInterestDis[i] = true;
		firstMeasureDataDis[i] = true;
	  }
	  	firstInterest = false;
	}

	if(measureInterestsDis[oiIndex])
	{
		double currentTime = Simulator::Now ().ToDouble (Time::S);

		//set the beginning for the interval
		if(firstMeasureInterestDis[oiIndex])
		{
			incomeInterestInt1[oiIndex] = incomingInterest[oiIndex];
			pitSizeInt1[oiIndex] = IntervalpSize(oiIndex);
			startInterestDis = Simulator::Now ().ToDouble (Time::S);
			firstMeasureInterestDis[oiIndex] = false;
		}
		
		// if the end of the interval is reached -> set the second value
		if(currentTime > (startInterestDis + 0.06))
		{
			pitSizeInt2[oiIndex] = IntervalpSize(oiIndex);
			incomeInterestInt2[oiIndex] = incomingInterest[oiIndex];
			measureInterestsDis[oiIndex] = false;
			firstMeasureInterestDis[oiIndex] = true;
		}
	}
	
    	if(limited[oiIndex]){}
    	else
    	{	
	    super::OnInterest (face, interest);
    	} 
}

/// check whether the values of every face exceed the limits -> alert will be generated 
template<class Parent>
void
PoseidonPushbackDistributed<Parent>::AnnounceLimits ()
{
    Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();

    if(first)
    {
            OutputPitSize ();
            //OutputSatisfied();
	    //OutputData();
	    first = false;
    }
    
    if(Names::FindName (l3 -> GetFace (0) -> GetNode ()).compare(0, 6, "Router") == 0)
    {
      for (uint32_t i = 0; i < ArraySize(faceID); i ++)
      {
	int currentIndex = i;
	Ptr<Face> inFace;
	
	measureDataDis[currentIndex] = true;
    	measureInterestsDis[currentIndex] = true;

        int currentoSize = IntervaloSize(currentIndex);
	int currentpSize =  pitSizeInt2[currentIndex] - pitSizeInt1[currentIndex];
	
	if(currentpSize < 0)
	{
	  currentpSize = 0;
	}

        if(currentoSize > oThreshold[currentIndex] && currentpSize > pThreshold[currentIndex])
        {
            if(lastAlert < (Seconds(Simulator::Now ().ToDouble (Time::S) - wait_time)))
            {
		inFace = l3 -> GetFace(i);
		lastAlert = Simulator::Now ().ToDouble (Time::S);
		limited[currentIndex] = true;

		Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
            	data->SetName (Create<Name> ("/pushback/alerts"));
            	data->SetTimestamp (Simulator::Now ());
            	data->SetFreshness (Seconds(0));
            	data->SetSignature (12345);
            	inFace->SendData (data);
            }
	 }
      }
    }

    Simulator::Schedule (Seconds (0.06), &PoseidonPushbackDistributed::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::PoseidonPushbackDistributed::PerOutFaceLimits
template class PerOutFaceLimits< PoseidonPushbackDistributed<BestRoute> >;
typedef PerOutFaceLimits< PoseidonPushbackDistributed<BestRoute> > PerOutFaceLimitsPoseidonPushbackDistributedBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsPoseidonPushbackDistributedBestRoute);


} // namespace fw
} // namespace ndn
} // namespace ns3
