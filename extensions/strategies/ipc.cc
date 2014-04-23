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

#include "ipc.h"

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


NS_LOG_COMPONENT_DEFINE("DEBUG_IPC");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

///logs the current PIT-size of every router
template<class Parent>
void
IPC<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &IPC::OutputPitSize, this);
	
}

///logs the size of incoming data of every consumer
template<class Parent>
void
IPC<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	//output only (router)
	if( Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ()) << "\t" << ConsumerData);	
	}
	
	ConsumerData = 0;
	
	Simulator::Schedule (Seconds (0.5), &IPC::OutputData, this);
}

///logs the ratio of data to interests of every consumer
template<class Parent>
void
IPC<Parent>::OutputSatisfied ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{
	    if(ConsumerInterests == 0)
	    {
	      ConsumerInterests = 1;
	    }
	    
	    NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (ConsumerData/ConsumerInterests));
	}
	
	Simulator::Schedule (Seconds (0.5), &IPC::OutputSatisfied, this);
}

///get the index of a specific FaceID
template<class Parent>
int
IPC<Parent>::FaceToIndex (uint32_t id)
{
    	bool found = false;
	int iD = 35505;

	for(int i = 0; i < ArraySize(faceID) && found == false; i++)
	{
		if(faceID[i] == id)
		{
			iD = i;
			found = true;
		}
	}
	
    return iD;
}

///get the max-prefix of every Interface
template<class Parent>
void
IPC<Parent>::GetMaxPrefixes ()
{
      	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);
	
	bool found = false;
	uint32_t currentFaceId = 0;
	uint32_t cId = 0;
	
	for (int i = 0; i < ArraySize(FaceSize); i ++)
	{
		 FaceSize[i] = 0;
	}
	
	for (int i = 0; i < ArraySize(faceID); i ++)
	{
	    cId = faceID[i];
	    
	    //delete all existing entries
	    for (int i = 0; i < ArraySize(prefixSize); i ++)
	    {
		 prefixSize[i] = 0;
		 prefixe[i] = "";
	    }
	    
	    //create a list of all prefixes and their sizes
	    for(Ptr<pit::Entry> pitEntry = this -> m_pit -> Begin ();
		pitEntry != this -> m_pit -> End ();
		pitEntry = this -> m_pit -> Next (pitEntry))
	    {	
		for(ns3::ndn::pit::Entry::in_container::iterator inIterator = pitEntry -> GetIncoming ().begin ();
		    inIterator != pitEntry -> GetIncoming ().end ();
		    inIterator ++)
            	{
                	Ptr<Face> currentFace =  inIterator -> m_face;
			currentFaceId = currentFace -> GetId();
		}
		
		if(currentFaceId == cId)
		{
		  FaceSize[FaceToIndex(cId)]++;
		  std::string prefix = pitEntry -> GetPrefix ().toUri().substr(0,21);
		
		  for(int l = 0; (l < ArraySize(prefixSize) && found == false); l++)
		  {
			if(prefix == prefixe[l])
			{
				prefixSize[l]++;
				found = true;
			}
			
			else if (prefixSize[l] == 0)
			{
				prefixe[l] = prefix;
				prefixSize[l]++;
				found = true;
			}
		  }

		  found = false;
		}
	    }
	    
	    int max = 0;
	    int iter = 0;
	
	    for (int k = 0; k < ArraySize(prefixSize); k ++)
	    {
		if(prefixSize[k] > max)
		{
			iter = k;
			max = prefixSize[k];
		}
	    }
	    
	    maxPrefixe[FaceToIndex(cId)] = prefixe[iter];   
	} 
}


template<class Parent>
LogComponent IPC<Parent>::g_log = LogComponent (IPC<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
IPC<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::IPC").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <IPC> ()
    	;
   return tid;
}

template<class Parent>
void
IPC<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
IPC<Parent>::GetLogName ()
{
   return super::GetLogName ()+".IPC";
}

template<class Parent>
void
IPC<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),Seconds (r.GetValue ()), &IPC<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
IPC<Parent>::AddFace (Ptr<Face> face)
{
	if(first)
	{
	  	for (int i = 0; i < ArraySize(faceID); i ++)
		{
		  faceID[i] = 9005;
		}
		first = false;
	}
	Cface[faceIt] = face;
   	faceID[faceIt] = face -> GetId();
 	LogComponentEnable("DEBUG_IPC",LOG_INFO);
 	super::AddFace (face);
	faceIt++;

}

