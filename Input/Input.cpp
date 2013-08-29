
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <angelscript.h>
#include "DXInput.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Input plug-in implementation
extern "C" PLUGINDECL IPlugin* CreatePlugin()
{
	return new DirectInput();
}

DirectInput* DirectInput::s_pThis = nullptr;

void DirectInput::CharCallback(GLFWwindow*, unsigned int c)
{
	if(s_pThis != nullptr)
	{
		s_pThis->m_iCharKeyDown = c;
	}
}

void DirectInput::KeyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	if(s_pThis != nullptr)
	{
		s_pThis->m_iKeyDown = key;
		s_pThis->m_iKeyAction = action;
	}
}

void DirectInput::MouseCallback(GLFWwindow*, double x, double y)
{
	if(s_pThis != nullptr)
	{
		s_pThis->UpdateMouse(x,y);
	}
}

void DirectInput::MouseButtonCallback(GLFWwindow*,int button, int action, int mods)
{
	if(s_pThis != nullptr)
	{
		s_pThis->m_selectedPos = s_pThis->m_tpos;
		s_pThis->m_MouseClickOnce[button] = action;
	}
}

void DirectInput::MouseScrollCallback(GLFWwindow*, double, double yOffset)
{
	if(s_pThis != nullptr)
	{
		s_pThis->m_fYScrollOffset = yOffset;
	}
}

