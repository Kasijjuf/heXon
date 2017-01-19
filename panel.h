#ifndef PANEL_H
#define PANEL_H

#include <Urho3D/Urho3D.h>

#include "luckey.h"
#include "mastercontrol.h"

class Panel : public LogicComponent
{
    URHO3D_OBJECT(Panel, LogicComponent);
public:
    Panel(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);
    void Initialize(int colorSet);
    void ActivatePanel(StringHash eventType, VariantMap& eventData);
    void DeactivatePanel(StringHash eventType, VariantMap& eventData);
    void EnterPlay(StringHash eventType, VariantMap& eventData);
    void EnterLobby(StringHash eventType, VariantMap& eventData);
private:
    int colorSet_;

    Scene* panelScene_;
    SharedPtr<Texture2D> panelTexture_;
    Node* panelTriggerNode_;
    Node* smallPanelNode_;
    Node* bigPanelNode_;

    void FadeOutPanel(bool immediate = false);
    void CreatePanels();
};

#endif // PANEL_H
