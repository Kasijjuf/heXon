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

#include "inputmaster.h"

#include "player.h"

InputMaster::InputMaster():
    Object(MC->GetContext())
{
    SubscribeToEvents();
}

void InputMaster::SubscribeToEvents()
{
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(Urho3D::E_MOUSEMOVE, URHO3D_HANDLER(InputMaster, HandleMouseMove));
    SubscribeToEvent(Urho3D::E_MOUSEWHEEL, URHO3D_HANDLER(InputMaster, HandleMouseWheel));
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(Urho3D::E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(Urho3D::E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(Urho3D::E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(Urho3D::E_JOYSTICKAXISMOVE, URHO3D_HANDLER(InputMaster, HandleJoystickAxisMove));
}

void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    int button{eventData[MouseButtonDown::P_BUTTON].GetInt()};
    if (button == MOUSEB_LEFT){
        //Set tile type
    }
    else if (button == MOUSEB_RIGHT){
        //Clear tile
    }
}

void InputMaster::HandleMouseMove(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseWheel(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseUp(StringHash eventType, VariantMap &eventData)
{
    int button{eventData[MouseButtonUp::P_BUTTON].GetInt()};
}

void InputMaster::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    //Get the triggering key
    int key{eventData[KeyDown::P_KEY].GetInt()};

    switch (key){
    //Exit when ESC is pressed
    case KEY_ESCAPE: EjectButtonPressed();
        break;
    //Take screenshot when 9 is pressed
    case KEY_9:
    {
        Screenshot();
    } break;
    //Pause/Unpause game on P or joystick Start
    case KEY_P:
    {
        PauseButtonPressed();
    } break;
    //Toggle music on M
    case KEY_M: MC->musicSource_->SetGain(MC->musicSource_->GetGain()==0.0f ? 0.32f : 0.0f);
        break;
    }
}

void InputMaster::HandleJoystickButtonDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    unsigned joystick{static_cast<unsigned>(eventData[JoystickButtonDown::P_JOYSTICKID].GetInt())};
    int button{eventData[JoystickButtonDown::P_BUTTON].GetInt()};

    JoystickState* joystickState{INPUT->GetJoystickByIndex(joystick)};
    // Process game event
    switch (button) {
    case JB_START: PauseButtonPressed();
        break;
    case JB_L2: case JB_R2:
        if (joystickState->GetButtonDown(JB_L2) &&
                joystickState->GetButtonDown(JB_R2) &&
                MC->GetPlayer((int)joystick+1)->IsAlive())
            EjectButtonPressed();
        break;
    case JB_L1: case JB_R1:
        if (joystickState->GetButtonDown(JB_L1) &&
                joystickState->GetButtonDown(JB_R1))
            Screenshot();
        break;
    default: break;
    }
}
void InputMaster::PauseButtonPressed()
{
    switch (MC->GetGameState()) {
    case GS_INTRO: break;
    case GS_LOBBY: /*MC->SetGameState(GS_PLAY);*/ break;
    case GS_PLAY: MC->SetPaused(!MC->GetPaused()); break;
    case GS_DEAD: MC->SetGameState(GS_LOBBY); break;
    case GS_EDIT: break;
        default: break;
    }
}
void InputMaster::EjectButtonPressed()
{
//    if (MC->GetGameState()==GS_PLAY && !MC->GetPaused()) {
//        MC->Eject();
//    } else if (MC->GetGameState()==GS_LOBBY) MC->Exit();
//    else if (MC->GetGameState()==GS_DEAD) MC->SetGameState(GS_LOBBY);
}

void InputMaster::Screenshot()
{
    Graphics* graphics{GetSubsystem<Graphics>()};
    Image screenshot{context_};
    graphics->TakeScreenShot(screenshot);
    //Here we save in the Data folder with date and time appended
    String fileName{GetSubsystem<FileSystem>()->GetProgramDir() + "Screenshots/Screenshot_" +
            Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_')+".png"};
    Log::Write(1, fileName);
    screenshot.SavePNG(fileName);
}

void InputMaster::HandleJoystickButtonUp(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    int joyID{eventData[JoystickButtonUp::P_JOYSTICKID].GetInt()};		//int
    int button{eventData[JoystickButtonUp::P_BUTTON].GetInt()};		//int
}

void InputMaster::HandleJoystickAxisMove(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    eventData[JoystickAxisMove::P_JOYSTICKID];		//int
    int axis{eventData[JoystickAxisMove::P_AXIS].GetInt()};			//int
    float pos{eventData[JoystickAxisMove::P_POSITION].GetFloat()};		//float
}
