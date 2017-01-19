#ifndef CREDIT_H
#define CREDIT_H

#include <Urho3D/Urho3D.h>

#include "sceneobject.h"

class Coin : public SceneObject
{
    URHO3D_OBJECT(Coin, SceneObject);
public:
    Coin(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);
    virtual void Set(const Vector3 position);
    virtual void Disable();

    void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData);
private:
    static StaticModelGroup* coinGroup_;

    RigidBody* rigidBody_;
};

#endif // CREDIT_H
