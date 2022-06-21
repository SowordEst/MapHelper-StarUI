#include <fstream>
#include "YDPluginManager.h"
#include "MapHelper.h"
#include "../resource.h"
#include <base\hook\iat.h>
#include "shellapi.h"
#include <regex>


static clock_t g_last_start_time;
static std::string g_last_exe_name;
static DWORD g_thread_id = 0;


// Ŀ�� ��ydwe���ò��ʱ ������ص��� maphelper�����Ĳ����

BOOL WINAPI fakeCreateProcessW(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation ) {


	auto& manager = get_ydplugin_manager();


	std::wstring cmd = lpCommandLine;

	int argn = 0;
	auto argv = CommandLineToArgvW(lpCommandLine, &argn);
	if (argn > 0) {
		fs::path exe_path = argv[0];
		std::wstring exe_name = exe_path.filename().wstring();
		std::transform(exe_name.begin(), exe_name.end(), exe_name.begin(), ::tolower);

		g_last_exe_name = base::w2a(exe_name);
		g_last_start_time = clock();

		print("��� [%s] ��ʼ����\n", g_last_exe_name.c_str());
	
		if (manager.m_enable) {
			auto it = manager.m_plugins_path.find(exe_name);
			if (it != manager.m_plugins_path.end()) {
				cmd = L"\"" + it->second + L"\"";
				for (int i = 1; i < argn; i++) {
					std::wstring value = std::wstring(argv[i]);
					value = std::regex_replace(value, std::wregex(L"\""), L"\\\"");
					cmd += L" \"" + value + L"\"";
				}
			}
		}

	}
		//print("execute %s\n", base::w2a(cmd).c_str());
		
	

	BOOL ret = CreateProcessW(
		lpApplicationName,
		(LPWSTR)cmd.c_str(),
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);

	return ret;
}




DWORD WINAPI fakeWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
	DWORD ret = WaitForSingleObject(hHandle, dwMilliseconds);

	if (GetCurrentThreadId() == g_thread_id && !g_last_exe_name.empty()) {
		print("��� [%s] ���н��� ��ʱ: %f ��\n", g_last_exe_name.c_str(), (double)(clock() - g_last_start_time) / CLOCKS_PER_SEC);
		g_last_exe_name.clear();

	}

	return ret;
}



YDPluginManager::YDPluginManager()
	:m_attach(false),
	m_extract(false),
	m_enable(false)
{
	
}

YDPluginManager::~YDPluginManager() {
	
}


void YDPluginManager::attach() {
	if (m_attach) {
		return;
	}

	std::vector<std::string> modules = {
		"sys.dll",
		"process.dll",
		"ydbase.dll"
	};

	g_thread_id = ::GetCurrentThreadId();

	for (auto& name : modules) {
		auto handle = GetModuleHandleA(name.c_str());
		if (handle) {
			HookInfo info;
			info.module = handle;
			info.dll_name = "Kernel32.dll";
			info.api_name = "CreateProcessW";
			info.real = base::hook::iat(info.module, info.dll_name.c_str(), info.api_name.c_str(), (uintptr_t)&fakeCreateProcessW);
			m_hook_list.push_back(info);

			HookInfo info2;
			info2.module = handle;
			info2.dll_name = "Kernel32.dll";
			info2.api_name = "WaitForSingleObject";
			info2.real = base::hook::iat(info2.module, info2.dll_name.c_str(), info2.api_name.c_str(), (uintptr_t)&fakeWaitForSingleObject);
			m_hook_list.push_back(info2);


		}
	}
	if (m_hook_list.empty()) {
		return;
	}

	m_attach = true;

}

void YDPluginManager::detach() {
	if (!m_attach) {
		return;
	}
	m_attach = false;

	for (auto& info : m_hook_list) {
		base::hook::iat(info.module, info.dll_name.c_str(), info.api_name.c_str(), info.real);
	}
}
 


void YDPluginManager::extract() {
	if (m_extract) {
		return;
	}
	m_extract = true;

	std::map<std::string, int> files = {
		{"wave.exe", IDR_EXE1},
	};
	
	fs::path path = fs::temp_directory_path() / "plugin";

	fs::create_directories(path);

	for (auto&& [name, id] : files) {
		fs::path file_path = path / name;

		fs::remove(file_path);

		auto resource = FindResourceA(g_hModule, MAKEINTRESOURCE(id), TEXT("exe"));
		if (!resource) {
			continue;
		}
		void* data = LoadResource(g_hModule, resource);
		if (!data) {
			continue;
		}
		void* file_data = LockResource(data);
		if (!file_data) {
			continue;
		}
		DWORD file_size = SizeofResource(g_hModule, resource);
		if (file_size == 0) {
			continue;
		}
		std::ofstream file(file_path, std::ios::binary);
		if (file.is_open()) {
			file.write((const char*)file_data, file_size);
			file.close();
			m_plugins_path[base::a2w(name)] = base::a2w(file_path.string());
		}
	}
}


YDPluginManager& get_ydplugin_manager() {
	static YDPluginManager instance;
	return instance;
}