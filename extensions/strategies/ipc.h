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
