#include "brick.h"


void Brick::RegisterObject(Context* context)
{
    context->RegisterFactory<Brick>();
}

Brick::Brick(Context* context) : LogicComponent(context)
{

}

void Brick::OnNodeSet(Node* node)
{ (void)node;
}

void Brick::Update(float timeStep)
{
}




