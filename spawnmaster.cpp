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

#include "spawnmaster.h"

#include "arena.h"
#include "tile.h"
#include "player.h"
#include "ship.h"
#include "chaoball.h"
#include "chaomine.h"
#include "chaozap.h"
#include "razor.h"
#include "spire.h"
#include "seeker.h"
#include "flash.h"
#include "bubble.h"
#include "line.h"
#include "coin.h"

SpawnMaster::SpawnMaster(Context* context):
    Object(context),
    spawning_{false},
    razorInterval_{2.0f},
    sinceRazorSpawn_{0.0f},
    spireInterval_{23.0f},
    sinceSpireSpawn_{0.0f},
    bubbleInterval_{0.23f},
    sinceBubbleSpawn_{bubbleInterval_},
    sinceLastChaoPickup_{0.0f},
    chaoInterval_{CHAOINTERVAL}
{
}

void SpawnMaster::Prespawn()
{
    AUDIO->SetMasterGain(SOUND_EFFECT, 0.0f);
    for (int r{0}; r < 23; ++r) { Create<Razor>(false); }
    for (int s{0}; s < 7; ++s) { Create<Spire>(false); }
    for (int m{0}; m < 8; ++m) { Create<ChaoMine>(false); }
    for (int s{0}; s < 13; ++s) { Create<Seeker>(false); }
    for (int h{0}; h < 16; ++h) { Create<HitFX>(false); }
    for (int e{0}; e < 9; ++e) { Create<Explosion>(false); }
    for (int f{0}; f < 13; ++f) { Create<Flash>(false); };
    for (int b{0}; b < 42; ++b) { Create<Bubble>(false); }
    for (int l{0}; l < 2048; ++l) { Create<Line>(false); }
    for (int z{0}; z < 8; ++z) { Create<ChaoZap>(false); }
    AUDIO->SetMasterGain(SOUND_EFFECT, 1.0f);
}

void SpawnMaster::Activate()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SpawnMaster, HandleUpdate));
}
void SpawnMaster::Deactivate()
{
    UnsubscribeFromAllEvents();
}
void SpawnMaster::Clear()
{
    for (SharedPtr<Node> n : MC->scene_->GetChildren()) {
        for (SharedPtr<Component> c : n->GetComponents()) {

            if (c->IsInstanceOf<Enemy>()
             || c->IsInstanceOf<Effect>()
             || c->IsInstanceOf<Seeker>()
             || c->IsInstanceOf<Coin>())
            {
                SceneObject* s{ static_cast<SceneObject*>(c.Get()) };
                s->Disable();
                break;
            }
        }
    }

    MC->chaoBall_->Disable();
}

void SpawnMaster::Restart()
{
    Clear();
    razorInterval_      = 2.0f;
    sinceRazorSpawn_    = 0.0f;
    spireInterval_      = 23.0f;
    sinceSpireSpawn_    = 0.0f;

    sinceLastChaoPickup_ = 0.0f;
    chaoInterval_ = CHAOINTERVAL;
    Activate();
}

Vector3 SpawnMaster::SpawnPoint()
{
    Tile* randomTile{MC->arena_->GetRandomTile()};
    if (randomTile) {
        Vector3 tilePosition = randomTile->node_->GetPosition();
        return Vector3(tilePosition.x_, -23.0f, tilePosition.z_);
    }
    else return Vector3(Random(-5.0f, 5.0f), -42.0f, Random(-5.0f, 5.0f));
}

void SpawnMaster::HandleUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    if (!MC->scene_->IsUpdateEnabled())
        return;

    const float timeStep{ eventData[Update::P_TIMESTEP].GetFloat() };

    sinceRazorSpawn_ += timeStep;
    sinceSpireSpawn_ += timeStep;

    if (sinceRazorSpawn_ > razorInterval_ && CountActive<Razor>() < 23) {

        Razor* razor{ Create<Razor>() };
        razor->Set(SpawnPoint());

        sinceRazorSpawn_ = 0.0f;
        razorInterval_ = (7.0f - CountActive<Ship>() * 0.42f)
                * pow(0.95f, ((MC->SinceLastReset()) + 10.0f) / 10.0f);

    }
    if (sinceSpireSpawn_ > spireInterval_ && CountActive<Spire>() < 7) {

        Spire* spire{ Create<Spire>() };
        spire->Set(SpawnPoint());

        sinceSpireSpawn_ = 0.0f;
        spireInterval_ = (23.0f - CountActive<Ship>() * 0.42f)
                * pow(0.95f, ((MC->scene_->GetElapsedTime() - MC->world.lastReset) + 42.0f) / 42.0f);

    }

    if (!MC->chaoBall_->IsEnabled() && MC->GetGameState() == GS_PLAY) {
        if (sinceLastChaoPickup_ > chaoInterval_)
            MC->chaoBall_->Respawn();
        else sinceLastChaoPickup_ += timeStep;
    }


    sinceBubbleSpawn_ += timeStep;

    if (sinceBubbleSpawn_ > bubbleInterval_) {
        Create<Bubble>()->Set(BubbleSpawnPoint());
        sinceBubbleSpawn_ = 0.0f;
    }
}
Vector3 SpawnMaster::BubbleSpawnPoint()
{
    return Quaternion(( Random(5) - 2 ) * 60.0f, Vector3::UP) *
            (Vector3::FORWARD * 20.0f + Vector3::RIGHT * Random(-10.0f, 10.0f))
            + Vector3::DOWN * 23.0f;
}
