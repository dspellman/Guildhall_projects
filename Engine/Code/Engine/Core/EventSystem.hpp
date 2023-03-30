#pragma once
#include <string>
#include <vector>
#include <map>
#include "NamedStrings.hpp"
#include <mutex>

struct EventSystemConfig
{

};

typedef NamedStrings EventArgs;
// C++ typedef for “any function which takes a (mutable) EventArgs by reference, and returns a bool”
typedef bool (*EventCallbackFunction)(EventArgs& args); // or you may alternatively use the new C++ “using” syntax for type aliasing

struct EventSubscription
{
	EventCallbackFunction callbackFunction = nullptr;
};

typedef std::vector<EventSubscription> subscriptionList;

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr ); 
	void UnsubscribeEventCallbackFunction( std::string const& eventName, EventCallbackFunction functionPtr ); 
	void FireEvent( std::string const& eventName,  EventArgs& args );
	void FireEvent( std::string const& eventName);
	void GetRegisteredEventNames( std::vector< std::string >& outNames ) const;
	
protected :
	EventSystemConfig	m_config;
	mutable std::recursive_mutex m_eventMutex;
	std::map< std::string, subscriptionList > m_subscriptionListByEventName;
};

	//	
	// Standalone global namespace helper functions; these forward to "the' event system, if it exists //
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
void FireEvent(std::string const& eventName);
