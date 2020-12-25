#ifndef _SERVERBASE_
#define _SERVERBASE_

#include "NetzwerkSumm.h"

namespace Netzwerk {
	template<typename T>
	class server_interface
	{
	public:
		server_interface(uint16_t port)
			: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

		}

		virtual ~server_interface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();

				m_threadContext = std::thread([this]() { m_asioContext.run(); });
			}
			catch (std::exception& e)
			{
				ERROR((std::string("NETZWERK::SERVER::START::Exception::")+e.what()).c_str());
				return false;
			}

			LTRACE("NETZWERK::SERVER::START::Sucsessfull");
			return true;
		}

		void Stop()
		{
			m_asioContext.stop();

			if (m_threadContext.joinable()) m_threadContext.join();

			LTRACE("NETZWERK::SERVER::STOPP::Sucsessfull");
		}

		void WaitForClientConnection()
		{
			m_asioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::shared_ptr<connection<T>> newconn =
							std::make_shared<connection<T>>(connection<T>::owner::server,
								m_asioContext, std::move(socket), m_qMessagesIn);

						if (OnClientRequestConnect(newconn))
						{
							m_deqConnections.push_back(std::move(newconn));

							m_deqConnections.back()->ConnectToClient(nIDCounter++);
							OnClientConnect(m_deqConnections.back());
						}
					}
					else
					{
						ERROR((std::string("NETZWERK::SERVER::CONNECTION::ERROR::") + ec.message()).c_str());
					}
					WaitForClientConnection();
				});
		}

		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				message<T> toSend = OnClientDisconnect(client);
				client.reset();
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
				MessageAllClients(toSend);
			}
		}
		void MessageClientGroup(const message<T>& msg, std::vector<std::shared_ptr<connection<T>>> group)
		{
			bool bInvalidClientExists = false;
			std::vector<message<T>> afterDisconectSend = std::vector<message<T>>();

			for (auto& client : group)
			{
				if (client && client->IsConnected())
					client->Send(msg);
				else
				{
					afterDisconectSend.push_back(OnClientDisconnect(client));
					client.reset();
					bInvalidClientExists = true;
				}
			}
			if (bInvalidClientExists) {
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
				for (message<T> msg : afterDisconectSend)
					MessageAllClients(msg);
			}
		}
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
		{
			bool bInvalidClientExists = false;
			std::vector<message<T>> afterDisconectSend = std::vector<message<T>>();

			for (auto& client : m_deqConnections)
			{
				if (client != pIgnoreClient)
				{
					if (client && client->IsConnected())
						client->Send(msg);
					else
					{
						afterDisconectSend.push_back(OnClientDisconnect(client));
						client.reset();
						bInvalidClientExists = true;
					}
				}
			}
			if (bInvalidClientExists) {
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
				for (message<T> msg : afterDisconectSend)
					MessageAllClients(msg);
			}
		}
		
		void Update(size_t nMaxMessages = -1, bool bWait = false)
		{
			if (bWait) m_qMessagesIn.wait();

			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
			{
				auto msg = m_qMessagesIn.pop_front();

				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

	protected:
		virtual void OnClientConnect(std::shared_ptr<connection<T>> client){}
		virtual bool OnClientRequestConnect(std::shared_ptr<connection<T>> client){	return false;}
		virtual message<T> OnClientDisconnect(std::shared_ptr<connection<T>> client) { return message<T>(); }
		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg){}

	protected:
		tsqueue<owned_message<T>> m_qMessagesIn;
		std::deque<std::shared_ptr<connection<T>>> m_deqConnections;
		asio::io_context m_asioContext;
		std::thread m_threadContext;
		asio::ip::tcp::acceptor m_asioAcceptor;

		uint32_t nIDCounter = 1000;
	};
}
#endif