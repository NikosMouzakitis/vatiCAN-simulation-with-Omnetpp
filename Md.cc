/*
 *  Md.cc
 *
 *  Created on: Jan 30, 2019
 *
 */
#include <string.h>
#include <omnetpp.h>
#include "vatig.h"  //vatiCAN-G protocol functions.
using namespace omnetpp;
/*
        Simulation of a very simple CAN Bus topology where
        middle node 'bridge' is used with symmetrical delays
        to give the impression of a bus-alike system.

        NODE_ONE:   Sender of CAN-frame.
        NODE_TWO:   Receiver of the CAN-frame.
        NODE_THREE: No access to keys and cannot validate vatiCAN-G messages.
*/
class Md : public cSimpleModule
{
    private:
        int counter;
        int seq;
        double timeout;
        cMessage *timeoutEvent;
        cMessage *message;
        int vg_cnt = 0; // vatiCAN-G counter for secure messages.
        int data[2];    // data to broadcast.
        int mac2[2];

    public:

        Md();
        virtual ~Md();

        unsigned int PartList[8];
        unsigned int PartListLen;
        unsigned int sessionKey;
        int secure_cnt = 0;     // used to determine the current phase of secure communication.
        int participant = 0;    // node is a participant in the current phase.

    protected:
        virtual void    initialize();
        virtual void    handleMessage(cMessage *msg);
        virtual void    sendCopyOf(cMessage * msg, int flag);
        virtual cMessage *newMessage();

        //CAN related functionality.

        virtual cMessage *createCANmsg(int id, int upper32, int lower32, int print_flag);
        virtual void    printCANFrame(char *tmp);

        void sendVatiCANmsg(int vg_cnt,int * data);
        void sendCopyOfG(cMessage *msg, int schedule_flag);
        void actOnMsg(cMessage *msg, int amLegacyNode);
};

Define_Module(Md);

Md::Md(){
    timeout = 1.0;
    timeoutEvent = NULL;
}

Md::~Md()
{
    cancelAndDelete(timeoutEvent);
    delete timeoutEvent;
}

// Used for the vatiCAN-G messages.
// Schedule flag is used on the 3rd message to reschelude
// a secure group communication.
void Md::sendCopyOfG(cMessage *msg, int schedule_flag)
{
    float timeOutG = 5.0;
    cMessage *copy = (cMessage *)msg->dup();

    if(strcmp("NODE_ONE",getName()) == 0){
            copy->setKind(1);
            send(copy, "o2");
            //schedules a self-message for periodic transmission.
            if(schedule_flag)
                scheduleAt(simTime() + timeOutG,timeoutEvent);
    }
}

void Md::sendCopyOf(cMessage *msg, int flag)
{
    cMessage *copy = (cMessage *)msg->dup();

    if(strcmp("NODE_ONE",getName()) == 0){
        copy->setKind(1);
        send(copy, "o2");
        //schedules a self-message for periodic transmission.
        scheduleAt(simTime() + timeout,timeoutEvent);
    } else if(strcmp("NODE_TWO",getName()) == 0){
        copy->setKind(1);
        send(copy, "o2");
        //scheduleAt(simTime() + timeout,timeoutEvent);
    } else if(flag == 1) {
                        //forwarding message from NODE_ONE
        copy->setKind(1);
        send(copy,"o2");
        //copy = (cMessage *)msg->dup();
        //send(copy,"o3");
        copy = (cMessage *)msg->dup();
        copy->setKind(1);
        send(copy,"o4");
    } else if(flag == 2) {
        //forwarding message from NODE_TWO
        copy->setKind(1);
        send(copy,"o1");
        //copy = (cMessage *)msg->dup();
        //send(copy,"o3");
    }
}
//                                  createCANmsg()                            --------------------------
// creation of a CAN message, | ID | Upper32 | lower32 |   upper+lower should be 8 bytes in theory.
// using small values here    --------------------------   so we can implement it with 3 characters.
// Valid values 0-127..(just for the simulation) // :)

cMessage * Md::createCANmsg(int id, int upper32, int lower32, int print_flag)
{
    int i;
    char msgname[3];

    //passing parameters
    msgname[0] = id;
    msgname[1] = upper32;
    msgname[2] = lower32;

    if(print_flag)
        for(i = 0; i < 3; i++)
            EV << "Frame[" << (int) i << "] " << (int)msgname[i] <<  "\n";
    cMessage *msg = new cMessage(msgname);

    return msg;
}

//            sendVatiCANGmsg()
// broadcasting a secure group message on the Bus.
void Md::sendVatiCANmsg(int secure, int * data)
{
    char msgname[3];
    msgname[0] = secure;
    // passing the data for the second phase of secure group communication.
    msgname[1] = *(int *) data;
    msgname[2] = *(int *) data+1;
    message = createCANmsg(msgname[0], msgname[1], msgname[2], 0);
    message->setKind(1);
    sendDelayed(message, 0.4, "o2");

    //creation of MAC for validation of the data ( second phase of secure group communication).

    createMac( mac2, vg_cnt, msgname[1], msgname[2]);
    vg_cnt++;
    msgname[1] = *(int *) mac2;
    msgname[2] = *(int *) (mac2 + 1);

    message = createCANmsg(msgname[0], msgname[1], msgname[2], 0);
    message->setKind(1);
    sendDelayed(message, 0.8, "o2");
    EV << "MAC2\n";
    EV << msgname[1] << "\n";
    EV << msgname[2] << "\n";

    scheduleAt(simTime() + 5.0,timeoutEvent);
}

