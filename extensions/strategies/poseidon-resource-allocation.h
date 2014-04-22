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
