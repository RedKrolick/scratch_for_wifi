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

uint32_t RcvPktCount1 = 0;

void
ReceiveTrace1 (Ptr<const Packet> pkt, const Address &addr)
{
  RcvPktCount1++;
}

uint32_t RcvPktCount2 = 0;

void
ReceiveTrace2 (Ptr<const Packet> pkt, const Address &addr)
{
  RcvPktCount2++;
}

uint32_t RcvPktCount3 = 0;

void
ReceiveTrace3 (Ptr<const Packet> pkt, const Address &addr)
{
  RcvPktCount3++;
}

uint32_t SndPktCount2 = 0;

void
SendTrace2 (Ptr<const Packet> pkt)
{
  SndPktCount2++;
}

uint32_t SndPktCount1=0;

void
SendTrace1 (Ptr<const Packet> pkt)
{
  SndPktCount1++;
}


uint32_t SndPktCount3=0;

void
SendTrace3 (Ptr<const Packet> pkt)
{
  SndPktCount3++;
}
int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::US); //set time resolution to us
  Time simTime = Seconds (20);
  std::string phyMode ("OfdmRate6Mbps");
  uint32_t packetSize = 100; // bytes
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

  CommandLine cmd;

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
  //Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue (1));
  Config::SetDefault("ns3::WifiRemoteStationManager::MaxSsrc", UintegerValue(1));
  Config::SetDefault ("ns3::WifiPhy::TxPowerStart", DoubleValue (18.0206));
  Config::SetDefault ("ns3::WifiPhy::TxPowerEnd", DoubleValue (18.0206));
  Config::SetDefault ("ns3::Txop::MinCw",UintegerValue(0) );
  Config::SetDefault("ns3::Txop::MaxCw",UintegerValue(0));
  // Enabling/disebling color code
  Config::SetDefault("ns3::WifiPhy::ColorCodeMode", BooleanValue (colorcodemode));
  // set CCA threshold to -95 dBm
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (cca));
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (cca));

  NodeContainer stations;
  stations.Create (5);

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
  loss->SetPathLossExponent (3.3);
  channel->SetPropagationLossModel (loss);
  wifiPhy.SetChannel (channel);

  WifiMacHelper wifiMac;
  // Set adhoc mode and select DCF/EDCA
  wifiMac.SetType ("ns3::AdhocWifiMac", "QosSupported", BooleanValue (isEDCA));
  // disable rate control
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode));
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, stations);

  //For incrementing distance for collecting data
  if (startvalue!=0)// what it is for?
  {
    distance=startvalue;
  }
  //Set node positions
  MobilityHelper mobility;
  Ptr<RandomRectanglePositionAllocator> positionAlloc = CreateObject<RandomRectanglePositionAllocator> ();
  positionAlloc->SetAttribute("X",StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  positionAlloc->SetAttribute("Y",StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (stations);

for (int i=0;i<5;i++)
{
  Ptr<Node> stat = stations.Get(i);
  auto mobil = stat->GetObject<MobilityModel> ();
  Vector position = mobil->GetPosition ();

std::cout << "X"<<i<<": " << position.x << std::endl;
std::cout << "Y"<<i<<": " << position.x << std::endl;
}

Ptr<Node> stat = stations.Get(1);
  auto mobil = stat->GetObject<MobilityModel> ();
  Vector position = mobil->GetPosition ();





  NodeContainer acpts;
  acpt.Create(1);

WifiMacHelper wifiMac2;
  // Set adhoc mode and select DCF/EDCA
  wifiMac2.SetType ("ns3::AdhocWifiMac", "QosSupported", BooleanValue (isEDCA));
  // disable rate control
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode));
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac2, acpts);

MobilityHelper mobility2;
  Ptr<UniformDiscPositionAllocator> positionAlloc2 = CreateObject<UniformDiscPositionAllocator> ();
  positionAlloc2.SetRho(linkRange);
  positionAlloc2.SetX();
  positionAlloc2.SetY();






  InternetStackHelper internet;
  internet.Install (stations);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

