#pragma once

#include "../Script.h"
#include "engine/systems/Animation.h"
#include <EngineCore.h>

namespace Toad
{

// only one instance of this script should exist in a game 
class ENGINE_API NetworkServer : public Script
{
public:
	SCRIPT_CONSTRUCT(NetworkServer)

public:
	static inline int port = 0;

	void AcceptClients();
	void Listen();
public:
	void OnStart(Object* obj) override;
	void OnStop(Object* obj) override;
	void OnEditorUI(Object* obj, ImGuiContext* ctx) override;
	void OnImGui(Object* obj, ImGuiContext* ctx) override;
	void ExposeVars() override;

private:
	static inline sf::TcpSocket m_clients{};
	static inline sf::TcpListener m_listener{};
};

}
