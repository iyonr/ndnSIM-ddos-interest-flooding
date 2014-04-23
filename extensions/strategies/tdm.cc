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

#include "tdm.h"

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


NS_LOG_COMPONENT_DEFINE("DEBUG_TDM");
namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {
namespace fw {

/// logs the current PIT-size of every router
template<class Parent>
void
TDM<Parent>::OutputPitSize ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if(Names::FindName (face -> GetNode ()).compare (0, 6, "Router") == 0)
	{
	  NS_LOG_INFO(Names::FindName (face -> GetNode ()) << "\t" << (this -> m_pit -> GetSize ()));
	}
	
	Simulator::Schedule (Seconds (0.5), &TDM::OutputPitSize, this);
	
}

/// logs the size of incoming data of every consumer
template<class Parent>
void
TDM<Parent>::OutputData ()
{
	Ptr<L3Protocol> l3 = this->template GetObject<L3Protocol> ();
	Ptr<Face> face = l3->GetFace (0);

	if( Names::FindName (face -> GetNode ()).compare (0, 13, "good-Consumer") == 0)
	{
		NS_LOG_INFO("\t" <<  Names::FindName (face -> GetNode ()) << "\t" << ConsumerData);	
	}
	
	ConsumerData = 0;
	
	Simulator::Schedule (Seconds (0.5), &TDM::OutputData, this);
}

/// logs the ratio of data to interests of every consumer
template<class Parent>
void
TDM<Parent>::OutputSatisfied ()
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
	
	Simulator::Schedule (Seconds (0.5), &TDM::OutputSatisfied, this);
}

/// returns the main part of a prefix 
template<class Parent>
Name
TDM<Parent>::PrefixToMainPrefix (Name prefix)
{
	if(prefix.toUri().substr(20,1) == "/")
	{
		prefix = Name(prefix.toUri ().substr(0,prefix.toUri ().size() - (prefix.toUri ().size() - 20)));
	}
	else
	{
	    if(prefix.toUri().substr(21,1) == "/")
	    {
	      prefix = Name(prefix.toUri ().substr(0,prefix.toUri ().size() - (prefix.toUri ().size() - 21)));
	    }
	    else if(prefix.toUri().substr(22,1) == "/")
	    {
		prefix = Name(prefix.toUri ().substr(0,prefix.toUri ().size() - (prefix.toUri ().size() - 22)));
	    }
	}
	
     return prefix.toUri();

}

/// update "prefixList" - so every prefix in the PIT is also in "prefixList"
template<class Parent>
void
TDM<Parent>::UpdatePrefixList ()
{
	bool found = false;
	Name fibPrefix;

	//delete all not-existing entries in PrefixList 
	for(int i = 0; i <  ArraySize(prefixList); i++)
	{
		if(this -> m_fib -> Find(prefixList[i]) == this -> m_fib -> End ())
		{			
			if(prefixList[i].toUri () == "//"){}
			if(prefixList[i].toUri() == "/")
			{
			  prefixList[i] = Name().append("/");
			}
		}
	}

	// insert all existing entries in PrefixList
	for(Ptr<fib::Entry> fibEntry = this -> m_fib->Begin ();
            fibEntry != this -> m_fib->End ();
            fibEntry = this -> m_fib->Next (fibEntry))
        {
	 	fibPrefix = fibEntry -> GetPrefix ();

            	for(int i = 0; i < ArraySize(prefixList) && found == false; i++)
            	{
                	if(fibPrefix == prefixList[i])
                	{
                    		found = true;
                	}
		
		if(prefixList[i] == Name().append("/"))
                {
                    prefixList[i] = fibPrefix;
                    found = true;
                }
            }

	    found = false;
	}
	
    	Simulator::Schedule (Seconds (0.05), &TDM::UpdatePrefixList, this);  
}

/// get the index of a specific prefix
template<class Parent>
int
TDM<Parent>::PrefixToIndex (Name prefix)
{
    	bool found = false;
	int prefixIndex = 35505;

	for(int i = 0; i < ArraySize(prefixList) && found == false; i++)
	{
		if(prefixList[i] == prefix)
		{
			prefixIndex = i;
			found = true;
		}
	}
 
    return prefixIndex;
}

