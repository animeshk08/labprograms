#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ccnx/misc-tools.h"

#include "ns3/log.h"
 #include "ns3/point-to-point-dumbbell.h"
 #include "ns3/constant-position-mobility-model.h"

 #include "ns3/node-list.h"
 #include "ns3/point-to-point-net-device.h"
 #include "ns3/vector.h"
 #include "ns3/ipv6-address-generator.h"



namespace ns3 {

 NS_LOG_COMPONENT_DEFINE ("PointToPointDumbbellHelper");

 PointToPointDumbbellHelper::PointToPointDumbbellHelper (uint32_t nLeftLeaf,
                                                         PointToPointHelper leftHelper,
                                                         uint32_t nRightLeaf,
                                                         PointToPointHelper rightHelper,
                                                         PointToPointHelper bottleneckHelper)
 {
   // Create the bottleneck routers
   m_routers.Create (2);
   // Create the leaf nodes
   m_leftLeaf.Create (nLeftLeaf);
   m_rightLeaf.Create (nRightLeaf);

   leftHelper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   leftHelper.SetChannelAttribute ("Delay", StringValue ("1ms"));


   rightHelper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   rightHelper.SetChannelAttribute ("Delay", StringValue ("1ms"));


   bottleneckHelper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   bottleneckHelper.SetChannelAttribute ("Delay", StringValue ("1ms"));


   // Add the link connecting routers
   m_routerDevices = bottleneckHelper.Install (m_routers);
   // Add the left side links
   for (uint32_t i = 0; i < nLeftLeaf; ++i)
     {
       NetDeviceContainer c = leftHelper.Install (m_routers.Get (0),
                                                  m_leftLeaf.Get (i));
       m_leftRouterDevices.Add (c.Get (0));
       m_leftLeafDevices.Add (c.Get (1));
     }
   // Add the right side links
   for (uint32_t i = 0; i < nRightLeaf; ++i)
     {
       NetDeviceContainer c = rightHelper.Install (m_routers.Get (1),
                                                   m_rightLeaf.Get (i));
       m_rightRouterDevices.Add (c.Get (0));
       m_rightLeafDevices.Add (c.Get (1));
     }
 }

 PointToPointDumbbellHelper::~PointToPointDumbbellHelper ()
 {
 }

 Ptr<Node> PointToPointDumbbellHelper::GetLeft () const
 { // Get the left side bottleneck router
   return m_routers.Get (0);
 }

 Ptr<Node> PointToPointDumbbellHelper::GetLeft (uint32_t i) const
 { // Get the i'th left side leaf
   return m_leftLeaf.Get (i);
 }

 Ptr<Node> PointToPointDumbbellHelper::GetRight () const
 { // Get the right side bottleneck router
   return m_routers.Get (1);
 }

                                                       
 Ptr<Node> PointToPointDumbbellHelper::GetRight (uint32_t i) const
 { // Get the i'th right side leaf
   return m_rightLeaf.Get (i);
 }

 Ipv4Address PointToPointDumbbellHelper::GetLeftIpv4Address (uint32_t i) const
 {
   return m_leftLeafInterfaces.GetAddress (i);
 }

 Ipv4Address PointToPointDumbbellHelper::GetRightIpv4Address (uint32_t i) const
 {
   return m_rightLeafInterfaces.GetAddress (i);
 }

 Ipv6Address PointToPointDumbbellHelper::GetLeftIpv6Address (uint32_t i) const
 {
   return m_leftLeafInterfaces6.GetAddress (i, 1);
 }

 Ipv6Address PointToPointDumbbellHelper::GetRightIpv6Address (uint32_t i) const
 {
   return m_rightLeafInterfaces6.GetAddress (i, 1);
 }

 uint32_t  PointToPointDumbbellHelper::LeftCount () const
 { // Number of left side nodes
   return m_leftLeaf.GetN ();
 }

 uint32_t  PointToPointDumbbellHelper::RightCount () const
 { // Number of right side nodes
   return m_rightLeaf.GetN ();
 }