///check whether the incoming data-packet is a IPCReport (forward it) or a normal data-packet
template<class Parent>
void IPC<Parent>::OnData (Ptr<Face> face,
                         Ptr<Data> data)
{
    bool foundPrefix = false;
    Ptr<Face> evilFace;
  
  if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
  {
    ConsumerData++;
  }
  
  if(data -> GetSignature () == 539047 && Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
  {
	std::string currentPrefix = data -> GetName ().toUri().substr(10,21) + "/";
	//NS_LOG_INFO(Names::FindName (face -> GetNode ())<<": " << "\t" << currentPrefix);

	for(Ptr<pit::Entry> pitEntry = this -> m_pit -> Begin ();
           pitEntry != this -> m_pit -> End () && foundPrefix == false;
            pitEntry = this -> m_pit -> Next (pitEntry))
        {
		std::string pitPrefix = pitEntry -> GetPrefix ().toUri().substr(0,21);

		if(pitPrefix == currentPrefix)
		{
			foundPrefix = true;
				
			ns3::ndn::pit::Entry::in_container inContainerPrefix = pitEntry -> GetIncoming ();
			evilFace = inContainerPrefix.begin () -> m_face;
			
			if (FaceSize[FaceToIndex(evilFace -> GetId())] > 500)
			{
			  evilFaces[evilIt] = evilFace -> GetId();
			  evilPrefixes[evilIt] = currentPrefix;
			  evilIt++;
			}
				
			//NS_LOG_INFO("transmitted face blacklisted");
		}
	}

	if(foundPrefix)
	{
		Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
		data->SetName (Create<Name> ("/IPCReport" + currentPrefix));
		data->SetFreshness (Seconds(0));
        	data->SetSignature (539047); 
        	bool ok = evilFace->SendData (data);
		if(ok)
		{
			//NS_LOG_INFO("IPCReport forwarded at: Interface " << evilFace -> GetId() << " with Prefix " << currentPrefix  << "\n");
	    	}
	}
  }
  else
  {
    	super::OnData (face, data);
  }
}

///check whether an interest prefix is blacklisted -> drop the interest or forward it whether its normal
template<class Parent>
void
IPC<Parent>::OnInterest (Ptr<Face> face,
                         Ptr<Interest> interest)
{
    bool found = false, foundFace = false;
    std::string evilPrefix = "", currentPrefix = interest -> GetName().toUri().substr(0,21);
    
    //output only
    if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
    {
      ConsumerInterests++;
    }
  
    for (int i = 0; i < ArraySize(evilPrefixes) && found == false; i ++)
    {
	if(evilPrefixes[i] == currentPrefix)
	{
	  if(evilFaces[i] == face -> GetId())
	  {
	    evilPrefix = currentPrefix;
	    foundFace = true;
	    found = true;
	  }
	}
    }
     
    if(evilPrefix != "" && foundFace){}
    else
    {
	super::OnInterest (face, interest);
    }
}

///if size of a face reaches the threshold, get the max-prefix of this face and send a IPCReport
template<class Parent>
void
IPC<Parent>::AnnounceLimits ()
{
    	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);
	
	uint32_t maxPrefixIndex = 0;
	bool foundSize = false;
	std::string maxPrefix;
	int maxSize = 0;
	
	if(firstAnn)
	{
	  OutputPitSize();
	  //OutputSatisfied();
	  //OutputData();
	
	  firstAnn = false;
	}	
	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  GetMaxPrefixes();

	  for (int i = 0; i < ArraySize(faceID); i ++)
	  {

	    if(FaceSize[i] > 500)
	    {
		maxPrefix = maxPrefixe[i]; 

		evilFaces[evilIt] = faceID[i];
		evilPrefixes[evilIt] = maxPrefix;
		evilIt++;
		
	     	Ptr<Data> data = Create<Data> (Create<Packet> (m_virtualPayloadSize));
		data->SetName (Create<Name> ("/IPCReport" + maxPrefix));
        	data->SetTimestamp (Simulator::Now ());
		data->SetFreshness (Seconds(0));
        	data->SetSignature (539047); 
		bool ok = Cface[i]->SendData (data);
		if(ok)
		{
		  //NS_LOG_INFO(Names::FindName (face -> GetNode ()) << " " "IPCReport sent at: Interface " << faceID[i] << " with Prefix " << maxPrefix  << "\n");
		}
	    }
	  }
      }
      
      Simulator::Schedule (Seconds (0.5), &IPC::AnnounceLimits, this);
}

} //namespace fw
} //namespace ndn
} //namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

//ns3::ndn::fw::BestRoute::Stats::IPC::PerOutFaceLimits
template class PerOutFaceLimits< IPC<BestRoute> >;
typedef PerOutFaceLimits< IPC<BestRoute> > PerOutFaceLimitsIPCBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsIPCBestRoute);


} //namespace fw
} //namespace ndn
} //namespace ns3
