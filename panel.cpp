#include "effectmaster.h"
#include "razor.h"
#include "pilot.h"
#include "player.h"

#include "panel.h"

void Panel::RegisterObject(Context* context)
{
    context->RegisterFactory<Panel>();
}

Panel::Panel(Context* context) : LogicComponent(context)
{
}

void Panel::OnNodeSet(Node* node)
{ (void)node;
}

void Panel::Initialize(int colorSet)
{
    colorSet_ = colorSet;

    CreatePanels();

    FadeOutPanel(true);

    SubscribeToEvent(E_ENTERLOBBY, URHO3D_HANDLER(Panel, EnterLobby));
    SubscribeToEvent(E_ENTERPLAY,  URHO3D_HANDLER(Panel, EnterPlay));
}

void Panel::CreatePanels()
{
    panelScene_ = new Scene(context_);
    panelScene_->CreateComponent<Octree>();

    Zone* panelZone{ panelScene_->CreateComponent<Zone>() };
    panelZone->SetFogColor(Color::WHITE);
    panelZone->SetFogStart(23.0f);

    Camera* panelCam{ panelScene_->CreateChild("Camera")->CreateComponent<Camera>() };


    Node* razor{ panelScene_->CreateChild("PanelRazor") };
    razor->SetScale(0.34f);
    razor->CreateComponent<StaticModel>()->SetModel(MC->GetModel("Core"));
    razor->CreateChild("Top")->CreateComponent<StaticModel>()->SetModel(MC->GetModel("RazorHalf"));
    razor->CreateChild("Bottom")->CreateComponent<StaticModel>()->SetModel(MC->GetModel("RazorHalf"));
    razor->SetPosition(Vector3::FORWARD * 5.0f);

    panelTexture_ = new Texture2D(context_);
    panelTexture_->SetSize(1024, 1024, GRAPHICS->GetRGBFormat(), TEXTURE_RENDERTARGET);

    RenderSurface* panelSurface{ panelTexture_->GetRenderSurface() };
    SharedPtr<Viewport> panelViewport{ new Viewport(context_, panelScene_, panelCam) };
    panelSurface->SetViewport(0, panelViewport);

    for (bool small : {true, false}) {

        Vector3 panelPos{};
        Quaternion panelRot{};

        switch(colorSet_) {
        case 1: {
            panelPos = small ? Vector3(-0.83787f, 0.4306f, 2.01268f)
                             : Vector3(-4.49769f, 0.0f, 2.59509f);

            panelRot = small ? Quaternion(-120.0f, Vector3::UP)
                             : Quaternion( -60.0f, Vector3::UP);
        } break;
        case 2: {
            panelPos = small ? Vector3(0.83787f, 0.4306f, 2.01268f)
                             : Vector3(4.49769f, 0.0f, 2.59509f);
            panelRot = small ? Quaternion(120.0f, Vector3::UP)
                             : Quaternion( 60.0f, Vector3::UP);
        } break;
        case 3: {
            panelPos = small ? Vector3(-3.46253f, 0.4306f, -0.47611f)
                             : Vector3(-4.49767f, 0.0f, -2.59507f);
            panelRot = small ? Quaternion( 120.0f, Vector3::UP)
                             : Quaternion(-120.0f, Vector3::UP);
        } break;
        case 4: {
            panelPos = small ? Vector3(3.46253f, 0.4306f, -0.47611f)
                             : Vector3(4.49767f, 0.0f, -2.59507f);
            panelRot = small ? Quaternion(-120.0f, Vector3::UP)
                             : Quaternion( 120.0f, Vector3::UP);
        } break;
        default: break;
        }

        Node* panelNode{ node_->CreateChild(small ? "SmallPanel" : "BigPanel") };
        panelNode->SetPosition(panelPos);
        panelNode->SetRotation(panelRot);
        panelNode->SetScale(small ? 1.0f : 3.472769409f);
        panelNode->SetEnabled(small);

        StaticModel* panelModel{ panelNode->CreateComponent<StaticModel>() };
        panelModel->SetModel(MC->GetModel("Panel"));

        SharedPtr<Material> panelMaterial{};
        if (small) {

            panelMaterial = MC->colorSets_[colorSet_].panelMaterial_->Clone();
            panelMaterial->SetTexture(TU_EMISSIVE, panelTexture_);

            smallPanelNode_ = panelNode;

            panelTriggerNode_ = node_->CreateChild("PanelTrigger");
            panelTriggerNode_->SetPosition(panelPos);
            panelTriggerNode_->CreateComponent<RigidBody>()->SetTrigger(true);
            CollisionShape* panelTrigger{ panelTriggerNode_->CreateComponent<CollisionShape>() };
            panelTrigger->SetBox(Vector3(0.7f, 0.9f, 1.23f));

            SubscribeToEvent(panelTriggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Panel, ActivatePanel));
            SubscribeToEvent(panelTriggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Panel, DeactivatePanel));

        } else {

            panelMaterial = MC->colorSets_[colorSet_].addMaterial_->Clone();
            panelMaterial->SetTexture(TU_DIFFUSE, panelTexture_);

            bigPanelNode_ = panelNode;
        }

        panelModel->SetMaterial(panelMaterial);
    }
}