/// get the index of a specific non-FIB prefix
template<class Parent>
int
TDM<Parent>::NonFibToIndex (Name prefix)
{
    	bool found = false;
	int prefixIndex = 35505;

	for(int i = 0; i < ArraySize(nonExistingFibEntry) && found == false; i++)
	{
		if(nonExistingFibEntry[i] == prefix)
		{
			prefixIndex = i;
			found = true;
		}
	}

    return prefixIndex + (ArraySize(nonExistingFibEntry) + 1);
}

template<class Parent>
LogComponent TDM<Parent>::g_log = LogComponent (TDM<Parent>::GetLogName ().c_str ());

template<class Parent>
TypeId
TDM<Parent>::GetTypeId (void)
{
  	static TypeId tid = TypeId ((super::GetTypeId ().GetName ()+"::TDM").c_str ())
    	.SetGroupName ("Ndn")
    	.template SetParent <super> ()
    	.template AddConstructor <TDM> ()
    	;
   return tid;
}

template<class Parent>
void
TDM<Parent>::DoDispose ()
{  
  	super::DoDispose ();
}

template<class Parent>
std::string
TDM<Parent>::GetLogName ()
{
   return super::GetLogName ()+".TDM";
}

template<class Parent>
void
TDM<Parent>::NotifyNewAggregate ()
{
  super::NotifyNewAggregate ();

      if (this->m_pit != 0 && this->m_fib != 0 && this->template GetObject<Node> () != 0)
        {
          UniformVariable r (0,1);
          Simulator::ScheduleWithContext (this->template GetObject<Node> ()->GetId (),Seconds (r.GetValue ()), &TDM<Parent>::AnnounceLimits, this);
        }
}

template<class Parent>
void
TDM<Parent>::AddFace (Ptr<Face> face)
{
 	LogComponentEnable("DEBUG_TDM",LOG_INFO);
 	super::AddFace (face);
}

//set the parameters for the current prefix
template<class Parent>
void
TDM<Parent>::OnData (Ptr<Face> face,
                     Ptr<Data> data)
{
    if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
    {
	ConsumerData++;
    }
     
    Name currentPrefix = PrefixToMainPrefix(data -> GetName ());
    int currentPrefixIndex = PrefixToIndex(currentPrefix);

    if(currentPrefixIndex == 35505)
    {
	currentPrefixIndex = NonFibToIndex(currentPrefix);
    }

    erased[currentPrefixIndex] = false;
    satisfied[currentPrefixIndex] = true;

    super::OnData (face, data);
}

/// while a Timeout for a pitEntry -> set the parameters for the current prefix
template<class Parent>
void
TDM<Parent>::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
    Name currentPrefix = PrefixToMainPrefix(pitEntry -> GetPrefix ());
    int currentPrefixIndex = PrefixToIndex(currentPrefix);

    if(currentPrefixIndex == 35505)
    {
	currentPrefixIndex = NonFibToIndex(currentPrefix);
    }

    satisfied[currentPrefixIndex] = false;
    erased[currentPrefixIndex] = true;
    super::WillEraseTimedOutPendingInterest (pitEntry);
}

