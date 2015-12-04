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

#ifndef BULLET_H
#define BULLET_H

#include <Urho3D/Urho3D.h>

#include "spawnmaster.h"
#include "razor.h"
#include "spire.h"
#include "tilemaster.h"
#include "hitfx.h"

#include "sceneobject.h"

namespace Urho3D {
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Bullet : public SceneObject
{
    friend class Player;
    friend class SpawnMaster;
    URHO3D_OBJECT(Bullet, SceneObject);
public:
    Bullet(Context *context, MasterControl* masterControl);
    void Set(const Vector3 position);
protected:
    SharedPtr<RigidBody> rigidBody_;
    SharedPtr<StaticModel> model_;

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
private:
    float age_ = 0.0f;
    float timeSinceHit_ = 0.0f;
    float lifeTime_;
    bool fading_ = false;
    float damage_;
    void HitCheck(const float timeStep);
    void Disable();
};

#endif // BULLET_H
