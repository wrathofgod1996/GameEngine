#ifndef _OGLRENDERER_
#define _OGLRENDERER_

#include "IRenderer.h"
#include "PluginManager.h"
#include "AbstractRenderer.h"
#include "LineRenderer.h"
#include "ResourceManager.h"
#include "VertexBuffer.h"

#include "Camera.h"
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class oglRenderer final : public IRenderer
{
public:

	oglRenderer();
	~oglRenderer();

	// IPlugin
	DLLType GetPluginType() const override;
	const char* GetName() const override;
	int GetVersion() const override;

	// IRenderer

	// DrawLine() caches a line to be drawn by Present()
	// Note: if pArray is NULL, then DrawLine() terminates
	void DrawLine(const glm::vec3* pArray, // array of 3d vertices to draw
						  unsigned int length, // number of vertices
						  float fWidth = 3.0f, // the width of the line
						  const glm::vec4& color = glm::vec4(1.0f), // color of the line
						  const glm::mat4& t = glm::mat4(1.0f)) override; // transformation to apply to the line

	void DrawCircle(const glm::vec3& center,
							float radius,
							float thickness,
							unsigned int segments,
							const glm::vec4& color) override;

	// DrawString() caches a string to be drawn by Present()
	// Note: if either str or font is NULL, then DrawString() terminates
	void DrawString(const char* str, // the string that gets drawn
							const glm::vec3& pos, // position of the text in the current render space
							const glm::vec4& color = glm::vec4(1.0f), // color of the text blended together with the texture
							float scale = 50.0f, // height of the text, the width of the text gets scaled accordingly
							const char* font = nullptr, // the desired font, may be null if you wish to use the default font
							FontAlignment alignment = FontAlignment::Left
							) override;

	// DrawSprite() caches a single sprite to be drawn by Present()
	void DrawSprite(const std::string& texture, // texture used to draw the sprite
							const glm::mat4& transformation, // transformation applied to the sprite
							const glm::vec4& color = glm::vec4(1.0f), // color that gets blended together with the sprite
							const glm::vec2& tiling = glm::vec2(1.0f), // the amount of tiling, 1.0 means the texture will be stretched across the whole polygon
							unsigned int iCellId = 0, // cellId if multiple frames are stored together in the same sprite image
							const std::string& tech = "sprite"
							) override;

	// Draws a single sprite with a white texture
	void DrawSprite(const glm::mat4& transformation, // transformation applied to the sprite
							const glm::vec4& color = glm::vec4(1.0f), // color that gets blended together with the sprite
							const glm::vec2& tiling = glm::vec2(1.0f), // the amount of tiling, 1.0 means the texture will be stretched across the whole polygon
							unsigned int iCellId = 0, // cellId if multiple frames are stored together in the same sprite image
							const std::string& tech = "sprite"
							) override;

	// Manage cursor creation
	// Todo: move this code into the input plugin
	int CreateCursor(const std::string& texture, int xhot, int yhot) override;
	void DestroyCursor(int cursor) override;
	void SetCursor(int cursor) override;

	// Returns the resource manager
	IResourceManager& GetResourceManager() override;

	// todo: add the ability to specify which component is being read
	// Returns depth value in the buffer at a single point in screen space
	float ReadPixels(const glm::ivec2& pos) const override;

	// Returns the current display mode
	const GLFWvidmode* GetDisplayMode() const;
	const GLFWvidmode* GetDisplayMode(int monitor, int mode) const;

	// Get the display mode on the specified monitor, return true on success, false on error
	// either width or height may be null if their values are not needed
	bool GetDisplayMode(int monitor, int mode, int* width, int* height) const override;

	// Get the current display mode, return true on success, false on error
	// either width, height, or vsync may be null if their values are not needed
	bool GetDisplayMode(int* width, int* height, bool* vsync = nullptr) const override;

	// Returns the number of monitors active on the current system
	int GetNumMonitors() const override;

	// Returns the number of video modes for the given monitor
	int GetNumDisplayModes(int monitor) const override;

	// Returns a tight bounding box that fits around the text,
	// if out is expected to be in user space, set out's topLeft position vector before calling this method, 
	// else, the bottomRight vector of out will be the width and height of the string and the topLeft vector will be zeroed
	// Note: if str is NULL, then GetStringRec() terminates without modifying out
	void GetStringRect(const char* str, float scale, FontAlignment alignment, Math::FRECT& inout) const override;

	// Sets the world space camera to use
	void SetCamera(class PerspectiveCamera*) override;

	// Sets the color of the screen after it gets cleared
	void SetClearColor(const glm::vec3& color) override;
	void EnableColorClearing(bool bEnable) override;

	// Sets the display mode
	void SetDisplayMode(int mode) override;

	// Sets the coordinate system to render all objects in(screen space or world space)
	void SetRenderSpace(RenderSpace) override;

	// todo: add comments here
	void SetShaderValue(const std::string& shader, const std::string& location, float value) override;
	void SetShaderValue(const std::string& shader, const std::string& location, const glm::vec2& value) override;

	// True enables VSync
	// False disables VSync
	void EnableVSync(bool) override;

	// Returns true if the window is iconified
	bool IsIconified() const override;

	// Render everything that has been cached so far to the back buffer and then swap the back buffer with the front buffer
	void Present() override;

	static void MonitorCallback(GLFWmonitor*, int);
	static void IconifyCallback(GLFWwindow*, int);

private:

	Camera m_OrthoCamera;
	PerspectiveCamera* m_pWorldCamera;

	GLFWwindow* m_pWindow;

	ResourceManager m_rm;

	std::unique_ptr<AbstractRenderer> m_pWorldSpaceSprites;
	std::unique_ptr<AbstractRenderer> m_pScreenSpaceSprites;

	GLFWmonitor** m_pMonitors;
	int m_iMonitorCount;

	std::vector<std::pair<const GLFWvidmode*,int>> m_videoModes;

	int m_iCurrentMonitor;
	int m_iCurrentDisplayMode;

	RenderSpace m_renderSpace;

	bool m_bVSync;
	bool m_bFullscreen;
	bool m_bIconify = false;
	bool m_bFirstRun = true;
	GLuint m_iClearBits;

	std::shared_ptr<Mesh> m_mesh;
	std::map<int, GLFWcursor*> m_cursors;

	static oglRenderer* s_pThis;
	static const std::string s_videoModeFile;

	// Helper functions
	void ConfigureGLFW();
	void ConfigureOpenGL();
	void EnumerateDisplayAdaptors();
	void ParseVideoSettingsFile();
	void SaveDisplayList();
	void BuildRenderers();
	void BuildCamera();
	void UpdateCamera();

	oglRenderer(const oglRenderer&) = delete;
	oglRenderer& operator = (const oglRenderer&) = delete;
};

#endif // _OGLRENDERER_
