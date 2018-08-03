/* heXon
// Copyright (C) 2017 LucKey Productions (luckeyproductions.nl)
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

#include "mirage.h"
#include "phaser.h"
#include "spawnmaster.h"

void Razor::RegisterObject(Context *context)
{
    context->RegisterFactory<Razor>();
}

Razor::Razor(Context* context):
    Enemy(context),
    topSpeed_{10.0f},
    aimSpeed_{0.25f * topSpeed_},
    spinRate_{}
{

}

void Razor::OnNodeSet(Node* node)
{ if (!node) return;

    Enemy::OnNodeSet(node);

    meleeDamage_ = 0.9f;

    SharedPtr<Material> black{ MC->GetMaterial("Razor")->Clone() };

    topNode_ = node_->CreateChild("RazorTop");
    topModel_ = topNode_->CreateComponent<StaticModel>();
    topModel_->SetModel(MC->GetModel("RazorHalf"));
    topModel_->SetMaterial(0, MC->GetMaterial("Razor"));
    topModel_->SetMaterial(1, centerModel_->GetMaterial());

    bottomNode_ = node_->CreateChild("RazorBottom");
    bottomNode_->SetRotation(Quaternion(180.0f, Vector3::RIGHT));
    bottomModel_ = bottomNode_->CreateComponent<StaticModel>();
    bottomModel_->SetModel(MC->GetModel("RazorHalf"));
    bottomModel_->SetMaterial(0, black);
    bottomModel_->SetMaterial(1, centerModel_->GetMaterial());

    rigidBody_->SetLinearRestThreshold(0.0023f);

    node_->CreateComponent<Mirage>()->SetColor(color_);
}

void Razor::Update(float timeStep)
{
    if (!node_->IsEnabled())
        return;

    Enemy::Update(timeStep);

    //Spin
    spinRate_ = timeStep * (75.0f * aimSpeed_ - 25.0 * rigidBody_->GetLinearVelocity().Length());
    topNode_->Rotate(Quaternion(0.0f, spinRate_, 0.0f));
    bottomNode_->Rotate(Quaternion(0.0f, spinRate_, 0.0f));
    //Pulse
    topModel_->GetMaterial(0)->SetShaderParameter("MatEmissiveColor", GetGlowColor());
}

void Razor::FixedUpdate(float timeStep)
{
    //Get moving
    if (rigidBody_->GetLinearVelocity().Length() < rigidBody_->GetLinearRestThreshold() && IsEmerged()) {
        rigidBody_->ApplyForce(timeStep * 42.0f * (Quaternion(Random(6) * 60.0f, Vector3::UP) * Vector3::FORWARD));
    }
    //Adjust speed
    else if (rigidBody_->GetLinearVelocity().Length() < aimSpeed_) {
        rigidBody_->ApplyForce(timeStep * 235.0f * rigidBody_->GetLinearVelocity().Normalized() * Max(aimSpeed_ - rigidBody_->GetLinearVelocity().Length(), 0.1f));
    }
    else {
        float overSpeed{ rigidBody_->GetLinearVelocity().Length() - aimSpeed_ };
        rigidBody_->ApplyForce(timeStep * 100.0f * -rigidBody_->GetLinearVelocity() * overSpeed);
    }

    //Update linear damping
    if (!IsEmerged()) {
        rigidBody_->SetLinearDamping(Clamp(-node_->GetPosition().y_ * 0.666f, 0.0f, 1.0f));
    } else {
        rigidBody_->SetLinearDamping(0.1f);
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
}
void Razor::Blink(Vector3 newPosition)
{
    for (StaticModel* sm: {topModel_, bottomModel_}) {

        Phaser* phaser{ SPAWN->Create<Phaser>() };
        phaser->Set(sm->GetModel(), GetPosition(), node_->GetRotation(), rigidBody_->GetLinearVelocity(), Quaternion(spinRate_, Vector3::UP));
    }

    SceneObject::Blink(newPosition);
}
