#include "Serializer_Include.h"

/*class DerivedA : public Component {
private:
	static ComponentRegister<DerivedA> reg;
public:

	virtual void set(SerializedObject& from) {

	}
	virtual SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	}
	virtual SerializedObject serialize(SerializedObject& inTo) {
		inTo << typeid(DerivedA).name();
		return inTo;
	}
	static std::vector<std::string> getRequirements() { 
		std::vector<std::string> toRet = std::vector<std::string>();
		return toRet; 
	};
};
ComponentRegister<DerivedA> DerivedA::reg(typeid(DerivedA).name());*/

int main() {

	SerializedObject so;
	
	GameObject* entry = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p0 = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p00 = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p01 = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p010 = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p1 = ObjectStorrage::getInstance()->createGameObject();
	GameObject* p2 = ObjectStorrage::getInstance()->createGameObject();

	p0->setParrent(entry);
	p1->setParrent(entry);
	p2->setParrent(entry);

	p00->setParrent(p0);
	p01->setParrent(p0);
	p010->setParrent(p01);

	//p010->addComponent<DerivedA>();

	std::cout << entry->toString(0);

	so = ObjectStorrage::getInstance()->serialize();

	writeToFile(so, "test.bin");
	
	ObjectStorrage::getInstance()->clear();

	so = readSerializedObjectFromFile("test.bin");

	std::string name;

	so >> name;

	ObjectStorrage::getInstance()->set(so);

	std::cout << ObjectStorrage::getInstance()->getById(0)->toString(0);
}