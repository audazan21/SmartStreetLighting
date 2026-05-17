#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include "LoRa/LoRaTagInfo_m.h"
#include <iomanip>
#include "LoRaApp/LoRaAppPacket_m.h"

using namespace omnetpp;
using namespace inet;
using namespace flora;

class StreetLightApp : public cSimpleModule
{
  private:
    cMessage *telemetryEvent;
    cMessage *faultEvent;
    simtime_t nextAllowedSendTime;
    cOutVector faultIntervalsVector;
    int sentTelemetryPackets;
    int sentFaultPackets;

    double randomnumbergenerator(){
        return uniform(0,1);
    }

    double randomfaultgenerator(){
        double faultInterval = exponential(par("faultMeanTime").doubleValue());
        faultIntervalsVector.record(faultInterval);
        return faultInterval;
    }

    void addLoRaTag(Packet *packet)
    {
        auto tag = packet->addTagIfAbsent<LoRaTag>();

        tag->setPower(mW(math::dBmW2mW(par("initialLoRaTP").doubleValue())));
        tag->setCenterFrequency(Hz(par("initialLoRaCF").doubleValue()));
        tag->setBandwidth(Hz(par("initialLoRaBW").doubleValue()));
        tag->setSpreadFactor(par("initialLoRaSF").intValue());
        tag->setCodeRendundance(par("initialLoRaCR").intValue());
        tag->setUseHeader(par("initialUseHeader").boolValue());
    }

  protected:
    virtual void initialize() override {
        EV << "StreetLightApp started to work.\n";

        telemetryEvent = new cMessage("telemetryEvent");
        faultEvent = new cMessage("faultEvent");
        nextAllowedSendTime = 0;
        faultIntervalsVector.setName("faultInterArrivalTimes");
        sentTelemetryPackets = 0;
        sentFaultPackets = 0;

        scheduleAt(simTime() + uniform(1, 20), telemetryEvent);
        scheduleAt(simTime() + randomfaultgenerator(), faultEvent);
    }

    virtual void handleMessage(cMessage *msg) override {
        if(msg == faultEvent){

            EV << getFullName() << " STREETLIGHT FAILED at time: "
               << std::fixed << std::setprecision(2)
               << simTime().dbl() << "\n";

            if (simTime() < nextAllowedSendTime) {
                cancelEvent(faultEvent);
                scheduleAt(nextAllowedSendTime + uniform(0.1, 1.0), faultEvent);
                return;
            }

            Packet *faultMsg = new Packet("fault");
            faultMsg->setKind(DATA);

            auto payload = makeShared<LoRaAppPacket>();
            payload->setChunkLength(B(10));
            payload->setSampleMeasurement(999);

            faultMsg->insertAtBack(payload);
            faultMsg->setTimestamp(simTime());
            addLoRaTag(faultMsg);

            sentFaultPackets++;
            send(faultMsg, "socketOut");

            nextAllowedSendTime = simTime() + par("minimumSendGap").doubleValue();

            scheduleAt(simTime() + randomfaultgenerator(), faultEvent);

            return;
        }

        if(msg == telemetryEvent){

            EV << getFullName() << " generated telemetry at time: "
               << std::fixed << std::setprecision(2)
               << simTime().dbl() << "\n";

            if (simTime() < nextAllowedSendTime) {
                cancelEvent(telemetryEvent);
                scheduleAt(nextAllowedSendTime + uniform(0.1, 1.0), telemetryEvent);
                return;
            }

            Packet *newMsg = new Packet("telemetry");
            newMsg->setKind(DATA);

            auto payload = makeShared<LoRaAppPacket>();
            payload->setChunkLength(B(10));
            payload->setSampleMeasurement(1);

            newMsg->insertAtBack(payload);
            newMsg->setTimestamp(simTime());
            addLoRaTag(newMsg);

            sentTelemetryPackets++;
            send(newMsg, "socketOut");

            nextAllowedSendTime = simTime() + par("minimumSendGap").doubleValue();

            double nexttime = par("telemetryBaseTime").doubleValue() + randomnumbergenerator();

            scheduleAt(simTime() + nexttime, telemetryEvent);

            return;
        }

        delete msg;
    }

    virtual void finish() override {
        recordScalar("sentTelemetryPackets", sentTelemetryPackets);
        recordScalar("sentFaultPackets", sentFaultPackets);

        EV << "StreetLightApp finished. Sent telemetry packets: "
           << sentTelemetryPackets
           << ", Sent fault packets: "
           << sentFaultPackets
           << "\n";

        cancelAndDelete(telemetryEvent);
        cancelAndDelete(faultEvent);
    }
};

Define_Module(StreetLightApp);
