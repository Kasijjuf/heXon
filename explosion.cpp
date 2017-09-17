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

#include "spawnmaster.h"
#include "chaomine.h"
#include "coin.h"
#include "effectinstance.h"

#include "explosion.h"

void Explosion::RegisterObject(Context *context)
{
    context->RegisterFactory<Explosion>();

    MC->GetSample("Explode");
}

Explosion::Explosion(Context* context):
    Effect(context),
    playerID_{0},
    initialMass_{3.0f},
    initialBrightness_{8.0f}
{
}

void Explosion::OnNodeSet(Node *node)
{ if (!node) return;

    Effect::OnNodeSet(node);

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(initialMass_);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);

    light_ = node_->CreateComponent<Light>();
    light_->SetRange(13.0f);
    light_->SetBrightness(initialBrightness_);

    particleEmitter_->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/Explosion.xml"));

    sampleSource_ = node_->CreateComponent<SoundSource>();
    sampleSource_->SetSoundType(SOUND_EFFECT);
}

void Explosion::Update(float timeStep)
{
    Effect::Update(timeStep);
    light_->SetBrightness(Max(initialBrightness_ * (0.32f - age_) / 0.32f, 0.0f));
}
void Explosion::FixedUpdate(float timeStep)
{
    rigidBody_->SetMass(Max(initialMass_ * ((0.1f - age_) / 0.1f), 0.01f));

    if (node_->IsEnabled() && MC->scene_->IsUpdateEnabled()) {
        PODVector<RigidBody*> hitResults{};
        float radius{ 2.0f * initialMass_ + age_ * 7.0f };

        if (MC->PhysicsSphereCast(hitResults, node_->GetPosition(), radius, M_MAX_UNSIGNED)) {

            for (RigidBody* h : hitResults){
                Node* hitNode{ h->GetNode() };
                if (hitNode->GetName() == "PickupTrigger"){
                    hitNode = hitNode->GetParent();
                    h = hitNode->GetComponent<RigidBody>();
                }

                Vector3 hitNodeWorldPos{ hitNode->GetWorldPosition() };
                if ((!h->IsTrigger() || hitNode->HasComponent<Coin>()) && h->GetPosition().y_ > -0.1f) {
                    //positionDelta is used for force calculation
                    Vector3 positionDelta{ hitNodeWorldPos - node_->GetWorldPosition() };
                    float distance{ positionDelta.Length() };
                    Vector3 force{ positionDelta.Normalized() * Max(radius-distance, 0.0f)
                                 * timeStep * 2342.0f * rigidBody_->GetMass() };
                    h->ApplyForce(force);

                    //Deal damage
                    float damage{rigidBody_->GetMass() * timeStep};

                    Enemy* e{ h->GetNode()->GetDerivedComponent<Enemy>() };
                    if (e && !e->IsInstanceOf<ChaoMine>()) {

                        e->Hit(damage, playerID_);
                    }
                }
            }
        }
    }
}

void Explosion::Set(const Vector3 position, const Color color, const float size, int colorSet, bool bubbles)
{
    playerID_ = colorSet;
    Effect::Set(position);
    node_->SetScale(size);
    initialMass_ = 3.0f * size;
    rigidBody_->SetMass(initialMass_);
    light_->SetColor(color);
    light_->SetBrightness(initialBrightness_);

    if (bubbles) {
        EffectInstance* bubbles{ GetSubsystem<SpawnMaster>()->Create<EffectInstance>() };
        ParticleEffect* bubbleEffect{ CACHE->GetResource<ParticleEffect>("Particles/ExplosionBubbles.xml") };
        bubbleEffect->SetMaxVelocity(size * 13.0f);
        bubbles->Set(GetPosition(), bubbleEffect);
    }

    ParticleEffect* particleEffect{ particleEmitter_->GetEffect() };
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(color, 0.1f));
    colorFrames.Push(ColorFrame(color * 0.1f, 0.35f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    particleEffect->SetColorFrames(colorFrames);

    PlaySample(MC->GetSample("Explode"), Min(0.5f + 0.25f * size, 1.0f));

    MC->arena_->AddToAffectors(node_);
}
