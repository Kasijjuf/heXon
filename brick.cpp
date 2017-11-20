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

#include "arena.h"
#include "TailGenerator.h"
#include "hitfx.h"
#include "chaomine.h"
#include "spire.h"
#include "seeker.h"
#include "mason.h"
#include "ship.h"
#include "spawnmaster.h"

#include "brick.h"

void Brick::RegisterObject(Context* context)
{
    context->RegisterFactory<Brick>();
}

Brick::Brick(Context* context) : SceneObject(context),
    damage_{4.2f},
    traveled_{}
{

}

void Brick::OnNodeSet(Node* node)
{ if (!node) return;

    SceneObject::OnNodeSet(node);

    big_ = false;
    MC->arena_->AddToAffectors(node_);

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(2.3f);
    rigidBody_->SetLinearDamping(0.23f);
    rigidBody_->SetTrigger(true);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);

    CollisionShape* trigger{ node_->CreateComponent<CollisionShape>() };
    trigger->SetSphere(0.666f);

    Node* particleNode{ node_->CreateChild("Particles") };
    particleNode->SetPosition(Vector3::UP);
    particleEmitter_ = particleNode->CreateComponent<ParticleEmitter>();
    particleEmitter_->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/Brick.xml"));

    Light* light{ node_->CreateComponent<Light>() };
    light->SetRange(4.2f);
    light->SetBrightness(3.4f);
    light->SetColor(Color(1.0f, 1.0f, 1.0f));
}

void Brick::Set(Vector3 position, Vector3 direction)
{
    SceneObject::Set(position);

    traveled_ = 0.0f;

    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);

    particleEmitter_->RemoveAllParticles();
    particleEmitter_->SetEmitting(true);

    node_->LookAt(position + direction);
    rigidBody_->ApplyImpulse(direction * 123.0f);

    PlaySample(MC->GetSample("Brick"), 0.88f);

    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Brick, HandleTriggerStart));
}

void Brick::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    if (!node_->IsEnabled())
        return;

    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (unsigned i{0}; i < collidingBodies.Size(); ++i) {

        RigidBody* collider{ collidingBodies[i] };
        Node* collidingNode{ collider->GetNode() };

        if (collidingNode->HasComponent<Ship>()) {

            collidingNode->GetComponent<Ship>()->Hit(damage_, false);
            collider->ApplyImpulse(rigidBody_->GetLinearVelocity() * 0.5f);

            Disable();

        } else if (collidingNode->HasComponent<ChaoMine>()) {

            collidingNode->GetComponent<ChaoMine>()->Hit(damage_, 0);

        } /*else if (Spire* spire = collider->GetNode()->GetComponent<Spire>()) {

            spire->Shoot(false)->SetLinearVelocity(rigidBody_->GetLinearVelocity() * 0.23f);
            Disable();
        }*/
    }
}

void Brick::Disable()
{
    GetSubsystem<SpawnMaster>()->Create<HitFX>()
            ->Set(node_->GetPosition(), 0, false);

    SceneObject::Disable();

    particleEmitter_->GetNode()->SetEnabled(true);
    particleEmitter_->SetEmitting(false);
}

void Brick::Update(float timeStep)
{
    traveled_ += rigidBody_->GetLinearVelocity().Length() * timeStep;

    if (traveled_ > 35.0f)
        Disable();
}
