#include "dc/net/tcpClient.h"
#include "dc/base/logging.h"
#include "dc/net/eventLoop.h"
#include "dc/net/inetAddress.h"

#include <utility>
#include <cstdio>
#include <unistd.h>
#include <vector>
using namespace dc;
using namespace dc::net;

int numThreads;
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;

int current;

class EchoClient : noncopyable
{
public:
    EchoClient(EventLoop* loop, const InetAddress& listenAddr, const string& id)
        : loop_(loop),
          client_(loop, listenAddr, "EchoClient" + id)
    {
        client_.setConnectionCallback(std::bind(&EchoClient::onConnection, this, _1));
        client_.setMessageCallback(std::bind(&EchoClient::onMessage, this, _1, _2, _3));
    }

    void connect()
    {
        client_.connect();
    }


private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_TRACE << conn->localAddress().toIpPort() << " -> "
                  << conn->peerAddress().toIpPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");
        if( conn->connected())
        {
            ++current;
            if( implicit_cast<size_t>(current) < clients.size())
            {
                clients[current]->connect();
            }
            LOG_INFO << "*** connected " << current;
        }
        conn->send(StringPiece("world\n"));

    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        string msg(buf->retrieveAllAsString());
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
        if (msg == "quit\n")
        {
            conn->send(string("bye\n"));
            conn->shutdown();
        }
        else if(msg == "shutdown\n")
        {
            loop_->quit();
        }
    }
    EventLoop* loop_;
    TcpClient client_;
};

int main(int argc, char** argv)
{
    g_logLevel = Logger::TRACE;
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    if( argc > 1)
    {
        EventLoop loop;
        bool ipv6 = argc > 3;
        InetAddress serverAddr(argv[1], 12345, ipv6);

        int n = 1;
        if( argc > 2)
        {
            n = atoi(argv[2]);
        }
        clients.reserve(n);
        for( int i = 0; i < n; ++i )
        {
            char buf[32];
            snprintf(buf, sizeof buf, "%d", i + 1);
            clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
        }
        clients[current]->connect();
        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip [current#]\n", argv[0]);
    }
    return 0;
}