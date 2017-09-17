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
    blunk_{0},
    free_{false}
{

}

void Brick::OnNodeSet(Node* node)
{ if (!node) return;

    SceneObject::OnNodeSet(node);

    big_ = false;

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

    blunk_ = 0;
    free_ = false;

    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    MC->arena_->AddToAffectors(node_);

    particleEmitter_->RemoveAllParticles();
    particleEmitter_->SetEmitting(true);

    node_->LookAt(position + direction);
    rigidBody_->ApplyImpulse(direction * 123.0f);

    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Brick, HandleTriggerStart));
    SubscribeToEvent(node_, E_NODECOLLISIONEND, URHO3D_HANDLER(Brick, HandleTriggerEnd));
}

void Brick::Blink(Vector3 newPosition)
{
    if (blunk_ > 1) {
        GetSubsystem<SpawnMaster>()->Create<HitFX>()
                ->Set(node_->GetPosition(), 0, false);
        Disable();
        return;
    }


    SceneObject::Blink(newPosition);

    ++blunk_;
}

void Brick::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    if (!node_->IsEnabled())
        return;

    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (unsigned i{0}; i < collidingBodies.Size(); ++i) {
        RigidBody* collider{ collidingBodies[i] };
        if (Ship* hitShip = collider->GetNode()->GetComponent<Ship>()) {

            hitShip->Hit(damage_, false);

            GetSubsystem<SpawnMaster>()->Create<HitFX>()
                    ->Set(node_->GetPosition(), 0, false);
            collider->ApplyImpulse(rigidBody_->GetLinearVelocity() * 0.5f);
            Disable();
        } else if (ChaoMine* chaoMine = collider->GetNode()->GetComponent<ChaoMine>()) {

            chaoMine->Hit(damage_, 0);

        } else if (Spire* spire = collider->GetNode()->GetComponent<Spire>()) {

            spire->Shoot(false)->SetLinearVelocity(rigidBody_->GetLinearVelocity() * 0.23f);
            Disable();
        } else if (Brick* brick = collider->GetNode()->GetComponent<Brick>()) {
            if (free_ && node_->GetDirection().DotProduct(brick->GetNode()->GetDirection()) < -0.9f){
                if (brick->GetNode()->IsEnabled())
                    GetSubsystem<SpawnMaster>()->Create<HitFX>()
                            ->Set((node_->GetPosition() + brick->GetNode()->GetPosition()) * 0.5f, 0, false);
                Disable();
            }
        }
    }
}
void Brick::HandleTriggerEnd(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    if (!free_)
        free_ = true;
}

void Brick::Disable()
{
    SceneObject::Disable();

    particleEmitter_->GetNode()->SetEnabled(true);
    particleEmitter_->SetEmitting(false);
}
