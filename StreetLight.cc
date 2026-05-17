#include <omnetpp.h>
#include <iomanip> //for printing time after . only 2 digit
using namespace omnetpp;
class StreetLight : public cSimpleModule   //our streetlight module
{
    private:
    cMessage *telemetryEvent;   // cmesssage means event * means dynamic telemtryevent is our time event
    cMessage *faultEvent;
    float randomnumbergenerator(){ //random number generator for random comes
       double rng;
       rng=uniform(0,1);
        return rng;
    }
    float randomfaultgenerator(){  //randomm number generator for random time faults
        double rfg;
        rfg=exponential(5);
        return rfg;
    }

    protected:
      virtual void initialize() override {    // start once when simulation works one time
            EV << "StreetLight started to work.\n";   //print

            telemetryEvent = new cMessage("telemetryEvent");  //we create a new event  send to us not real, inside
            faultEvent = new cMessage("faultEvent");  //we created a new event for faults

            scheduleAt(1.0, telemetryEvent);  // in 1st second this event done
            scheduleAt(simTime() + randomfaultgenerator(), faultEvent);   // at random time event fault came and street light fault
      }

      virtual void handleMessage(cMessage *msg) override {   // works when event comes
          if(msg == faultEvent){

              EV << getFullName()<< " STREETLIGHT FAILED at time: "<< std::fixed << std::setprecision(2)<< simTime().dbl() << "\n";  //print when fault occurs

              scheduleAt(simTime() + randomfaultgenerator(), faultEvent);

              cMessage *faultMsg = new cMessage("fault");   // when a case of fault send fault message
              faultMsg->setTimestamp(simTime());  // when fault occurs we wrote the time inside of the pack
              EV << getFullName() << " is sending FAULT message...\n";  //when fault to see the which one has fault
              send(faultMsg, "out");  //send fault message

              return;
          }


            EV << getFullName()<<"generated telemetry at time: " << std::fixed << std::setprecision(2) << simTime().dbl() << "\n";  //print at that time time

            cMessage *newMsg = new cMessage("telemetry");  //message that sended real
                // in omnet ++ everything is message event, packet,signal,alarm....


                EV<<getFullName()<< "is sending telemetry message...\n";  //before send give info to console
                send(newMsg, "out");  // sending message from out door from NED Streetlight
                float nexttime = 2.0+randomnumbergenerator();  //randomness
            scheduleAt(simTime() + nexttime, telemetryEvent);   //after 2 seconds done again same event
      }
      virtual void finish() override {  //works after simulation done
            cancelAndDelete(telemetryEvent);  //delete the telemetry event
            cancelAndDelete(faultEvent);  //same delete the fault event
        }

};
Define_Module(StreetLight);
