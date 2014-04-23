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


#ifndef IPC_H
#define IPC_H

#include <ns3/event-id.h>

#include "stats.h"

namespace ns3 {
namespace ndn {
namespace fw {

    /**
 * \ingroup ndn
 * \brief Interface- and Prefix-based Countermeasure-Strategy based on own implementation: 
 */
    
template<class Parent>
class IPC :
    public Stats<Parent>
{
private:
  typedef Stats<Parent> super;

public:
    
  static TypeId
  GetTypeId ();

  IPC ()
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
  GetMaxPrefixes ();
  
  int
  FaceToIndex (uint32_t id);

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

  uint32_t m_virtualPayloadSize, faceID[100], prefixID[100], evilFaces[100];

  double prefixSize[100] , ConsumerData, ConsumerInterests;

  int faceIt = 0, evilIt, FaceSize[100];

  bool first = true, firstAnn = true, limited[100];
  
  Ptr<Face> Cface[100];
  
  std::string prefixe[100], evilPrefixes[100], maxPrefixe[100];


};


} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // IPC_H