// DirectInput ctor
DirectInput::DirectInput() : m_fMouseSensistivity(100.0f), m_tpos(0.0f,0.0f)
{
	s_pThis = this;

	Reset();

	glfwSetCharCallback(glfwGetCurrentContext(),CharCallback);
	glfwSetKeyCallback(glfwGetCurrentContext(),KeyCallback);
	glfwSetCursorPosCallback(glfwGetCurrentContext(),MouseCallback);
	glfwSetMouseButtonCallback(glfwGetCurrentContext(),MouseButtonCallback);
	glfwSetScrollCallback(glfwGetCurrentContext(),MouseScrollCallback);

	glfwSetInputMode(glfwGetCurrentContext(),GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
	glfwSetInputMode(glfwGetCurrentContext(),GLFW_STICKY_KEYS,GL_TRUE);

	CenterMouse();
}

void DirectInput::Init(asIScriptEngine* pAS)
{
	RegisterScript(pAS);
}

void DirectInput::Destroy(asIScriptEngine* pAS)
{
	// remove config group from script
	pAS->RemoveConfigGroup("Input");
}

int DirectInput::GetVersion() const
{
	return 0;
}

DLLType DirectInput::GetPluginType() const
{
	return DLLType::Input;
}

bool DirectInput::LoadKeyBindFile(const string& file)
{
	ifstream inFile(file);
	if(!inFile.is_open())
		return false;

	string line;
	while(getline(inFile,line))
	{
		istringstream stream(line);

		string command;
		stream >> command;

		if(command.size() > 0 && command[0] != ';')
		{
			if(command == "bind")
			{
				string from;
				string to;

				stream >> from;
				stream >> to;

				if(from.size() > 0 && to.size() > 0)
				{
					m_bindings[to[0]].push_back(from[0]);
				}
			}
		}
	}

	inFile.close();

	return true;
}

void DirectInput::CenterMouse()
{
	int width;
	int height;

	glfwGetWindowSize(glfwGetCurrentContext(),&width,&height);
	glfwSetCursorPos(glfwGetCurrentContext(),width / 2, height / 2);
}

void DirectInput::Reset()
{
	m_MouseClickOnce[0] = m_MouseClickOnce[1] = -1;
	m_iKeyAction = m_iKeyDown = -1;
	m_iCharKeyDown = -1;
	m_iMouseX = m_iMouseY = 0;
	m_fYScrollOffset = 0.0f;
}

void DirectInput::Poll()
{
	Reset();

	/*if(glfwJoystickPresent(GLFW_JOYSTICK_1) == GL_TRUE)
	{
		int count;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1,&count);
		if(axes[0] > 0.2f && fabsf(axes[1]) < 0.5f)
		{
			m_bJoyRight = true;
		}
		else
		{
			m_bJoyRight = false;
		}

	}*/
	glfwPollEvents();
}

void DirectInput::UpdateMouse(double x, double y)
{
	int width = 0;
	int height = 0;

	glfwGetWindowSize(glfwGetCurrentContext(),&width,&height);

	m_iMouseX = x - (width / 2);
	m_iMouseY = -y + (height / 2);

	m_tpos += m_fMouseSensistivity * glm::vec2(m_iMouseX / (float)width ,m_iMouseY / (float)height);

	CenterMouse();
	ClampMouse();
}

void DirectInput::ClampMouse()
{
	if(m_tpos.x < -100.0f)
	{
		m_tpos.x = -100.0f;
	}
	else if(m_tpos.x > 100.0f)
	{
		m_tpos.x = 100.0f;
	}

	if(m_tpos.y < -100.0f)
	{
		m_tpos.y = -100.0f;
	}
	else if(m_tpos.y > 100.0f)
	{
		m_tpos.y = 100.0f;
	}
}

void DirectInput::CursorPos(double& x, double& y) const
{
	glfwGetCursorPos(glfwGetCurrentContext(),&x,&y);
}

const glm::vec2& DirectInput::GetTransformedMousePos() const
{
	return m_tpos;
}

bool DirectInput::CheckKey(int Key, bool once, int flag)
{
	if(once && (m_iKeyAction != flag))
		return false;

	bool bSuccess = false;
	auto iter = m_bindings.find(Key);
	if(once)
	{
		if(iter == m_bindings.end())
		{
			bSuccess = (Key == m_iKeyDown);
		}
		else
		{
			unsigned int i = 0;
			while(i < iter->second.size() && (iter->second[i] != m_iKeyDown)) { ++i; }

			bSuccess = (i < iter->second.size());
		}
	}
	else
	{
		if(iter == m_bindings.end())
		{
			bSuccess = (glfwGetKey(glfwGetCurrentContext(), Key  ) == flag);
		}
		else
		{
			unsigned int i = 0;
			while(i < iter->second.size() && (glfwGetKey(glfwGetCurrentContext(), iter->second[i]  ) != flag)) { ++i; }

			bSuccess = (i < iter->second.size());
		}
	}

	return bSuccess;
}

bool DirectInput::KeyDown(int Key, bool once)
{
	return CheckKey(Key,once,GLFW_PRESS);
}
bool DirectInput::KeyUp(int Key, bool once)
{
	return CheckKey(Key,once,GLFW_RELEASE);
}
bool DirectInput::CharKeyDown(char& out) const
{
	if(m_iCharKeyDown == (unsigned int)-1)
		return false;

	out = m_iCharKeyDown;

	return true;
}

bool DirectInput::MouseClick(int iButton, bool once) const
{
	if(iButton > 1)
		return false;

	return (once ? (m_MouseClickOnce[iButton] == GLFW_PRESS) : glfwGetMouseButton(glfwGetCurrentContext(),iButton) == GLFW_PRESS);
}
bool DirectInput::MouseRelease(int iButton, bool once) const
{
	if(iButton > 1)
		return false;

	return (once ? (m_MouseClickOnce[iButton] == GLFW_RELEASE) : glfwGetMouseButton(glfwGetCurrentContext(),iButton) == GLFW_RELEASE);
}
int DirectInput::MouseX() const
{
	return m_iMouseX;
}

int DirectInput::MouseY() const
{
	return m_iMouseY;
}

double DirectInput::MouseZ() const
{
	return m_fYScrollOffset;
}

bool DirectInput::GetSelectedRect(Math::AABB& out)
{
	if(!MouseClick(0,false))
		return false;

	out.min = glm::vec2(min(m_selectedPos.x,m_tpos.x),min(m_selectedPos.y,m_tpos.y));
	out.max = glm::vec2(max(m_selectedPos.x,m_tpos.x),max(m_selectedPos.y,m_tpos.y));

	return true;
}

void DirectInput::SetCursorSensitivity(float s)
{
	m_fMouseSensistivity = s;
}

void DirectInput::RegisterScript(asIScriptEngine* pAS)
{
	pAS->BeginConfigGroup("Input");

	(pAS->RegisterObjectType("IKMInput",0,asOBJ_REF | asOBJ_NOHANDLE));
	(pAS->RegisterObjectMethod("IKMInput","int mouseX()",asMETHOD(DirectInput,MouseX),asCALL_THISCALL));
	(pAS->RegisterObjectMethod("IKMInput","int mouseY()",asMETHOD(DirectInput,MouseY),asCALL_THISCALL));
	(pAS->RegisterObjectMethod("IKMInput","int mouseZ()",asMETHOD(DirectInput,MouseZ),asCALL_THISCALL));
	(pAS->RegisterGlobalProperty("IKMInput input",this));

	pAS->EndConfigGroup();
}