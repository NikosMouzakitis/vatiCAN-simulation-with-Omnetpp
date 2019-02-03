simple Md
{
    gates:
        input i1;
        input i2;
        input i3;
        input i4;
        output o1;
        output o2;
        output o3;
        output o4;
}

network Vic
{
    @display("bgb=761,314");

    submodules:

        NODE_ONE: Md {
            @display("p=186,271");
        }

        // bridge to simulate that is a bus 'alike' topology
        bridge: Md {
            @display("p=408,43");
        }

        //bridging the messages between [node a] and [node c]
        NODE_TWO: Md {

            @display("p=408,264");
        }
        //future GRC-Server functionality
        // to be implemented for node 'd'.
       // GRC_SERVER: Md {


          //  @display("p=73,43");
        //}

        NODE_THREE: Md {

            @display("p=647,271");
        }


    connections allowunconnected:

        bridge.o1 --> {  delay = 100ms; } --> NODE_ONE.i1;
        bridge.o2 --> {  delay = 100ms; } --> NODE_TWO.i2;
        //bridge.o3 --> {  delay = 100ms; } --> GRC_SERVER.i1;
        bridge.o4 --> {  delay = 100ms; } --> NODE_THREE.i4;

        NODE_ONE.o2 --> {  delay = 100ms; } --> bridge.i1;
        NODE_TWO.o2 --> {  delay = 100ms; } --> bridge.i2;
        //GRC_SERVER.o2 --> {  delay = 100ms; } --> bridge.i3;
        NODE_THREE.o2 --> {  delay = 100ms; } --> bridge.i4;

}
