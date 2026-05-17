#include <omnetpp.h>
#include <cstring>
#include <iomanip>
#include <queue>

#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace omnetpp;
using namespace inet;

class SmartNetworkServerApp : public cSimpleModule
{
private:
    UdpSocket socket;

    std::queue<Packet *> faultQueue;
    std::queue<Packet *> telemetryQueue;

    cMessage *serviceDoneEvent;
    Packet *currentPacket;

    bool isBusy;

    int receivedTelemetryPackets;
    int receivedFaultPackets;
    int servedTelemetryPackets;
    int servedFaultPackets;
    int servedTotalPackets;
    int maxQueueSize;

    simtime_t totalFaultDelay;
    simtime_t totalTelemetryDelay;

    double randomServiceTime() {
        return exponential(1.0);
    }

protected:
    virtual int numInitStages() const override {
        return NUM_INIT_STAGES;
    }

    virtual void initialize(int stage) override {
        if(stage == INITSTAGE_LOCAL) {
            EV << "SmartNetworkServerApp with PRIORITY QUEUE started.\n";

            serviceDoneEvent = new cMessage("serviceDoneEvent");
            currentPacket = nullptr;
            isBusy = false;

            receivedTelemetryPackets = 0;
            receivedFaultPackets = 0;
            servedTelemetryPackets = 0;
            servedFaultPackets = 0;
            servedTotalPackets = 0;
            maxQueueSize = 0;

            totalFaultDelay = 0;
            totalTelemetryDelay = 0;
        }
        else if(stage == INITSTAGE_APPLICATION_LAYER) {
            int localPort = par("localPort");

            socket.setOutputGate(gate("socketOut"));
            socket.bind(localPort);

            EV << "SmartNetworkServerApp UDP bind completed on port: "
               << localPort << "\n";
        }
    }

    void startNextService() {
        if(!faultQueue.empty()) {
            currentPacket = faultQueue.front();
            faultQueue.pop();

            EV << "Server starts servicing PRIORITY FAULT packet. "
               << "Fault queue: " << faultQueue.size()
               << " Telemetry queue: " << telemetryQueue.size()
               << "\n";

            isBusy = true;
            scheduleAt(simTime() + randomServiceTime(), serviceDoneEvent);
        }
        else if(!telemetryQueue.empty()) {
            currentPacket = telemetryQueue.front();
            telemetryQueue.pop();

            EV << "Server starts servicing TELEMETRY packet. "
               << "Fault queue: " << faultQueue.size()
               << " Telemetry queue: " << telemetryQueue.size()
               << "\n";

            isBusy = true;
            scheduleAt(simTime() + randomServiceTime(), serviceDoneEvent);
        }
        else {
            currentPacket = nullptr;
            isBusy = false;
            EV << "Server queue is empty now.\n";
        }
    }

    virtual void handleMessage(cMessage *msg) override {
        if(msg == serviceDoneEvent) {
            servedTotalPackets++;

            simtime_t delay = simTime() - currentPacket->getTimestamp();

            if(strcmp(currentPacket->getName(), "fault") == 0) {
                servedFaultPackets++;
                totalFaultDelay += delay;

                EV << "FAULT packet serviced. Total delay: "
                   << std::fixed << std::setprecision(2)
                   << delay.dbl()
                   << " seconds\n";
            }
            else if(strcmp(currentPacket->getName(), "telemetry") == 0) {
                servedTelemetryPackets++;
                totalTelemetryDelay += delay;

                EV << "TELEMETRY packet serviced. Total delay: "
                   << std::fixed << std::setprecision(2)
                   << delay.dbl()
                   << " seconds\n";
            }

            delete currentPacket;
            currentPacket = nullptr;

            startNextService();
            return;
        }

        Packet *pkt = check_and_cast<Packet *>(msg);

        EV << "SmartNetworkServerApp received packet: "
           << pkt->getName()
           << " at time: "
           << std::fixed << std::setprecision(2)
           << simTime().dbl()
           << "\n";

        if(strcmp(pkt->getName(), "fault") == 0) {
            receivedFaultPackets++;
            faultQueue.push(pkt);

            EV << "FAULT packet added to PRIORITY queue. Fault queue size: "
               << faultQueue.size() << "\n";
        }
        else if(strcmp(pkt->getName(), "telemetry") == 0) {
            receivedTelemetryPackets++;
            telemetryQueue.push(pkt);

            EV << "TELEMETRY packet added to normal queue. Telemetry queue size: "
               << telemetryQueue.size() << "\n";
        }
        else {
            EV << "Unknown packet type received, deleting packet.\n";
            delete pkt;
            return;
        }

        int totalQueueSize = faultQueue.size() + telemetryQueue.size();
        if(totalQueueSize > maxQueueSize) {
            maxQueueSize = totalQueueSize;
        }

        if(!isBusy) {
            startNextService();
        }
    }

    virtual void finish() override {
        EV << "SmartNetworkServerApp finished.\n";

        while(!faultQueue.empty()) {
            delete faultQueue.front();
            faultQueue.pop();
        }

        while(!telemetryQueue.empty()) {
            delete telemetryQueue.front();
            telemetryQueue.pop();
        }

        cancelAndDelete(serviceDoneEvent);

        EV << "Total packets received at server: "
           << receivedTelemetryPackets + receivedFaultPackets << "\n";

        EV << "Total telemetry packets received at server: "
           << receivedTelemetryPackets << "\n";

        EV << "Total fault packets received at server: "
           << receivedFaultPackets << "\n";

        EV << "Total packets served by priority queue: "
           << servedTotalPackets << "\n";

        EV << "Served telemetry packets: "
           << servedTelemetryPackets << "\n";

        EV << "Served fault packets: "
           << servedFaultPackets << "\n";

        EV << "Maximum server queue size: "
           << maxQueueSize << "\n";

        recordScalar("receivedTelemetryPackets", receivedTelemetryPackets);
        recordScalar("receivedFaultPackets", receivedFaultPackets);
        recordScalar("servedTelemetryPackets", servedTelemetryPackets);
        recordScalar("servedFaultPackets", servedFaultPackets);
        recordScalar("servedTotalPackets", servedTotalPackets);
        recordScalar("maximumServerQueueSize", maxQueueSize);

        if(servedFaultPackets > 0) {
            double avgFaultDelay = totalFaultDelay.dbl() / servedFaultPackets;
            EV << "Average FAULT delay with priority queue: "
               << std::fixed << std::setprecision(2)
               << avgFaultDelay << " seconds\n";

            recordScalar("averageFaultDelayPriorityQueue", avgFaultDelay);
        }
        else {
            EV << "No fault packet was served.\n";
            recordScalar("averageFaultDelayPriorityQueue", 0);
        }

        if(servedTelemetryPackets > 0) {
            double avgTelemetryDelay = totalTelemetryDelay.dbl() / servedTelemetryPackets;
            EV << "Average TELEMETRY delay with queue: "
               << std::fixed << std::setprecision(2)
               << avgTelemetryDelay << " seconds\n";

            recordScalar("averageTelemetryDelayQueue", avgTelemetryDelay);
        }
        else {
            recordScalar("averageTelemetryDelayQueue", 0);
        }
    }
};

Define_Module(SmartNetworkServerApp);
