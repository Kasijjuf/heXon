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

#ifndef GUI3D_H
#define GUI3D_H

#include "mastercontrol.h"

class GUI3D : public LogicComponent
{
    URHO3D_OBJECT(GUI3D, LogicComponent);
public:
    GUI3D(Context* context);
    static void RegisterObject(Context* context);
    virtual void Update(float timeStep);
    virtual void OnNodeSet(Node* node);

    void Initialize(int playerId);
    void EnterLobby();
    void EnterPlay();
    void SetHealth(float health);
    void SetHeartsAndApples(int hearts, int apples);
    void SetScore(unsigned score);
private:
    int playerId_;
    unsigned score_;
    unsigned toCount_;

    float health_;
    int appleCount_;
    int heartCount_;

    AnimatedModel* healthIndicator_;

    Node* scoreNode_;
    HashMap< int, Node* > scoreDigits_;
    Node* healthBarNode_;
    StaticModel* healthBarModel_;
    Node* shieldBarNode_;
    StaticModel* shieldBarModel_;
    Node* appleCounterRoot_;
    HashMap< int, Node* > appleCounter_;
    Node* heartCounterRoot_;
    HashMap< int, Node* > heartCounter_;
    Color HealthToColor(float health);
    void CountScore();
};

#endif // GUI3D_H
