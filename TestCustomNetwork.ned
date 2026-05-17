network TestCustomNetwork
{
    submodules:
        light[5]: StreetLightApp;
        gatewayQueue: GatewayQueue;

    connections allowunconnected:
        for i=0..4 {
            light[i].socketOut --> gatewayQueue.in[i];
        }
}