 void PointToPointDumbbellHelper::InstallStack (InternetStackHelper stack)
 {

   stack.Install (m_routers);
   stack.Install (m_leftLeaf);
   stack.Install (m_rightLeaf);



 }

 void PointToPointDumbbellHelper::AssignIpv4Addresses (Ipv4AddressHelper leftIp,
                                                       Ipv4AddressHelper rightIp,
                                                       Ipv4AddressHelper routerIp)
 {
   // Assign the router network
   m_routerInterfaces = routerIp.Assign (m_routerDevices);
   // Assign to left side 
  leftIp.SetBase ("10.1.1.0", "255.255.255.252");
   for (uint32_t i = 0; i < LeftCount (); ++i)
     {
       NetDeviceContainer ndc;
       ndc.Add (m_leftLeafDevices.Get (i));
       ndc.Add (m_leftRouterDevices.Get (i));
       Ipv4InterfaceContainer ifc = leftIp.Assign (ndc);
       m_leftLeafInterfaces.Add (ifc.Get (0));
       m_leftRouterInterfaces.Add (ifc.Get (1));
       leftIp.NewNetwork ();
     }
   // Assign to right side
   for (uint32_t i = 0; i < RightCount (); ++i)
     {
       NetDeviceContainer ndc;
       ndc.Add (m_rightLeafDevices.Get (i));
       ndc.Add (m_rightRouterDevices.Get (i));
       Ipv4InterfaceContainer ifc = rightIp.Assign (ndc);
       m_rightLeafInterfaces.Add (ifc.Get (0));
       m_rightRouterInterfaces.Add (ifc.Get (1));
       rightIp.NewNetwork ();
     }
 }


 void PointToPointDumbbellHelper::BoundingBox (double ulx, double uly, // Upper left x/y
                                               double lrx, double lry) // Lower right x/y
 {
   double xDist;
   double yDist;
   if (lrx > ulx)
     {
       xDist = lrx - ulx;
     }
                                                           
   else
     {
       xDist = ulx - lrx;
     }
   if (lry > uly)
     {
       yDist = lry - uly;
     }
   else
     {
       yDist = uly - lry;
     }

   double xAdder = xDist / 3.0;
   double  thetaL = M_PI / (LeftCount () + 1.0);
   double  thetaR = M_PI / (RightCount () + 1.0);

   // Place the left router
   Ptr<Node> lr = GetLeft ();
   Ptr<ConstantPositionMobilityModel> loc = lr->GetObject<ConstantPositionMobilityModel> ();
   if (loc == 0)
     {
       loc = CreateObject<ConstantPositionMobilityModel> ();
       lr->AggregateObject (loc);
     }
   Vector lrl (ulx + xAdder, uly + yDist/2.0, 0);
   loc->SetPosition (lrl);

   // Place the right router
   Ptr<Node> rr = GetRight ();
   loc = rr->GetObject<ConstantPositionMobilityModel> ();
   if (loc == 0)
     {
       loc = CreateObject<ConstantPositionMobilityModel> ();
       rr->AggregateObject (loc);
     }
   Vector rrl (ulx + xAdder * 2, uly + yDist/2.0, 0); // Right router location
   loc->SetPosition (rrl);

   // Place the left leaf nodes
   double theta = -M_PI_2 + thetaL;
   for (uint32_t l = 0; l < LeftCount (); ++l)

     {
       // Make them in a circular pattern to make all line lengths the same
       // Special case when theta = 0, to be sure we get a straight line
       if ((LeftCount () % 2) == 1)
         { // Count is odd, see if we are in middle
           if (l == (LeftCount () / 2))
             {
               theta = 0.0;
             }
         }
       Ptr<Node> ln = GetLeft (l);
       loc = ln->GetObject<ConstantPositionMobilityModel> ();
       if (loc == 0)
         {
           loc = CreateObject<ConstantPositionMobilityModel> ();
           ln->AggregateObject (loc);
         }
       Vector lnl (lrl.x - std::cos (theta) * xAdder,
                   lrl.y + std::sin (theta) * xAdder, 0);   // Left Node Location
       // Insure did not exceed bounding box
       if (lnl.y < uly)
         {
           lnl.y = uly; // Set to upper left y
         }
       if (lnl.y > lry)
         {
           lnl.y = lry; // Set to lower right y
         }
       loc->SetPosition (lnl);
       theta += thetaL;
     }
   // Place the right nodes
   theta = -M_PI_2 + thetaR;
   for (uint32_t r = 0; r < RightCount (); ++r)
     {
       // Special case when theta = 0, to be sure we get a straight line
       if ((RightCount () % 2) == 1)
         { // Count is odd, see if we are in middle
           if (r == (RightCount () / 2))
             {
               theta = 0.0;
             }

         }
       Ptr<Node> rn = GetRight (r);
       loc = rn->GetObject<ConstantPositionMobilityModel> ();
       if (loc == 0)
         {
           loc = CreateObject<ConstantPositionMobilityModel> ();
           rn->AggregateObject (loc);
         }
       Vector rnl (rrl.x + std::cos (theta) * xAdder, // Right node location
                   rrl.y + std::sin (theta) * xAdder, 0);
       // Insure did not exceed bounding box
       if (rnl.y < uly)
         {
           rnl.y = uly; // Set to upper left y
         }
       if (rnl.y > lry)
         {
           rnl.y = lry; // Set to lower right y
         }
       loc->SetPosition (rnl);
       theta += thetaR;
     }
 }

 } // 


