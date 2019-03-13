/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/propagation-module.h"
#include "ns3/wifi-module.h"
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WiFiInterference");
/*
class WiFiInterference: public Object
{
  public:
  WiFiInterference();
  virtual ~WiFiInterference();
  static TypeId GetTypeId (void);
  void Set_cur_time(Time m_cur_time);
  Time Get_cur_time(void);
  protected:
  Time m_curtime;
};
TypeId
WiFiInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WiFiInterference")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddAttribute("CurrTime",
                   "time",
                   TimeValue (MicroSeconds (0)),
                   MakeTimeAccessor (&WiFiInterference::m_curtime),
                    MakeTimeChecker ());
return tid;
}
void
WiFiInterference::Set_cur_time(Time m_cur_time)
{
  m_curtime = m_cur_time;
}
Time
WiFiInterference::Get_cur_time(void)
{
  return m_curtime;
}

*/
bool
closest_check (NodeContainer cur, uint32_t i, Ptr<Node> sta, uint32_t size)
{

  Ptr<Node> stat = cur.Get(i);
  auto mobil_cur = stat->GetObject<MobilityModel> ();
  Vector position = mobil_cur->GetPosition ();
  auto mob_sta = sta->GetObject<MobilityModel>();
  Vector pos = mob_sta->GetPosition ();
  double range = sqrt((pos.x - position.x)*(pos.x - position.x) + (pos.y - position.y)*(pos.y - position.y));
  double dif=0;
  bool flag = true;
  //std::cout<<"range"<<i<<" = "<<range<<std::endl;
  for (uint32_t j=0;j<size;j++)
  {
    if (j!=i)
    {
      Ptr<Node> aps = cur.Get(j);
      auto mobil = aps->GetObject<MobilityModel>();
      Vector p = mobil->GetPosition();

      dif = sqrt((pos.x - p.x)*(pos.x - p.x) + (pos.y - p.y)*(pos.y - p.y));
     // std::cout<<"dif"<<j<<" = "<<dif<<"\t";
      if (dif<=range)
      {
        flag = false;
      }


    }
  }
  //std::cout<<std::endl;
  return flag;
}


uint32_t RcvPktCount1 = 0;
void
ReceiveTrace1 (Ptr<const Packet> pkt, const Address &addr)
{
  RcvPktCount1++;
  //std::cout<<"rx1\n";
}

uint32_t SndPktCount1=0;
void
SendTrace1 (Ptr<const Packet> pkt)
{
  SndPktCount1++;

  //std::cout<<"rtx1\n";
}




int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::US); //set time resolution to us
  Time simTime = Seconds (20);
  std::string phyMode ("OfdmRate6Mbps");
  uint32_t packetSize = 1500; // bytes
  uint32_t numPackets = 1;
  uint32_t startvalue=0;
  uint32_t endvalue=0;
  double cca=-95.0;
  Time clientStart = Seconds (2);
  Time serverStart = Seconds (1);
  Time dt_for_second = MicroSeconds (10);
  Time packetInterval = MicroSeconds (100);
  bool verbose = false;
  bool isUnidirectional = false;
  double linkRange = 40.0;
  double distance = 65.0;
  std::string outFileName = "result.txt";
  bool isEDCA = true;
  bool print_graph = false;
  bool colorcodemode = true;
  double SquareSize = 100.0;
  uint32_t pairs;
  uint32_t lay=3;
  //double c_frequency=2.4;

  CommandLine cmd;


  cmd.AddValue ("lay", "lay", lay);
  cmd.AddValue ("Pairs", " Amount of STAs and APs", pairs);
  cmd.AddValue ("SqSize", " Determines size of field", SquareSize);
  cmd.AddValue ("color", "Enables colorcode functions", colorcodemode);
  cmd.AddValue ("phyMode", "Wi-Fi PHY mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent [bytes]", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval between packets [s]", packetInterval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("isUnidirectional", "Switch for unidirectional or differently directed links",
                isUnidirectional);
  cmd.AddValue("delta_t","Time between sending",dt_for_second);
  cmd.AddValue ("simTime", "Simulation time [s]", simTime);
  cmd.AddValue ("linkRange", "Distance between STA composing a link [m]", linkRange);
  cmd.AddValue ("distance", "Distance between STA 0 and STA 3 [m]", distance);
  cmd.AddValue ("outFileName", "Out file name", outFileName);
  cmd.AddValue ("isEDCA", "DCF/EDCA switch", isEDCA);
  cmd.AddValue("CCA", "Power threshold", cca);
  cmd.AddValue("startvalue","Value for first Distance", startvalue);
  cmd.AddValue("endvalue","Value for last Distance",endvalue);
  cmd.AddValue("graph","Saving plot after collecting data",print_graph);

  cmd.Parse (argc, argv);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // turn off fragmentation at IP layer
  Config::SetDefault ("ns3::WifiNetDevice::Mtu", StringValue ("2200"));
  //Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue (100));
  //Config::SetDefault("ns3::WifiRemoteStationManager::MaxSsrc", UintegerValue(10));
  Config::SetDefault ("ns3::WifiPhy::TxPowerStart", DoubleValue (18.0206));
  Config::SetDefault ("ns3::WifiPhy::TxPowerEnd", DoubleValue (18.0206));
  //Config::SetDefault ("ns3::Txop::MinCw",UintegerValue(15) );
  //Config::SetDefault("ns3::Txop::MaxCw",UintegerValue(1023));
  // Enabling/disebling color code
  Config::SetDefault("ns3::WifiPhy::ColorCodeMode", BooleanValue (colorcodemode));
  // set CCA threshold to -95 dBm
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-96.0));
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-62.0));
  
  pairs = lay*lay;
  NodeContainer accesspoints;
  accesspoints.Create (pairs);

  NodeContainer stations;
  stations.Create (pairs);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

  // Configure PHY layer
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  //Configure propagation
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3);
  Ptr <FriisPropagationLossModel> Friis_loss = CreateObject <FriisPropagationLossModel>();  
  Ptr <ThreeLogDistancePropagationLossModel> Three_log_loss = CreateObject <ThreeLogDistancePropagationLossModel>();
  
  
  //double custom_loss_model=40.05 + 20*std::log10(c_frequency/2.4) + 20*std::log10(SquareSize) + SquareSize*35*std::log10(SquareSize/10);
  channel->SetPropagationLossModel (loss);
  wifiPhy.SetChannel (channel);

  WifiMacHelper wifiMac;
  // Set adhoc mode and select DCF/EDCA
  wifiMac.SetType ("ns3::AdhocWifiMac", "QosSupported", BooleanValue (isEDCA));
  // disable rate control
  wifi.SetRemoteStationManager ("ns3::MinstrelWifiManager");//, "DataMode", StringValue (phyMode));
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, accesspoints);
  NetDeviceContainer devices2 = wifi.Install (wifiPhy, wifiMac, stations);

  //Set node positions


  Ptr <UniformRandomVariable> squaresize = CreateObject <UniformRandomVariable>();
  squaresize->SetAttribute("Min",DoubleValue (0.0));
  squaresize->SetAttribute("Max",DoubleValue (SquareSize));


  
  MobilityHelper mobility;
  Ptr<GridPositionAllocator> positionAlloc = CreateObject<GridPositionAllocator> ();
  positionAlloc->SetAttribute("DeltaX", DoubleValue (SquareSize));
  positionAlloc->SetAttribute("DeltaY", DoubleValue (SquareSize));
  positionAlloc->SetAttribute("GridWidth", UintegerValue (lay));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (accesspoints);

  MobilityHelper mobility2;


