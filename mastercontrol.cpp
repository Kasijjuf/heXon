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

#include "mastercontrol.h"

#include <fstream>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Core/Context.h>
#include "TailGenerator.h"

#include "hexocam.h"
#include "effectmaster.h"
#include "inputmaster.h"
#include "spawnmaster.h"
#include "player.h"
#include "apple.h"
#include "heart.h"
#include "chaoball.h"
#include "chaoflash.h"
#include "chaomine.h"
#include "chaozap.h"
#include "bullet.h"
#include "seeker.h"
#include "flash.h"
#include "hitfx.h"
#include "explosion.h"
#include "bubble.h"
#include "line.h"
#include "muzzle.h"
#include "pilot.h"
#include "ship.h"
#include "splatterpillar.h"
#include "door.h"
#include "arena.h"
#include "tile.h"
#include "lobby.h"
#include "door.h"
#include "highest.h"
#include "phaser.h"
#include "gui3d.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl* MasterControl::instance_ = NULL;

MasterControl* MasterControl::GetInstance()
{
    return MasterControl::instance_;
}

MasterControl::MasterControl(Context *context):
    Application(context),
    aspectRatio_{},
    paused_{false},
    currentState_{GS_INTRO},
    sinceStateChange_{0.0f},
    sinceFrameRateReport_{0.0f}
{
    instance_ = this;
}

void MasterControl::Setup()
{
    SetRandomSeed(TIME->GetSystemTime());

    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"heXon.log";
    engineParameters_["WindowTitle"] = "heXon";
    engineParameters_["WindowIcon"] = "icon.png";

    //Add resource path
//    Vector<String> resourcePaths{};
//    resourcePaths.Push(FILES->GetAppPreferencesDir("luckey", "hexon"));
    engineParameters_["ResourcePaths"] = "Resources;";
    /*resourcePaths.Push(String("Resources"));
    resourcePaths.Push(String("../heXon/Resources"));

    for (String path : resourcePaths)
        if (FILES->DirExists(path)){
            engineParameters_["ResourcePaths"] = path;
            break;
        }*/
//    engineParameters_["VSync"] = true;
//    engineParameters_["FullScreen"] = false;
//    engineParameters_["Headless"] = true;
//    engineParameters_["WindowWidth"] = 1920/2;
//    engineParameters_["WindowHeight"] = 1080/2;
//    engineParameters_["borderless"] = true;
}
void MasterControl::Start()
{
    ENGINE->SetMaxFps(100);
    aspectRatio_ = static_cast<float>(GRAPHICS->GetWidth()) / GRAPHICS->GetHeight();

    TailGenerator::RegisterObject(context_);

    heXoCam::RegisterObject(context_);
    Lobby::RegisterObject(context_);
    Door::RegisterObject(context_);
    SplatterPillar::RegisterObject(context_);
    Arena::RegisterObject(context_);
    Apple::RegisterObject(context_);
    Heart::RegisterObject(context_);
    ChaoBall::RegisterObject(context_);
    Tile::RegisterObject(context_);
    Highest::RegisterObject(context_);
    Ship::RegisterObject(context_);
    Pilot::RegisterObject(context_);
    Bullet::RegisterObject(context_);
    Muzzle::RegisterObject(context_);
    Phaser::RegisterObject(context_);
    GUI3D::RegisterObject(context_);

    ChaoFlash::RegisterObject(context_);
    ChaoMine::RegisterObject(context_);
    ChaoZap::RegisterObject(context_);

    Razor::RegisterObject(context_);
    Spire::RegisterObject(context_);
    Seeker::RegisterObject(context_);

    HitFX::RegisterObject(context_);
    Bubble::RegisterObject(context_);
    Flash::RegisterObject(context_);
    Explosion::RegisterObject(context_);
    Line::RegisterObject(context_);

    context_->RegisterSubsystem(new EffectMaster(context_));
    context_->RegisterSubsystem(new InputMaster(context_));
    context_->RegisterSubsystem(new SpawnMaster(context_));

    CreateColorSets();
    CreateScene();
    CreateUI();

    Node* announcerNode{ scene_->CreateChild("Announcer") };
    announcerNode->CreateComponent<SoundSource>()->Play(GetSample("Welcome"));

    menuMusic_ = GetMusic("Modanung - BulletProof MagiRex");
    menuMusic_->SetLooped(true);
    gameMusic_ = GetMusic("Alien Chaos - Disorder");
    gameMusic_->SetLooped(true);
    Node* musicNode{ scene_->CreateChild("Music") };
    musicSource_ = musicNode->CreateComponent<SoundSource>();
    musicSource_->SetGain(0.32f);
    musicSource_->SetSoundType(SOUND_MUSIC);

//    GetSubsystem<Audio>()->Stop(); ///////////////////////////////////////////////////////////////////////

    SetGameState(GS_LOBBY);

    SubscribeToEvents();

    // Precache shaders if possible
//    if (!ENGINE->IsHeadless() && CACHE->Exists("heXonShaders.xml"))
//        GRAPHICS->PrecacheShaders(CACHE->GetFile("heXonShaders.xml").);
}
void MasterControl::Stop()
{
    engine_->DumpResources(true);
}

