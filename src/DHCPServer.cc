#include "DHCPServer.h"
#include <cstring>

using namespace omnetpp;

Define_Module(DHCPServer);

void DHCPServer::initialize()
{
    EV << "DHCPServer started.\n";
    ipPool = {"192.168.1.10", "192.168.1.11", "192.168.1.12"};

    cMessage *timer = new cMessage("cleanupTimer");
    scheduleAt(simTime() + 1.0, timer);
}

void DHCPServer::handleMessage(cMessage *msg) {

    if (msg->isSelfMessage() && strcmp(msg->getName(), "cleanupTimer") == 0) {
        cleanExpiredLease();
        cMessage *nextTimer = new cMessage("cleanupTimer");
        scheduleAt(simTime() + 1.0, nextTimer);
        delete msg;
        return;
    }

    DHCPMessage *dhcpMsg = check_and_cast<DHCPMessage *>(msg);

    if (strcmp(dhcpMsg->getMsgType(), "DISCOVER") == 0) {
        std::string deviceType = dhcpMsg->getDeviceType();
        int clientIndex = msg->getArrivalGate()->getIndex();
        EV << "Received DISCOVER from " << deviceType << "\n";

        // IP Assignment
        std::string assignedIP = assignIP(deviceType, clientIndex);

        if (assignedIP.empty()) {
            EV << "DHCP request from " << deviceType << " DENIED: no free IP\n";
            delete msg;
            return;
        }

        // Lease time determination
        double leaseTime;
        if (deviceType == "IoT") leaseTime = 10;
        else if (deviceType == "Desktop") leaseTime = 30;
        else leaseTime = 60;


        // OFFER Message
        DHCPMessage *offer = new DHCPMessage("OFFER");
        offer->setMsgType("OFFER");
        offer->setDeviceType(deviceType.c_str());
        offer->setAssignedIP(assignedIP.c_str());
        offer->setLeaseTime(leaseTime);

        // Returning to client
        send(offer, "out", clientIndex);
        EV << "Sent OFFER: " << assignedIP << " (lease=" << leaseTime << "s)\n";
    }
    else if (strcmp(dhcpMsg->getMsgType(), "REQUEST") == 0) {

        std::string deviceType = dhcpMsg->getDeviceType();
        std::string requestedIP = dhcpMsg->getAssignedIP();
        int clientIndex = msg->getArrivalGate()->getIndex();

        EV << "Received REQUEST from " << deviceType
           << " for IP " << requestedIP << "\n";

        auto it = leaseTable.find(requestedIP);
        if (it == leaseTable.end() || !it->second.active) {
           EV << "REQUEST for unassigned IP " << requestedIP << " ignored\n";
           delete msg;
           return;
       }

        if (it != leaseTable.end() && it->second.active) {

            // Extending lease duration
            double leaseTime = (deviceType == "IoT") ? 10 : (deviceType == "Desktop" ? 30 : 60);
            it->second.expiryTime = simTime() + leaseTime;
            it->second.deviceType = deviceType;
        }

        // Sending ACK
        DHCPMessage *ack = new DHCPMessage("ACK");
        ack->setMsgType("ACK");
        ack->setDeviceType(deviceType.c_str());
        ack->setAssignedIP(requestedIP.c_str());
        ack->setLeaseTime(dhcpMsg->getLeaseTime());

        send(ack, "out", clientIndex);
        EV << "Sent ACK for IP " << requestedIP << "\n";
    }

    delete msg;
}

void DHCPServer::cleanExpiredLease() {

    bool expiredFound = false;

    for (auto& entry : leaseTable) {
        if (entry.second.active && simTime() >= entry.second.expiryTime) {
            entry.second.active = false;
            EV << "LEASE EXPIRED: " << entry.first << "\n";
            expiredFound = true;
        }
    }

    std::vector<std::string> freeIPs;
    for (const auto& ip : ipPool) {
        auto it = leaseTable.find(ip);
        if (it == leaseTable.end() || !it->second.active) {
            freeIPs.push_back(ip);
        }
    }

    if (expiredFound || freeIPs.size() == ipPool.size()) {
        EV << "Free IPs: [";
        for (size_t i = 0; i < freeIPs.size(); ++i) {
            if (i > 0) EV << ", ";
            EV << freeIPs[i];
        }
        EV << "]\n";
    }
}

std::string DHCPServer::assignIP(const std::string& deviceType, int clientIndex) {
    cleanExpiredLease();

    // Reusing existing IP
    for (auto& ip : ipPool) {
        auto it = leaseTable.find(ip);
        if (it == leaseTable.end() || !it->second.active) {
            double leaseTime;
            if (deviceType == "IoT") leaseTime = 10;
            else if (deviceType == "Desktop") leaseTime = 30;
            else leaseTime = 60;

            LeaseInfo info;
            info.deviceType = deviceType;
            info.expiryTime = simTime() + leaseTime;
            info.active = true;
            leaseTable[ip] = info;

            EV << "Assigned " << ip << " to " << deviceType << " (lease=" << leaseTime << "s)\n";
            return ip;
        }
    }
    EV << "No IP free for " << deviceType << " .\n";
    return "";
}
