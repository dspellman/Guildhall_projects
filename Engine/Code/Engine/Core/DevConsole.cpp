#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include <stdlib.h>
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Network/RemoteDevConsole.hpp"
#include "Game/EngineBuildPreferences.hpp"

DevConsole* g_theConsole = nullptr;
Rgba8 const DevConsole::TINT_ERROR = Rgba8::RED;
Rgba8 const DevConsole::TINT_WARNING = Rgba8::YELLOW;
Rgba8 const DevConsole::TINT_INFO_MAJOR = Rgba8::BLUE;
Rgba8 const DevConsole::TINT_INFO_MINOR = Rgba8::WHITE;

constexpr double CARET_BLINK_TIME = 0.5;

// note that the DevConsole can be created with the renderer equal to nullptr but it
// needs to be updated later with a call the SetRenderer or nothing  will ever be printed
DevConsole::DevConsole(DevConsoleConfig const& config)
	: m_config(config)
{
	m_linesToShow = m_config.linesToShow;
	m_theRenderer = m_config.theRenderer;
	m_caretPosition = 0;
	m_clock = new Clock(g_systemClock);
// 	m_clock->SetParent(g_systemClock);
// 	g_systemClock.AddChild(m_clock);
}

DevConsole::~DevConsole()
{
	if (m_clock)
	{
		delete m_clock;
		m_clock = nullptr;
	}
#if !defined( ENGINE_DISABLE_NETWORK )
	if (m_remoteConsole)
	{
		delete m_remoteConsole;
		m_remoteConsole = nullptr;
	}
#endif
}