/// check all parameters for the incoming interest and act accordingly
template<class Parent>
void
TDM<Parent>::OnInterest (Ptr<Face> face,
                         Ptr<Interest> interest)
{
    if(Names::FindName (face -> GetNode ()).compare (5, 8, "Consumer") == 0)
  {
    ConsumerInterests++;
  }
	Name currentPrefix = PrefixToMainPrefix(interest -> GetName ());
	int currentPrefixIndex = PrefixToIndex(currentPrefix);
	
	if(currentPrefixIndex == 35505)
	{
		if(fibIt == ArraySize(nonExistingFibEntry) * 2)
		{
			fibIt = ArraySize(nonExistingFibEntry) + 1;
		}

		if(NonFibToIndex(currentPrefix) < ArraySize(nonExistingFibEntry) * 2)
		{
			currentPrefixIndex = NonFibToIndex(currentPrefix);
		}

		else
		{
			nonExistingFibEntry[fibIt - (ArraySize(nonExistingFibEntry) + 1)] = currentPrefix;
			currentPrefixIndex = NonFibToIndex(currentPrefix);
		}

		fibIt++;
	}

        // if the capacity mechanism for the incoming prefix is enabled -> decrease the interest rate to capacity
        if(limited[currentPrefixIndex])
        {
		capacity[currentPrefixIndex] = 0.5 * capacity[currentPrefixIndex];

            	if (dropIt[currentPrefixIndex] == 100)
            	{
                	dropIt[currentPrefixIndex] = 0;
            	}
            
            	if(dropIt[currentPrefixIndex] > capacity[currentPrefixIndex] -1)
            	{
                	dropIt[currentPrefixIndex]++;
            	}
            
            	else
            	{
                	dropIt[currentPrefixIndex]++;
               
                	// OI arrives
                	if(satisfied[currentPrefixIndex] && erased[currentPrefixIndex] == false)
                	{
                    		if(mode[currentPrefixIndex] == true)
                    		{
					ci[currentPrefixIndex] = 1;
                       			mode[currentPrefixIndex] = false;
                    		}

                    		else
                    		{
					ci[currentPrefixIndex]++;
                    		}
                	}
                
                	// MI arrives
                	else if(satisfied[currentPrefixIndex] == false && erased[currentPrefixIndex])
                	{
                    		if(mode[currentPrefixIndex] == false)
                    		{
					ci[currentPrefixIndex] = 1;
                        		mode[currentPrefixIndex] = true;
                    		}
                    	
				else
                    		{
					ci[currentPrefixIndex]++;
                    		}
				
				if(ci[currentPrefixIndex] >= cth[currentPrefixIndex])
				{
					fs[currentPrefixIndex] = 0;
					limited[currentPrefixIndex] = true;
				}
                	}

                	super::OnInterest (face, interest);
            	}
        }
        
        // if the incoming interest-prefix has already an fibentry and capacity mechanism is deactivied
        else
        {
		//OI arrives
            	if(satisfied[currentPrefixIndex] == true && erased[currentPrefixIndex] == false)
            	{
                	if(mode[currentPrefixIndex] == true)
                	{
                    		ci[currentPrefixIndex] = 1;
                    		mode[currentPrefixIndex] = false;
                	}

                	else
                	{
                    		ci[currentPrefixIndex]++;
                	}

			if(ci[currentPrefixIndex] >= cth[currentPrefixIndex])
			{
				fs[currentPrefixIndex]++;

				if(fs[currentPrefixIndex] >= fth[currentPrefixIndex])
				{
					limited[currentPrefixIndex] = false;
				}
			}
            	}
            
		//MI arrives
            	else if (satisfied[currentPrefixIndex] == false && erased[currentPrefixIndex] == true)
            	{
                	if(mode[currentPrefixIndex] == false)
                	{
                    		ci[currentPrefixIndex] = 1;
                    		mode[currentPrefixIndex] = true;
                	}

                	else
                	{
              			ci[currentPrefixIndex]++;
                	}

			if(ci[currentPrefixIndex] >= cth[currentPrefixIndex])
			{
				fs[currentPrefixIndex] = 0;
				limited[currentPrefixIndex] = true;
			}
            	}

            	super::OnInterest (face, interest);	
        }
}

/// set some intial parameters and update the "prefixList" for the first time
template<class Parent>
void
TDM<Parent>::AnnounceLimits ()
{
	if(first)
	{
	  	UpdatePrefixList();
		
		for(int i = 0; i < ArraySize(prefixList); i++)
		{
			nonExistingFibEntry[i] = Name().append("/"); prefixList[i] = Name().append("/");
			limited[i] = false; capacity[i] = 50; satisfied[i] = true; mode[i] = false;
			erased[i] = false; cth[i] = 3; fth[i] = 2; fs[i] = 6;	
		}
		for(int i = 20; i < ArraySize(nonExistingFibEntry) * 2; i++)
		{
			 limited[i] = false; capacity[i] = 50; satisfied[i] = true; mode[i] = false;
			 erased[i] = false; cth[i] = 3;  fth[i] = 2; fs[i] = 6;	 	
		}

		first = false;
		
		OutputPitSize();
		//OutputSatisfied();
		//OutputData();

	}

	Simulator::Schedule (Seconds (0.5), &TDM::AnnounceLimits, this);
}

} // namespace fw
} // namespace ndn
} // namespace ns3


#include <ns3/ndnSIM/model/fw/per-out-face-limits.h>
#include <ns3/ndnSIM/model/fw/best-route.h>

namespace ns3 {
namespace ndn {
namespace fw {

// ns3::ndn::fw::BestRoute::Stats::TDM::PerOutFaceLimits
template class PerOutFaceLimits< TDM<BestRoute> >;
typedef PerOutFaceLimits< TDM<BestRoute> > PerOutFaceLimitsTDMBestRoute;
NS_OBJECT_ENSURE_REGISTERED(PerOutFaceLimitsTDMBestRoute);


} // namespace fw
} // namespace ndn
} // namespace ns3
