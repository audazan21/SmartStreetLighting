simple StreetLight
{
    gates:
        output out;
}

simple Gateway{
    gates:
        input in[5];
}

network Network
{
    submodules:
        light[5]: StreetLight;
        gateway: Gateway;
        
    connections:
        	for i=0..4{
            light[i].out --> gateway.in[i];
            }
}