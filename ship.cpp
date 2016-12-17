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
#include "spawnmaster.h"
#include "arena.h"
#include "heart.h"
#include "apple.h"
#include "player.h"
#include "gui3d.h"
#include "pilot.h"
#include "bullet.h"
#include "muzzle.h"
#include "phaser.h"
#include "chaoball.h"
#include "chaomine.h"
#include "chaoflash.h"
#include "seeker.h"

#include "ship.h"

void Ship::RegisterObject(Context *context)
{
    context->RegisterFactory<Ship>();
}

Ship::Ship(Context* context) : Controllable(context),
    initialized_{false},
    initialPosition_{},
    initialRotation_{},
    colorSet_{0},
    initialHealth_{1.0f},
    health_{initialHealth_},
    weaponLevel_{0},
    bulletAmount_{1},
    initialShotInterval_{0.30f},
    shotInterval_{initialShotInterval_},
    sinceLastShot_{0.0f},
    appleCount_{0},
    heartCount_{0}
{
    thrust_ = 2342.0f;
    maxSpeed_ = 23.0f;
}

void Ship::OnNodeSet(Node *node)
{
    Controllable::OnNodeSet(node);

    PODVector<Node*> ships{};
    MC->scene_->GetChildrenWithComponent<Ship>(ships, true);
    colorSet_ = ships.Size();

    Node* guiNode{ MC->scene_->CreateChild("GUI3D") };
    gui3d_ = guiNode->CreateComponent<GUI3D>();
    gui3d_->Initialize(colorSet_);

    model_->Remove();
    model_ = node_->CreateChild("Model")->CreateComponent<AnimatedModel>();
    model_->SetModel(MC->GetModel("KlåMk10"));

    particleEmitter_ = node_->CreateComponent<ParticleEmitter>();
    particleEmitter_->SetEmitting(false);

    shieldNode_ = node_->CreateChild("Shield");
    shieldModel_ = shieldNode_->CreateComponent<StaticModel>();
    shieldModel_->SetModel(MC->GetModel("Shield"));
    shieldMaterial_ = MC->GetMaterial("Shield")->Clone();
    shieldModel_->SetMaterial(shieldMaterial_);

    muzzle_ = GetSubsystem<SpawnMaster>()->Create<Muzzle>(false);
    muzzle_->SetColor(colorSet_);

    //Setup ship physics
    rigidBody_->SetRestitution(0.666f);
    rigidBody_->SetMass(0.0f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetLinearDamping(0.5f);
    rigidBody_->SetAngularFactor(Vector3::ZERO);
    rigidBody_->SetLinearRestThreshold(0.01f);
    rigidBody_->SetAngularRestThreshold(0.1f);
    rigidBody_->SetCollisionLayerAndMask(1, M_MAX_UNSIGNED);

    collisionShape_->SetSphere(2.0f);
    node_->CreateComponent<Navigable>();

    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Ship, BlinkCheck));
}
void Ship::Set(const Vector3 position, const Quaternion rotation)
{
    if (!initialized_) {
        initialPosition_ = position;
        initialRotation_ = rotation;

        SetColors();

        initialized_ = true;
    }

    Controllable::Set(position, rotation);
}

void Ship::SetColors()
{
    model_->SetMaterial(0, MC->colorSets_[colorSet_].glowMaterial_);
    model_->SetMaterial(1, MC->colorSets_[colorSet_].hullMaterial_);

    SharedPtr<ParticleEffect> particleEffect{ CACHE->GetTempResource<ParticleEffect>("Particles/Shine.xml") };
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(MC->colorSets_[colorSet_].colors_.first_ * 0.23f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.4f));
    particleEffect->SetColorFrames(colorFrames);
    particleEmitter_->SetEffect(particleEffect);
}

void Ship::HandleSetControlled()
{
    GetPlayer()->gui3d_ = gui3d_;
    Player::colorSets_[GetPlayer()->GetPlayerId()] = colorSet_;
}

void Ship::EnterPlay(StringHash eventType, VariantMap &eventData)
{
    if (!GetPlayer()) {

        node_->SetEnabledRecursive(false);
        gui3d_->GetNode()->SetEnabledRecursive(false);
        return;
    }

    SetHealth(initialHealth_);
    Pickup(PT_RESET);

    rigidBody_->SetMass(1.0f);
    rigidBody_->ApplyImpulse(node_->GetDirection() * 13.0f);
    particleEmitter_->SetEmitting(true);

    CreateTails();

}
void Ship::EnterLobby(StringHash eventType, VariantMap &eventData)
{
    health_ = initialHealth_;
    weaponLevel_ = 0;
    bulletAmount_ = 1;
    shotInterval_ = initialShotInterval_;
    sinceLastShot_ = 0.0f;

    Set(initialPosition_, initialRotation_);
    model_->GetNode()->SetPosition(Vector3::ZERO);
    rigidBody_->SetMass(0.0f);
    particleEmitter_->SetEmitting(false);

    RemoveTails();
}
void Ship::SetTailsEnabled(bool enabled)
{
    for (TailGenerator* t : tailGens_){
        t->SetEnabled(enabled);
    }
}

