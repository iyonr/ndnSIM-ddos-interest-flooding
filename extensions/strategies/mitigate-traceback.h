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


#ifndef MITIGATE_TRACEBACK_H
#define MITIGATE_TRACEBACK_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {
 
    /**
 * \ingroup ndn
 * \brief Traceback-Strategy based on article: "Mitigate DDoS Attacks in NDN by Interest Traceback"
 */
template<class Parent>
class MitigateTraceback :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  MitigateTraceback ()
  {
  }

  static std::string
  GetLogName ();
  
  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);

  virtual void
  OnData (Ptr<Face> inFace,
            Ptr<Data> data);
    

  virtual void
  AddFace (Ptr<Face> face);
  
private:
  void
    OutputData();
    
  void
    OutputPitSize();
    
  void
    OutputSatisfied();

  void
    CalculateRate(); 
 
  int
    IdToIndex (uint32_t id);
    
  int
    NameToIndex(std::string name);

  void
    AnnounceLimits ();

protected:

  virtual void
  DoDispose ();
    
  virtual void
  NotifyNewAggregate ();
    
private:

  	static LogComponent g_log;

  	uint32_t m_virtualPayloadSize;

	int maxPrefix[400], faceID[400], iterator = 0, dropIterator[400], currentID;

	int pitSizeInterval1, pitSizeInterval2, pitRateThreshold = 50, pitThreshold = 200, numberAttacker;

	double ConsumerInterests, ConsumerData, lastTimeRouterEdge[400];

	bool first = true, last[400], edge[100];

	std::string prefixe[400], maxPrefix1;
	
	std::vector<std::string> edgeRouter = {"Router2","Router3","Router4","Router5", "Router6", "Router7"};
	
	Ptr<Face> evilFace;
};


} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // MITIGATE_TRACEBACK_H
