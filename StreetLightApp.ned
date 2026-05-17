import inet.applications.contract.IApp;

simple StreetLightApp like IApp
{
    parameters:
        double telemetryBaseTime @unit(s) = default(120s);
        double faultMeanTime @unit(s) = default(600s);
        double minimumSendGap @unit(s) = default(20s);

        double initialLoRaTP @unit(dBm) = default(14dBm);
        double initialLoRaCF @unit(Hz) = default(868MHz);
        double initialLoRaBW @unit(Hz) = default(125kHz);
        int    initialLoRaSF = default(7);
        int    initialLoRaCR = default(4);
        bool   initialUseHeader = default(true);

    gates:
        input socketIn;
        output socketOut;
}