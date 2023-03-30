#include "Engine/Core/EventSystem.hpp"

EventSystem* g_theEventSystem = nullptr;

EventSystem::EventSystem(EventSystemConfig const& config)
	: m_config(config)
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{

}

void EventSystem::Shutdown()
{

}

void EventSystem::BeginFrame()
{

}

void EventSystem::EndFrame()
{

}

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	m_eventMutex.lock();
	EventSubscription subscription;
	subscription.callbackFunction = functionPtr;
	subscriptionList subList;
	std::map< std::string, subscriptionList >::const_iterator iter = m_subscriptionListByEventName.find(eventName);
	if (iter == m_subscriptionListByEventName.end())
	{
		subList.push_back(subscription);
		m_subscriptionListByEventName.insert(std::pair<std::string, subscriptionList>(eventName, subList));
		m_eventMutex.unlock();
		return; // new event registered
	}
	subList = iter->second;
	subList.push_back(subscription);
	m_eventMutex.unlock();
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	m_eventMutex.lock();
	std::map< std::string, subscriptionList >::const_iterator iter = m_subscriptionListByEventName.find(eventName);
	if (iter == m_subscriptionListByEventName.end())
	{
		m_eventMutex.unlock();
		return; // event has not been registered
	}
	subscriptionList subList = iter->second;
	for (int index = 0; index < static_cast<int>(subList.size()); index++)
	{
		if (subList[index].callbackFunction == functionPtr)
		{
			subList[index].callbackFunction = nullptr; // we could break, but lets handle multiples by default
		}
	}
	m_eventMutex.unlock();
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	m_eventMutex.lock();
	bool wasHandled = false;
	std::map< std::string, subscriptionList >::const_iterator iter = m_subscriptionListByEventName.find(eventName);
	if (iter == m_subscriptionListByEventName.end())
	{
		m_eventMutex.unlock();
		return; // event has not been registered
	}
	subscriptionList subList = iter->second;
	for (int index = 0; index < static_cast<int>(subList.size()); index++)
	{
		if (subList[index].callbackFunction)
		{
			wasHandled = subList[index].callbackFunction(args);
			if (wasHandled)
			{
				break;
			}
		}
	}
	m_eventMutex.unlock();
}

void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs dummyArgs;
	FireEvent(eventName, dummyArgs);
}

void EventSystem::GetRegisteredEventNames(std::vector< std::string >& outNames) const
{
	m_eventMutex.lock();
	std::map< std::string, subscriptionList >::const_iterator iter = m_subscriptionListByEventName.cbegin();
	while (iter != m_subscriptionListByEventName.end())
	{
		subscriptionList subList = iter->second;
		if (subList.size() > 0)
		{
			outNames.push_back(iter->first);
		}
		iter++;
	}
	m_eventMutex.unlock();
}

// global functions that call the global event system
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
}

void FireEvent(std::string const& eventName, EventArgs& args)
{
	g_theEventSystem->FireEvent(eventName, args);
}

void FireEvent(std::string const& eventName)
{
	g_theEventSystem->FireEvent(eventName);
}
