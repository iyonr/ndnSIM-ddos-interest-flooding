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



#ifndef TDM_H
#define TDM_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {

 /**
 * \ingroup ndn
 * \brief TDM-Strategy based on article: "Detecting and mitigating interest flooding attacks in content-centric network"
 */
    
template<class Parent>
class TDM :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  TDM ()
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
  WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry);
      
  virtual void
  AddFace (Ptr<Face> face);
  
private:

  void
  UpdatePrefixList();

  int
  PrefixToIndex (Name prefix);

  int
  NonFibToIndex (Name prefix);

  Name
  PrefixToMainPrefix (Name prefix);

  virtual void
  OutputPitSize ();
  
  virtual void
  OutputData ();
  
  virtual void
  OutputSatisfied ();

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

  double capacity[400], ConsumerData, ConsumerInterests;

  int fibIt = 101, fs[400], fth[400], ci[400], cth[400], cth2[400], dropIt[400];

  bool first = true, erased[400], mode[400],limited[400], satisfied[400], expired[400];

  Name prefixList[100], nonExistingFibEntry[100];

};


} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // TDM_H
