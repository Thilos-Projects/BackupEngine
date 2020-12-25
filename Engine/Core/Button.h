#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <Serializer_Include.h>
#include "../other/Transform.h";

//#include <d3d11.h>
//#include <d3dx11.h>
#include <xnamath.h>

class Button : public Component {
private:
	static ComponentRegister<Button> reg;
public:
	Transform* transform;
	bool up, down;

	class ButtonActor : public Component {
	public:
		virtual void akt() {}; // if over
		virtual void klicked() {}; // if down and up over
		virtual void down() {}; // if down over
		virtual void up() {}; //if up over
	};

	void checkKlicked(uint16_t x, uint16_t y, XMMATRIX &kameraViewMatrix, D3D11_VIEWPORT &viewPort, bool _down, bool _up) {
		XMFLOAT4 rect = transform->screenPos(kameraViewMatrix, viewPort);

		if (x < rect.x && y > rect.y && x > rect.z && y < rect.w) {
			if (_down) {
				for (int i = 0; i < payload.size(); i++)
					payload[i]->akt();
				for (int i = 0; i < payload.size(); i++)
					payload[i]->down();
				down = true;
				if (up) {
					up = false;
					down = false;
					for (int i = 0; i < payload.size(); i++)
						payload[i]->klicked();
				}
			}else if (_up) {
				for (int i = 0; i < payload.size(); i++)
					payload[i]->akt();
				for (int i = 0; i < payload.size(); i++)
					payload[i]->up();
				up = true;
				if (down) {
					up = false;
					down = false;
					for (int i = 0; i < payload.size(); i++)
						payload[i]->klicked();
				}
			}
		} else {
			up = false;
			down = false;
		}
			
	}

	void addActor(ButtonActor* ba) {
		std::vector<ButtonActor*> c = ba->gameobject->findComponents<ButtonActor>();
		for (int i = 0; i < c.size(); i++) {
			if (c[i] == ba) {
				payload.push_back(ba);
				payloadIDs.push_back(std::pair<int, int>(ba->gameobject->getID(), i));
				return;
			}
		}
	}
	void remActor(ButtonActor* ba) {
		for (int i = 0; i < payload.size(); i++) {
			if (payload[i] == ba) {
				payload.erase(payload.begin() + i);
				payloadIDs.erase(payloadIDs.begin() + i);
				return;
			}
		}
	}

	//Component stuf
	static std::vector<std::string> getRequirements() {
		std::vector<std::string> requirements = std::vector<std::string>();
		requirements.push_back(typeid(Transform).name());
		return requirements;
	};
	static bool getIsLonly() { return true; }; 
	void setRequirements(std::vector <Component*> toSet) {
		transform = (Transform*)toSet[0];
	}
	virtual void set(SerializedObject& from) {
		int size;
		from >> size;
		int temp0, temp1;
		for (int i = 0; i < size; i++) {
			from >> temp0;
			from >> temp1;
			payloadIDs.push_back(std::pair<int, int>(temp1, temp0));
		}
	};
	virtual void afterSet() {
		for (int i = 0; i < payloadIDs.size(); i++) {
			payload.push_back((Button::ButtonActor*)ObjectStorrage::getInstance()->getById(payloadIDs[i].first)->findComponents<Button::ButtonActor>()[payloadIDs[i].second]);
		}
	}
	virtual SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	};
	virtual SerializedObject serialize(SerializedObject& inTo) {
		int size = payloadIDs.size();
		for (int i = size - 1; i > -1; i--) {
			inTo << payloadIDs[i].first;
			inTo << payloadIDs[i].second;
		}
		inTo << size;
		inTo << typeid(Button).name();
		return inTo;
	};

protected:

	std::vector<Button::ButtonActor*> payload;
	std::vector<std::pair<int, int>> payloadIDs;
};
ComponentRegister<Button> Button::reg(typeid(Button).name());

#endif