void MasterControl::SubscribeToEvents()
{
    //Subscribe scene update event.
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(MasterControl, HandleSceneUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MasterControl, HandlePostRenderUpdate));
}

void MasterControl::CreateUI()
{
    //Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will control the camera
    world.cursor.uiCursor = new Cursor(context_);
    world.cursor.uiCursor->SetVisible(true);
    GetSubsystem<UI>()->SetCursor(world.cursor.uiCursor);

    //Set starting position of the cursor at the rendering window center
    world.cursor.uiCursor->SetPosition(GRAPHICS->GetWidth()/2, GRAPHICS->GetHeight()/2);
}

Sound* MasterControl::GetMusic(String name) const {
    Sound* song{ CACHE->GetResource<Sound>("Music/"+name+".ogg") };
    song->SetLooped(true);
    return song;
}
Sound* MasterControl::GetSample(String name) const {
    Sound* sample{ CACHE->GetResource<Sound>("Samples/"+name+".ogg") };
//    sample->SetLooped(false);
    return sample;
}

void MasterControl::CreateColorSets()
{
    for (int c : { 0, 1, 2, 3, 4 }) {

        ColorSet set{};

        switch (c){
        case 0: set.colors_.first_ = Color(0.23f, 0.5f, 1.0f);
                set.colors_.second_ = Color(0.05f, 0.05f, 0.05f);
            break;
        case 1: set.colors_.first_ = Color(0.42f, 0.5f, 0.0f);
                set.colors_.second_ = Color(0.1f, 0.3f, 0.05f);
            break;
        case 2: set.colors_.first_ = Color(0.55f, 0.32f, 0.0f);
                set.colors_.second_ = Color(0.16f, 0.0f, 0.38f);
            break;
        case 3: set.colors_.first_ = Color(0.45f, 0.1f, 0.42f);
                set.colors_.second_ = Color(0.0f, 0.27f, 0.42f);
            break;
        case 4: set.colors_.first_ = Color(0.42f, 0.07f, 0.0f);
                set.colors_.second_ = Color(0.34f, 0.34f, 0.34f);
            break;
        }

        set.glowMaterial_ = GetMaterial("Glow")->Clone();
        set.hullMaterial_ = GetMaterial("Hull")->Clone();
        set.bulletMaterial_ = GetMaterial("Bullet")->Clone();

        set.glowMaterial_->SetShaderParameter("MatEmissiveColor", set.colors_.first_);
        set.glowMaterial_->SetShaderParameter("MatDiffColor", set.colors_.first_ * 0.23f);
        set.glowMaterial_->SetShaderParameter("MatSpecColor", (set.colors_.first_ + Color::WHITE) * 0.23f);

        set.hullMaterial_->SetShaderParameter("MatDiffColor", set.colors_.second_);
        set.hullMaterial_->SetShaderParameter("MatSpecColor", set.colors_.second_);

        set.bulletMaterial_->SetShaderParameter("MatDiffColor", set.colors_.first_ * 1.23f);

        SharedPtr<Material> flash{ GetMaterial("Flash")->Clone() };
        flash->SetShaderParameter("MatDiffColor", set.colors_.first_ * 1.23f);
        set.hitFx_ = CACHE->GetResource<ParticleEffect>("Particles/HitFX.xml")->Clone();
        set.hitFx_->SetMaterial(flash);

        colorSets_[c] = set;
    }
}

