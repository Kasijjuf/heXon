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

#ifndef MASTERCONTROL_H
#define MASTERCONTROL_H

#include <Urho3D/Urho3D.h>

#include "luckey.h"
#include "player.h"

namespace Urho3D {

URHO3D_EVENT(E_ENTERPLAY, EnterPlay){}
URHO3D_EVENT(E_ENTERLOBBY, EnterLobby){}

class Node;
class Scene;
}

class heXoCam;
class InputMaster;
class Arena;
class SpawnMaster;
class Razor;
//class Player;
class Door;
class SplatterPillar;
class Apple;
class ChaoBall;
class Heart;
class Lobby;

typedef struct GameWorld
{
    SharedPtr<heXoCam> camera;
    float lastReset;
    SharedPtr<Octree> octree;
    SharedPtr<Node> backdropNode;
    SharedPtr<Node> voidNode;
    struct {
        SharedPtr<Node> sceneCursor;
        SharedPtr<Cursor> uiCursor;
        PODVector<RayQueryResult> hitResults;
    } cursor;
} GameWorld;

typedef struct ColorSet
{
    Pair<Color, Color> colors_;
    SharedPtr<Material> glowMaterial_;
    SharedPtr<Material> hullMaterial_;
    SharedPtr<Material> bulletMaterial_;
    SharedPtr<Material> panelMaterial_;
    SharedPtr<Material> addMaterial_;
    SharedPtr<ParticleEffect> hitFx_;
} ColorSet;

typedef struct HitInfo
{
    Vector3 position_;
    Vector3 hitNormal_;
    Node* hitNode_;
    Drawable* drawable_;
} HitInfo;

enum GameState {GS_INTRO, GS_LOBBY, GS_PLAY, GS_DEAD, GS_EDIT};
enum PickupType {PT_RESET, PT_APPLE, PT_HEART, PT_CHAOBALL};

#define MC MasterControl::GetInstance()

#define NAVMESH MC->scene_->GetComponent<NavigationMesh>()

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
public:
    MasterControl(Context* context);
    static MasterControl* GetInstance();
    String GetResourceFolder() const { return resourceFolder_; }

    GameWorld world;
    Scene* scene_;
    PhysicsWorld* physicsWorld_;
    SoundSource* musicSource_;
    SharedPtr<XMLFile> defaultStyle_;
    Lobby* lobby_;
    Arena* arena_;

    Vector< SharedPtr<Player> > players_;
    HashMap< int, ColorSet > colorSets_;

    HashMap< unsigned, SharedPtr<Sound> > samples_;

    Apple* apple_;
    Heart* heart_;
    ChaoBall* chaoBall_;

    // Setup before engine initialization. Modifies the engine paramaters.
    void Setup() override;
    // Setup after engine initialization.
    void Start() override;
    // Cleanup after the main loop. Called by Application.
    void Stop() override;
    void Exit();

    Material* GetMaterial(String name) const { return CACHE->GetResource<Material>("Materials/" + name + ".xml"); }
    Model* GetModel(String name) const { return CACHE->GetResource<Model>("Models/" + name + ".mdl"); }
    Texture* GetTexture(String name) const { return CACHE->GetResource<Texture>("Textures/" + name + ".png"); }
    Sound* GetMusic(String name) const;
    Sound* GetSample(String name);

    void AddPlayer();
    void RemoveAutoPilot();
    Player* GetPlayer(int playerId) const;
    Player* GetPlayerByColorSet(int colorSet);
    Player* GetNearestPlayer(Vector3 pos);
    Vector< SharedPtr<Player> > GetPlayers();
    void RemovePlayer(Player *player);
    bool AllReady(bool onlyHuman);
    bool AllPlayersAtZero(bool onlyHuman);

    float SinceLastReset() const { return scene_->GetElapsedTime() - world.lastReset; }
    void SetGameState(GameState newState);
    GameState GetGameState(){ return currentState_; }
    GameState GetPreviousGameState(){ return previousState_; }
    float GetAspectRatio() const noexcept { return aspectRatio_; }
    bool IsPaused() { return paused_; }
    void SetPaused(bool paused);
    void Pause() { SetPaused(true);}
    void Unpause() { SetPaused(false); }
    float GetSinceStateChange() const noexcept { return sinceStateChange_; }
    bool GetAntiAliasing() const noexcept { return antiAliasing_; }

    bool PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, Urho3D::Ray ray, const float distance, const unsigned collisionMask = M_MAX_UNSIGNED);
    bool PhysicsSphereCast(PODVector<RigidBody*> &hitResults, Vector3 center, const float radius, const unsigned collisionMask = M_MAX_UNSIGNED);

    void Eject();
    bool NoHumans();
    void LoadHighest();

    float Sine(const float freq, const float min, const float max, const float shift = 0.0f);
    float Cosine(const float freq, const float min, const float max, const float shift = 0.0f);

    template <class T> Vector<T*> GetComponentsInScene(bool onlyEnabled = false)
    {
        PODVector<Node*> matchingNodes;

        scene_->GetChildrenWithComponent<T>(matchingNodes, true);

        if (onlyEnabled) {
            for (Node* n : matchingNodes)
                if (!n->IsEnabled())
                    matchingNodes.Remove(n);
        }

        Vector<T*> matchingComponents{};
        for (Node* n : matchingNodes)
            matchingComponents.Push(n->GetComponent<T>());

        return matchingComponents;
    }
    Ship* GetShipByColorSet(int colorSet_);
    Door* GetDoor();


    void HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData);
private:
    static MasterControl* instance_;
    String resourceFolder_;

    Vector<double> sine_;
    bool antiAliasing_;

    float aspectRatio_;
    bool paused_;
    GameState previousState_;
    GameState currentState_;
    float sinceStateChange_;

    SharedPtr<Sound> menuMusic_;
    SharedPtr<Sound> gameMusic_;

    Light* lobbySpotLight_;

    Material* lobbyGlowGreen_;
    Material* lobbyGlowPurple_;

    void CreateColorSets();
    void CreateScene();
    void CreateUI();
    void SubscribeToEvents();

    void HandleUpdate(StringHash eventType, VariantMap &eventData);

    void UpdateCursor(const float timeStep);
    bool CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults);

    void LeaveGameState();
    void EnterGameState();

    float secondsPerFrame_;
    float sinceFrameRateReport_;
    float SinePhase(float freq, float shift);
    void LoadSettings();
    void SaveSettings();
};

#endif // MASTERCONTROL_H
