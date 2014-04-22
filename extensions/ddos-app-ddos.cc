#include "ddos-app-ddos.h"
#include <ns3/point-to-point-module.h>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

NS_LOG_COMPONENT_DEFINE ("DdosDdosApp");

NS_OBJECT_ENSURE_REGISTERED (DdosDdosApp);

TypeId
DdosDdosApp::GetTypeId ()
{
  static TypeId tid = TypeId ("DdosDdosApp")
    .SetParent<App> ()
    .AddConstructor<DdosDdosApp> ()

    .AddAttribute ("AvgGap", "AverageGap",
		   StringValue ("1ms"),
                   MakeTimeAccessor (&DdosDdosApp::m_avgGap),
                   MakeTimeChecker ())
    .AddAttribute ("Prefix","Name of the Interest",
                   StringValue ("/"),
                   MakeNameAccessor (&DdosDdosApp::m_prefix),
                   MakeNameChecker ())
    .AddAttribute ("LifeTime","Interest lifetime",
                   StringValue ("1s"),
                   MakeTimeAccessor (&DdosDdosApp::m_lifetime),
                   MakeTimeChecker ())
    .AddAttribute ("Evil", "Evil bit",
                   BooleanValue (false),
                   MakeBooleanAccessor (&DdosDdosApp::m_evilBit),
                   MakeBooleanChecker ())
    .AddAttribute ("DataBasedLimits", "Calculate frequency based on how many data packets can be returned",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DdosDdosApp::m_dataBasedLimit),
                   MakeBooleanChecker ())
    ;
  return tid;
}

DdosDdosApp::DdosDdosApp ()
  : m_rand (0, std::numeric_limits<uint32_t>::max ())
  , m_jitter (0,1)
  , m_seq (0)
{
}

DdosDdosApp::~DdosDdosApp ()
{
}

void
DdosDdosApp::OnNack (const Ptr<const Interest> &interest)
{
  // immediately send new packet, without taking into account periodicity
  // m_nextSendEvent.Cancel ();
  // m_nextSendEvent = Simulator::Schedule (Seconds (0.001),
  //       				 &DdosDdosApp::SendPacket, this);
}

void
DdosDdosApp::OnData (const Ptr<const Data> &data)
{
  // who cares
}

void
DdosDdosApp::SendPacket ()
{
  m_seq++;
  // send packet
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
        				 &DdosDdosApp::SendPacket, this);
}

void
DdosDdosApp::StartApplication ()
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
DdosDdosApp::StopApplication ()
{
  m_nextSendEvent.Cancel ();

  // std::cerr << "# references before delayed stop: " << m_face->GetReferenceCount () << std::endl;
  Simulator::Schedule (Seconds (10.0), &DdosDdosApp::DelayedStop, this);
}

void
DdosDdosApp::DelayedStop ()
{
  // std::cerr << "# references after delayed stop: " << m_face->GetReferenceCount () << std::endl;
  App::StopApplication ();
}
