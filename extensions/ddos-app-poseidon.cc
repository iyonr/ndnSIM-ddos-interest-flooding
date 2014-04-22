/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 PARC
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ddos-app-poseidon.h"
#include <ns3/point-to-point-module.h>

NS_LOG_COMPONENT_DEFINE ("DdosAppPoseidon");

NS_OBJECT_ENSURE_REGISTERED (DdosAppPoseidon);

TypeId
DdosAppPoseidon::GetTypeId ()
{
  static TypeId tid = TypeId ("DdosAppPoseidon")
    .SetParent<App> ()
    .AddConstructor<DdosAppPoseidon> ()

    .AddAttribute ("AvgGap", "AverageGap",
		   StringValue ("10ms"),
                   MakeTimeAccessor (&DdosAppPoseidon::m_avgGap),
                   MakeTimeChecker ())
    .AddAttribute ("Prefix","Name of the Interest",
                   StringValue ("/"),
                   MakeNameAccessor (&DdosAppPoseidon::m_prefix),
                   MakeNameChecker ())
    .AddAttribute ("LifeTime","Interest lifetime",
                   StringValue ("1s"),
                   MakeTimeAccessor (&DdosAppPoseidon::m_lifetime),
                   MakeTimeChecker ())
    .AddAttribute ("Evil", "Evil bit",
                   BooleanValue (false),
                   MakeBooleanAccessor (&DdosAppPoseidon::m_evilBit),
                   MakeBooleanChecker ())
    .AddAttribute ("DataBasedLimits", "Calculate frequency based on how many data packets can be returned",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DdosAppPoseidon::m_dataBasedLimit),
                   MakeBooleanChecker ())
    ;
  return tid;
}

DdosAppPoseidon::DdosAppPoseidon ()
  : m_rand (0, std::numeric_limits<uint32_t>::max ())
  , m_jitter (0,1)
  , m_seq (0)
{
}

DdosAppPoseidon::~DdosAppPoseidon ()
{
}

void
DdosAppPoseidon::OnNack (const Ptr<const Interest> &interest)
{
  // immediately send new packet, without taking into account periodicity
  // m_nextSendEvent.Cancel ();
  // m_nextSendEvent = Simulator::Schedule (Seconds (0.001),
  //       				 &DdosAppPoseidon::SendPacket, this);
}

void
DdosAppPoseidon::OnData (const Ptr<const Data> &data)
{
  // who cares
}

void
DdosAppPoseidon::SendPacket ()
{
  timer++;
  m_seq++;
  // send packet
  //NS_LOG_INFO(m_seq);
  //NS_LOG_INFO(m_prefix);

  Ptr<NameComponents> nameWithSequence = Create<NameComponents> (m_prefix);
  nameWithSequence->appendSeqNum (m_seq);

  Ptr<Interest> interest = Create<Interest> ();
  interest->SetNonce            (m_rand.GetValue ());
  interest->SetName             (nameWithSequence);
  interest->SetInterestLifetime (m_lifetime);
        
  // NS_LOG_INFO ("\n > Interest for " << m_seq << ", lifetime " << m_lifetime.ToDouble (Time::S) << "s \n");

  m_face->ReceiveInterest (interest);
  m_transmittedInterests (interest, this, m_face);

  // std::cout << "Size: " << packet->GetSize () << std::endl;
  
  // NS_LOG_DEBUG (m_avgGap+MilliSeconds (m_rand.GetValue ()));
  Time nextTime = m_avgGap + Time::FromDouble (m_jitter.GetValue (), Time::US);
  //NS_LOG_DEBUG ("next time: " << nextTime.ToDouble (Time::S) << "s");
  m_nextSendEvent = Simulator::Schedule (nextTime,
        				 &DdosAppPoseidon::SendPacket, this);
}

void
DdosAppPoseidon::StartApplication ()
{
  // calculate outgoing rate and set Interest generation rate accordingly
  
  double sumOutRate = 0.0;
  
  Ptr<Node> node = GetNode ();
  for (uint32_t deviceId = 0; deviceId < node->GetNDevices (); deviceId ++)
    {
      Ptr<PointToPointNetDevice> device = DynamicCast<PointToPointNetDevice> (node->GetDevice (deviceId));
      if (device == 0)
        continue;

      DataRateValue dataRate; device->GetAttribute ("DataRate", dataRate);
      sumOutRate += (dataRate.Get ().GetBitRate () / 8);
    }

  double maxInterestTo = sumOutRate / 40;
  double maxDataBack = sumOutRate / 1146;
 
  if (m_evilBit)
    {
       m_avgGap = Seconds (0.001);
      // std::cout << "evil Gap: " << m_avgGap.ToDouble (Time::S) << "s\n";
    }
  else
    {
      // m_avgGap = Seconds (2 * 1 / maxDataBack); // request 50% of maximum link capacity
      // std::cout << "good Gap: " << m_avgGap.ToDouble (Time::S) << "s\n";
    }
  
  App::StartApplication ();
  SendPacket ();
}

void
DdosAppPoseidon::StopApplication ()
{
  m_nextSendEvent.Cancel ();

  // std::cerr << "# references before delayed stop: " << m_face->GetReferenceCount () << std::endl;
  Simulator::Schedule (Seconds (10.0), &DdosAppPoseidon::DelayedStop, this);
}

void
DdosAppPoseidon::DelayedStop ()
{
  // std::cerr << "# references after delayed stop: " << m_face->GetReferenceCount () << std::endl;
  App::StopApplication ();
}
