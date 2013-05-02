// Programmed By Bryce Mehring
// 1/20/2011

//#ifdef _WIN32
//#include <Windows.h>
//#endif

// Read chapter 16, use the dynamic object mapper with the DLL files.
#include "PluginManager.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <angelscript.h>
#include <iostream>

using namespace std;

PluginManager::PluginManager() : m_pAS(nullptr)
{
}

// ===== Destructor =====
PluginManager::~PluginManager()
{
	FreeAllPlugins();
    m_pAS->Release();
}

void PluginManager::FreeAllPlugins()
{
	for(auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		FreePlugin(iter->second);
	}

	m_plugins.clear();
}

//  ===== interface with dlls =====

const IPlugin* PluginManager::GetPlugin(DLLType type) const
{
	plugin_type::const_iterator iter = m_plugins.find(type);

	if(iter != m_plugins.end())
	{
		return iter->second.pPlugin;
	}

	return nullptr;
}

IPlugin* PluginManager::GetPlugin(DLLType type)
{
	plugin_type::const_iterator iter = m_plugins.find(type);

	if(iter != m_plugins.end())
	{
		return iter->second.pPlugin;
	}

	return nullptr;
}

/*bool PluginManager::Good(const char* pDLL) const
{
	PluginInfo dll = {0,0};

	dll.mod = LoadLibrary(pDLL);

	// todo: fix the error handling here
	if(dll.mod == nullptr)
	{
		// error, could not load dll

		// need to output message to log
		// todo: should create a logging singleton that should interact with the console.
		ostringstream stream;
		stream <<"Could not load: " << pDLL << endl;

		//fm.WriteToLog(stream.str());
		
		return false;
	}
	
	CREATEPLUGIN pFunct = nullptr;

#ifdef _WIN32
	pFunct = (CREATEPLUGIN)GetProcAddress((HMODULE)(dll.mod),"CreatePlugin");
#else
	pFunct = (CREATEPLUGIN)dlsym(dll.mod,"CreatePlugin");
#endif

	if(pFunct == nullptr)
	{
		ostringstream stream;
		stream <<"CreatePlugin() function not found in: " << pDLL << endl;
		fm.WriteToLog(stream.str());

		return false;
	}

#ifdef _WIN32
		FreeLibrary((HMODULE)dll.mod);
#else
		dlclose(dll.mod);
#endif

	return true;
}*/

IPlugin* PluginManager::LoadDLL(const char* pDLL)
{
    PluginInfo dll;
    dll.mod = 0;
    dll.pPlugin = 0;

#ifdef _WIN32
	dll.mod = LoadLibrary(pDLL);
#else
    dll.mod = dlopen(pDLL,RTLD_NOW);
#endif

	// todo: fix the error handling here
	if(dll.mod == nullptr)
	{
        cout << "Cannot open library: " << dlerror() << endl;
		return nullptr;
	}

	CREATEPLUGIN pFunct = nullptr;

#ifdef _WIN32
	pFunct = (CREATEPLUGIN)GetProcAddress((HMODULE)(dll.mod),"CreatePlugin");
#else
	pFunct = (CREATEPLUGIN)dlsym(dll.mod,"CreatePlugin");
#endif

	if(pFunct == nullptr)
	{
#ifdef _WIN32
		// error, corrupted dll
		FreeLibrary((HMODULE)dll.mod);
#else
		dlclose(dll.mod);
#endif

		return nullptr;
    }
		
	// Create the plugin
	m_pAS->AddRef();
    dll.pPlugin = pFunct(m_pAS);
    assert(dll.pPlugin != nullptr);

	// Get the type of the plugin
	DLLType type = dll.pPlugin->GetPluginType();
	auto iter = m_plugins.find(type); // see if the plugin is already loaded

	// If it is already loaded
	if(iter != m_plugins.end())
	{
		// Free the old plugin
		FreePlugin(iter->second);

		// replace with new
		iter->second = dll;
	}
	else
	{

		// insert plugin into hash table
		m_plugins.insert(make_pair(type,dll));
	}

	// return the plugin interface
	return dll.pPlugin;
}

void PluginManager::FreePlugin(const PluginInfo& plugin)
{
    delete plugin.pPlugin;

#ifdef _WIN32
	FreeLibrary((HMODULE)plugin.mod);
#else
	dlclose(plugin.mod);
#endif
}

void PluginManager::FreePlugin(DLLType type)
{
	auto iter = m_plugins.find(type);
	if(iter != m_plugins.end())
	{
		FreePlugin(iter->second);

		m_plugins.erase(iter);
	}
}

