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


#ifndef DDOS_PUSHBACK_H
#define DDOS_PUSHBACK_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {
  
  /**
 * \ingroup ndn
 * \brief DDoS-Strategy based on article: "DoS & DDoS in Named-Data Networking"
 */

template<class Parent>
class DDoSPushback :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  DDoSPushback ()
  {
  }

  static std::string
  GetLogName ();
  
  virtual void
  OnInterest (Ptr<Face> face,
              Ptr<Interest> interest);

  virtual void
  OnData (Ptr<Face> face,
          Ptr<Data> data);
    
  virtual void
  AddFace (Ptr<Face> face);
  
private:

  void
  OutputPitSize ();
  
  void
  OutputSatisfied ();
  
  void
  OutputData ();

  bool
  PrefixOnEvilPrefixes(std::string prefix); 

  int
  IdToIndex (uint32_t id);

  int
  EvilPrefixToIndex (std::string prefix);
  
  void
  GetPrefixSize ();

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
	
	bool first = true;
	
	double ConsumerInterests, ConsumerData;

	int  c, faceIterator = 0, prefixSize[100], faceid[100], prefixThreshold[100];

	std::string prefix[100], evilPrefixes[100];

	Ptr<Face> evilPrefixFace, evilFace;
	
	Name exactEntry[100];
};


} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // DDOS_PUSHBACK_H
