/* heXon
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
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

#include "razor.h"

void Razor::RegisterObject(Context *context)
{
    context->RegisterFactory<Razor>();
}

Razor::Razor():
    Enemy(),
    topSpeed_{10.0f},
    aimSpeed_{0.25f * topSpeed_}
{

}

void Razor::OnNodeSet(Node *node)
{
    Enemy::OnNodeSet(node);

    node_->SetName("Razor");
    meleeDamage_ = 0.9f;

    SharedPtr<Material> black{MC->GetMaterial("Razor")->Clone()};

    topNode_ = node_->CreateChild();
    topModel_ = topNode_->CreateComponent<StaticModel>();
    topModel_->SetModel(MC->GetModel("RazorTop"));
    topModel_->SetMaterial(0, black);
    topModel_->SetMaterial(1, centerModel_->GetMaterial());

    bottomNode_ = node_->CreateChild();
    bottomModel_ = bottomNode_->CreateComponent<StaticModel>();
    bottomModel_->SetModel(MC->GetModel("RazorBottom"));
    bottomModel_->SetMaterial(0, black);
    bottomModel_->SetMaterial(1, centerModel_->GetMaterial());

    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(Razor, HandleRazorUpdate));

}

void Razor::HandleRazorUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    float timeStep{eventData[ScenePostUpdate::P_TIMESTEP].GetFloat()};

    //Spin
    topNode_->Rotate(Quaternion(0.0f, timeStep*50.0f*aimSpeed_, 0.0f));
    bottomNode_->Rotate(Quaternion(0.0f, timeStep*-50.0f*aimSpeed_, 0.0f));
    //Pulse
    topModel_->GetMaterial(0)->SetShaderParameter("MatEmissiveColor", GetGlowColor());
    //Get moving
    if (rigidBody_->GetLinearVelocity().Length() < rigidBody_->GetLinearRestThreshold() && IsEmerged()) {
        rigidBody_->ApplyImpulse(0.23f*(Quaternion(Random(6)*60.0f, Vector3::UP)*Vector3::FORWARD));
    }
    //Adjust speed
    else if (rigidBody_->GetLinearVelocity().Length() < aimSpeed_) {
        rigidBody_->ApplyForce(4.2f * rigidBody_->GetLinearVelocity().Normalized() * Max(aimSpeed_ - rigidBody_->GetLinearVelocity().Length(), 0.1f));
    }
    else {
        float overSpeed = rigidBody_->GetLinearVelocity().Length() - aimSpeed_;
        rigidBody_->ApplyForce(-rigidBody_->GetLinearVelocity()*overSpeed);
    }
}

void Razor::Hit(float damage, int ownerID)
{
    Enemy::Hit(damage, ownerID);
    aimSpeed_ = (0.25f + 0.75f * panic_) * topSpeed_;
}

void Razor::Set(Vector3 position)
{
    aimSpeed_ = 0.25f * topSpeed_;
    Enemy::Set(position);
    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(Razor, HandleRazorUpdate));
}
