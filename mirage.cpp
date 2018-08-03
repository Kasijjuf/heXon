/* heXon
// Copyright (C) 2018 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "mirage.h"



void Mirage::RegisterObject(Context* context)
{
    context->RegisterFactory<Mirage>();
}

Mirage::Mirage(Context* context) : LogicComponent(context),
    billboardSet_{},
    color_{0.42f, 0.42f, 0.42f, 0.42f},
    fade_{1.0f}
{
}

void Mirage::OnNodeSet(Node* node)
{ if(!node) return;

    billboardSet_ = node_->GetScene()->CreateChild("Mirage")->CreateComponent<AnimatedBillboardSet>();

    billboardSet_->SetNumBillboards(6);
    billboardSet_->SetMaterial(CACHE->GetResource<Material>("Materials/Mirage.xml"));
    billboardSet_->SetSorted(true);

    for (Billboard& bb : billboardSet_->GetBillboards()) {

        bb.size_ = Vector2::ONE;
        bb.position_ = Vector3::ZERO;
        bb.color_ = color_;
        bb.enabled_ = false;

    }

    billboardSet_->LoadFrames(CACHE->GetResource<XMLFile>("Textures/Mirage.xml"));
    billboardSet_->Commit();

    SubscribeToEvent(node_->GetScene(), E_NODEENABLEDCHANGED, URHO3D_HANDLER(Mirage, HandleNodeEnabledChanged));
}
void Mirage::HandleNodeEnabledChanged(StringHash eventType, VariantMap& eventData)
{
    if (static_cast<Node*>(eventData[NodeEnabledChanged::P_NODE].GetPtr()) == node_)
            billboardSet_->SetEnabled(node_->IsEnabled());
}


void Mirage::PostUpdate(float timeStep)
{
    if (!billboardSet_->IsEnabled())
        return;

    for (unsigned b{0}; b < billboardSet_->GetNumBillboards(); ++b) {

        Billboard& bb{ billboardSet_->GetBillboards()[b] };

        float radius{ 20.0f };
        Vector3 offset{ Quaternion(60.0f * b, Vector3::UP) * Vector3::FORWARD * 2.0f * radius };
        float intensity{ Clamp((1.666f + radius - bb.position_.ProjectOntoAxis(offset)) * 0.6f, 0.0f, 1.0f) };
//        bb.size_ = Vector2(Random(0.42f, 0.55f), 0.5f);
        bb.position_ = node_->GetWorldPosition() + offset;
        bb.color_ = intensity * color_;
        bb.enabled_ = intensity > 0.01f;

    }
    billboardSet_->Commit();
}