void MasterControl::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->SetTimeScale(1.23f);

    world.octree = scene_->CreateComponent<Octree>();
    physicsWorld_ = scene_->CreateComponent<PhysicsWorld>();
    physicsWorld_->SetGravity(Vector3::ZERO);
    scene_->CreateComponent<DebugRenderer>();

    //Create a Zone component for fog control
    Node* zoneNode{ scene_->CreateChild("Zone") };
    Zone* zone{ zoneNode->CreateComponent<Zone>() };
    zone->SetBoundingBox(BoundingBox( Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 5.0f, 100.0f) ));
    zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
    zone->SetFogStart(60.0f);
    zone->SetFogEnd(62.3f);
    zone->SetHeightFog(true);
    zone->SetFogHeight(-10.0f);
    zone->SetFogHeightScale(0.23f);

    //Create cursor
    world.cursor.sceneCursor = scene_->CreateChild("Cursor");
    StaticModel* cursorObject{ world.cursor.sceneCursor->CreateComponent<StaticModel>() };
    cursorObject->SetModel(GetModel("Hexagon"));
    cursorObject->SetMaterial(GetMaterial("Glow"));
    world.cursor.sceneCursor->SetEnabled(false);

    //Create an solid black plane to hide everything beyond full fog
    world.voidNode = scene_->CreateChild("Void");
    world.voidNode->SetPosition(Vector3::DOWN * 10.0f);
    world.voidNode->SetScale(Vector3(1000.0f, 1.0f, 1000.0f));
    StaticModel* planeObject{ world.voidNode->CreateComponent<StaticModel>() };
    planeObject->SetModel(GetModel("Plane"));
    planeObject->SetMaterial(GetMaterial("PitchBlack"));

    //Create camera
    Node* cameraNode{ scene_->CreateChild("Camera") };
    world.camera = cameraNode->CreateComponent<heXoCam>();

    //Create arena
    Node* arenaNode{ scene_->CreateChild("Arena") };
    arena_ = arenaNode->CreateComponent<Arena>();

    //Construct lobby
    Node* lobbyNode{ scene_->CreateChild("Lobby") };
    lobby_ = lobbyNode->CreateComponent<Lobby>();
    //Create ships
//    for (float x : { -4.2f, 4.2f }) {
//        GetSubsystem<SpawnMaster>()->Create<Ship>()
//                ->Set(Vector3(x, 0.6f, 0.0f), Quaternion(x < 0 ? 90.0f : -90.0f, Vector3::UP));
//    }
    for (int s : { 3, 4, 2, 1 }) {
        GetSubsystem<SpawnMaster>()->Create<Ship>()
                ->Set(Quaternion(60.0f + ((s % 2) * 60.0f - (s / 2) * 180.0f), Vector3::UP) * Vector3(0.0f, 0.6f, 2.3f),
                      Quaternion(60.0f + ((s % 2) * 60.0f - (s / 2) * 180.0f), Vector3::UP));
    }

    Node* appleNode{ scene_->CreateChild("Apple") };
    apple_ = appleNode->CreateComponent<Apple>();
    Node* heartNode{ scene_->CreateChild("Heart") };
    heart_ = heartNode->CreateComponent<Heart>();
    Node* chaoBallNode{ scene_->CreateChild("ChaoBall") };
    chaoBall_ = chaoBallNode->CreateComponent<ChaoBall>();

    NavigationMesh* navMesh{ scene_->CreateComponent<NavigationMesh>() };
    navMesh->SetAgentRadius(0.42f);
    navMesh->SetPadding(Vector3::UP);
    navMesh->SetAgentMaxClimb(0.05f);
    navMesh->SetCellSize(0.05f);
    navMesh->SetTileSize(256);
    navMesh->Build();

    for (int p{1}; p <= Max(static_cast<int>(INPUT->GetNumJoysticks()), 2); ++p){

        players_.Push(SharedPtr<Player>(new Player(p, context_)));

        Node* pilotNode{ scene_->CreateChild("Pilot") };
        pilotNode->Rotate(Quaternion(180.0f, Vector3::UP));
        Pilot* pilot{ pilotNode->CreateComponent<Pilot>() };
        GetSubsystem<InputMaster>()->SetPlayerControl(p, pilot);
        pilot->Initialize(false);
    }
}

