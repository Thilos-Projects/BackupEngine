#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <Serializer_Include.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

class Transform : public Component {
private:
    static ComponentRegister<Transform> reg;
public:
    Transform() {
        m_parrent = NULL;
        m_position.x = m_position.y = m_position.z = 0;
        m_rotation.x = m_rotation.y = m_rotation.z = 0;
        m_scale.x = m_scale.y = m_scale.z = 1.0f;
        m_dirty = true;
    };
    virtual ~Transform() {
    };

    XMMATRIX GetWorldMatrix() {
        if (m_dirty) {
            XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
            XMMATRIX rotationX = XMMatrixRotationX(m_rotation.x);
            XMMATRIX rotationY = XMMatrixRotationY(m_rotation.y);
            XMMATRIX rotationZ = XMMatrixRotationZ(m_rotation.z);
            XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
            m_matrix = (rotationX * rotationY * rotationZ) * translation * scale ;
            m_dirty = false;
        }
        if (m_parrent)
            return XMMatrixMultiply(m_matrix, m_parrent->GetWorldMatrix());
        return m_matrix;
    };

    XMFLOAT4 screenPos(XMMATRIX& kaneraViewMatrix, D3D11_VIEWPORT& viewPort) {

        XMMATRIX mat = XMMatrixMultiply(GetWorldMatrix(), kaneraViewMatrix);
        mat = XMMatrixTranspose(mat);

        XMFLOAT4 out;
        float temp;

        temp = (0.5f * mat.m[3][0] + 0.5f * mat.m[3][1] + mat.m[3][2] + mat.m[3][3]);

        out.x = (0.5f * mat.m[0][0] + 0.5f * mat.m[0][1] + mat.m[0][2] + mat.m[0][3]) / temp;
        out.y = (0.5f * mat.m[1][0] + 0.5f * mat.m[1][1] + mat.m[1][2] + mat.m[1][3]) / temp;

        out.x = (out.x + 1) * viewPort.Width * 0.5f + viewPort.TopLeftX;
        out.y = (1 - out.y) * viewPort.Height * 0.5f + viewPort.TopLeftY;

        temp = (-0.5f * mat.m[3][0] + -0.5f * mat.m[3][1] + mat.m[3][2] + mat.m[3][3]);

        out.z = (-0.5f * mat.m[0][0] + -0.5f * mat.m[0][1] + mat.m[0][2] + mat.m[0][3]) / temp;
        out.w = (-0.5f * mat.m[1][0] + -0.5f * mat.m[1][1] + mat.m[1][2] + mat.m[1][3]) / temp;

        out.z = (out.z + 1) * viewPort.Width * 0.5f + viewPort.TopLeftX;
        out.w = (1 - out.w) * viewPort.Height * 0.5f + viewPort.TopLeftY;

        return out;
    };

    void SetPosition(XMFLOAT3& position) {
        m_position = position;
        m_dirty = true;
    };
    void SetPosition(float positionX, float positionY, float positionZ) {
        m_position = XMFLOAT3(positionX, positionY, positionZ);
        m_dirty = true;
    };
    void SetRotation(XMFLOAT3& rotation) {
        m_rotation = rotation;
        m_dirty = true;
    };
    void SetRotation(float rotationX, float rotationY, float rotationZ) {
        m_rotation = XMFLOAT3(rotationX, rotationY, rotationZ);
        m_dirty = true;
    };
    void SetScale(XMFLOAT3& scale) {
        m_scale = scale;
        m_dirty = true;
    };
    void SetScale(float scaleX, float scaleY, float scaleZ) {
        m_scale = XMFLOAT3(scaleX, scaleY, scaleZ);
        m_dirty = true;
    };

    void AddPosition(XMFLOAT3& position) {
        m_position.x += position.x;
        m_position.y += position.y;
        m_position.z += position.z;
        m_dirty = true;
    };
    void AddPosition(float positionX, float positionY, float positionZ) {
        m_position.x += positionX;
        m_position.y += positionY;
        m_position.z += positionZ;
        m_dirty = true;
    };
    void AddRotation(XMFLOAT3& rotation) {
        m_rotation.x += rotation.x;
        m_rotation.y += rotation.y;
        m_rotation.z += rotation.z;
        m_dirty = true;
    };
    void AddRotation(float rotationX, float rotationY, float rotationZ) {
        m_rotation.x += rotationX;
        m_rotation.y += rotationY;
        m_rotation.z += rotationZ;
        m_dirty = true;
    };
    void AddScale(XMFLOAT3& scale) {
        m_scale.x += scale.x;
        m_scale.y += scale.y;
        m_scale.z += scale.z;
        m_dirty = true;
    };
    void AddScale(float scaleX, float scaleY, float scaleZ) {
        m_scale.x += scaleX;
        m_scale.y += scaleY;
        m_scale.z += scaleZ;
        m_dirty = true;
    };

    void setParrent(Transform* parrent) {
        m_parrent = parrent;
        if (parrent->gameobject->findComponents<Transform>().size() != 1)
            throw "parrent is not the only transform in Parrent Gameobject";

        pGOID = parrent->gameobject->getID();
    }

    static bool getIsLonly() { return true; }; 
    void setRequirements(std::vector <Component*> toSet) {
    }
    virtual void set(SerializedObject& from) {
        from >> pGOID;
        from >> m_scale;
        from >> m_rotation;
        from >> m_position;
        m_dirty = true;
    };
    virtual void afterSet() {
        if(pGOID != -1)
            m_parrent = (Transform*)ObjectStorrage::getInstance()->getById(pGOID)->findComponents<Transform>()[0];
    }
    virtual SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    virtual SerializedObject serialize(SerializedObject& inTo) {
        inTo << m_position;
        inTo << m_rotation;
        inTo << m_scale;
        inTo << pGOID;
        inTo << typeid(Transform).name();
        return inTo;
    };

    XMFLOAT3 m_position;
    XMFLOAT3 m_rotation;
    XMFLOAT3 m_scale;
    bool m_dirty;
    XMMATRIX m_matrix;
    //parrentGameObjectID
    int pGOID = -1;
    Transform* m_parrent;
};
ComponentRegister<Transform> Transform::reg(typeid(Transform).name());
#endif