cMessage * Md::newMessage()
{
    char msgname[10];
    sprintf(msgname,"SeqNo-%d",++seq);
    cMessage *msg = new cMessage(msgname);
    return msg;
}

void Md::initialize()
{
    memset( (void *) mac2, 0, 2*sizeof(int)); // clearing the Mac2 field on initialization.
    // data to transmit ( in this test scenario case transmiting 111, 112 as Upper32 and Lower32 respectively)
    data[0] = 111;
    data[1] = 112;

    timeout = 11.0;
    timeoutEvent = new cMessage("timeoutEvent");

    for(int i = 0; i < SUPPORTED_NODES; i++)
        PartList[i] = 0;

    PartListLen = 0;

    if(strcmp("NODE_ONE", getName()) == 0){
        EV << "Sending Initial message...\n";
        //creation of a CAN message.
        //message = createCANmsg(legacy[0], 101, 102, 1);
        //sendCopyOf(message,0);

        //sending a vatiCAN-G message with participants NODE_ONE and NODE_TWO.
        sendVatiCANmsg(secure[0], data);
    }
}

void Md::printCANFrame(char *tmp)
{
    EV << "ID:    " << (int)tmp[0] << " Upper: " << (int)tmp[1] << " Lower: " << (int)tmp[2] << "\n" ;
}

//  Determining whether the message comes from a secure sender
//  and taking appropriate action on decoding an authenticating
//  the data in case node is participating in the secure communication.
void Md::actOnMsg(cMessage *msg, int amLegacyNode)
{
    int ret;
    char *tmp;
    int isSec = 0;
    tmp = (char *) msg->getFullName();

    int id = (int) tmp[0];
    int upper = (int) tmp[1];
    int lower = (int) tmp[2];

    for( int i = 0; i < SUPPORTED_NODES; i++) {

        if(id == secure[i]) {
            EV << "SECURE GROUP MESSAGE\n";
            secure_cnt++;
            isSec++;
            break;
        }
    }

    if( (!isSec)) {
            EV << "Legacy msg\n";
            return;
        }

    if( (amLegacyNode)) {
        EV << "Secure msg, not for me as a legacy node\n";
        return;
    }

    // data reception
    if(secure_cnt == 1) {
        EV << "SECURE NODE RECEPTION\n";
        EV << "DATA RECEIVED: " << upper << " , " << lower << "\n";
        data[0] = upper;
        data[1] = lower;
    }

    if(secure_cnt == 2) {

        createMac( mac2, vg_cnt, data[0], data[1]);
        vg_cnt++;

        if( (mac2[0] == upper) && ( mac2[1] == lower) ) {
            EV << "AUTHENTICATED SECURE MESSAGE\n";
        } else {
            EV << "NOT VALID MAC2, DROPPING MESSAGE\n";
        }

    }

    // MAC2 was received.(reset related counters)
    if( (secure_cnt % 2 ) == 0 ) {
        secure_cnt = 0;
        memset(mac2, 0, 2*sizeof(int));
    }
}

void Md::handleMessage(cMessage *msg)
{
    char *tmp;
    tmp = (char *) msg->getFullName();

    // to see which node i am.
    if(strcmp("bridge",getName()) == 0){

        if( msg->arrivedOn("i1")) { // message from NODE_ONE
            //forwarding it to NODE_TWO and NODE_THREE
            EV << "Bridge reception: \n";
            printCANFrame(tmp);
            message = createCANmsg( (int)tmp[0], (int)tmp[1], (int)tmp[2], 0);
            sendCopyOf(message,1);
        } else if( msg->arrivedOn("i2"))  { // message from Node B
            message = createCANmsg( (int)tmp[0], (int)tmp[1], (int)tmp[2], 0);
            sendCopyOf(message,2);
        }
        return;
    }

    // NODE_ONE will continue transmission in periodic time intervals.
    if(msg == timeoutEvent) {
        //message = createCANmsg(legacy[0], 101, 102, 1);
        //sendCopyOf(message,0);
        sendVatiCANmsg(secure[0], data);

    } else { // can be any message in NODE_ONE or NODE_TWO or NODE_THREE.

        if(strcmp("NODE_ONE",getName())==0){
            EV << "ACK ARRIVED.\n";

            delete msg;
            delete message;
            message = newMessage();
            sendCopyOf(message,0);
        } else if(strcmp("NODE_TWO",getName())==0) {

            EV << "NODE_TWO received: \n";
            printCANFrame(tmp);
            actOnMsg(msg, 0);

            delete msg;
        } else if(strcmp("NODE_THREE",getName())==0) {

            EV << "NODE_THREE received: \n";
            printCANFrame(tmp);
            actOnMsg(msg, 1);

            delete msg;
        }

        /*else if(strcmp("GRC_SERVER",getName())==0) {
            EV << "GRC_SERVER received: \n";
            printCANFrame(tmp);
            actOnMsg(msg);
            delete msg;
        } */


    }
}
