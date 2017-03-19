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

#include "hitfx.h"
#include "arena.h"
#include "player.h"
#include "ship.h"
#include "chaomine.h"
#include "spawnmaster.h"

#include "seeker.h"

void Seeker::RegisterObject(Context *context)
{
    context->RegisterFactory<Seeker>();

    MC->GetSample("Seeker");
}

Seeker::Seeker(Context* context):
    SceneObject(context),
    tailGen_{},
    age_{0.0f},
    lifeTime_{7.5f},
    damage_{2.3f}
{

}

void Seeker::OnNodeSet(Node *node)
{ if (!node) return;

    SceneObject::OnNodeSet(node);

    node_->SetName("Seeker");
    big_ = false;

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(2.3f);
    rigidBody_->SetLinearDamping(0.23f);
    rigidBody_->SetTrigger(true);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);

    CollisionShape* trigger{ node_->CreateComponent<CollisionShape>() };
    trigger->SetSphere(1.0f);

    ParticleEmitter* particleEmitter{ node_->CreateComponent<ParticleEmitter>() };
    particleEmitter->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/Seeker.xml"));

    AddTail();

    Light* light{ node_->CreateComponent<Light>() };
    light->SetRange(6.66f);
    light->SetBrightness(2.3f);
    light->SetColor(Color(1.0f, 1.0f, 1.0f));
}

void Seeker::Update(float timeStep)
{
    if (!IsEnabled()) return;

    age_ += timeStep;
    if (age_ > lifeTime_ && node_->IsEnabled()) {
        HitFX* hitFx{ GetSubsystem<SpawnMaster>()->Create<HitFX>() };
        hitFx->Set(GetPosition(), 0, false);
        Disable();
    }

    rigidBody_->ApplyForce((TargetPosition() - node_->GetPosition()).Normalized() * timeStep * 666.0f);
}

void Seeker::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    if (!node_->IsEnabled())
        return;

    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (unsigned i{0}; i < collidingBodies.Size(); ++i) {
        RigidBody* collider{ collidingBodies[i] };
        if (collider->GetNode()->HasComponent<Ship>()) {
            Ship* hitShip{ collider->GetNode()->GetComponent<Ship>() };

            hitShip->Hit(damage_, false);

            GetSubsystem<SpawnMaster>()->Create<HitFX>()
                    ->Set(node_->GetPosition(), 0, false);
            collider->ApplyImpulse(rigidBody_->GetLinearVelocity() * 0.5f);
            Disable();
        }
        else if (collider->GetNode()->HasComponent<ChaoMine>()){
            collider->GetNode()->GetComponent<ChaoMine>()->Hit(damage_, 0);
        }
        else if (collider->GetNode()->HasComponent<Seeker>()) {

            GetSubsystem<SpawnMaster>()->Create<HitFX>()
                    ->Set(node_->GetPosition(), 0, false);

            Disable();
        }
    }
}
Vector3 Seeker::TargetPosition()
{
    Player* nearestPlayer{ MC->GetNearestPlayer(GetPosition()) };
    if (nearestPlayer)
        return nearestPlayer->GetPosition();
    else
        return Vector3::ZERO;
}

void Seeker::Set(Vector3 position)
{
    age_ = 0.0f;
    SceneObject::Set(position);
    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
    AddTail();
    PlaySample(MC->GetSample("Seeker"), 0.666f);

    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Seeker, HandleTriggerStart));

    rigidBody_->ApplyImpulse((TargetPosition() - node_->GetPosition()).Normalized() * 2.3f);
}
void Seeker::Disable()
{
    RemoveTail();
    SceneObject::Disable();
}

void Seeker::AddTail()
{
    RemoveTail();

    tailGen_ = node_->CreateComponent<TailGenerator>();
    tailGen_->SetWidthScale(0.666f);
    tailGen_->SetTailLength(0.13f);
    tailGen_->SetNumTails(7);
    tailGen_->SetColorForHead(Color(0.5f, 0.23f, 0.666f, 0.42f));
    tailGen_->SetColorForTip(Color(0.0f, 0.1f, 0.23f, 0.0f));
}
void Seeker::RemoveTail()
{
    if (tailGen_){
        tailGen_->Remove();
        tailGen_ = nullptr;
    }
}
