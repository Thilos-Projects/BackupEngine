#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include <map>
#include <vector>

#include "FileWriter.h"
#include "baseSingleton.h"
#include "Serializer.h"

#define EMPTY_ID -1

class Component;
class ComponentFactory;
class GameObject;

class Component : public Serializable {
public:
	bool isActiv;
	GameObject* gameobject;
	static bool getIsLonly() { return false; };
	static std::vector<std::string> getRequirements() { return std::vector<std::string>(); };
	virtual void setRequirements(std::vector <Component*> toSet) = 0;

};

struct ComponentFactory : public baseFactory<Component> {
};
ComponentFactory::map_type* ComponentFactory::map = 0;
template<typename T>
struct ComponentRegister : ComponentFactory {
	ComponentRegister(std::string const& s) {
		getMap()->insert(std::make_pair(s, &createT<T,Component>));
	}
};

class FileComponent : public Component {
private:
	static ComponentRegister<FileComponent> reg;
public:
	virtual void set(SerializedObject& from) {
		from >> filePath;
		int size;
		from >> size;
		uint8_t temp;
		for (int i = 0; i < size; i++) {
			from >> temp;
			toPutInFile.push_back(temp);
		}
		createFileWith(filePath.c_str(), toPutInFile);
	}
	virtual void afterSet() {
	}
	virtual SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	}
	virtual SerializedObject serialize(SerializedObject& inTo) {
		int size = toPutInFile.size();
		for(int i = size - 1; i > -1; i--)
			inTo << toPutInFile[i];
		inTo << size;
		inTo << filePath;
		inTo << typeid(FileComponent).name();
		return inTo;
	}
	void setRequirements(std::vector <Component*> toSet) {

	}
	std::vector<uint8_t> toPutInFile;
	std::string filePath;
};
ComponentRegister<FileComponent> FileComponent::reg(typeid(FileComponent).name());

class GameObject : public Serializable {
private:
	int ID = EMPTY_ID;
	int parrentID = EMPTY_ID;

	std::vector<int> childrenIds;
	std::vector<Component*> componenten;

	bool isActiv;

public:
	std::string name = "";

	GameObject() {
		childrenIds = std::vector<int>();
		componenten = std::vector<Component*>();
	}
	~GameObject() {
		for (int i = 0; i < componenten.size(); i++)
			delete(componenten[i]);
	}

	int getID() { return ID; };
	void setID(int _ID) { ID = _ID; };
	int getParrentID() { return parrentID; };
	void setParrentID(int _ID);
	void setParrent(GameObject* _parrent);

	void addChild(int _ID) {
		childrenIds.push_back(_ID);
	}
	void remChild(int _ID) {
		childrenIds.erase(std::find(childrenIds.begin(), childrenIds.end(), _ID));
	}
	std::vector<int>* getChilds() {
		return &childrenIds;
	}

	void setActiv(bool value) {
		isActiv = value;
		for (int i = 0; i < componenten.size(); i++) {
			componenten[i]->isActiv = value;
		}
		for (int i = 0; i < childrenIds.size(); i++) {
			
		}
	}

	template<typename T>
	T* addComponent();
	template<typename T>
	std::vector<T*> findComponents();
	std::vector<Component*> findComponents(std::string name);
	template<typename T>
	std::vector<T*> findComponentsInChildren();
	std::vector<Component*> findComponentsInChildren(std::string name);

	std::string toString(int layer);

	friend GameObject& operator << (GameObject& msg, Component& data)
	{
		data.gameobject = &msg;
		msg.componenten.push_back(&data);
	}

	virtual void set(SerializedObject& from);
	virtual void afterSet();
	virtual SerializedObject serialize();
	virtual SerializedObject serialize(SerializedObject& inTo);
};

//declare storrage
class ObjectStorrage : public baseMapStorrage<int, GameObject,ObjectStorrage>, public Serializable {
private:
	int counter = 0;
public:
	GameObject* createGameObject() {
		GameObject* temp = new GameObject();
		temp->setID(counter);
		addOnId(counter,temp);
		counter++;
		return temp;
	}

	//is dangerous can throw exception
	void clear() {
		for (std::map<int, GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
			delete(it->second);
		}
		counter = 0;
		objects.clear();
	}

	std::vector<GameObject*> findByName(std::string name) {
		std::vector<GameObject*> toRet = std::vector<GameObject*>();
		for (int i = 0; i < objects.size(); i++) {
			if (objects[i]->name == name)
				toRet.push_back(objects[i]);
		}
		return toRet;
	}

	virtual void set(SerializedObject& from) {
		size_t size;
		from >> size;
		std::string name;
		for (int i = 0; i < size; i++) {
			GameObject* temp = new GameObject();
			from >> name;
			temp->set(from);
			addOnId(temp->getID(), temp);
			counter = std::max<int>(counter, (temp->getID() + 1));
		}
	}
	virtual void afterSet() {
		for (int i = 0; i < objects.size(); i++) {
			objects[i]->afterSet();
		}
	}
	virtual SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	}
	virtual SerializedObject serialize(SerializedObject& inTo) {
		for (std::map<int, GameObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
			it->second->serialize(inTo);
		}
		inTo << objects.size();
		inTo << typeid(ObjectStorrage).name();
		return inTo;
	}
};
//instantiate storrage
ObjectStorrage* ObjectStorrage::instance = 0;

//example 1
/*
class DerivedA : public Component {
private:
	static ComponentRegister<DerivedA> reg;
};
ComponentRegister<DerivedA> DerivedA::reg(typeid(DerivedA).name());
*/