void Ship::RemoveTails()
{
    for (TailGenerator* t : tailGens_)
        t->GetNode()->Remove();

    tailGens_.Clear();
}

void Ship::CreateTails()
{
    RemoveTails();

    for (int t{0}; t < 3; ++t) {

        Node* tailNode{node_->CreateChild("Tail")};
        tailNode->SetPosition(Vector3(-0.85f + 0.85f * t, t==1? 0.0f : -0.5f, t==1? -0.5f : -0.23f));
        TailGenerator* tailGen{ tailNode->CreateComponent<TailGenerator>() };
        tailGen->SetDrawHorizontal(true);
        tailGen->SetDrawVertical(false);
        tailGen->SetTailLength(t==1? 0.05f : 0.025f);
        tailGen->SetNumTails(t==1? 13 : 7);
        tailGen->SetWidthScale(t==1? 0.5f : 0.13f);
        tailGen->SetColorForHead(MC->colorSets_[colorSet_].colors_.first_);
        tailGen->SetColorForTip(MC->colorSets_[colorSet_].colors_.first_);
        tailGens_.Push(tailGen);
    }
}

void Ship::Update(float timeStep)
{

    if (MC->GetGameState() != GS_PLAY || !node_->IsEnabled())
        return;

    Controllable::Update(timeStep);

    //Update shield
    Quaternion randomRotation{ Quaternion(0.0f, TIME->GetElapsedTime() * 2000.0f, 0.0f) };
    shieldNode_->SetRotation(shieldNode_->GetRotation().Slerp(randomRotation, Random(1.0f)));
    Color shieldColor{ shieldMaterial_->GetShaderParameter("MatDiffColor").GetColor() };
    Color newColor = Color(shieldColor.r_ * Random(0.5f, 0.8f),
                           shieldColor.g_ * Random(0.6f, 0.9f),
                           shieldColor.b_ * Random(0.7f, 0.8f));
    shieldMaterial_->SetShaderParameter("MatDiffColor", shieldColor.Lerp(newColor, Min(timeStep * 23.5f, 1.0f)));

    //Float
    model_->GetNode()->SetPosition(Vector3::UP * MC->Sine(0.34f, -0.1f, 0.1f));
    //Apply movement
    Vector3 force{ move_ * thrust_ * timeStep };
    if (rigidBody_->GetLinearVelocity().Length() < maxSpeed_
    || (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.0f)
    {
        rigidBody_->ApplyForce(force);
    }

    //Update rotation according to direction of the ship's movement.
    if (rigidBody_->GetLinearVelocity().Length() > 0.1f)
        node_->LookAt(node_->GetPosition() + rigidBody_->GetLinearVelocity());

    //Update tails
    float velocityToScale{ Clamp(0.13f * rigidBody_->GetLinearVelocity().Length(), 0.0f, 1.0f) };

    for (TailGenerator* tailGen : tailGens_) {

        bool centerTail{ tailGen->GetNode()->GetPosition().x_ == 0.0f };

        tailGen->SetTailLength(velocityToScale * (centerTail ? 0.1f : 0.075f));
        tailGen->SetWidthScale(velocityToScale * (centerTail ? 0.666f : 0.23f));
    }

    //Shooting
    sinceLastShot_ += timeStep;
    if (aim_.Length()) {

        if (sinceLastShot_ > shotInterval_)
            Shoot(aim_);
    }
}

void Ship::Shoot(Vector3 aim)
{
    for (int i{0}; i < bulletAmount_; ++i) {

        float angle{ 0.0f };
        switch (i) {
        case 0: angle = (bulletAmount_ == 2 || bulletAmount_ == 3) ? -5.0f
                                                                   :  0.0f;
            break;
        case 1: angle = bulletAmount_ < 4 ? 5.0f
                                          : 7.5f;
            break;
        case 2: angle = bulletAmount_ < 5 ? 180.0f
                                          : 175.0f;
            break;
        case 3: angle = -7.5f;
            break;
        case 4: angle = 185.0f;
            break;
        default: break;
        }
        Vector3 direction{ Quaternion(angle, Vector3::UP) * aim };
        FireBullet(direction);
    }
    sinceLastShot_ = 0.0f;
    //Create a single muzzle flash
    if (bulletAmount_ > 0) {

        MoveMuzzle();
        PlaySample(MC->GetSample("Shot"), 0.17f);
    }
}
void Ship::FireBullet(Vector3 direction)
{
    direction.Normalize();

    Vector3 position{ node_->GetPosition() + direction + Vector3::DOWN * 0.42 };
    Vector3 force{ direction * (1500.0f + 23.0f * weaponLevel_) };
    float damage{ 0.15f + 0.00666f * weaponLevel_ };

    GetSubsystem<SpawnMaster>()->Create<Bullet>()
            ->Set(position, colorSet_, direction, force, damage);
}
void Ship::MoveMuzzle()
{
    muzzle_->Set(node_->GetPosition() + Vector3::DOWN * 0.42f);
}

void Ship::Pickup(PickupType pickup)
{
    if (health_ <= 0.0f && pickup != PT_RESET)
        return;

    switch (pickup) {
    case PT_APPLE: {
        heartCount_ = 0;
        GetPlayer()->AddScore(23 * (1 + (3 * weaponLevel_ == 23)));
        if (weaponLevel_ < 23)
            ++appleCount_;
        if (appleCount_ >= 5){
            UpgradeWeapons();
            appleCount_ = 0;
        } else {
            PlayPickupSample(appleCount_);
        }
    } break;
    case PT_HEART: {
        ++heartCount_;
        appleCount_ = 0;
        if (heartCount_ >= 5){
            ChargeShield();
            heartCount_ = 0;
        }
        else {
            SetHealth(Max(health_, Clamp(health_ + 5.0f, 0.0f, 10.0f)));
            PlayPickupSample(heartCount_);
        }
    } break;
    case PT_CHAOBALL: {
       PickupChaoBall();
    } break;
    case PT_RESET: {
        appleCount_ = 0;
        heartCount_ = 0;
    }
    }

    GetPlayer()->gui3d_->SetHeartsAndApples(heartCount_, appleCount_);

}

void Ship::PlayPickupSample(int pickupCount)
{
    PlaySample(MC->GetSample("Pickup" + String(Clamp(pickupCount, 1, 4))), 0.42f);
}

void Ship::UpgradeWeapons()
{
    ++weaponLevel_;
    bulletAmount_ = 1 + ((weaponLevel_ + 5) / 6);
    shotInterval_ = initialShotInterval_ - 0.0042f * weaponLevel_;
    PlaySample(MC->GetSample("Powerup"), 0.42f);
}
void Ship::ChargeShield()
{
    SetHealth(15.0f);
    PlaySample(MC->GetSample("Powerup"), 0.42f);
}
void Ship::PickupChaoBall()
{
    ChaoFlash* chaoFlash{ GetSubsystem<SpawnMaster>()->Create<ChaoFlash>() };
    chaoFlash->Set(MC->chaoBall_->GetPosition(), colorSet_);
    PlaySample(MC->GetSample("Chaos"), 0.8f);
}

void Ship::SetHealth(float health)
{
    health_ = Clamp(health, 0.0f, 15.0f);
    GetPlayer()->gui3d_->SetHealth(health_);

    if (health_ <= 0.0f){
        Explode();
    }
}

void Ship::Hit(float damage, bool melee)
{
    if (health_ > 10.0f){
        damage *= (melee ? 0.75f : 0.25f);
        shieldMaterial_->SetShaderParameter("MatDiffColor", Color(2.0f, 3.0f, 5.0f, 0.25f + 0.75f * (health_ - damage > 10.0f)));
        PlaySample(MC->GetSample((health_ - damage) > 10.0f ? "ShieldHit"
                                                            : "ShieldDown"), 0.23f);
    }
    else if (!melee)
        PlaySample(MC->GetSample("SeekerHit" + String(Random(4)+1)));
    SetHealth(health_ - damage);
}

void Ship::Explode()
{
    GetPlayer()->Die();

    Disable();
    Explosion* explosion{ GetSubsystem<SpawnMaster>()->Create<Explosion>() };
    explosion->Set(node_->GetPosition(),
                   MC->colorSets_[colorSet_].colors_.first_,
                   2.0f, 0);

    gui3d_->PlayDeathSound();

    Pickup(PT_RESET);

    for (Player* p : MC->GetPlayers())
    {
        if (p->GetShip()->IsEnabled())
            return;
    }
    MC->SetGameState(GS_DEAD);
}

void Ship::Eject()
{
    if (!IsEnabled() || MC->GetGameState() != GS_PLAY)
        return;

    GetSubsystem<SpawnMaster>()->Create<Phaser>()->Set(model_->GetModel(),
                                                       GetPosition(),
                                                       rigidBody_->GetLinearVelocity() + node_->GetDirection() * 10e-5);
    Disable();
}

void Ship::Think()
{
    Vector3 move{ move_ };

    float playerFactor{ 1.0f };
    switch (GetPlayer()->GetPlayerId()){
    case 1: playerFactor = 2.3f; break;
    case 2: playerFactor = 3.4f; break;
    case 3: playerFactor = 6.66f; break;
    case 4: playerFactor = 5.0f; break;
    }

    if (MC->GetSinceStateChange() < playerFactor * 0.1f){
        move = Vector3::ZERO;
        return;
    }

    Vector3 pickupPos{ Vector3::ZERO };
    Vector3 smell{ Sniff(playerFactor, move, false) * playerFactor };
    Vector3 taste{ Sniff(playerFactor, move, true) * playerFactor };

    if (health_ < (5.0f - appleCount_)
     || GetPlayer()->GetFlightScore() == 0
     || ((health_ + 0.5f * appleCount_) < 8.0f)
     || (heartCount_ != 0 && health_ <= 10.0f && weaponLevel_ > 13)
     || (weaponLevel_==23 && health_ <= 10.0f)
     ||  heartCount_ == 4)
    {
        pickupPos = MC->heart_->GetPosition();
    } else {
        pickupPos = MC->apple_->GetPosition();
    }
    //Calculate shortest route
    Vector3 newPickupPos{pickupPos};
    for (int i{0}; i < 6; ++i){
        Vector3 projectedPickupPos{pickupPos + (Quaternion(i * 60.0f, Vector3::UP) * Vector3::FORWARD * 46.0f)};
        if (LucKey::Distance(GetPosition(), projectedPickupPos - rigidBody_->GetLinearVelocity() * 0.42f) < LucKey::Distance(GetPosition(), pickupPos))
            newPickupPos = projectedPickupPos;
    }
    pickupPos = newPickupPos;
    //Calculate move vector
    if (pickupPos.y_ < -10.0f || LucKey::Distance(
                GetPosition(), LucKey::Scale(pickupPos, Vector3(1.0f, 0.0f, 1.0f))) < playerFactor)
        pickupPos = GetPosition() + node_->GetDirection() * playerFactor;

    move = 0.5f * (move +
                    LucKey::Scale(pickupPos - node_->GetPosition()
                                  - 0.05f * playerFactor * rigidBody_->GetLinearVelocity()
                                  - 0.1f * playerFactor * node_->GetDirection()
                                  , Vector3(1.0f, 0.0f, 1.0f)).Normalized());

    if (LucKey::Distance(pickupPos, GetPosition()) > playerFactor)
        move += smell * 5.0f;
    move += Vector3(
                 MC->Sine(playerFactor, -0.05f, 0.05f, playerFactor),
                 0.0f,
                 MC->Sine(playerFactor, -0.05f, 0.05f, -playerFactor));

    SetMove(move);
    //Pick firing target
    bool fire{false};
    Pair<float, Vector3> target{};
    for (Razor* r : MC->GetComponentsInScene<Razor>()){
        if (r->IsEnabled() && r->GetPosition().y_ > (-playerFactor * 0.1f)){
            float distance{ LucKey::Distance(this->GetPosition(), r->GetPosition()) };
            float panic{ r->GetPanic() };
            float weight{ (5.0f * panic) - (distance / playerFactor) + 42.0f };
            if (weight > target.first_){
                target.first_ = weight;
                target.second_ = r->GetPosition() + r->GetLinearVelocity() * 0.42f;
                fire = true;
            }
        }
    }
    for (Spire* s : MC->GetComponentsInScene<Spire>()){
        if (s->IsEnabled() && s->GetPosition().y_ > (-playerFactor * 0.23f) && GetPlayer()->GetFlightScore() != 0){
            float distance{ LucKey::Distance(this->GetPosition(), s->GetPosition()) };
            float panic{ s->GetPanic() };
            float weight{ (23.0f * panic) - (distance / playerFactor) + 32.0f };
            if (weight > target.first_){
                target.first_ = weight;
                target.second_ = s->GetPosition();
                fire = true;
            }
        }
    }
    if (fire){
        SetAim((target.second_ - GetPosition()).Normalized());
        float aimFactor{ 23.0f / playerFactor };
        if (bulletAmount_ == 2 || bulletAmount_ == 3)
            SetAim((Quaternion((GetPlayer()->GetPlayerId() == 2 ? -1.0f : 1.0f) * (Min(0.666f * LucKey::Distance(this->GetPosition(), target.second_), 5.0f) + MC->Sine(aimFactor * aimFactor, -aimFactor, aimFactor)), Vector3::UP) * aim_).Normalized());
        else SetAim((Quaternion((GetPlayer()->GetPlayerId() == 2 ? -1.0f : 1.0f) * MC->Sine(aimFactor * aimFactor, -aimFactor, aimFactor), Vector3::UP) * aim_).Normalized());
    }
    else SetAim(Vector3::ZERO);
//    SetAim((aim_ - taste).Normalized());
}

Vector3 Ship::Sniff(float playerFactor, Vector3& move, bool taste)
{
    Vector3 smell;
    int whiskers{23};
    int detected{0};

    //Smell across borders
    for (int p{-1}; p < (taste ? 0 : 6); ++p){
        Vector3 projectedPlayerPos{( (p != -1) ? GetPosition() + (Quaternion(p * 60.0f, Vector3::UP) * Vector3::FORWARD * 46.0f)
                                               : GetPosition() )};
        for (int w = 0; w < whiskers; ++w){
            PODVector<PhysicsRaycastResult> hitResults{};
            Vector3 whiskerDirection{ Quaternion((360.0f / whiskers) * w, Vector3::UP)
                        * (2.0f * node_->GetDirection() + 3.0f * move_.Normalized())};
            Ray whiskerRay{projectedPlayerPos + Vector3::DOWN * Random(0.666f), whiskerDirection};
            if (MC->PhysicsRayCast(hitResults, whiskerRay, playerFactor + playerFactor * (w == 0), M_MAX_UNSIGNED)){
                ++detected;
                PhysicsRaycastResult r{hitResults[0]};
                Node* node{ r.body_->GetNode() };
                float distSquared{(r.distance_ * r.distance_) *
                            (0.005f * whiskerDirection.Angle(move_) +
                             playerFactor * playerFactor)};

                if (node->HasComponent<Apple>()) {
                    smell += 230.0f * (whiskerDirection / (distSquared)) * (appleCount_ - static_cast<float>(GetPlayer()->GetFlightScore() == 0));
                } else if (node->HasComponent<Heart>()) {
                    smell += 235.0f * (whiskerDirection / (distSquared)) * (heartCount_ * 2.0f - appleCount_ * 10.0f + (10.0f - health_));
                } else if ((node->HasComponent<ChaoBall>()) && !taste) {
                    if (r.body_->GetNode()->GetComponent<RigidBody>()->GetLinearVelocity().Length() < 5.0f)
                        smell += 666.0f * whiskerDirection / (distSquared * distSquared);
                } else if ((node->HasComponent<ChaoMine>()) && !taste) {
                    if (r.body_->GetNode()->GetComponent<RigidBody>()->GetLinearVelocity().Length() < 5.0f)
                        smell += 9000.0f * whiskerDirection / (distSquared * distSquared * Random(5.0f));
                } else if (node->HasComponent<Razor>()) {
                    smell -= ((w == 0) * 2300.0f + 320.0f) * (whiskerDirection / (distSquared));
                } else if (node->HasComponent<Spire>()) {
                    smell -= ((w == 0) * 4200.0f + 3200.0f) * (whiskerDirection / (distSquared * distSquared));
                } else if (node->HasComponent<Seeker>() && !taste) {
                    smell -= 1000.0f * (whiskerDirection / r.distance_) * (3.0f - 2.0f * static_cast<float>(health_ > 10.0f));
                    ++detected;
                }
                if (!taste){
                    if (!node->HasComponent<Apple>() && !node->HasComponent<Heart>()
                            && node->HasComponent<ChaoBall>() && node->HasComponent<ChaoMine>()
                            && node->GetNameHash() != StringHash("PickupTrigger"))
                         smell += 0.005f * whiskerDirection * r.distance_;
                    else smell += 0.005f * whiskerDirection * playerFactor * playerFactor;
                }
            }
            else if (!taste) smell += 0.005f * whiskerDirection * playerFactor * playerFactor;
        }
    }
    //Rely more on scent in crowded spaces
    if (!taste){
        float scentEffect{0.23f * static_cast<float>(detected) / whiskers};
        move = move * (1.0f - scentEffect);
        move = move - 0.5f * scentEffect * rigidBody_->GetLinearVelocity();
        smell *= 1.0f + (0.5f * scentEffect);
    }
    return smell / whiskers;
}
