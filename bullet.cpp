/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
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

#include "bullet.h"

Bullet::Bullet(Context *context, MasterControl *masterControl):
    SceneObject(context, masterControl),
    lifeTime_{1.0f},
    damage_{0.0f}
{
    blink_ = false;

    rootNode_->SetName("Bullet");
    rootNode_->SetEnabled(false);
    rootNode_->SetScale(Vector3(1.0f+damage_, 1.0f+damage_, 0.1f));
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Bullet.mdl"));
    model_->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Bullet.xml"));

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(0.5f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetFriction(0.0f);

    Light* light = rootNode_->CreateComponent<Light>();
    light->SetRange(6.66f);
    light->SetColor(Color(0.6f, 1.0f+damage_, 0.2f));
}

void Bullet::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    age_ += timeStep;
    rootNode_->SetScale(Vector3(Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Min(Min(35.0f*age_, 2.0f), Max(2.0f-timeSinceHit_*42.0f, 0.1f))
                                ));
    if (age_ > lifeTime_) {
        Disable();
    }

    if (timeStep > 0.0f && !fading_) HitCheck(timeStep);
}

void Bullet::Set(const Vector3 position)
{
    age_ = 0.0f;
    timeSinceHit_ = 0.0f;
    fading_ = false;

    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();
    SceneObject::Set(position);
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Bullet, HandleSceneUpdate));
}

void Bullet::Disable()
{
    fading_ = true;
    SceneObject::Disable();
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void Bullet::HitCheck(float timeStep) {
    float velocity = rigidBody_->GetLinearVelocity().Length();
    if (!fading_) {
        PODVector<PhysicsRaycastResult> hitResults{};
        Ray bulletRay(rootNode_->GetPosition() - rigidBody_->GetLinearVelocity()*timeStep*0.5f, rootNode_->GetDirection());
        if (masterControl_->PhysicsRayCast(hitResults, bulletRay, 1.5f*velocity*timeStep, M_MAX_UNSIGNED)){
            for (int i = 0; i < hitResults.Size(); i++){
                if (!hitResults[i].body_->IsTrigger() && hitResults[i].body_->GetNode()->GetNameHash() != N_PLAYER){
                    hitResults[i].body_->ApplyImpulse(rigidBody_->GetLinearVelocity()*0.05f);
                    masterControl_->spawnMaster_->SpawnHitFX(hitResults[i].position_);
                    //Deal damage
                    unsigned hitID = hitResults[i].body_->GetNode()->GetID();
                    if(masterControl_->spawnMaster_->spires_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->spires_[hitID]->Hit(damage_, 1);
                    }
                    else if(masterControl_->spawnMaster_->razors_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->razors_[hitID]->Hit(damage_, 1);
                    }
                    else if(masterControl_->spawnMaster_->chaoMines_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->chaoMines_[hitID]->Hit(damage_, 1);
                    }
                    Disable();
                }
            }
        }
    }
}
