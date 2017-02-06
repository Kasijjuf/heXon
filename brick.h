#ifndef BRICK_H
#define BRICK_H

#include <Urho3D/Urho3D.h>


class Brick : public LogicComponent
{
    URHO3D_OBJECT(Brick, LogicComponent);
public:
    Brick(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);
};

#endif // BRICK_H