void DevConsole::Startup()
{
	m_accessMutex.lock();
	m_caretStopwatch = new Stopwatch(m_clock, CARET_BLINK_TIME);
	// set up events
	g_theEventSystem->SubscribeEventCallbackFunction("WM_CHAR", Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("WM_KEYDOWN", Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("WM_KEYUP", Event_KeyReleased);
	g_theEventSystem->SubscribeEventCallbackFunction("clear", Command_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("help", Command_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("controls", Command_Controls);
	g_theEventSystem->SubscribeEventCallbackFunction("debugrenderclear", Command_DebugRenderClear);
	g_theEventSystem->SubscribeEventCallbackFunction("debugrendertoggle", Command_DebugRenderToggle);
	g_theEventSystem->SubscribeEventCallbackFunction("debugraycasttoggle", Command_DebugRaycastToggle);

	// network-related commands
	g_theEventSystem->SubscribeEventCallbackFunction("run", Event_RunScript);
#if !defined( ENGINE_DISABLE_NETWORK )
	g_theEventSystem->SubscribeEventCallbackFunction("host", Event_Host);
	g_theEventSystem->SubscribeEventCallbackFunction("join", Event_Join);
	g_theEventSystem->SubscribeEventCallbackFunction("disconnect", Event_Join);
	g_theEventSystem->SubscribeEventCallbackFunction("echo", Event_Echo);
	g_theEventSystem->SubscribeEventCallbackFunction("command", Event_Command);
	g_theEventSystem->SubscribeEventCallbackFunction("broadcast", Event_Broadcast);
	g_theEventSystem->SubscribeEventCallbackFunction("kick", Event_Kick);
	g_theEventSystem->SubscribeEventCallbackFunction("blacklist", Event_Blacklist);

	RemoteConsoleConfig config;
	config.console = this;
	m_remoteConsole = new RemoteDevConsole(config);
	m_remoteConsole->Startup();
#endif
	m_accessMutex.unlock();
}

void DevConsole::Shutdown()
{
	m_accessMutex.lock();
	delete m_caretStopwatch;
#if !defined( ENGINE_DISABLE_NETWORK )
	m_remoteConsole->Shutdown();
#endif
	m_accessMutex.unlock();
}

void DevConsole::BeginFrame()
{
	m_frameNumber++; // used to mark log lines
}

void DevConsole::EndFrame()
{
	m_accessMutex.lock();
	if (m_caretStopwatch->CheckDurationElapsedAndDecrement())
	{
		m_caretVisible = !m_caretVisible;
	}
	m_accessMutex.unlock();
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	Strings strings = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (int command = 0; command < strings.size(); command++)
	{
		Strings commandLine = SplitStringOnDelimiter(strings[command], ' ');
		EventArgs args;
		for (int argCount = 1; argCount < commandLine.size(); argCount++)
		{
			Strings keyValue = SplitStringOnDelimiter(commandLine[argCount], '=');
			if (keyValue.size() != 2)
			{
				std::string warning("[DevConsole::Execute] Malformed command arg: ");
				for (int index = 0; index < keyValue.size(); index++)
				{
					warning += keyValue[index];
					warning += " ";
				}
				warning.pop_back();
				g_theConsole->AddLine(DevConsole::TINT_WARNING, warning);
				continue; // malformed command is ignored for safety
			}
			args.SetValue(keyValue[0], keyValue[1]);
		}
		FireEvent(commandLine[0], args);
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	DevConsoleLine line(color, text, m_frameNumber, GetCurrentTimeSeconds());
	m_lines.push_back(line);
}

void DevConsole::Update(float deltaSeconds)
{
#if !defined( ENGINE_DISABLE_NETWORK )
	if (m_remoteConsole)
	{
		m_remoteConsole->Update(deltaSeconds);
	}
#else
	UNUSED(deltaSeconds);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride /*= nullptr*/) const
{
	m_accessMutex.lock();
	if (m_mode == DevConsoleMode::HIDDEN)
	{
		m_accessMutex.unlock();
		return; // this should never happen
	}

	Renderer* localRenderer = rendererOverride ? rendererOverride : m_theRenderer;
	if (!m_consoleFont && localRenderer)
	{
		m_consoleFont = localRenderer->CreateOrGetBitmapFont(m_config.consoleFontName.c_str());
	}
	if (!m_consoleFont)
	{
		m_accessMutex.unlock();
		return; // we have no font to print with yet
	}

	// set caret blink

	// assuming other modes will be in the future
	Render_OpenFull(bounds, *localRenderer, *m_consoleFont, 1.0f);
	m_accessMutex.unlock();
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	float linesOfText = static_cast<float>(m_linesToShow) + 1;
	float cellHeight = (bounds.m_maxs.y - bounds.m_mins.y) / linesOfText;
	std::vector<Vertex_PCU> textVerts;
	Vec2 printPosition(bounds.m_mins);
	AABB2 lineBox;
	lineBox.SetDimensions(Vec2(bounds.GetDimensions().x, cellHeight));
	lineBox.SetCenter(Vec2(bounds.GetCenter().x, cellHeight * 0.5f));

	// Draw dark background on entire screen for FULL
	std::vector<Vertex_PCU> overlayVerts;
	m_theRenderer->BindTexture(nullptr); // disable texturing while drawing map tiles
	m_theRenderer->SetBlendMode(BlendMode::ALPHA);
	AddVertsForAABB2D(overlayVerts, bounds, Rgba8(0, 0, 0, 96));
	m_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
	// make input line visually distinct (darker)
	overlayVerts.clear();
	AddVertsForAABB2D(overlayVerts, lineBox, Rgba8(0, 0, 0, 64));
	m_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
	///

	// first print the command line and set up caret
	float charWidth = font.AddVertsForTextInBox2D(textVerts, lineBox, cellHeight, m_inputText, Rgba8::GREEN, fontAspect, Vec2(0.0f, 1.0f), SHRINK);
	AABB2 caretBox;
	caretBox.SetDimensions(Vec2(2.0f + (m_caretPosition ? 1.0f : 0.0f), charWidth));
	caretBox.SetCenter(Vec2(static_cast<float>(m_caretPosition) * charWidth + (m_caretPosition ? 0.0f : 1.0f), charWidth * 0.5f));
	overlayVerts.clear();
	AddVertsForAABB2D(overlayVerts, caretBox, Rgba8(0, 255, 255, 255));

	//then print saved console lines
	lineBox.SetCenter(Vec2(lineBox.GetCenter().x, lineBox.GetCenter().y + cellHeight));
	int startLine = m_linesToShow > static_cast<int>(m_lines.size()) ? 0 : static_cast<int>(m_lines.size()) - m_linesToShow;
// 	for (int index = startLine; index < static_cast<int>(m_lines.size()); index++)
	for (int index = static_cast<int>(m_lines.size()) - 1; index >= startLine; index--)
		{
 		font.AddVertsForTextInBox2D(textVerts, lineBox, cellHeight, m_lines[index].ToString(), m_lines[index].tint, fontAspect, Vec2(0.0f, 1.0f), SHRINK);
//		font.AddVertsForTextInBox2D(textVerts, lineBox, cellHeight, m_lines[index].text, m_lines[index].tint, fontAspect, Vec2(0.0f, 1.0f), SHRINK);
		lineBox.SetCenter(Vec2(lineBox.GetCenter().x, lineBox.GetCenter().y + cellHeight));
	}

	renderer.BindTexture(&font.GetTexture());
//	renderer.SetBlendMode(BlendMode::ALPHA);
	if (textVerts.size())
	{
		renderer.DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	}

	// draw caret if visible
	if (m_caretVisible)
	{
		m_theRenderer->BindTexture(nullptr); 
		m_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
	}
}

bool DevConsole::RunScript(const char* scriptFile)
{
	UNUSED(scriptFile);
	// extract the commands and parameters from the file and create an event command to run each line
	return false;
}

bool DevConsole::IsOpen()
{
	m_accessMutex.lock();
	bool result = (m_mode == DevConsoleMode::HIDDEN) ? false : true;
	m_accessMutex.unlock();
	return result;
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_accessMutex.lock();
	m_mode = mode;
	m_accessMutex.unlock();
}

void DevConsole::SetRenderer(Renderer* theRenderer)
{
	m_accessMutex.lock();
	m_theRenderer = theRenderer;  // needed to be able to create DevConsole before Renderer, but useful to change the renderer
	m_accessMutex.unlock();
}

void DevConsole::Togglemode(DevConsoleMode mode)
{
	m_accessMutex.lock();
	m_mode = (m_mode == mode) ? DevConsoleMode::HIDDEN : mode;
	m_accessMutex.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool DevConsole::ProcessCharacter(int charCode)
{
	m_accessMutex.lock();
	if ((charCode >= 32) && (charCode <= 126) && (charCode != '`') && (charCode != '~'))
	{
		m_inputText.insert(m_caretPosition, 1, static_cast<char>(charCode));
		m_caretPosition++;
		m_accessMutex.unlock();
		return true;
	}
	m_accessMutex.unlock();
	return false;
}

bool DevConsole::ProcessCommand(int charCode)
{
	if (charCode == 0x09) // TAB key for paste
	{
		std::string pasteText = g_theWindow->CopyClipboardToString();
		for (int index = 0; index < pasteText.size(); index++)
		{
			ProcessCharacter(pasteText[index]);
		}
		return true;
	}

	m_accessMutex.lock();
	switch (charCode)
	{
	case 0xC0: // KEYCODE_TILDE
		m_mode = DevConsoleMode::HIDDEN;
		break;

	case 0x1B: // KEYCODE_ESC doesn't compile for some reason
		m_inputText.clear();
		m_caretPosition = 0;
		break;

	case 0x0D:  //KEYCODE_ENTER:
		if (m_inputText.size() == 0)
			break; // no command to execute
		Execute(m_inputText);
		AddLine(Rgba8::GREEN, m_inputText);
		if (m_commandHistory.size() > m_maxCommandHistory)
		{
			m_commandHistory.erase(m_commandHistory.begin());
		}
		m_commandHistory.push_back(m_inputText);
		m_historyIndex = static_cast<int>(m_commandHistory.size()) - 1; // point to just added line
		m_inputText.clear();
		m_caretPosition = 0;
		break;

	case 0x27: // KEYCODE_RIGHTARROW
		m_caretPosition++;
		m_caretPosition = m_caretPosition > static_cast<int>(m_inputText.size()) ? static_cast<int>(m_inputText.size()) : m_caretPosition;
		break;

	case 0x25: // KEYCODE_LEFTARROW
		m_caretPosition--;
		m_caretPosition = m_caretPosition < 0 ? 0 : m_caretPosition;
		break;

	case 0x26: // KEYCODE_UPARROW
		if (m_historyIndex == -1)
			break; // no history yet
		m_historyIndex++;
		if (m_historyIndex == m_commandHistory.size())
			m_historyIndex = 0;
		m_inputText = m_commandHistory[m_historyIndex];
		m_caretPosition = static_cast<int>(m_inputText.size());
		break;

	case 0x28: // KEYCODE_DOWNARROW
		if (m_historyIndex == -1)
			break; // no history yet
		m_historyIndex--;
		if (m_historyIndex < 0)
			m_historyIndex = static_cast<int>(m_commandHistory.size()) - 1;
		m_inputText = m_commandHistory[m_historyIndex];
		m_caretPosition = static_cast<int>(m_inputText.size());
		break;

	case 0x24: // KEYCODE_HOME:
		m_caretPosition = 0;
		break;

	case 0x23: // KEYCODE_END:
		m_caretPosition = static_cast<int>(m_inputText.size());
		break;

	case 0x2E: // KEYCODE_DELETE
		if (m_caretPosition < static_cast<int>(m_inputText.size()))
		{
			m_inputText.erase(m_caretPosition, 1);
		}
		break;

	case 0x08: // KEYCODE_BACK
		if (m_caretPosition > 0)
		{
			m_inputText.erase(m_caretPosition - 1, 1);
			m_caretPosition--;
		}
		break;

	default:
		break;
	}
	m_accessMutex.unlock();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
void DevConsole::Clear()
{
	m_lines.clear();
}

DevConsoleLine::DevConsoleLine(Rgba8 const& color, std::string const& txt, int frameNumber, double time)
	: tint(color), text(txt), frame(frameNumber), timeStamp(time)
{

}

std::string DevConsoleLine::ToString() const
{
	std::string formatted;
	formatted += "(F:";
// 	formatted += frame < 10 ? " " : "";
// 	formatted += frame < 100 ? " " : "";
// 	formatted += frame < 1000 ? " " : "";
// 	formatted += frame < 10000 ? " " : "";
	formatted += std::to_string(frame);
	formatted += " T:";
	formatted += std::to_string(timeStamp);
	formatted = formatted.substr(0, formatted.size() - 1);
	formatted += ") ";
	formatted += text;
	return formatted;
}

bool Event_KeyPressed(EventArgs& args)
{
	std::string key = args.GetValue("key", "");
	UNUSED(key);
	return false;
}

bool Event_KeyReleased(EventArgs& args)
{
	std::string key = args.GetValue("key", "");
	int charCode = atoi(key.c_str());
	return g_theConsole->ProcessCommand(charCode);
}

bool Event_CharInput(EventArgs& args)
{
	std::string key = args.GetValue("key", "");
	int charCode = atoi(key.c_str());
	return g_theConsole->ProcessCharacter(charCode);
}

bool Command_Clear(EventArgs& args)
{
	UNUSED(args);
	g_theConsole->Clear();
	return true;
}

bool Command_Help(EventArgs& args)
{
	std::string filter = args.GetValue("filter", "");
	Strings commands;
	g_theEventSystem->GetRegisteredEventNames(commands);
	for (int index = 0; index < commands.size(); index++)
	{
		if (commands[index].substr(0, filter.size()).compare(filter) == 0)
		{
			g_theConsole->AddLine(Rgba8(0, 255, 255, 255), commands[index]);
		}
	}
	return true;
}

bool Command_Controls(EventArgs& args)
{
	UNUSED(args);
	// create screen layout view to print
	g_theConsole->AddLine(Rgba8(255, 255, 255, 255), "Key/Mouse   Gamepad       Description");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "W           LS Forward    Forward (+X)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "S           LS Backward   Backward (-X)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "A           LS Left       Left (+Y)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "D           LS Right      Right (-Y)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Z           L Shoulder    Up (+Z)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "C           R Shoulder    Down (-Z)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Q           L Trigger     Roll Left");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "E           L Trigger     Roll Right");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Mouse Left  RS Left       Yaw Left");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Mouse Right RS Right      Yaw Right");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Mouse Fwd   RS Forward    Pitch Down");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Mouse Back  RS Backward   Pitch Up");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "H           Start         Reset to Origin");
	g_theConsole->AddLine(Rgba8(255, 255, 255, 255), "-----------------------------------------");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Doomenstein controls are same as demo (out of time)");
	g_theConsole->AddLine(Rgba8(255, 255, 255, 255), "-----------------------------------------");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "LEFT SHIFT  4x Speed Boost");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "F5  Decrease Volume");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "F6  Increase Volume");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "F8  Reset");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "T  Slow Mo");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "Y  Fast Mo");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "O  Single step");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "P  Pause (or Play)");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "ESC  Pause/Exit Play, Exit Game");
	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "TILDE  Enter/Exit Dev Console");