using namespace std;
using namespace ns3;

int main(){
  bool useUdp = 0;
  std::string bandWidth = "1m";

        PointToPointHelper leftHelper, rightHelper, bottleneckHelper;


//      PointToPointDumbbellHelper *p=(PointToPointDubbellHelper *)malloc(sizeof(PointToPointDumbbellHelper));
        PointToPointDumbbellHelper p=  PointToPointDumbbellHelper(1,leftHelper,1,rightHelper,bottleneckHelper);


        DceManagerHelper dceManager;

        dceManager.SetTaskManagerAttribute ("FiberManagerType", StringValue ("UcontextFiberManager"));
        uint32_t i;
        for(i=0;i<=1;i++){
                dceManager.Install(p.GetLeft(i));
        }
        for(i=0;i<=1;i++){
                dceManager.Install(p.GetRight(i));
        }

        InternetStackHelper stack;

        p.InstallStack(stack);


        Ipv4AddressHelper leftIp,rightIp,routerIp;

        p.AssignIpv4Addresses(leftIp,rightIp,routerIp);

DceApplicationHelper dce;
 ApplicationContainer apps;

  dce.SetStackSize (1 << 22);

  // iperf client on 
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-c");
  //uint32_t x=p.GetLeftIpv4Address(1).Get();
  dce.AddArgument ("10.1.1.2");
  dce.AddArgument ("-i");
  dce.AddArgument ("1");
  dce.AddArgument ("--time");
  dce.AddArgument ("10");
  if (useUdp)

   {
      dce.AddArgument ("-u");
      dce.AddArgument ("-b");
      dce.AddArgument (bandWidth);
    }

  apps = dce.Install (p.GetLeft(1));
  apps.Start (Seconds (0.7));
  apps.Stop (Seconds (20));

  // Launch iperf server
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  dce.AddArgument ("-P");
  dce.AddArgument ("1");
  if (useUdp)
    {
      dce.AddArgument ("-u");
    }

  apps = dce.Install (p.GetRight(1));

  rightHelper.EnablePcapAll ("iperf-ns3", false);
  bottleneckHelper.EnablePcapAll ("iperf-ns3", false);
  leftHelper.EnablePcapAll ("iperf-ns3", false);

  apps.Start (Seconds (0.6));

  setPos (p.GetLeft(1), 1, 10, 0);
  setPos (p.GetRight(1), 50,10, 0);

  Simulator::Stop (Seconds (40.0));
  Simulator::Run ();
  Simulator::Destroy ();


return 0;
}