void MasterControl::SetGameState(const GameState newState)
{
    if (newState != currentState_) {

        LeaveGameState();
        previousState_ = currentState_;
        currentState_ = newState;
        sinceStateChange_ = 0.0f;
        EnterGameState();
    }
}
void MasterControl::LeaveGameState()
{
    switch (currentState_) {
    case GS_INTRO : break;
    case GS_LOBBY : {
//        lobby_->SetEnabledRecursive(false);
//        if (highestScore_ != 0)
//            highestScoreText_->SetColor(Color(0.23f, 0.75f, 1.0f, 0.23f));
    } break;
    case GS_PLAY : {
        GetSubsystem<SpawnMaster>()->Deactivate();
    } break; //Eject when alive
    case GS_DEAD : {
        world.camera->SetGreyScale(false);
    } break;
    case GS_EDIT : break; //Disable EditMaster
    default: break;
    }
}
void MasterControl::EnterGameState()
{
    switch (currentState_) {
    case GS_INTRO : break;
    case GS_LOBBY : {
        SendEvent(E_ENTERLOBBY);

        GetSubsystem<SpawnMaster>()->Clear();

        musicSource_->Play(menuMusic_);

        apple_->Disable();
        heart_->Disable();
        chaoBall_->Disable();
    } break;
    case GS_PLAY : {
        SendEvent(E_ENTERPLAY);

        musicSource_->Play(gameMusic_);

        apple_->Respawn(true);
        heart_->Respawn(true);
        chaoBall_->Deactivate();

        world.lastReset = scene_->GetElapsedTime();
        GetSubsystem<SpawnMaster>()->Restart();
    } break;
    case GS_DEAD : {
        GetSubsystem<SpawnMaster>()->Deactivate();
        world.camera->SetGreyScale(true);
        musicSource_->SetGain(musicSource_->GetGain() * 0.666f);
    } break;
    default: break;
    }
}

void MasterControl::Eject()
{
    SetGameState(GS_LOBBY);
}

bool MasterControl::AllReady(bool onlyHuman)
{
    for (Controllable* c : GetSubsystem<InputMaster>()->GetControlled()) {

        if (c){
            if (c->IsInstanceOf<Pilot>()){
                if ( onlyHuman && c->GetPlayer()->IsHuman() )
                    return false;
                else if (!onlyHuman)
                    return false;
            }
        }

    }
    return true;
}

void MasterControl::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    float t{eventData[Update::P_TIMESTEP].GetFloat()};

/*
    //Output FPS
    secondsPerFrame_ *= 1.0f - t;
    secondsPerFrame_ += t * t;
    sinceFrameRateReport_ += t;
    if (sinceFrameRateReport_ >= 1.0f) {
        Log::Write(1, String(1.0f / secondsPerFrame_));
        sinceFrameRateReport_ = 0.0f;
    }
*/
    sinceStateChange_ += t;
    UpdateCursor(t);

    switch (currentState_) {
    case GS_LOBBY: {

        for (Door* d : GetComponentsInScene<Door>()){
            if (d->HidesAllPilots(false))
                Exit();
        }

        if (AllReady(false))
            SetGameState(GS_PLAY);

    } break;
    case GS_PLAY: {
//        lobby_->SetPosition((Vector3::DOWN * 23.0f) * Min(1.0f, sinceStateChange_));
    } break;
    case GS_DEAD: {
        if (sinceStateChange_ > 5.0f && NoHumans())
            SetGameState(GS_LOBBY);
    }
    default: break;
    }
}

void MasterControl::UpdateCursor(const float timeStep)
{ (void)timeStep;/*
    world.cursor.sceneCursor->Rotate(Quaternion(0.0f,100.0f*timeStep,0.0f));
    //world.cursor.sceneCursor->SetScale((world.cursor.sceneCursor->GetWorldPosition() - world.camera->GetWorldPosition()).Length()*0.05f);
    if (CursorRayCast(250.0f, world.cursor.hitResults))
        for (RayQueryResult r : world.cursor.hitResults)
            if (r.node_->GetNameHash() == N_TILE)
                world.cursor.sceneCursor->SetWorldPosition(r.node_->GetPosition()+Vector3::UP);
                Vector3 camHitDifference = world.camera->rootNode_->GetWorldPosition() - world.cursor.hitResults[i].position_;
                camHitDifference /= world.camera->rootNode_->GetWorldPosition().y_ - world.voidNode->GetPosition().y_;
                camHitDifference *= world.camera->rootNode_->GetWorldPosition().y_;
                world.cursor.sceneCursor->SetWorldPosition(world.camera->rootNode_->GetWorldPosition()-camHitDifference);*/
}

