/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/propagation-module.h"
#include "ns3/wifi-module.h"
#include "ns3/buildings-module.h"
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "ns3/ieee-buildings-propagation-loss-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WiFiInterference");

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
  double cca=-62.0;
  Time clientStart = Seconds (2);
  Time serverStart = Seconds (1);
  Time dt_for_second = MicroSeconds (10);
  Time packetInterval = MicroSeconds (20);
  bool verbose = false;
  bool isUnidirectional = false;
  double linkRange = 40.0;
  double distance = 65.0;
  std::string outFileName = "result.txt";
  std::string outFileName_TX = "resl.txt";
  bool isEDCA = true;
  bool print_graph = false;
  bool colorcodemode = true;
  //double SquareSize = 100.0;
  uint32_t pairs;
  //uint32_t lay;
  //double c_frequency=2.4;
  double h_max = 10;
  double l_max = 10;
  double w_max = 10;
  double energylvl = -62.0;

  CommandLine cmd;


  cmd.AddValue ("elvl", "elvl", energylvl);
  cmd.AddValue ("w"," w", w_max);
  cmd.AddValue ("l"," l", l_max);
  cmd.AddValue ("h"," h", h_max);
  //cmd.AddValue ("lay", "lay", lay);
  //cmd.AddValue ("Pairs", " Amount of STAs and APs", pairs);
  //cmd.AddValue ("SqSize", " Determines size of field", SquareSize);
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
  Config::SetDefault("ns3::WifiPhy::EnablingColor", BooleanValue (colorcodemode));
  // set CCA threshold to -95 dBm
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-96.0));
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (cca));
  Config::SetDefault ("ns3::WifiPhy::EnergyRec", DoubleValue (energylvl));
  
    //-----------------------------------------building---------------------------------------------//
  #if 1
  double h_min = 0.0;
  double l_min = 0.0;
  double w_min = 0.0;
  h_max = h_max + 0.5;
  uint32_t hh = (int)h_max;
  l_max = l_max + 0.5;
  uint32_t ll = (int)l_max;
  w_max = w_max + 0.5;
  uint32_t ww = (int)w_max;

  h_max = h_max - 0.5;
  l_max = l_max - 0.5;
  w_max = w_max - 0.5;
  uint32_t lay_x = div(ll,8).quot;
  uint32_t lay_y = div(ww,5).quot;
  uint32_t lay_z = div(hh,3).quot;
  pairs = lay_x*lay_y*lay_z;// XxYxZ == 5x8x3
  std::cout<<pairs<< " rooms\n";
  #endif
  //std::cout<<"--------------------------------------"<<pairs<<"----------------------------------------------\n";
 // uint32_t floors = 1;
  //uint32_t rooms_x = 2;
  //uint32_t rooms_y = 1;
  // ap pos = i*5 + 0.5; in all axisses
  // sta pos = 2.5 + i*5 + random(2.5); in all axisses
  //--------------------------------------------------------------------------------------------------//

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
  #if 1
  Ptr <IeeeBuildingsPropagationLossModel> Ieee_Build_loss = CreateObject <IeeeBuildingsPropagationLossModel> ();
  channel->SetPropagationLossModel (Ieee_Build_loss);
  #endif
  #if 0
  Ptr<LogDistancePropagationLossModel> loss = CreateObject <LogDistancePropagationLossModel> ();
  channel->SetPropagationLossModel (loss);
  #endif
  wifiPhy.SetChannel (channel);


  
  WifiMacHelper wifiMac;
  // Set adhoc mode and select DCF/EDCA
  wifiMac.SetType ("ns3::AdhocWifiMac", "QosSupported", BooleanValue (isEDCA));
  // disable rate control
  wifi.SetRemoteStationManager ("ns3::MinstrelWifiManager");//, "DataMode", StringValue (phyMode));
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, accesspoints);
  NetDeviceContainer devices2 = wifi.Install (wifiPhy, wifiMac, stations);
  #if 0
  for (uint8_t i=0;i<pairs;i++)
  {
    uint8_t col = i % 64;
    //std::cout<<"color "<<unsigned(col)<<std::endl;
    devices.Get(i)->SetColorCode(col);
    devices2.Get(i)->SetColorCode(col);
    //std::cout<<"color "<<unsigned(devices.Get(i)->GetColorCode())<<std::endl;
    //std::cout<<"color "<<unsigned(devices2.Get(i)->GetColorCode())<<std::endl;
  }
  #endif

  #if 1
  BuildingsHelper buildHelper;
  Ptr<Building> build = CreateObject <Building>();
  build->SetBoundaries (Box(l_min,l_max,w_min,w_max,h_min,h_max));
  build->SetBuildingType (Building::Residential);
  build->SetExtWallsType (Building::Ieee);
  build->SetNFloors (lay_z);
  build->SetNRoomsX (lay_x);
  build->SetNRoomsY (lay_y);
  #endif
  #if 0
  Ptr <UniformRandomVariable> squaresize = CreateObject <UniformRandomVariable>();
  squaresize->SetAttribute("Min",DoubleValue (0.0));
  squaresize->SetAttribute("Max",DoubleValue (SquareSize));

  MobilityHelper mobility;
  Ptr<GridPositionAlocator> positionAlloc = CreateObject <GridPositionAllocator>();
  positionAlloc->SetAttribute("DeltaX",DoubleValue(squaresize));
  positionAlloc->SetAttribute("DeltaY",DoubleValue(squaresize));
  positionAlloc->SetAttribute("GridWidth",DoubleValue(3));
  #endif

