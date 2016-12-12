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

#include "inputmaster.h"
#include "player.h"

#include "phaser.h"

void Phaser::RegisterObject(Context *context)
{
    context->RegisterFactory<Phaser>();
}

Phaser::Phaser(Context* context) : Effect(context),
    phaseMaterial_{MC->GetMaterial("Phase")->Clone()},
    staticModel_{},
    velocity_{}
{
}

void Phaser::OnNodeSet(Node *node)
{
    Effect::OnNodeSet(node);

    node_->SetName("Phaser");
    staticModel_ = node_->CreateComponent<StaticModel>();
}

void Phaser::Set(Model* model, const Vector3 position, const Vector3 velocity)
{

    Effect::Set(position);

    velocity_ = velocity;
    node_->LookAt(position+velocity);

    staticModel_->SetModel(model);
    staticModel_->SetMaterial(phaseMaterial_);
}

void Phaser::Update(float timeStep)
{
    Effect::Update(timeStep);

    node_->Translate(velocity_ * timeStep, TS_WORLD);
    phaseMaterial_->SetShaderParameter("Dissolve", age_ * 2.3f);
    if (age_ > 2.0f){

        Disable();
        for (Controllable* c : GetSubsystem<InputMaster>()->GetControlled())
            if (c->IsEnabled())
                return;

        MC->SetGameState(GS_LOBBY);
    }
}
