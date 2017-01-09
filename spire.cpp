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

#include "spire.h"

#include "mastercontrol.h"
#include "player.h"
#include "seeker.h"
#include "spawnmaster.h"

void Spire::RegisterObject(Context *context)
{
    context->RegisterFactory<Spire>();
}

Spire::Spire(Context* context):
    Enemy(context),
    initialShotInterval_{5.0f},
    shotInterval_{initialShotInterval_},
    sinceLastShot_{0.0f}
{
}

void Spire::OnNodeSet(Node *node)
{ if (!node) return;

    Enemy::OnNodeSet(node);

    node_->SetName("Spire");

    health_ = initialHealth_ = 5.0f;
    worth_ = 10;

    rigidBody_->SetMass(3.0f);
    rigidBody_->SetLinearFactor(Vector3::ZERO);

    SharedPtr<Material> black{MC->GetMaterial("Spire")->Clone()};

    topNode_ = node_->CreateChild();
    topModel_ = topNode_->CreateComponent<StaticModel>();
    topModel_->SetModel(MC->GetModel("SpireTop"));
    topModel_->SetMaterial(black);

    bottomNode_ = node_->CreateChild();
    bottomModel_ = bottomNode_->CreateComponent<StaticModel>();
    bottomModel_->SetModel(MC->GetModel("SpireBottom"));
    bottomModel_->SetMaterial(black);

}

void Spire::Update(float timeStep)
{
    if (!node_->IsEnabled()) return;

    Enemy::Update(timeStep);
    //Pulse
    topModel_->GetMaterial()->SetShaderParameter("MatEmissiveColor", GetGlowColor());
    //Spin
    topNode_->Rotate(Quaternion(0.0f, timeStep*(50.0f+panic_*300.0f), 0.0f));
    bottomNode_->Rotate(Quaternion(0.0f, timeStep*-(50.0f+panic_*300.0f), 0.0f));

    if (MC->GetGameState() == GS_PLAY && IsEmerged()){
        //Shoot
        sinceLastShot_ += timeStep;
        if (sinceLastShot_ > shotInterval_){
            sinceLastShot_ = 0.0f;
            Seeker* seeker{ GetSubsystem<SpawnMaster>()->Create<Seeker>() };
            seeker->Set(node_->GetPosition());
        }
    }
}

void Spire::Hit(float damage, int ownerID)
{
    Enemy::Hit(damage, ownerID);
    shotInterval_ = initialShotInterval_ - 3.0f * panic_;
}

void Spire::Set(Vector3 position)
{
    Enemy::Set(position);
    shotInterval_ = initialShotInterval_;
}