bool check;
for (uint32_t i=0;i<pairs;i++)
{
  Ptr<Node> stat = accesspoints.Get(i);
  auto mobil = stat->GetObject<MobilityModel> ();
  Vector position = mobil->GetPosition ();


  //std::cout << "apX"<<i<<": " << position.x << std::endl;
  //std::cout << "apY"<<i<<": " << position.y << std::endl;

  Ptr<UniformDiscPositionAllocator> positionAlloc2 = CreateObject<UniformDiscPositionAllocator> ();
  do{
  positionAlloc2->SetAttribute("rho",DoubleValue (linkRange));
  positionAlloc2->SetAttribute("X",DoubleValue (position.x));
  positionAlloc2->SetAttribute("Y",DoubleValue (position.y));
  positionAlloc2->SetAttribute("Z",DoubleValue (position.z));

  Ptr<Node> stap = stations.Get(i);
  mobility2.SetPositionAllocator (positionAlloc2);
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility2.Install (stap);
  check = closest_check(accesspoints,i,stap,pairs);
  } while(!check) ;
  //std::cout<<"staX"<<i<<" : "<<stations.Get(i)->GetObject<MobilityModel>()->GetPosition().x<<std::endl;
  //std::cout<<"staY"<<i<<" : "<<stations.Get(i)->GetObject<MobilityModel>()->GetPosition().y<<std::endl;
  
}


  InternetStackHelper internet;
  internet.Install (accesspoints);

  InternetStackHelper internet2;
  internet2.Install (stations);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);
  Ipv4InterfaceContainer interfaces2 = ipv4.Assign (devices2);
  


  ArpCache::PopulateArpCache ();

  //Install applications

  for (uint32_t i=0;i<pairs;i++){
  UdpEchoClientHelper echoClient (interfaces2.GetAddress (i),
                                  9); // The destination station is station # 1
  echoClient.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  echoClient.SetAttribute ("Interval", TimeValue (packetInterval));
  echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer clientApps = echoClient.Install (accesspoints.Get (i));
  clientApps.Start (clientStart);
  clientApps.Stop (simTime);
  clientApps.Get (0)->TraceConnectWithoutContext ("Tx", MakeCallback (&SendTrace1));



  PacketSinkHelper server ("ns3::UdpSocketFactory",
                           InetSocketAddress (interfaces2.GetAddress (i), 9));

  ApplicationContainer serverApps = server.Install (stations.Get (i));
  serverApps.Start (serverStart);
  serverApps.Stop (simTime);
  serverApps.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&ReceiveTrace1));
  }
  

  Simulator::Stop (simTime);
  Simulator::Run ();
  Simulator::Destroy ();
  //Print simulation results in the file
  std::ofstream outStream;
  outStream.open (outFileName.c_str (), std::ios::out);
  if (!outStream.is_open ())
    {
      NS_FATAL_ERROR ("Cannot open file " << outFileName);
    }
  outStream <<RcvPktCount1 <<std::endl;

  return 0;
}