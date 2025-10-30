#ifndef __SMARTDHCP_DHCPCLIENT_H_
#define __SMARTDHCP_DHCPCLIENT_H_


#include <omnetpp.h>
#include "DHCPMessage_m.h"

using namespace omnetpp;

class DHCPClient : public cSimpleModule
{
  protected:
    std::string deviceType;
    std::string assignedIP;
    double leaseTime;
    int renewalCount = 0;
    bool hasIP = false;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
