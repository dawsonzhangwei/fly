/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                    _______    _                                     *
 *                   (  ____ \  ( \     |\     /|                      * 
 *                   | (    \/  | (     ( \   / )                      *
 *                   | (__      | |      \ (_) /                       *
 *                   |  __)     | |       \   /                        *
 *                   | (        | |        ) (                         *
 *                   | )        | (____/\  | |                         *
 *                   |/         (_______/  \_/                         *
 *                                                                     *
 *                                                                     *
 *     fly is an awesome c++11 network library.                        *
 *                                                                     *
 *   @author: lichuan                                                  *
 *   @qq: 308831759                                                    *
 *   @email: 308831759@qq.com                                          *
 *   @github: https://github.com/lichuan/fly                           *
 *   @date: 2015-06-10 13:34:21                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <unistd.h>
#include <unordered_map>
#include "fly/init.hpp"
#include "fly/net/server.hpp"
#include "fly/base/logger.hpp"

using namespace std::placeholders;

class Test_Server : public fly::base::Singleton<Test_Server>
{
public:
    bool allow(std::shared_ptr<fly::net::Connection> connection)
    {
        return true;
    }
    
    void init(std::shared_ptr<fly::net::Connection> connection)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections[connection->id()] = connection;
        LOG_INFO("connection count: %u", m_connections.size());
    }
    
    void dispatch(std::unique_ptr<fly::net::Message_Pack> connection)
    {
        LOG_INFO("disaptch message");
    }
    
    void close(std::shared_ptr<fly::net::Connection> connection)
    {
        LOG_INFO("close connection from %s:%d", connection->peer_addr().m_host.c_str(), connection->peer_addr().m_port);
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections.erase(connection->id());
    }
    
    void be_closed(std::shared_ptr<fly::net::Connection> connection)
    {
        LOG_INFO("connection from %s:%d be closed", connection->peer_addr().m_host.c_str(), connection->peer_addr().m_port);
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections.erase(connection->id());
    }
    
    void main()
    {
        //init library
        fly::init();
        
        //init logger
        fly::base::Logger::instance()->init(fly::base::DEBUG, "server", "./log/");
        
        //test tcp server
        std::unique_ptr<fly::net::Server> server(new fly::net::Server(fly::net::Addr("127.0.0.1", 8899),
                                                                      std::bind(&Test_Server::allow, this, _1),
                                                                      std::bind(&Test_Server::init, this, _1),
                                                                      std::bind(&Test_Server::dispatch, this, _1),
                                                                      std::bind(&Test_Server::close, this, _1),
                                                                      std::bind(&Test_Server::be_closed, this, _1)));

        if(server->start())
        {
            LOG_INFO("start server ok!");
            server->wait();
        }
        else
        {
            LOG_ERROR("start server failed");
        }
    }
    
private:
    std::unordered_map<uint64, std::shared_ptr<fly::net::Connection>> m_connections;
    std::mutex m_mutex;
};

int main()
{
    Test_Server::instance()->main();
}