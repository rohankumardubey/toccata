#include "../include/component.h"

toccata::Component::Component() {
    m_engine = nullptr;
    m_textRenderer = nullptr;

    m_settings = nullptr;

    m_parent = nullptr;
    m_activeElement = nullptr;
    m_enabled = true;
    m_visible = true;
}

toccata::Component::~Component() {
    /* void */
}

void toccata::Component::Construct() {
    /* void */
}

void toccata::Component::Initialize(dbasic::DeltaEngine *engine, dbasic::TextRenderer *renderer, Settings *settings) {
    Construct();
    
    m_engine = engine;
    m_textRenderer = renderer;
    m_settings = settings;

    for (Component *component : m_children) {
        component->Initialize(engine, renderer, settings);
    }
}

void toccata::Component::RenderAll() {
    if (!IsVisible()) return;

    Render();

    for (Component *component : m_children) {
        component->RenderAll();
    }
}

void toccata::Component::ProcessAll() {
    Update();
    if (InputEnabled()) ProcessInput();

    for (Component *component : m_children) {
        component->ProcessAll();
    }
}

void toccata::Component::AddChild(Component *component) {
    component->SetParent(this);
    m_children.push_back(component);
}

void toccata::Component::RequestControl() {
    m_activeElement = this;

    Component *parent = m_parent;
    while (parent != nullptr) {
        parent->m_activeElement = this;
        parent = parent->m_parent;
    }
}

void toccata::Component::ReleaseControl() {
    if (!HasControl()) return;

    m_activeElement = nullptr;

    Component *parent = m_parent;
    while (parent != nullptr) {
        parent->m_activeElement = nullptr;
        parent = parent->m_parent;
    }
}

bool toccata::Component::HasControl() const {
    const Component *parent = m_parent;
    const Component *root = this;
    while (parent != nullptr) {
        root = parent;
        parent = parent->m_parent;
    }

    return root->m_activeElement == this;
}

bool toccata::Component::InputEnabled() const {
    if (m_parent != nullptr) return m_enabled && m_parent->InputEnabled();
    else return m_enabled;
}

bool toccata::Component::IsVisible() const {
    if (m_parent != nullptr) return m_visible && m_parent->IsVisible();
    else return m_visible;
}

void toccata::Component::Render() {
    /* void */
}

void toccata::Component::Update() {
    /* void */
}

void toccata::Component::ProcessInput() {
    /* void */
}
