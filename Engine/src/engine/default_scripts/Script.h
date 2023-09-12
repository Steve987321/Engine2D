#pragma once

#include <EngineCore.h>

#define SCRIPTBASE(T) NULL

/// Expose a variable in the editor,
/// must be used inside a function
#define EXPOSE_VAR(VAR) m_exposed_vars.insert({#VAR, VAR});

namespace Toad
{
	class Object;

class ENGINE_API Script
{
public:
	Script(std::string_view name);

	const std::string& GetName() const;

	virtual void OnUpdate(Object* obj);
	virtual void OnCreate(Object* obj);
	virtual void OnDestroy(Object* obj);

private:
	std::string m_name;

	template <typename T>
	std::unordered_map<std::string, T> m_exposed_vars;
};

}
