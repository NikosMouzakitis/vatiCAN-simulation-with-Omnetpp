# vatiCAN-simulation-with-Omnetpp
![img](https://github.com/NikosMouzakitis/vatiCAN-simulation-with-Omnetpp/blob/master/sample.png)
Simulation of the vatiCAN automotive protocol for secure message exhanges, with the omnet simulation kernel.

More information for the protocol can be found here: www.automotive-security.net/vatican/
Bus is simulated with a bridge node connected to all the nodes and using symmetrical propagation
delays in all gateways.

In this example NODE_ONE will transmit a secure message to NODE_TWO who is also a secure node, while NODE_THREE
who is just supporting Legacy messages will not be able to validate the data transmitted by NODE_ONE.

#automotive #security #omnetpp #omnest #vatiCAN
