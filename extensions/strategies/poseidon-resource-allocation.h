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



#ifndef POSEIDON_RESOURCE_ALLOCATION_H
#define POSEIDON_RESOURCE_ALLOCATION_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {

    /**
 * \ingroup ndn
 * \brief Poseidon-Resource-Allocation-Strategy based on article: "Poseidon: Mitigating Interest Flooding DDoS Attacks in Named Data Networking"
 */
template<class Parent>
class PoseidonResourceAllocation :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  PoseidonResourceAllocation ()
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
  
  virtual void 
  DidSendOutInterest (Ptr< Face > inFace,
		      Ptr< Face > outFace,
		      Ptr< const Interest > interest, 
		      Ptr< pit::Entry > pitEntry);
private:

  virtual void
  OutputPITSize ();
  
    virtual void
  OutputSatisfied ();

  virtual void
  OutputData ();

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
  
  bool first=true;
  
  double ConsumerInterests, ConsumerData;

};

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // POSEIDON_RESOURCE_ALLOCATION_H
