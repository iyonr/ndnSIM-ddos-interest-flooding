#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#include <ns3/point-to-point-module.h>

#include <boost/lexical_cast.hpp>

using namespace ns3;
using namespace ns3::ndn;
using namespace std;

#include "calculate-max-capacity.h"



uint32_t Run = 1;

void PrintTime (Time next, const string name)
{
  cerr << " === " << name << " " << Simulator::Now ().ToDouble (Time::S) << "s" << endl;
  Simulator::Schedule (next, PrintTime, next, name);
}

int main (int argc, char**argv)
{
  string topology = "topo";
  string prefix = "";
  string producerLocation = "gw";
  Time evilGap = Time::FromDouble (0.02, Time::MS);
  Time defaultRtt = Seconds (0.25);
  uint32_t badCount = 1;
  uint32_t goodCount = 0;
  string folder = "tmp";
  
  CommandLine cmd;
  cmd.AddValue ("topology", "Topology", topology);
  cmd.AddValue ("run", "Run", Run);
  cmd.AddValue ("algorithm", "DDoS mitigation algorithm", prefix);
  cmd.AddValue ("producer", "Producer location: gw or bb", producerLocation);
  cmd.AddValue ("badCount", "Number of bad guys", badCount);
  cmd.AddValue ("goodCount", "Number of good guys", goodCount);
  cmd.AddValue ("folder", "Folder where results will be placed", folder);
  cmd.AddValue ("defaultRtt", "Default RTT for BDP limits", defaultRtt);
  cmd.Parse (argc, argv);
      
  Config::SetGlobal ("RngRun", IntegerValue (Run));
  StackHelper helper;

  AppHelper evilAppHelper ("DdosAppTDM");
  evilAppHelper.SetAttribute ("Evil", BooleanValue (true));
  evilAppHelper.SetAttribute ("LifeTime", StringValue ("1s"));
  evilAppHelper.SetAttribute ("DataBasedLimits", BooleanValue (true));
  
  AppHelper goodAppHelper ("DdosAppTDM");
  goodAppHelper.SetAttribute ("LifeTime",  StringValue ("1s"));
  goodAppHelper.SetAttribute ("DataBasedLimits", BooleanValue (true));

  AppHelper ph ("ns3::ndn::Producer");
  ph.SetPrefix ("/good");
  ph.SetAttribute ("PayloadSize", StringValue("1100"));

  string name = prefix;
  name += "-topo-" + topology;
  name += "-evil-" + boost::lexical_cast<string> (badCount);
  name += "-good-" + boost::lexical_cast<string> (goodCount);
  name += "-producer-" + producerLocation;
  name += "-run-"  + boost::lexical_cast<string> (Run);
  
  string results_file = "results/" + folder + "/" + name + ".txt";
  string meta_file    = "results/" + folder + "/" + name + ".meta";
  string graph_dot_file    = "results/" + folder + "/" + name + ".dot";
  string graph_pdf_file    = "results/" + folder + "/" + name + ".pdf";
  
    AnnotatedTopologyReader topologyReader ("", 1.0);
    topologyReader.SetFileName ("topologies/" + topology + ".txt");
    topologyReader.Read ();

  if (prefix == "attack-tdm")
    {
      //helper.EnableLimits (true, defaultRtt);
      helper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute::Stats::TDM");
    }
  else
    {
      cerr << "Invalid scenario prefix" << endl;
      return -1;
    }

    helper.Install (topologyReader.GetNodes ());
    
  
    //FIB-Entries on each Router
    //small
    
    //ZUFR1
    
    /*helper.AddRoute("Consumer1", "/good/good-Consumer1/", "Router1",1);
    helper.AddRoute("Consumer2", "/good/good-Consumer2/", "Router1",1);
    helper.AddRoute("Consumer3", "/good/good-Consumer3/", "Router1",1);
    helper.AddRoute("Consumer4", "/evil/evil-Consumer4/", "Router1",1);
    helper.AddRoute("Consumer5", "/evil/evil-Consumer5/", "Router1",1);
    helper.AddRoute("Consumer6", "/evil/evil-Consumer6/", "Router1",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer1/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer2/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer6/", "gw-root0",1);*/
    
    //ZUFR2
    
    /*helper.AddRoute("Consumer1", "/good/good-Consumer1/", "Router2",1);
    helper.AddRoute("Consumer2", "/evil/evil-Consumer2/", "Router2",1);
    helper.AddRoute("Consumer3", "/good/good-Consumer3/", "Router2",1);
    helper.AddRoute("Consumer4", "/evil/evil-Consumer4/", "Router2",1);
    
    helper.AddRoute("Router2", "/good/good-Consumer1/", "Router1",1);
    helper.AddRoute("Router2", "/evil/evil-Consumer2/", "Router1",1);
    helper.AddRoute("Router2", "/good/good-Consumer3/", "Router1",1);
    helper.AddRoute("Router2", "/evil/evil-Consumer4/", "Router1",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer1/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer2/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer4/", "gw-root0",1);*/
    
    //CC
    
    /*helper.AddRoute("Consumer1", "/good/good-Consumer1/", "Router1",1);
    helper.AddRoute("Consumer2", "/good/good-Consumer2/", "Router2",1);
    helper.AddRoute("Consumer3", "/good/good-Consumer3/", "Router3",1);
    helper.AddRoute("Consumer4", "/evil/evil-Consumer4/", "Router1",1);
    helper.AddRoute("Consumer5", "/evil/evil-Consumer5/", "Router1",1);
    helper.AddRoute("Consumer6", "/evil/evil-Consumer6/", "Router1",1);
    
    helper.AddRoute("Router3", "/good/good-Consumer3/", "Router2",1);
    helper.AddRoute("Router2", "/good/good-Consumer3/", "Router1",1);
    helper.AddRoute("Router2", "/good/good-Consumer2/", "Router1",1);
    helper.AddRoute("Router1", "/good/good-Consumer1/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer2/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router1", "/evil/evil-Consumer6/", "gw-root0",1);*/
    
    /*helper.AddRoute("Consumer1", "/good/good-Consumer1/", "Router1",1);
    helper.AddRoute("Consumer2", "/good/good-Consumer2/", "Router1",1);
    helper.AddRoute("Consumer3", "/good/good-Consumer3/", "Router2",1);
    helper.AddRoute("Consumer4", "/good/good-Consumer4/", "Router2",1);
    helper.AddRoute("Consumer5", "/good/good-Consumer5/", "Router3",1);
    helper.AddRoute("Consumer6", "/good/good-Consumer6/", "Router3",1);
    helper.AddRoute("Consumer7", "/good/good-Consumer7/", "Router4",1);
    helper.AddRoute("Consumer8", "/good/good-Consumer8/", "Router4",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer1/", "gw-root0",1);
    helper.AddRoute("Router1", "/good/good-Consumer2/", "gw-root0",1);
    
    helper.AddRoute("Router2", "/good/good-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer4/", "gw-root0",1);
    
    helper.AddRoute("Router3", "/good/good-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer6/", "gw-root0",1);
    
    helper.AddRoute("Router4", "/good/good-Consumer7/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer8/", "gw-root0",1);*/
    
    /*
    helper.AddRoute("Router3", "/good/good-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer3/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer4/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer5/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer6/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer6/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer6/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer6/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer7/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer7/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer7/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer7/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer8/", "gw-root0",1);
    helper.AddRoute("Router2", "/good/good-Consumer8/", "gw-root0",1);
    helper.AddRoute("Router3", "/good/good-Consumer8/", "gw-root0",1);
    helper.AddRoute("Router4", "/good/good-Consumer8/", "gw-root0",1);*/

    //medium
    
    //large
   /* helper.AddRoute("Consumer1", "/good/good-Consumer1/", "Router1",1);
    helper.AddRoute("Consumer2", "/good/good-Consumer2/", "Router1",1);
    helper.AddRoute("Consumer3", "/evil/evil-Consumer3/", "Router1",1);
    helper.AddRoute("Consumer4", "/good/good-Consumer4/", "Router1",1);
    
    helper.AddRoute("Consumer5", "/good/good-Consumer5/", "Router2",1);
    helper.AddRoute("Consumer6", "/good/good-Consumer6/", "Router2",1);
    helper.AddRoute("Consumer7", "/evil/evil-Consumer7/", "Router2",1);
    helper.AddRoute("Consumer8", "/evil/evil-Consumer8/", "Router2",1);
    
    helper.AddRoute("Consumer9", "/good/good-Consumer9/", "Router4",1);
    helper.AddRoute("Consumer10", "/good/good-Consumer10/", "Router4",1);
    helper.AddRoute("Consumer11", "/good/good-Consumer11/", "Router4",1);
    helper.AddRoute("Consumer12", "/good/good-Consumer12/", "Router4",1);
  
    helper.AddRoute("Router1", "/good/good-Consumer1/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer1/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer1/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer1/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer1/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer2/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer2/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer2/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer2/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer2/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/evil/evil-Consumer3/", "Router3",1);
    helper.AddRoute("Router2", "/evil/evil-Consumer3/", "Router3",1);
    helper.AddRoute("Router4", "/evil/evil-Consumer3/", "Router5",1);
    helper.AddRoute("Router3", "/evil/evil-Consumer3/", "gw-root0",1);
    helper.AddRoute("Router5", "/evil/evil-Consumer3/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer4/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer4/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer4/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer4/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer4/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer5/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer5/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer5/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer5/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer5/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer6/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer6/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer6/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer6/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer6/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/evil/evil-Consumer7/", "Router3",1);
    helper.AddRoute("Router2", "/evil/evil-Consumer7/", "Router3",1);
    helper.AddRoute("Router4", "/evil/evil-Consumer7/", "Router5",1);
    helper.AddRoute("Router3", "/evil/evil-Consumer7/", "gw-root0",1);
    helper.AddRoute("Router5", "/evil/evil-Consumer7/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/evil/evil-Consumer8/", "Router3",1);
    helper.AddRoute("Router2", "/evil/evil-Consumer8/", "Router3",1);
    helper.AddRoute("Router4", "/evil/evil-Consumer8/", "Router5",1);
    helper.AddRoute("Router3", "/evil/evil-Consumer8/", "gw-root0",1);
    helper.AddRoute("Router5", "/evil/evil-Consumer8/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer9/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer9/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer9/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer9/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer9/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer10/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer10/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer10/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer10/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer10/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer11/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer11/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer11/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer11/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer11/", "gw-root0",1);
    
    helper.AddRoute("Router1", "/good/good-Consumer12/", "Router3",1);
    helper.AddRoute("Router2", "/good/good-Consumer12/", "Router3",1);
    helper.AddRoute("Router4", "/good/good-Consumer12/", "Router5",1);
    helper.AddRoute("Router3", "/good/good-Consumer12/", "gw-root0",1);
    helper.AddRoute("Router5", "/good/good-Consumer12/", "gw-root0",1);*/

  cerr <<  topologyReader.GetNodes ().Get(0) ->GetId();

  topologyReader.ApplyOspfMetric ();
  
  GlobalRoutingHelper grouter;
  grouter.Install (topologyReader.GetNodes ());
  
  NodeContainer leaves;
  NodeContainer gw;
  NodeContainer bb;
  for_each (NodeList::Begin (), NodeList::End (), [&] (Ptr<Node> node) {
      if (Names::FindName (node).compare (0, 8, "Consumer")==0)
        {
          leaves.Add (node);
        }
      else if (Names::FindName (node).compare (0, 3, "gw-")==0)
        {
          gw.Add (node);
        }
      else if (Names::FindName (node).compare (0, 3, "Router")==0)
        {
          bb.Add (node);
        }
    });

  system (("mkdir -p \"results/" + folder + "\"").c_str ());
  ofstream os (meta_file.c_str(), ios_base::out | ios_base::trunc);
  
  os << "Total_numbef_of_nodes      " << NodeList::GetNNodes () << endl;
  os << "Total_number_of_leaf_nodes " << leaves.GetN () << endl;
  os << "Total_number_of_gw_nodes   " << gw.GetN () << endl;
  os << "Total_number_of_bb_nodes   " << bb.GetN () << endl;

  NodeContainer producerNodes;
  
  NodeContainer evilNodes;
  NodeContainer goodNodes;

  set< Ptr<Node> > producers;
  set< Ptr<Node> > evils;
  set< Ptr<Node> > angels;

  if (goodCount == 0)
    {
      goodCount = leaves.GetN () - badCount;
    }

  if (goodCount < 1)
    {
      NS_FATAL_ERROR ("Number of good guys should be at least 1");
      exit (1);
    }
  
  if (leaves.GetN () < goodCount+badCount)
    {
      NS_FATAL_ERROR ("Number of good and bad guys ("<< (goodCount+badCount) <<") cannot be less than number of leaves in the topology ("<< leaves.GetN () <<")");
      exit (1);
    }

  if (producerLocation == "gw")
    {
      if (gw.GetN () < 1)
        {
          NS_FATAL_ERROR ("Topology does not have gateway nodes that can serve as producers");
          exit (1);
        }
    }
  else if (producerLocation == "bb")
    {
      if (bb.GetN () < 1)
        {
          NS_FATAL_ERROR ("Topology does not have backbone nodes that can serve as producers");
          exit (1);
        }
    }
  else
    {
      NS_FATAL_ERROR ("--producer can be either 'gw' or 'bb'");
      exit (1);
    }
  
  os << "Number of evil nodes: " << badCount << endl;
	cerr << "\n" << "Evils:" << endl;
  while (evils.size () < badCount)
    {
      UniformVariable randVar (0, leaves.GetN ());
      Ptr<Node> node = leaves.Get (randVar.GetValue ());
      string name = Names::FindName (node);
      
      if(name.substr(8,1) == "2" || name.substr(8,1) == "4" || name.substr(8,1) == "6")// || name.substr(8,1) == "9")
      {
	if (evils.find (node) != evils.end ())
	  continue;
	evils.insert (node);
	Names::Rename (name, "evil-"+name);
      }
    }

  while (angels.size () < goodCount)
    {
      UniformVariable randVar (0, leaves.GetN ());
      Ptr<Node> node = leaves.Get (randVar.GetValue ());
      if (angels.find (node) != angels.end () ||
          evils.find (node) != evils.end ())
        continue;
      
      angels.insert (node);
      string name = Names::FindName (node);
      Names::Rename (name, "good-"+name);
    }

  while (producers.size () < 1)
    {
      Ptr<Node> node = 0;
      if (producerLocation == "gw")
        {
          UniformVariable randVar (0, gw.GetN ());
          node = gw.Get (randVar.GetValue ());
        }
      else if (producerLocation == "bb")
        {
          UniformVariable randVar (0, bb.GetN ());
          node = bb.Get (randVar.GetValue ());
        }
      
      producers.insert (node);
      string name = Names::FindName (node);
      Names::Rename (name, "producer-"+name);
	
      if(Names::FindName (node) == "producer-producer-gw-root1")
	{
		string name = Names::FindName (node);
		string name1 = name.substr(9,17);
		Names::Rename (name, name1);
	}

    }
  
  auto assignNodes = [&os](NodeContainer &aset, const string &str) {
    return [&os, &aset, &str] (Ptr<Node> node)
    {
      string name = Names::FindName (node);
      os << name << " ";
      cerr << name << endl;
      aset.Add (node);
    };
  };
  os << endl;
  
  // a little bit of C++11 flavor, compile with -std=c++11 flag
  os << "Evil: ";
  std::for_each (evils.begin (), evils.end (), assignNodes (evilNodes, "Evil"));
  os << "\nGood: ";
  std::for_each (angels.begin (), angels.end (), assignNodes (goodNodes, "Good"));
  os << "\nProducers: ";
  std::for_each (producers.begin (), producers.end (), assignNodes (producerNodes, "Producers"));
  os << "\n";
  
  grouter.AddOrigins ("/", producerNodes);
  
  //PITLAST
  /*grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer4/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer5/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer6/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer7/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer8/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer9/", producerNodes);*/
  
  //ZUFR1
  
  /*grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer4/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer5/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer6/", producerNodes);*/
  
  //ZUFR2
  
  /*grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer4/", producerNodes);*/
  
  //CC
  /*grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer4/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer5/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer6/", producerNodes);*/
  
  //lokal
  
  grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer4/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer5/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer6/", producerNodes);
  
  //small
  /*grouter.AddOrigins ("/good/good-Consumer1/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer2/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer3/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer4/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer5/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer6/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer7/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer8/", producerNodes);*/
  
  //medium
  
  /*grouter.AddOrigins ("/evil/evil-Consumer1/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer2/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer3/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer4/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer5/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer6/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer7/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer8/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer9/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer10/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer11/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer12/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer13/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer14/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer15/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer16/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer17/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer18/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer19/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer20/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer21/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer22/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer23/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer24/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer25/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer26/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer27/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer28/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer29/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer30/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer31/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer32/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer33/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer34/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer35/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer36/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer37/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer38/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer39/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer40/", producerNodes);*/
  
  //large
  
  /*grouter.AddOrigins ("/evil/evil-Consumer1/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer2/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer3/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer4/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer5/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer6/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer7/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer8/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer9/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer10/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer11/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer12/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer13/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer14/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer15/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer16/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer17/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer18/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer19/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer20/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer21/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer22/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer23/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer24/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer25/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer26/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer27/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer28/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer29/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer30/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer31/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer32/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer33/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer34/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer35/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer36/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer37/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer38/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer39/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer40/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer41/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer42/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer43/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer44/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer45/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer46/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer47/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer48/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer49/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer50/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer51/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer52/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer53/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer54/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer55/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer56/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer57/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer58/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer59/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer60/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer61/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer62/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer63/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer64/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer65/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer66/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer67/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer68/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer69/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer70/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer71/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer72/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer73/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer74/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer75/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumee76/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer77/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer78/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer79/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer80/", producerNodes);
    
  grouter.AddOrigins ("/good/good-Consumer81/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer82/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer83/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer84/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer85/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer86/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer87/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer88/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer89/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer90/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer91/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer92/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer93/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer94/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer95/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer96/", producerNodes);
  
  grouter.AddOrigins ("/good/good-Consumer97/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer98/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer99/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer100/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer101/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer102/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer103/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer104/", producerNodes);
  
  grouter.AddOrigins ("/evil/evil-Consumer105/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer106/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer107/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer108/", producerNodes);
  grouter.AddOrigins ("/evil/evil-Consumer109/", producerNodes);
  grouter.AddOrigins ("/good/good-Consumer110/", producerNodes);*/
  
  grouter.CalculateRoutes ();

  // verify topology RTT
  for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
    {
      Ptr<Fib> fib = (*node)->GetObject<Fib> ();
      if (fib == 0 || fib->Begin () == 0) continue;

      if (2* fib->Begin ()->m_faces.begin ()->GetRealDelay ().ToDouble (Time::S) > defaultRtt.ToDouble (Time::S))
        {
          cout << "DefaultRTT is smaller that real RTT in the topology: " << 2*fib->Begin ()->m_faces.begin ()->GetRealDelay ().ToDouble (Time::S) << "s" << endl;
          os << "DefaultRTT is smaller that real RTT in the topology: " << 2*fib->Begin ()->m_faces.begin ()->GetRealDelay ().ToDouble (Time::S) << "s" << endl;
        }
    }

  double maxNonCongestionShare = 0.8 * calculateNonCongestionFlows (goodNodes, producerNodes);
  os << "maxNonCongestionShare   " << maxNonCongestionShare << endl;

  saveActualGraph (graph_dot_file, NodeContainer (goodNodes, evilNodes));
  system (("twopi -Tpdf \"" + graph_dot_file + "\" > \"" + graph_pdf_file + "\"").c_str ());
  cout << "Write effective topology graph to: " << graph_pdf_file << endl;
  cout << "Max non-congestion share:   " << maxNonCongestionShare << endl;

  // exit (1);
    
  for (NodeContainer::Iterator node = goodNodes.Begin (); node != goodNodes.End (); node++)
    {
      ApplicationContainer goodApp;
      goodAppHelper.SetPrefix ("/good/"+Names::FindName (*node));
      goodAppHelper.SetAttribute ("AvgGap", TimeValue (Seconds (1.100 / maxNonCongestionShare)));
      goodApp.Add (goodAppHelper.Install (*node));
      UniformVariable rand (0, 1);
      goodApp.Start (Seconds (0.0) + Time::FromDouble (rand.GetValue (), Time::S));
    }
  
  for (NodeContainer::Iterator node = evilNodes.Begin (); node != evilNodes.End (); node++)
    {
      ApplicationContainer evilApp;
      evilAppHelper.SetPrefix ("/evil/"+Names::FindName (*node));
      evilAppHelper.SetAttribute ("AvgGap", TimeValue (Seconds (0.001)));
      evilApp.Add (evilAppHelper.Install (*node));
      UniformVariable rand (0, 1);
      evilApp.Start (Seconds (0.0) + Time::FromDouble (rand.GetValue (), Time::MS));
      evilApp.Stop  (Seconds (900.0) + Time::FromDouble (rand.GetValue (), Time::MS));  
    }

  ph.Install (producerNodes);
	
  L3RateTracer::InstallAll (results_file, Seconds (1.0)); 

  //Simulator::Schedule (Seconds (10.0), PrintTime, Seconds (10.0), name);
  Simulator::Stop (Seconds (900.0));
  Simulator::Run ();
  Simulator::Destroy ();
  
  L3RateTracer::Destroy (); 

  cerr << "Archiving to: " << results_file << ".bz2" << endl;
  system (("rm -f \"" + results_file + ".bz2" + "\"").c_str() );
  system (("bzip2 \"" + results_file + "\"").c_str() );
  
  return 0;
}