std::cout<<stations.GetN()<<"\n";

  ArpCache::PopulateArpCache ();

  //Install applications

 
    //Install client on station # 0 ap 1
  UdpEchoClientHelper echoClient (interfaces.GetAddress (3),
                                  9); // The destination station is station # 1
  echoClient.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  echoClient.SetAttribute ("Interval", TimeValue (packetInterval));
  echoClient.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer clientApps = echoClient.Install (stations.Get (0));
  clientApps.Start (clientStart);
  clientApps.Stop (simTime);
  clientApps.Get (0)->TraceConnectWithoutContext ("Tx", MakeCallback (&SendTrace1));


  //Install client on station # 1 ap 2
  UdpEchoClientHelper echoClient2 (interfaces.GetAddress (4),
                                   9); // The destination station is station # 3
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  echoClient2.SetAttribute ("Interval", TimeValue (packetInterval));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer clientApps2 = echoClient2.Install (stations.Get (1));
  clientApps2.Start (clientStart+dt_for_second);
  clientApps2.Stop (simTime);
  clientApps2.Get (0)->TraceConnectWithoutContext ("Tx", MakeCallback (&SendTrace2)); 


  //Install client on station # 2 ap 3
  UdpEchoClientHelper echoClient3 (interfaces.GetAddress (5),
                                   9); // The destination station is station # 3
  echoClient3.SetAttribute ("MaxPackets", UintegerValue (numPackets));
  echoClient3.SetAttribute ("Interval", TimeValue (packetInterval));
  echoClient3.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer clientApps3 = echoClient3.Install (stations.Get (2));
  clientApps3.Start (clientStart+dt_for_second);
  clientApps3.Stop (simTime);
  clientApps3.Get (0)->TraceConnectWithoutContext ("Tx", MakeCallback (&SendTrace3));


//STAs


  //Install server on station # 3 st 1 
  PacketSinkHelper server ("ns3::UdpSocketFactory",
                           InetSocketAddress (interfaces.GetAddress (3), 9));

  ApplicationContainer serverApps = server.Install (stations.Get (3));
  serverApps.Start (serverStart);
  serverApps.Stop (simTime);
  serverApps.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&ReceiveTrace1));


  //Install server on station # 4 st 2
  PacketSinkHelper server2 ("ns3::UdpSocketFactory",
                            InetSocketAddress (interfaces.GetAddress (4), 9));

  ApplicationContainer serverApps2 = server2.Install (stations.Get (4));
  serverApps2.Start (serverStart);//+dt_for_second);
  serverApps2.Stop (simTime);
  serverApps2.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&ReceiveTrace2));

  //Install server on station # 5 st 1
  PacketSinkHelper server3 ("ns3::UdpSocketFactory",
                           InetSocketAddress (interfaces.GetAddress (5), 9));

  ApplicationContainer serverApps3 = server3.Install (stations.Get (5));
  serverApps3.Start (serverStart);
  serverApps3.Stop (simTime);
  serverApps3.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&ReceiveTrace3));

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

  outStream << distance << "\t1:" << RcvPktCount1 <<"\t"<<SndPktCount1 << "\t2:" << RcvPktCount2 <<"\t" <<SndPktCount2<<std::endl;
  if (endvalue!=0)
  {
    system((std::string("echo \"")+std::to_string(int(distance))+std::string(" ")+std::to_string(RcvPktCount1)+
    std::string(" ")+std::to_string(RcvPktCount2)+std::string("\" >> results/combresult.txt")).c_str());
    if (endvalue!=startvalue)
    {
      startvalue++;
      system((std::string("./waf --run \"wifi-interference --CCA=")+std::to_string(cca)+
      std::string(" --startvalue=")+std::to_string(startvalue)+std::string(" --endvalue=")+
      std::to_string(endvalue)+std::string(" --linkRange=")+std::to_string(linkRange)+
      std::string(" --graph=")+std::to_string(print_graph)+std::string(" --packetSize=")+
      std::to_string(packetSize)+std::string("\"")).c_str());
    }
    else
      if (print_graph)
        system("python3 results/script.py");
  }  
  else 
    system("> results/combresult.txt");
  return 0;
}