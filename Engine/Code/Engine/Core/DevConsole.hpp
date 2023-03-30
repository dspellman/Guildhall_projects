#pragma once
#include <vector>
#include <string>
#include "Engine\Core\Rgba8.hpp"
#include "Engine\Math\AABB2.hpp"
#include "EventSystem.hpp"
#include "Clock.hpp"
#include "Stopwatch.hpp"
#include <mutex>

class Renderer;
class BitmapFont;
class RemoteDevConsole;

struct DevConsoleConfig
{
	Renderer* theRenderer = nullptr;
	int linesToShow = 1;
	std::string consoleFontName;
};

enum class DevConsoleMode
{
	HIDDEN,
	FULL
};

struct DevConsoleLine
{
	std::string text;
	Rgba8 tint = Rgba8::WHITE;
	int frame = 0;
	double timeStamp = 0.0;

	DevConsoleLine(Rgba8 const& color, std::string const& txt, int frameNumber, double time);
	std::string ToString() const;
};

class DevConsole
{
public:
	static Rgba8 const TINT_ERROR;
	static Rgba8 const TINT_WARNING;
	static Rgba8 const TINT_INFO_MAJOR;
	static Rgba8 const TINT_INFO_MINOR;

	DevConsole(DevConsoleConfig const& config);
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Update(float deltaSeconds);
	void Render(AABB2 const& bounds, Renderer* rendererOverride = nullptr) const;
	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode); 
	void SetRenderer( Renderer* theRenderer);
	void Togglemode(DevConsoleMode mode);
	bool ProcessCharacter(int charCode);
	bool ProcessCommand(int charCode);
	bool RunScript(const char* scriptFile);

	// commands
	void Clear();

protected:
	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect = 1.0f) const;

protected:
	DevConsoleConfig m_config;
	Renderer* m_theRenderer = nullptr;
	mutable BitmapFont* m_consoleFont = nullptr;
	DevConsoleMode	m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lines;	// ToDo: support a max limited number of lines (e.g. fixed circular buffer)
	int m_linesToShow = 1;
	std::atomic<int>	m_frameNumber = 0;
	Stopwatch*	m_caretStopwatch;
	std::string	m_inputText; // = std::string("Test input line");
	int		m_caretPosition = 0;
	bool	m_caretVisible = true;
	int		m_maxCommandHistory = 128;
	std::vector< std::string >	m_commandHistory;
	int		m_historyIndex = -1;
	mutable std::mutex m_accessMutex;
	RemoteDevConsole* m_remoteConsole = nullptr;

public:
	Clock*	m_clock;
	bool IsOpen();
};

static bool Event_KeyPressed(EventArgs& args);
static bool Event_KeyReleased(EventArgs& args);
static bool Event_CharInput(EventArgs& args);
static bool Command_Clear(EventArgs& args);
static bool Command_Help(EventArgs& args);
static bool Command_Controls(EventArgs& args);
static bool Command_DebugRenderClear(EventArgs& args);
static bool Command_DebugRenderToggle(EventArgs& args);
static bool Command_DebugRaycastToggle(EventArgs& args);

static bool Event_RunScript(EventArgs& args);
static bool Event_Host(EventArgs& args);
static bool Event_Join(EventArgs& args);
static bool Event_Echo(EventArgs& args);
static bool Event_Command(EventArgs& args);
static bool Event_Broadcast(EventArgs& args);
static bool Event_Kick(EventArgs& args);
static bool Event_Blacklist(EventArgs& args);