//	g_theConsole->AddLine(Rgba8(255, 255, 255, 255), "-----------------------------------------");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "1  DebugRender Spawn Wire Sphere");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "2  DebugRender Spawn Line to Player");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "3  DebugRender Spawn Basis at Player");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "4  DebugRender Spawn Billboard Text");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "5  DebugRender Spawn Wire Cylinder");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "6  DebugRender Message Camera Orientation");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "7  DebugRender Spawn Cube at Player");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "8  DebugRender Spawn Arrow/Point to Player");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "9  DebugRender Decrease Clock Speed");
// 	g_theConsole->AddLine(Rgba8(252, 165, 4, 255), "0  DebugRender Increase Clock Speed");
	return true;
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	DebugRenderToggle();
	return true;
}

bool Command_DebugRaycastToggle(EventArgs& args)
{
	UNUSED(args);
	if (true) //g_theGame->m_map)
	{
		DebugRaycastToggle();
	}
	else
	{
		g_theConsole->AddLine(DevConsole::TINT_ERROR, "No map loaded!");
	}
	return true;
}

//======================================================================================================
// console commands
bool Event_RunScript(EventArgs& args)
{
	std::string script = args.GetValue("script", "");
	if (script == "")
	{
		return false;
	}
	// should this run as a member function or run the commands here on the console object?  How is mutex handled?
	return g_theConsole->RunScript(script.c_str());
}

/*
bool Event_Host(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Join(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Echo(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Command(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Broadcast(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Kick(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Event_Blacklist(EventArgs& args)
{
	UNUSED(args);
	return true;
}
*/
