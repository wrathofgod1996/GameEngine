// Programmed by Bryce Mehring

/*
 This class will manage the Execution of all scripts
 There will only be a single instance of the asVM class. And it
 will be publicly accessible.

*/
#ifndef _ASMANAGER_
#define _ASMANAGER_
#pragma once

#include <angelscript.h>
#include <vector>
#include "scriptbuilder.h"
#include "singleton.h"

// Internal structure for holding contents
struct Script;

// There is only one instance of this class because there needs to be only
// one asIScriptEngine* instance.
class asVM : public Singleton<asVM>
{
public:

	void ExecuteScript(unsigned int id);

	// Registers objects with the script. The application then must
	// Provide these functions for the script to work. I need to look into
	// this more...
	void RegisterScript(const char* file);

	// Returned int is the id to the script
	// Build script and then add to vector
	unsigned int BuildScriptFromFile(const char*);
	unsigned int BuildScriptFromMemory(const char*);

	// Release script, then remove from vector
	void RemoveScript(unsigned int id);

	// access to the asIScriptEngine
	asIScriptEngine* GetScriptEngine() const;

private:

	asIScriptEngine* m_pEngine;
	std::vector<Script> m_scripts;
	CScriptBuilder m_builder;

	// constructor/destructor

	// in the constructor, the engine is created and is registered with the std::string type
	// the message callback function is also registered along with a global print function
	// for ints
	asVM();

	// the destructor releases all script contexts and then releases the engine
	~asVM();

	// helper functions
	unsigned int AddScript();
	asETokenClass GetToken(std::string& token, const std::string& text, unsigned int& pos);
	
	friend class Singleton<asVM>;

};

#endif // _ASMANAGER_