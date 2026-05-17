#include <omnetpp.h>
#include <queue>
#include <cstring>   // I added this library to use strcmp
#include <iomanip> //for printing time after . only 2 digit
using namespace omnetpp;

class GatewayQueue : public cSimpleModule
{
  private:
    std::queue<cMessage *> FaultQueue;
    std::queue<cMessage *> TelemetryQueue;
    cMessage *servicedoneeventsignal;   //timer when service done call me  service is done signal
    cMessage *currentpacket;  //packets is servicing at that time
    bool isbusy;  //is gateway busy
    int servedpackets; // to count
    int servedfaultpackets; // serviced fault packets
    int servedtelemetrypackets;  // serviced telemetry packets
    simtime_t totalfaultdelay;   // total fault value
    int maximumqueuesize;  // to keep the biggest queue size
    double randomservicetimegenerator(){
        double rstg=exponential(1.0);
        return rstg;
    }

  protected:
    virtual void initialize() override{
        EV <<"Gateway started.\n";
// initialize starting values
        servicedoneeventsignal=new cMessage("serviceEvent");  //service event
        isbusy=false;  //gateway empty in the start
        servedpackets=0;
        currentpacket=nullptr;
        servedfaultpackets=0;
        servedtelemetrypackets=0;
        totalfaultdelay=0;
        maximumqueuesize=0;
    }

    virtual void handleMessage(cMessage *msg) override {
        if (msg==servicedoneeventsignal){   //if my service time finished
            servedpackets++;  //message served increment the packets
            if(strcmp(currentpacket->getName(),"fault")==0){ // if fault packet

                simtime_t faultdelay=simTime()-currentpacket->getTimestamp();

                servedfaultpackets++;

                totalfaultdelay+=faultdelay;

                EV<<"FAULT packet delay: "
                  <<std::fixed
                  <<std::setprecision(2)
                  <<faultdelay.dbl()
                  <<" seconds\n";
            }
            else{ // if telemetry packet

                servedtelemetrypackets++;
            }
            delete currentpacket;
            currentpacket=nullptr;  // there is no current packet now because we did our work
            EV <<"Gateway finished servicing a packet at time: "<<std::fixed<<std::setprecision(2)<<simTime().dbl()<<" Total packets served:"<<servedpackets<<"\n";   //print done

            if(FaultQueue.empty()==0){
                currentpacket = FaultQueue.front();
                    FaultQueue.pop();

                    EV << "Gateway starts servicing PRIORITY FAULT packet. Fault queue size: "
                       << FaultQueue.size()
                       << " Telemetry queue size: "
                       << TelemetryQueue.size()
                       << "\n";

                    scheduleAt(simTime()+randomservicetimegenerator(), servicedoneeventsignal);
                    isbusy = true;
            }
            else if(TelemetryQueue.empty()==0){
                currentpacket = TelemetryQueue.front();
                    TelemetryQueue.pop();

                    EV << "Gateway starts servicing TELEMETRY packet. Fault queue size: "
                       << FaultQueue.size()
                       << " Telemetry queue size: "
                       << TelemetryQueue.size()
                       << "\n";

                    scheduleAt(simTime()+randomservicetimegenerator(), servicedoneeventsignal);
                    isbusy = true;

            }
            else{   //queue is empty gateway idle
                EV<<"Gateway queue is empty now.\n";
                isbusy=false;  // gateway idle
    }
    }
        else{  // message came
            //EV<<"Gateway received packet at time: "<< std::fixed<<std::setprecision(2)<<simTime().dbl()<<"\n";

            if(strcmp(msg->getName(),"fault")==0){  //check the packet is it fault or not
                EV<<"Gateway received FAULT packet at time: "<<std::fixed<<std::setprecision(2)<<simTime().dbl()<<"\n";  // it is fault print
            }
            else{
                EV<<"Gateway received TELEMETRY packet at time: "<<std::fixed<<std::setprecision(2)<<simTime().dbl()<<"\n"; // it is telemetry packet
            }

            if (isbusy==0){  //if gateway is empty
                EV<<"Gateway starts servicing packet immediately.\n";   //start service

                currentpacket=msg;  //make coming packet current packet
                scheduleAt(simTime()+randomservicetimegenerator(),servicedoneeventsignal);  //after 1 second it will be done the service the I updated to random number first it was 1.0 + ....
                isbusy=true;
            }
            else{  //if gateway is full add the queue
                if(strcmp(msg->getName(),"fault")==0){
                    FaultQueue.push(msg);

                    EV << "Gateway is busy. FAULT packet added to PRIORITY queue. Fault queue size: "
                       << FaultQueue.size()
                       << "\n";
                }
                else{
                    TelemetryQueue.push(msg);

                    EV << "Gateway is busy. TELEMETRY packet added to normal queue. Telemetry queue size: "
                       << TelemetryQueue.size()
                       << "\n";
                }

                int totalqueuesize = FaultQueue.size() + TelemetryQueue.size();

                if(totalqueuesize > maximumqueuesize){
                    maximumqueuesize = totalqueuesize;
                }
            }}
    }

    virtual void finish() override{
        cancelAndDelete(servicedoneeventsignal);  //delete event
        while(FaultQueue.empty()==0){
            delete FaultQueue.front();
            FaultQueue.pop();
        }

        while(TelemetryQueue.empty()==0){
            delete TelemetryQueue.front();
            TelemetryQueue.pop();
        }
        if(servedfaultpackets>0){
            EV<<"Average FAULT delay: "
              <<std::fixed<<std::setprecision(2)
              <<(totalfaultdelay.dbl()/servedfaultpackets)
              <<" seconds\n";
        }
        else{
            EV<<"No fault packet was serviced.\n";
        }
        EV<<"Total serviced packets: "
          << servedpackets
          << "\n";

        EV<<"Total serviced FAULT packets: "
          << servedfaultpackets
          << "\n";

        EV<<"Total serviced TELEMETRY packets: "
          << servedtelemetrypackets
          << "\n";
        EV<<"Maximum gateway queue size: "<< maximumqueuesize<< "\n";
    }

};

Define_Module(GatewayQueue);   // omnet and ned connection
