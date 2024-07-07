#include "pch.h"
#include "engine/Engine.h"
#include "NetworkServer.h"

namespace Toad
{
	void NetworkServer::OnStart(Object* obj)
	{
		LOGDEBUGF("[NetworkServer] Listening on port {}", port);
		sf::Socket::Status res = m_listener.listen(port);

		if (res != sf::Socket::Done)
		{
			LOGERRORF("[NetworkServer] Error listening {}", (int)res);
		}
	}

	void NetworkServer::OnStop(Object* obj)
	{
		LOGDEBUGF("[NetworkServer] Closing listening socket");
		m_listener.close();
	}

	void NetworkServer::OnEditorUI(Object* obj, ImGuiContext* ctx)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void NetworkServer::OnImGui(Object* obj, ImGuiContext* ctx)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void NetworkServer::ExposeVars()
	{
		EXPOSE_VAR(port);
	}

}
