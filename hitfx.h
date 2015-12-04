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

#ifndef HITFX_H
#define HITFX_H

#include <Urho3D/Urho3D.h>

#include "effect.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

class HitFX : public Effect
{
    URHO3D_OBJECT(HitFX, Effect);
public:
    HitFX(Context* context, MasterControl* masterControl);
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void Set(Vector3 position, bool sound = true);
private:
    SharedPtr<Sound> sample_;
};

#endif // HITFX_H
