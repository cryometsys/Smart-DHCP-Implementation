// Defining Server module
simple DHCPServer
{
    gates:
        input in[4];
        output out[4];
}
package smartdhcp;

// Defining Client module
simple DHCPClient
{
    parameters:
        string deviceType;
    gates:
        input in;
        output out;
}


network SmartDHCPNetwork
{
    parameters:
        int numClients = default(3);
    submodules:
        server: DHCPServer {
            @display("p=100,100;i=device/server");
        }
        clients[numClients]: DHCPClient {
            @display("p=50,200+i*60;i=device/pc");  // ← This spreads them horizontally
        }
    connections:
        for i=0..numClients-1 {
            clients[i].out --> server.in[i];
            server.out[i] --> clients[i].in;
        }
}