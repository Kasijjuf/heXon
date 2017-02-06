#include "mason.h"


void Mason::RegisterObject(Context* context)
{
    context->RegisterFactory<Mason>();
}

Mason::Mason(Context* context) : LogicComponent(context)
{

}

void Mason::OnNodeSet(Node* node)
{ (void)node;
}

void Mason::Update(float timeStep)
{
}




