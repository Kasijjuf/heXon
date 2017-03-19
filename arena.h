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

#ifndef TILEMASTER_H
#define TILEMASTER_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"

namespace Urho3D {
class Node;
}

class Tile;
class Slot;

enum TileElement {TE_CENTER = 0, TE_NORTH, TE_EAST, TE_SOUTH, TE_WEST, TE_NORTHWEST, TE_NORTHEAST, TE_SOUTHEAST, TE_SOUTHWEST, TE_LENGTH};
enum TileType {B_SPACE, B_EMPTY, B_ENGINE};

class Arena : public LogicComponent
{
    URHO3D_OBJECT(Arena, LogicComponent);
    friend class InputMaster;
    friend class MasterControl;
    friend class Tile;
public:
    Arena(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);

    void AddToAffectors(Node* affector, RigidBody* rigidBody);
    void RemoveFromAffectors(Node* affector);
    HashMap<Node*, RigidBody* >* GetAffectors() { return &hexAffectors_; }

    Tile* GetRandomTile();
    void FlashX(Color color);
private:
    Vector3 targetPosition_;
    Vector3 targetScale_;
    Node* logoNode_;
    Material* logoMaterial_;
    Material* xMaterial_;
    Light* playLight_;
    HashMap<Node*, RigidBody*> hexAffectors_;
    Vector<Tile*> tiles_;

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void EnterPlay(StringHash eventType, VariantMap &eventData);
    void EnterLobby(StringHash eventType, VariantMap &eventData);

};

#endif // TILEMASTER_H
