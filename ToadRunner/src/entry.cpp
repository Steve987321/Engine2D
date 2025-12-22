#include "pch.h"
#include <EngineCore.h>
#include <engine/Engine.h>

#ifdef TOAD_EDITOR
#include "ui/UI.h"
#endif 

#ifdef _WIN32
#include <dbghelp.h>
#include <ShlObj_core.h>
#pragma comment(lib, "dbghelp.lib")

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
void CreateMiniDump(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
	HMODULE mhLib = ::LoadLibrary(L"dbghelp.dll");
	MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(mhLib, "MiniDumpWriteDump");

	HANDLE hFile = ::CreateFileA("core.dmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = apExceptionInfo;
	ExInfo.ClientPointers = FALSE;

	pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
	::CloseHandle(hFile);
}

LONG WINAPI UnhandledExceptionHandler(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
	CreateMiniDump(apExceptionInfo);
	return EXCEPTION_CONTINUE_SEARCH;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif 
{	
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExceptionHandler);
#endif

#ifdef TOAD_EDITOR
	Toad::SetPreUICallback(ui::update_ini);
	Toad::SetEngineUI(ui::engine_ui);
	Toad::SetEventCallback(ui::event_callback);
	Toad::SetEditorTextureDrawCallback(ui::editor_texture_draw_callback);
	Toad::SetOnCloseCallback(ui::save_ini_files);
#endif 

	if (!Toad::Init())
		return 1;
	
	ImGui::SetCurrentContext(Toad::GetWindow().GetImGuiContext());
	
	Toad::Run();

	return 0;
}
