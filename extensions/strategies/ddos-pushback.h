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
