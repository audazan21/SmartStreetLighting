import inet.applications.contract.IApp;

simple SmartNetworkServerApp like IApp
{
    parameters:
        int localPort = default(1000);
        int destPort = default(2000);
        string localAddress = default("");
        string destAddresses = default("");
        string adrMethod = default("avg");
        bool evaluateADRinServer = default(false);
        double adrDeviceMargin = default(10);

    gates:
        input socketIn;
        output socketOut;
}