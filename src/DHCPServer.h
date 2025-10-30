#ifndef __SMARTDHCP_DHCPSERVER_H_
#define __SMARTDHCP_DHCPSERVER_H_

#include <omnetpp.h>
#include "DHCPMessage_m.h"
#include <map>
#include <string>

using namespace omnetpp;

struct LeaseInfo {
    std::string clientID;
    std::string deviceType;
    SimTime expiryTime;
    bool active = true;
};

class DHCPServer : public cSimpleModule
{
  protected:
    std::vector<std::string> ipPool;
    std::map<std::string, LeaseInfo> leaseTable;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    std::string assignIP(const std::string& deviceType, int clientIndex);
    void cleanExpiredLease();
};

#endif
