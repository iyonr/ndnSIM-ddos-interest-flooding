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


#ifndef POSEIDON_PUSHBACK_DISTRIBUTED_H
#define POSEIDON_PUSHBACK_DISTRIBUTED_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {

  /**
 * \ingroup ndn
 * \brief Poseidon-Pushback-Distributed-Strategy based on article: "Poseidon: Mitigating Interest Flooding DDoS Attacks in Named Data Networking"
 */
template<class Parent>
class PoseidonPushbackDistributed :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  PoseidonPushbackDistributed ()
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

  int
  IdToIndex (uint32_t id);

  uint32_t
  IntervalpSize (int index);

  uint32_t
  IntervaloSize (int index);

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

  uint32_t m_virtualPayloadSize, faceID[20];

  bool first = true, firstInterest = true, incAlert[20], limited[20], firstMeasureDataDis[20], firstMeasureInterestDis[20], measureDataDis[20], measureInterestsDis[20];

  double lastincAlert[20], lastAlert = 0.29 ,wait_time = 0.06, startInterestDis, startDataDis, oThreshold[20], pThreshold[20], ConsumerInterests, ConsumerData;

  int entries, it = 0, scaling = 2, pSize, oSize, incomeInterestInt1[20],incomeInterestInt2[20] , incomeDataInt1[20], incomeDataInt2[20];
 		
  int pitSizeInt1[20], pitSizeInt2[20], dropit[20], incomingData[20], incomingInterest[20], incomingDataRouter[20];

  std::string limitedPrefix[20];

};

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // POSEIDON_PUSHBACK_DISTRIBUTED_H