bool MasterControl::CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults)
{
    IntVector2 mousePos{world.cursor.uiCursor->GetPosition()};
    Ray cameraRay{ world.camera->camera_->GetScreenRay((float)mousePos.x_/GRAPHICS->GetWidth(),
                                                       (float)mousePos.y_/GRAPHICS->GetHeight())
                 };
    RayOctreeQuery query{hitResults, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY};
    scene_->GetComponent<Octree>()->Raycast(query);

    if (hitResults.Size())
        return true;
    else
        return false;
}

bool MasterControl::PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, const Ray ray,
                                   const float distance, const unsigned collisionMask)
{
    if (distance > 1.0e-9)
        physicsWorld_->Raycast(hitResults, ray, distance, collisionMask);

    return (hitResults.Size() > 0);
}

bool MasterControl::PhysicsSphereCast(PODVector<RigidBody*> &hitResults, const Vector3 center,
                                      const float radius, const unsigned collisionMask)
{
    physicsWorld_->GetRigidBodies(hitResults, Sphere(center, radius), collisionMask);
    if (hitResults.Size()) return true;
    else return false;
}

void MasterControl::Exit()
{
    //Save pilots and their scores
//    player1_->SavePilot();
//    player2_->SavePilot();

    //...and exit to the left
    engine_->Exit();
}

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase{SinePhase(freq, shift)};
    float add{0.5f * (min + max)};
    return LucKey::Sine(phase) * 0.5f * (max - min) + add;
}
float MasterControl::Cosine(const float freq, const float min, const float max, const float shift)
{
    return Sine(freq, min, max, shift + 0.25f);
}
float MasterControl::SinePhase(float freq, float shift)
{
    return M_PI * 2.0f * (freq * scene_->GetElapsedTime() + shift);
}


Player* MasterControl::GetPlayer(int playerID) const
{
    for (Player* p : players_) {

        if (p->GetPlayerId() == playerID)
            return p;
    }
    return nullptr;
}
Player* MasterControl::GetPlayerByColorSet(int colorSet)
{
    for (Ship* s : GetComponentsInScene<Ship>()) {

        if (s->GetColorSet() == colorSet)
            return s->GetPlayer();
    }
    return nullptr;
}
Player* MasterControl::GetNearestPlayer(Vector3 pos)
{
    Player* nearest{};
    for (Player* p : players_){
        if (p->IsAlive()){

            if (!nearest
            || (Distance(GetSubsystem<InputMaster>()->GetControllableByPlayer(p->GetPlayerId())->GetPosition(), pos) <
                Distance(GetSubsystem<InputMaster>()->GetControllableByPlayer(nearest->GetPlayerId())->GetPosition(), pos)))
            {
                nearest = p;
            }
        }
    }
    return nearest;
}
bool MasterControl::AllPlayersAtZero(bool onlyHuman)
{
    for (Player* p : players_)
        if ((p->IsHuman() || !onlyHuman) && p->GetScore() != 0)
            return false;

    return true;
}

Ship* MasterControl::GetShipByColorSet(int colorSet_)
{
    for (Ship* s : GetComponentsInScene<Ship>())
        if (s->GetColorSet() == colorSet_)
            return s;

    return nullptr;
}

Door *MasterControl::GetDoor()
{
    for (Door* d : GetComponentsInScene<Door>())
        return d;

    return nullptr;
}

bool MasterControl::NoHumans()
{
    for (Player* p : players_)
        if (p->IsHuman())
            return false;
    return true;
}

void MasterControl::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
//    physicsWorld_->DrawDebugGeometry(true);
//    scene_->GetComponent<NavigationMesh>()->DrawDebugGeometry(false);
}