#if 1
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  for (uint32_t i=0;i<h_max;i+=3)
  {
    for (uint32_t j=0;j<w_max;j+=5)
    {
      for (uint32_t k=0;k<l_max;k+=8)
      {
        positionAlloc->Add(Vector(k+4,j+2.5,i+1.5));
        std::cout<<k+4<<" "<<j+2.5<<" "<<i+1.5<<"\n";
      }
    }
  
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (accesspoints);
  buildHelper.Install(accesspoints);
 
  MobilityHelper mobility2;
  #endif

  #if 1
  Ptr<ListPositionAllocator> posAlloc = CreateObject <ListPositionAllocator>();
  for (uint32_t i=0;i<pairs;i++)
  {
    Ptr<Node> stat = accesspoints.Get(i);
    auto mobil = stat->GetObject<MobilityModel> ();
    Vector position = mobil->GetPosition ();
       Ptr <UniformRandomVariable> x = CreateObject <UniformRandomVariable>();
    x->SetAttribute("Min",DoubleValue (position.x-3.99));
    x->SetAttribute("Max",DoubleValue (position.x+3.99));
    Ptr <UniformRandomVariable> y = CreateObject <UniformRandomVariable>();
    y->SetAttribute("Min",DoubleValue (position.y-2.49));
    y->SetAttribute("Max",DoubleValue (position.y+2.49));
    Ptr <UniformRandomVariable> z = CreateObject <UniformRandomVariable>();
    z->SetAttribute("Min",DoubleValue (position.z-1.49));
    z->SetAttribute("Max",DoubleValue (position.z+1.49));
    double pos_x = x->GetValue();
    double pos_y = y->GetValue();
    double pos_z = z->GetValue();
    std::cout<<pos_x<<" "<<pos_y<<" "<<pos_z<<"\n";
    posAlloc->Add(Vector(pos_x,pos_y,pos_z));
    #if 0
    Ptr <UniformRandomVariable> x1 = CreateObject <UniformRandomVariable>();
    x1->SetAttribute("Min",DoubleValue (position.x-3.99));
    x1->SetAttribute("Max",DoubleValue (position.x+3.99));
    Ptr <UniformRandomVariable> y1 = CreateObject <UniformRandomVariable>();
    y1->SetAttribute("Min",DoubleValue (position.y-2.49));
    y1->SetAttribute("Max",DoubleValue (position.y+2.49));
    Ptr <UniformRandomVariable> z = CreateObject <UniformRandomVariable>();
    z1->SetAttribute("Min",DoubleValue (position.z-1.49));
    z1->SetAttribute("Max",DoubleValue (position.z+1.49));
    pos_x = x1->GetValue();
    pos_y = y1->GetValue();
    pos_z = z1->GetValue();
    std::cout<<pos_x<<" "<<pos_y<<" "<<pos_z<<"\n";
    posAlloc->Add(Vector(pos_x,pos_y,pos_z));
    #endif 
  }
  mobility2.SetPositionAllocator (posAlloc);
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility2.Install (stations);
  buildHelper.Install(stations);


  buildHelper.MakeMobilityModelConsistent();
  #endif
  #if 0
  for (uint32_t i = 0;i<pairs;i++)
  {
    Ptr<Node> stat = accesspoints.Get(i);
    auto mobil = stat->GetObject<MobilityModel> ();
    Vector position = mobil->GetPosition ();
    std::cout << "apX"<<i<<": " << position.x << std::endl;
    std::cout << "apY"<<i<<": " << position.y << std::endl;
    std::cout << "apZ"<<i<<": " << position.z << std::endl;
    Ptr<Node> st = stations.Get(i);
    auto mobil2 = st->GetObject<MobilityModel> ();
    Vector posi = mobil2->GetPosition ();
    std::cout << "staX"<<i<<": " << posi.x << std::endl;
    std::cout << "staY"<<i<<": " << posi.y << std::endl;
    std::cout << "staZ"<<i<<": " << posi.z << std::endl;
  }
  #endif
#if 0
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
#endif

  InternetStackHelper internet;
  internet.Install (accesspoints);

  InternetStackHelper internet2;
  internet2.Install (stations);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.128.0");
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
  //check echoclient attributes for tag placement, but for sertain it is avilible for devices onphy lvl
  //chek echoclient attributes on packet conection
  //find how to insert sertain packet

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
  std::ofstream outStream_tx;
  outStream.open (outFileName.c_str (), std::ios::out);
  if (!outStream.is_open ())
    {
      NS_FATAL_ERROR ("Cannot open file " << outFileName);
    }
  outStream <<RcvPktCount1 <<std::endl;
  outStream_tx.open (outFileName_TX.c_str (), std::ios::out);
if (!outStream_tx.is_open())
{
  NS_FATAL_ERROR ("Cannot open file " << outFileName_TX);
}
  outStream_tx << SndPktCount1 << std::endl;
  outStream_tx.open (outFileName_TX.c_str (), std::ios::out);

  return 0;
}