void GameObject::setParrentID(int _parrentID) {
	GameObject* temp;
	if (parrentID != EMPTY_ID) {
		temp = ObjectStorrage::getInstance()->getById(parrentID);
		temp->remChild(ID);
	}
	if (_parrentID != EMPTY_ID) {
		temp = ObjectStorrage::getInstance()->getById(_parrentID);
		if (temp) {
			temp->addChild(ID);
		}
		else {
			throw "parren not found";
		}
	}
	parrentID = _parrentID;
}

void GameObject::setParrent(GameObject* _parrent)
{
	if (parrentID != EMPTY_ID)
		ObjectStorrage::getInstance()->getById(parrentID)->remChild(ID);

	_parrent->addChild(ID);
	parrentID = _parrent->getID();
}

std::string GameObject::toString(int layer)
{
	std::string s;
	s += ID + '0';
	s += "\t";
	
	if (childrenIds.size() == 0)
		s += "\n";
	else
		for (int i = 0; i < childrenIds.size(); i++) {
			if (i > 0)
				for (int i = 0; i < layer + 1; i++)
					s.append("\t");
			s += "->";
			s += ObjectStorrage::getInstance()->getById(childrenIds[i])->toString(layer + 1);
		}
	return s;
}

void GameObject::set(SerializedObject& from) {
	from >> ID;
	from >> name;
	from >> parrentID;
	int size;
	from >> size;
	int temp;
	for (int i = 0; i < size; i++)
	{
		from >> temp;
		childrenIds.push_back(temp);
	}
	from >> size;
	std::string name;
	for (int i = 0; i < size; i++)
	{
		from >> name;
		componenten.push_back(ComponentFactory::createInstance(name));
		componenten[componenten.size() - 1]->set(from);
		componenten[componenten.size() - 1]->gameobject = this;
	}
}
void GameObject::afterSet() {
	for (int i = 0; i < componenten.size(); i++) {
		componenten[i]->afterSet();
	}
};
SerializedObject GameObject::serialize() {
	SerializedObject so;
	return serialize(so);
}
SerializedObject GameObject::serialize(SerializedObject& inTo) {
	int size = componenten.size();
	for (int i = size - 1; i > -1; i--) {
		componenten[i]->serialize(inTo);
	}
	inTo << size;	
	size = childrenIds.size();
	for (int i = size - 1; i > -1; i--)
	{
		inTo << childrenIds[i];
	}
	inTo << size;
	inTo << parrentID;
	inTo << name;
	inTo << ID;
	inTo << typeid(GameObject).name();
	return inTo;
}

template<typename T>
T* GameObject::addComponent() {
	static_assert(std::is_base_of<Component, T>::value, "T is not a Componente");

	if (T::getIsLonly() && findComponents<T>().size() != 0)
		throw "es ist bereits eine Instanz dieser componente enthalten und die Componente ist als Lonly Makiert";

	std::vector<std::string> temp = T::getRequirements();
	std::vector<Component*> toSetInComponent = std::vector<Component*>();
	std::vector<Component*> tempC;
	for (int i = 0; i < temp.size(); i++) {
		tempC = findComponents(temp[i]);
		if (tempC.size() != 0 && tempC[0] != NULL)
			toSetInComponent.push_back(tempC[0]);
		else
			throw std::string("required components nicht vorhanden: ").append(temp[i]);
	}

	componenten.push_back(ComponentFactory::createInstance(typeid(T).name()));
	componenten[componenten.size() - 1]->gameobject = this;
	componenten[componenten.size() - 1]->setRequirements(toSetInComponent);
	componenten[componenten.size() - 1]->isActiv = isActiv;
	return (T*)componenten[componenten.size() - 1];
}
template<typename T>
std::vector<T*> GameObject::findComponents() {
	static_assert(std::is_base_of<Component, T>::value, "T is not a Componente");
	std::vector<T*> toRet = std::vector<T*>();
	for (int i = 0; i < componenten.size(); i++) {
		if (dynamic_cast<T*>(componenten[i]) != NULL)
			toRet.push_back((T*)componenten[i]);
	}

	return toRet;
}
std::vector<Component*> GameObject::findComponents(std::string name) {
	std::vector<Component*> toRet = std::vector<Component*>();
	for (int i = 0; i < componenten.size(); i++) {
		if (std::string(typeid(*componenten[i]).name())._Equal(name))
			toRet.push_back(componenten[i]);
	}

	return toRet;
}
template<typename T>
std::vector<T*> GameObject::findComponentsInChildren() {
	static_assert(std::is_base_of<Component, T>::value, "T is not a Componente");
	std::vector<T*> toRet = std::vector<T*>();
	std::vector<T*> temp = findComponents<T>();
	for (int i = 0; i < temp.size(); i++)
		toRet.push_back(temp[i]);
	for (int i = 0; i < childrenIds.size(); i++) {
		temp = ObjectStorrage::getInstance()->getById(childrenIds[i])->findComponents<T>(); 
		for (int j = 0; j < temp.size(); j++)
			toRet.push_back(temp[j]);
	}
	return toRet;
}
std::vector<Component*> GameObject::findComponentsInChildren(std::string name) {
	std::vector<Component*> toRet = std::vector<Component*>();
	std::vector<Component*> temp = findComponents(name);
	for (int i = 0; i < temp.size(); i++)
		toRet.push_back(temp[i]);
	for (int i = 0; i < childrenIds.size(); i++) {
		temp = ObjectStorrage::getInstance()->getById(childrenIds[i])->findComponentsInChildren(name);
		for (int j = 0; j < temp.size(); j++)
			toRet.push_back(temp[j]);
	}
	return toRet;
}
#endif