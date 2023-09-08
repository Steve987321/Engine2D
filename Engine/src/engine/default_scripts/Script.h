#pragma once

#define SCRIPTBASE(T) NULL

namespace Toad
{

class Script
{
public:
	Script(std::string_view name);

	const std::string& GetName() const;

	virtual void OnUpdate();
	virtual void OnCreate();
	virtual void OnDestroy();

private:
	std::string m_name;
};

}
