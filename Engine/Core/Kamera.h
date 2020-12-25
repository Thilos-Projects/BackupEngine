#ifndef _KAMERA_H_
#define _KAMERA_H_

#include <Serializer_Include.h>

#include "../other/Transform.h"

class Kamera : public Component
{
private:
    static ComponentRegister<Kamera> reg;
public:

	XMMATRIX getProjectionMatrix() {
        if (m_projectionDirty) {

			float screenWidthH = m_screenWidth / 2;
			float screenHeigthH = m_screenHeigth / 2;

			if (m_orthogonal)
				m_projectionMatrix = XMMatrixOrthographicOffCenterLH(-screenWidthH, screenWidthH, -screenHeigthH, screenHeigthH, m_minViewDist, m_maxViewDist);
			else
				m_projectionMatrix = XMMatrixPerspectiveOffCenterLH(-screenWidthH, screenWidthH, -screenHeigthH, screenHeigthH, m_minViewDist, m_maxViewDist);
            m_projectionDirty = false;
        }
        return XMMatrixMultiply(transform->GetWorldMatrix(), m_projectionMatrix);
    }

	void setProjection(float screenWidth, float screenHeigth, float maxViewDist, float minViewDist) {
        m_screenWidth = screenWidth;
        m_screenHeigth = screenHeigth;
        m_maxViewDist = maxViewDist;
        m_minViewDist = minViewDist;
        m_projectionDirty = true;
    }

	void setOrthogonal(bool value) {
		m_orthogonal = value;
		m_projectionDirty = true;
	}

	static std::vector<std::string> getRequirements() {
		std::vector<std::string> outPut = std::vector<std::string>();
		outPut.push_back(typeid(Transform).name());
		return outPut;
	};
	static bool getIsLonly() { return true; }; 
	void setRequirements(std::vector <Component*> toSet) {
		transform = (Transform*)toSet[0];
	}
	virtual void set(SerializedObject& from) {
		from >> m_orthogonal >> m_minViewDist >> m_maxViewDist >> m_screenHeigth >> m_screenWidth;
		m_projectionDirty = true;
	};
	virtual void afterSet() {
	}
	virtual SerializedObject serialize() {
		SerializedObject so;
		return serialize(so);
	};
	virtual SerializedObject serialize(SerializedObject& inTo) {
		inTo << m_screenWidth << m_screenHeigth << m_maxViewDist << m_minViewDist << m_orthogonal;
		inTo << typeid(Kamera).name();
		return inTo;
	};

	Transform* transform;
protected:

	XMMATRIX m_projectionMatrix;

	bool m_projectionDirty;

	float m_screenWidth;
    float m_screenHeigth;
    float m_maxViewDist;
    float m_minViewDist;

	bool m_orthogonal;
};
ComponentRegister<Kamera> Kamera::reg(typeid(Kamera).name());
#endif