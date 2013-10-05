#ifndef _OGLRENDERER_
#define _OGLRENDERER_

#include "IRenderer.h"
#include "PluginManager.h"
#include "FontEngine.h"
#include "LineEngine.h"
#include "SpriteEngine.h"
#include "ResourceManager.h"

#include "Camera.h"
#include <angelscript.h>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class oglRenderer : public IRenderer
{
public:

	static void MonitorCallback(GLFWmonitor*,int);

	oglRenderer();
	~oglRenderer();

	// IPlugin
	DLLType GetPluginType() const override;
	const char* GetName() const override;
	int GetVersion() const override;

	void Init(class asIScriptEngine*) override;
	void Destroy(class asIScriptEngine*) override;

	// IRenderer

	// Lines
	void DrawLine(const glm::vec3* pArray, // array of 3d vertices to draw
						  unsigned int length, // number of vertices
						  float fWidth = 3.0f, // the width of the line
						  const glm::vec4& color = glm::vec4(1.0f), // color of the line
						  const glm::mat4& t = glm::mat4(1.0f)) override; // transformation to apply to the line

	// Fonts
	void DrawString(const char* str, // the string that gets drawn
							const glm::vec3& pos, // pos of the text in world space
							const glm::vec2& scale = glm::vec2(10.0f), // scaling the text
							const glm::vec3& color = glm::vec3(1.0f), // color of the text blended together with the texture
							const char* font = nullptr, // the desired font, may be null if you wish to use the default font
							FontAlignment options = FontAlignment::Left) override;

	void DrawSprite(const std::string& texture, // texture used to draw the sprite
							const glm::mat4& transformation, // transformation applied to the sprite
							const glm::vec3& color = glm::vec3(1.0f),
							const glm::vec2& tiling = glm::vec2(1.0f), // the amount of tiling, 1.0 means the texture will be stretched across the whole polygon
							unsigned int iCellId = 0, // cellId if multiple frames are stored together in the same sprite image
							const std::string& tech = "sprite"
							) override;

	IResourceManager& GetResourceManager() override; // Returns the resource manager

	void GetCurrentDisplayMode(int& monitor, int& mode) const override; // returns the current display mode given the monitor
	bool GetDisplayMode(int monitor, int mode, int& width, int& height) const override; // get the display mode, return true if success, false if error
	int  GetNumMonitors() const override;
	int  GetNumDisplayModes(int monitor) const override; // returns the number of video modes for the given monitor
	void GetLineWidthRange(float& min, float& max) const override; // Gets the range of the width of the lines supported on the current hardware, x = min, y = max
	void GetStringRec(const char* str, const glm::vec2& scale, Math::FRECT& out) const override;
	void SetCamera(class Camera*) override; // Sets the camera to use
	void SetClearColor(const glm::vec3& color); // Color of the screen after it gets cleared
	void SetDisplayMode(int mode) override; // sets the display mode
	void SetRenderSpace(RenderSpace) override;
	bool SetShaderValue(const std::string& shader, const std::string& location, float value ) override;
	bool SetShaderValue(const std::string& shader, const std::string& location, const glm::vec2& value ) override;

	void EnableVSync(bool);

	void Present(); // draw everything to screen

private:

	static oglRenderer* s_pThis;

	Camera* m_pCamera;
	Camera* m_pOrthoCamera;

	GLFWwindow* m_pWindow;

	ResourceManager m_rm;
	std::auto_ptr<FontEngine> m_pFonts;
	std::auto_ptr<LineEngine> m_pLines;
	std::auto_ptr<SpriteEngine> m_pSprites;

	GLFWmonitor** m_pMonitors;
	int m_iMonitorCount;

	std::vector<std::pair<const GLFWvidmode*,int>> m_videoModes;

	int m_iCurrentMonitor;
	int m_iCurrentDisplayMode;

	RenderSpace m_renderSpace;

	bool m_bFullscreen;

	// Helper functions
	void ConfigureGLFW();
	void ConfigureOpenGL();
	void EnumerateDisplayAdaptors();
	void GLFWOpenWindowHints();
	bool CheckShader(const std::string& shader, const std::string& location, GLuint& shaderID, GLuint& outLocation) const;
	void ParseVideoSettingsFile();
	void SaveDisplayList();
};

#endif // _OGLRENDERER_
