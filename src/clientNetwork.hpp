#include <string>
#include <vector>
using namespace std;

class ClassNetwork {
    public:
        clientNetwork();
        
        ~clientNetwork();

        bool connectToServer (cosnt string& IP, int port);

        void disconnect();

        bool sendBuffer(
            const vector<char>& buffer
        );

        int receiveBuffer(
            vector<char>& buffer
        );

        int getSocketFD() const;

    private:
        int SocketFD;
};