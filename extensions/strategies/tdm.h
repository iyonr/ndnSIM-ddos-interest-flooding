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
