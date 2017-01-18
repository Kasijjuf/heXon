#include "effectmaster.h"

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

        if (small){
            panelTriggerNode_ = node_->CreateChild("PanelTrigger");
            panelTriggerNode_->SetPosition(panelPos);
            panelTriggerNode_->CreateComponent<RigidBody>()->SetTrigger(true);
            CollisionShape* panelTrigger{ panelTriggerNode_->CreateComponent<CollisionShape>() };
            panelTrigger->SetBox(Vector3(0.7f, 0.9f, 1.23f));

            SubscribeToEvent(panelTriggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Panel, ActivatePanel));
            SubscribeToEvent(panelTriggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Panel, DeactivatePanel));
        }

        Node* panelNode{ node_->CreateChild(small ? "SmallPanel" : "BigPanel") };
        panelNode->SetPosition(panelPos);
        panelNode->SetRotation(panelRot);
        panelNode->SetScale(small ? 1.0f : 3.472769409f);
        panelNode->SetEnabled(small);

        if (small)
            smallPanelNode_ = panelNode;
        else
            bigPanelNode_ = panelNode;

        StaticModel* panelModel{ panelNode->CreateComponent<StaticModel>() };
        panelModel->SetModel(MC->GetModel("Panel"));
        panelModel->SetMaterial(small ? MC->colorSets_[colorSet].glowMaterial_->Clone()
                                      : MC->colorSets_[colorSet].addMaterial_->Clone());
    }

    FadeOutPanel();

    SubscribeToEvent(E_ENTERLOBBY, URHO3D_HANDLER(Panel, EnterLobby));
    SubscribeToEvent(E_ENTERPLAY,  URHO3D_HANDLER(Panel, EnterPlay));
}

void Panel::Update(float timeStep)
{
}


void Panel::EnterLobby(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    smallPanelNode_->SetEnabled(true);
    panelTriggerNode_->SetEnabled(true);
}
void Panel::EnterPlay(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    node_->SetEnabledRecursive(false);
}

void Panel::ActivatePanel(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    bigPanelNode_->SetEnabled(true);

    GetSubsystem<EffectMaster>()->FadeTo(smallPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                         MC->colorSets_[colorSet_].glowMaterial_->GetShaderParameter("MatEmissiveColor").GetColor(),
                                         0.23f, 0.0f, "MatEmissiveColor");
}
void Panel::DeactivatePanel(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    FadeOutPanel();
}
void Panel::FadeOutPanel()
{
    bigPanelNode_->SetEnabled(false);
    GetSubsystem<EffectMaster>()->FadeTo(smallPanelNode_->GetComponent<StaticModel>()->GetMaterial(),
                                         MC->colorSets_[colorSet_].glowMaterial_->GetShaderParameter("MatEmissiveColor").GetColor() * 0.23f,
                                         0.23f, 0.1f, "MatEmissiveColor");
}


