#include "DHCPClient.h"
#include <cstring>
#include <string>

using namespace omnetpp;

Define_Module(DHCPClient);

void DHCPClient::initialize()
{
    deviceType = par("deviceType").stdstringValue();
    EV << "Client [" << deviceType << "] starting...\n";

    if (deviceType == "NewIoT") {

        // The client first attempts to join at t = 5s
        cMessage *retryTimer = new cMessage("retryTimer");
        scheduleAt(5.0, retryTimer);
    } else {

        // First three clients request IP at the beginning
        cMessage *timer = new cMessage("sendDiscover");
        scheduleAt(simTime() + 0.1, timer);
    }
}

void DHCPClient::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {

        // Sending DISCOVER to server
        if (strcmp(msg->getName(), "sendDiscover") == 0) {
            DHCPMessage *discover = new DHCPMessage("DISCOVER");
            discover->setMsgType("DISCOVER");
            discover->setDeviceType(deviceType.c_str());
            send(discover, "out");
            EV << "Sent DISCOVER (" << deviceType << ")\n";
            delete msg;
        }

        // Handling RETRY DISCOVER
        else if (strcmp(msg->getName(), "retryTimer") == 0) {
            if(hasIP == true) {
                delete msg;
                return;
            }
            DHCPMessage *discover = new DHCPMessage("DISCOVER");
            discover->setMsgType("DISCOVER");
            discover->setDeviceType(deviceType.c_str());
            send(discover, "out");
            EV << "RETRY: Sent DISCOVER (" << deviceType << ")\n";

            // retry is scheduled every 3 seconds
            scheduleAt(simTime() + 3.0, msg);
        }

        // Message for timer renewal
        else if (strcmp(msg->getName(), "renewTimer") == 0) {
            renewalCount++;

            // Simulating IoT device disconnection
            if (deviceType == "IoT" && renewalCount > 2) {
                EV << "IoT device disconnecting after " << renewalCount << " renewals.\n";
                delete msg;
                return;
            }

            EV << "Renewing IP " << assignedIP << "\n";
            DHCPMessage *request = new DHCPMessage("REQUEST");
            request->setMsgType("REQUEST");
            request->setDeviceType(deviceType.c_str());
            request->setAssignedIP(assignedIP.c_str());
            request->setLeaseTime(leaseTime);
            send(request, "out");
            delete msg;
        }
    } else {
        DHCPMessage *dhcpMsg = check_and_cast<DHCPMessage *>(msg);

        // Received OFFER
        if (strcmp(dhcpMsg->getMsgType(), "OFFER") == 0) {
            std::string ip = dhcpMsg->getAssignedIP();
            double lease = dhcpMsg->getLeaseTime();
            EV << "Received OFFER: IP=" << ip << ", lease=" << lease << "s\n";

            // Sending REQUEST
            DHCPMessage *request = new DHCPMessage("REQUEST");
            request->setMsgType("REQUEST");
            request->setDeviceType(deviceType.c_str());
            request->setAssignedIP(ip.c_str());
            request->setLeaseTime(lease);

            send(request, "out");
            EV << deviceType.c_str() << " sent REQUEST for IP " << ip << "\n";
        }

        // Received ACK
        else if (strcmp(dhcpMsg->getMsgType(), "ACK") == 0) {
            assignedIP = dhcpMsg->getAssignedIP();
            leaseTime = dhcpMsg->getLeaseTime();
            hasIP = true;

            if (simTime() == 0 || simTime() < 1) {
                EV << "IP assigned: " << assignedIP << " to " << dhcpMsg->getDeviceType() << "(lease=" << leaseTime << "s)\n";
            } else {
                EV << "Lease renewed for: " << dhcpMsg->getDeviceType() << ", IP: " << assignedIP << "(lease=" << leaseTime << "s)\n";
            }

            // Scheduling renewal before expiration
            SimTime renewTime = simTime() + leaseTime * 0.8;
            cMessage *renewTimer = new cMessage("renewTimer");
            scheduleAt(renewTime, renewTimer);
        }

        delete dhcpMsg;
    }
}
