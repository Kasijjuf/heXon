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

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"

namespace Urho3D {
class Node;
class Scene;
}

class SceneObject : public LogicComponent
{
    friend class SpawnMaster;
    friend class Seeker;
    friend class Door;
    URHO3D_OBJECT(SceneObject, LogicComponent);
public:
    SceneObject(Context* context);
    virtual void OnNodeSet(Node* node);
    void Set(const Vector3 position);
    void Set(const Vector3 position, const Quaternion rotation);
    void Disable();

    Vector3 GetPosition() const { return node_->GetPosition(); }
    bool IsEmerged() const { return GetPosition().y_ > -0.042f; }
    bool IsEnabled() const { return node_->IsEnabled(); }

    void PlaySample(Sound *sample, const float gain = 0.5f);
protected:
    bool blink_;
    bool big_;
    SharedPtr<Node> soundNode_;
    SharedPtr<Node> graphicsNode_;
    Vector<SoundSource*> sampleSources_;

    void Emerge(const float timeStep);

    bool IsPlayingSound();
    void StopAllSound();

    void BlinkCheck(StringHash eventType, VariantMap &eventData);
};

#endif // SCENEOBJECT_H
