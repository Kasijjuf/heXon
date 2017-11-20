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

#include "settings.h"

#include "mastercontrol.h"

Settings::Settings(Context* context) : Object(context),
    antiAliasing_{true},
    manyLights_{true}
{ 
}

bool Settings::Load()
{
    if (FILES->FileExists(MC->GetResourceFolder() + "/Settings.xml")) {

        File file(context_, MC->GetResourceFolder() + "/Settings.xml", FILE_READ);
        XMLFile configFile(context_);
        configFile.Load(file);
        XMLElement graphics{ configFile.GetRoot().GetChild("Graphics") };
        XMLElement audio{ configFile.GetRoot().GetChild("Audio") };

        if (graphics) {

            width_ = graphics.GetInt("Width");
            height_ = graphics.GetInt("Height");
            fullscreen_ = graphics.GetBool("Fullscreen");

            antiAliasing_ = graphics.GetBool("AntiAliasing");
            manyLights_ = graphics.GetBool("ManyLights");

            if (ENGINE->IsInitialized()) {

                GRAPHICS->SetMode(width_,
                                  height_);

                if (GRAPHICS->GetFullscreen() != fullscreen_)
                    GRAPHICS->ToggleFullscreen();
            }
        }
        if (audio) {

            AUDIO->SetMasterGain(SOUND_MUSIC, audio.GetFloat("MusicGain"));
        }
        return true;

    } else {

        return false;
    }
}
void Settings::Save()
{
    XMLFile file(context_);
    XMLElement root(file.CreateRoot("Settings"));

    XMLElement graphicsElement(root.CreateChild("Graphics"));
    graphicsElement.SetInt("Width", GRAPHICS->GetWidth());
    graphicsElement.SetInt("Height", GRAPHICS->GetHeight());
    graphicsElement.SetBool("Fullscreen", GRAPHICS->GetFullscreen());
    graphicsElement.SetBool("AntiAliasing", antiAliasing_);
    graphicsElement.SetBool("ManyLights", manyLights_);

    XMLElement audioElement(root.CreateChild("Audio"));
    audioElement.SetFloat("MusicGain", AUDIO->GetMasterGain(SOUND_MUSIC));

    file.SaveFile(MC->GetResourceFolder() + "/Settings.xml");
}
