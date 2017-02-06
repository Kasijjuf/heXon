#ifndef MASON_H
#define MASON_H

#include <Urho3D/Urho3D.h>


class Mason : public LogicComponent
{
    URHO3D_OBJECT(Mason, LogicComponent);
public:
    Mason(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);
};

#endif // MASON_H
