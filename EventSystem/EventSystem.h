#ifndef _EVENTSYSTEM_H_
#define _EVENTSYSTEM_H_

#include <Serializer_Include.h>

class EventSystem;

struct Event {
public:
	bool usable;
	bool destroy;
	virtual void instantFireEvent();
	virtual void updateFireEvent();
};

class EventListener : public Component {
public:
	virtual bool onEvent(Event *e) = 0;
};

class EventSystem : public Serializable, public baseSingleton<EventSystem> {
public:
	EventSystem() {
		listener = std::vector<EventListener*>();
		listenerIDs = std::vector<std::pair<int, int>>();
		onNextUpdate = std::vector<Event*>();
	};
	~EventSystem() {};

	void addEventListener(EventListener* el) {
		std::vector<EventListener*> c = el->gameobject->findComponents<EventListener>();
		for (int i = 0; i < c.size(); i++) {
			if (c[i] == el)
			{
				listener.push_back(el);
				listenerIDs.push_back(std::pair<int, int>(el->gameobject->getID(), i));
				return;
			}
		}
	}
	void removeEventListener(EventListener* el) {
		for (int i = 0; i < listener.size(); i++) {
			if (listener[i] == el)
			{
				listener.erase(listener.begin() + i);
				listenerIDs.erase(listenerIDs.begin() + i);
				return;
			}
		}
	}

	void addInstantTrigger(Event *e) {
		for (int i = 0; i < listener.size(); i++)
			if (listener[i]->onEvent(e) && e->usable)
				return;
	}
	void addUpdateTrigger(Event *e) {
		onNextUpdate.push_back(e);
	}

	void update() {
		for (int i = 0; i < onNextUpdate.size(); i++) {
			for (int j = 0; j < listener.size(); j++)
				if (listener[j]->onEvent(onNextUpdate[i]) && onNextUpdate[i]->usable)
					break;
			if (onNextUpdate[i]->destroy)
					delete(onNextUpdate[i]);
		}
		onNextUpdate.clear();
	}

	void set(SerializedObject& from) {};
	void afterSet() {};
	SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	};
	SerializedObject serialize(SerializedObject& inTo) {
		for (int i = 0; i < listenerIDs.size(); i++){
			inTo << listenerIDs[i].first;
			inTo << listenerIDs[i].second;
		}
		inTo << listenerIDs.size();
		inTo << typeid(EventSystem).name();
		return inTo;
	};

private:
	std::vector<EventListener*> listener;
	std::vector<std::pair<int,int>> listenerIDs;

	std::vector<Event*> onNextUpdate;
};
EventSystem* EventSystem::instance = 0;

void Event::instantFireEvent() {
	EventSystem::getInstance()->addInstantTrigger(this);
};
void Event::updateFireEvent() {
	EventSystem::getInstance()->addUpdateTrigger(this);
};

#endif