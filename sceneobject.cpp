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

#include "sceneobject.h"

#include "arena.h"
#include "spawnmaster.h"
#include "settings.h"
#include "bullet.h"
#include "brick.h"
#include "seeker.h"
#include "flash.h"
#include "mirage.h"
#include "hitfx.h"

SceneObject::SceneObject(Context* context):
    LogicComponent(context),
    blink_{true},
    big_{true}
{
}

void SceneObject::OnNodeSet(Node *node)
{ if (!node) return;

    for (int i{0}; i < 2; ++i){
        SoundSource3D* sampleSource3D{ node_->CreateComponent<SoundSource3D>() };
        sampleSource3D->SetDistanceAttenuation(42.0f, 256.0f, 2.0f);
        sampleSource3D->SetSoundType(SOUND_EFFECT);
        sampleSources3D_.Push(sampleSource3D);
    }
    for (int i{0}; i < 3; ++i){
        SoundSource* sampleSource{ node_->CreateComponent<SoundSource>() };
        sampleSource->SetSoundType(SOUND_EFFECT);
        sampleSources_.Push(sampleSource);
    }
}

void SceneObject::Set(const Vector3 position)
{
    StopAllSound();
    node_->SetEnabledRecursive(true);
    node_->SetPosition(position);

    if (blink_)
        SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(SceneObject, BlinkCheck));

    if (node_->HasComponent<Light>()) {

        node_->GetComponent<Light>()->SetEnabled(GetSubsystem<Settings>()->GetManyLights());
    }
}
void SceneObject::Set(const Vector3 position, const Quaternion rotation){
    node_->SetRotation(rotation);
    Set(position);
}

void SceneObject::Disable()
{
//    if (node_->HasComponent<Mirage>())
//        node_->GetComponent<Mirage>()
    node_->SetEnabledRecursive(false);

    if (blink_)
        UnsubscribeFromEvent(E_BEGINFRAME);

    UnsubscribeFromEvent(E_NODECOLLISIONSTART);
}

void SceneObject::PlaySample(Sound* sample, const float gain, bool localized)
{
    if (localized) {

        for (SoundSource3D* s : sampleSources3D_)
            if (!s->IsPlaying()){
                s->SetGain(gain);
                s->Play(sample);
                return;
            }
    } else {

        for (SoundSource* s : sampleSources_)
            if (!s->IsPlaying()){
                s->SetGain(gain);
                s->Play(sample);
                return;
            }
    }
}
void SceneObject::StopAllSound()
{
    for (SoundSource3D* s : sampleSources3D_)
        s->Stop();

    for (SoundSource* s : sampleSources_)
        s->Stop();
}
bool SceneObject::IsPlayingSound()
{
    for (SoundSource3D* s : sampleSources3D_)
        if (s->IsPlaying()) return true;
    for (SoundSource* s : sampleSources_)
        if (s->IsPlaying()) return true;
    return false;
}

void SceneObject::Blink(Vector3 newPosition)
{
    Vector3 oldPosition{ GetPosition() };
    node_->SetPosition(newPosition);

    Player* nearestPlayerA{ MC->GetNearestPlayer(oldPosition) };
    Player* nearestPlayerB{ MC->GetNearestPlayer(newPosition) };

    float distanceToNearestPlayer{};

    if (nearestPlayerA && nearestPlayerB) {
        distanceToNearestPlayer = Min(LucKey::Distance(nearestPlayerA->GetPosition(), oldPosition),
                                      LucKey::Distance(nearestPlayerB->GetPosition(), newPosition));
    } else {
        distanceToNearestPlayer = 23.0f;
    }

    float gain{ Max(0.07f, 0.13f - distanceToNearestPlayer * 0.0023f) };

    SPAWN->Create<Flash>()->Set(oldPosition, gain, big_);
    SPAWN->Create<Flash>()->Set(newPosition, gain, big_);
}

void SceneObject::BlinkCheck(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    if (MC->IsPaused())
        return;

    Vector3 flatPosition{ node_->GetPosition() * Vector3(1.0f, 0.0f, 1.0f) };
    float radius{ 20.0f };

    if (flatPosition.Length() > radius){
        Vector3 hexantNormal{ Vector3::FORWARD };
        int sides{ 6 };
        for (int h{0}; h < sides; ++h){
            Vector3 otherHexantNormal{Quaternion(h * (360.0f / sides), Vector3::UP) * Vector3::FORWARD};
            hexantNormal = flatPosition.Angle(otherHexantNormal) < flatPosition.Angle(hexantNormal)
                    ? otherHexantNormal : hexantNormal;
        }
        float boundsCheck{ flatPosition.Length() * LucKey::Cosine(M_DEGTORAD * flatPosition.Angle(hexantNormal)) };
        if (boundsCheck > radius){
            if (node_->HasComponent<Bullet>()
             || node_->HasComponent<Seeker>()
             || node_->HasComponent<Brick>()){

                HitFX* hitFx{ GetSubsystem<SpawnMaster>()->Create<HitFX>() };
                hitFx->Set(GetPosition(), 0, false);
                Disable();

            } else if (blink_){
                Vector3 newPosition{ node_->GetPosition() - (1.995f * radius) * hexantNormal };
                Blink(newPosition);
            }
        }
    }
}

void SceneObject::Emerge(const float timeStep)
{
    if (!IsEmerged())
        node_->Translate(2.3f * Vector3::UP * timeStep *
                             (0.023f - node_->GetPosition().y_),
                             TS_WORLD);
}