void Panel::Update(float timeStep)
{
}

void Panel::EnterLobby(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    smallPanelNode_->SetEnabled(true);
    bigPanelNode_->SetEnabled(true);
    panelTriggerNode_->SetEnabled(true);
}
void Panel::EnterPlay(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    node_->SetEnabledRecursive(false);
}

void Panel::ActivatePanel(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    Node* otherNode{ static_cast<Node*>(eventData[NodeCollisionStart::P_OTHERNODE].GetPtr()) };

    if (Pilot* pilot = otherNode->GetComponent<Pilot>()) {
        int pilotColorSet{};
        Player::takenColorSets_.TryGetValue(pilot->GetPlayerId(), pilotColorSet);

        if (IsOwner(pilot->GetPlayerId()) || (!HasOwner() && !pilotColorSet))
        {

            GetSubsystem<EffectMaster>()->FadeTo(bigPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                                 MC->colorSets_[colorSet_].addMaterial_->GetShaderParameter("MatDiffColor").GetColor(),
                                                 0.23f, 0.1f);
            GetSubsystem<EffectMaster>()->FadeTo(smallPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                                 MC->colorSets_[colorSet_].glowMaterial_->GetShaderParameter("MatEmissiveColor").GetColor(),
                                                 0.23f, 0.0f, "MatEmissiveColor");
        }
    }
}
void Panel::DeactivatePanel(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    Node* otherNode{ static_cast<Node*>(eventData[NodeCollisionStart::P_OTHERNODE].GetPtr()) };

    if (Pilot* pilot = otherNode->GetComponent<Pilot>()) {
        int pilotColorSet{};
        Player::takenColorSets_.TryGetValue(pilot->GetPlayerId(), pilotColorSet);

        PODVector<RigidBody*> bodies{};
        panelTriggerNode_->GetComponent<RigidBody>()->GetCollidingBodies(bodies);
        for (RigidBody* body : bodies) {

            if (Pilot* otherPilot = body->GetNode()->GetComponent<Pilot>()) {
                int otherPilotColorSet{};

                if (otherPilot == pilot || Player::takenColorSets_.TryGetValue(otherPilot->GetPlayerId(), otherPilotColorSet))
                    bodies.Remove(body);

            } else {

                bodies.Remove(body);
            }
        }


        if (IsOwner(pilot->GetPlayerId()) || (!HasOwner() && !bodies.Size()))
        {

            FadeOutPanel();
        }
    }
}
void Panel::FadeOutPanel(bool immediate)
{
    GetSubsystem<EffectMaster>()->FadeTo(bigPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                         Color::BLACK,
                                         0.23f * !immediate, 0.1f * !immediate);
    GetSubsystem<EffectMaster>()->FadeTo(smallPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                         Color::BLACK,
                                         0.23f * !immediate, 0.1f * !immediate, "MatEmissiveColor");
}

bool Panel::IsOwner(int playerId)
{
    for (int p : Player::takenColorSets_.Keys()) {
        if (playerId == p && Player::takenColorSets_[p] == colorSet_)
        {
            return true;
        }
    }
    return false;
}
bool Panel::HasOwner()
{
    if (Player::takenColorSets_.Values().Contains(colorSet_))
        return true;
    else
        return false;
}
