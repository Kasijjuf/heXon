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

#ifndef PILOT_H
#define PILOT_H

#include <Urho3D/Urho3D.h>

#include "sceneobject.h"

enum PilotColor { PC_SKIN, PC_SHIRT, PC_PANTS, PC_SHOES, PC_HAIR, PC_ALL };
enum Hair{HAIR_BALD, HAIR_SHORT, HAIR_MOHAWK, HAIR_SEAGULL, HAIR_MUSTAIN, HAIR_FROTOAD, HAIR_FLATTOP, HAIR_ALL};

class Pilot : public SceneObject
{
    URHO3D_OBJECT(Pilot, SceneObject);
    friend class Player;
    friend class MasterControl;
public:
    Pilot(Context* context);
//    Pilot(Node *parent, const std::string file, unsigned &score);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);

    void Randomize(bool autoPilot = false);
private:
    bool male_;
    int hairStyle_;
    HashMap<int, Color> pilotColors_;
    AnimatedModel* bodyModel_;
    StaticModel* hairModel_;
    AnimationController* animCtrl_;

//    void Load(std::__cxx11::string file, unsigned &score);
    void UpdateModel();
    void Save(int playerID, unsigned score);
};

#endif // PILOT_H
