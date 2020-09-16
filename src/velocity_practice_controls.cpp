#include "../include/velocity_practice_controls.h"

toccata::VelocityPracticeControls::VelocityPracticeControls() {
    /* void */
}

toccata::VelocityPracticeControls::~VelocityPracticeControls() {
    /* void */
}

void toccata::VelocityPracticeControls::Construct() {
    AddChild(&m_threshold);
    AddChild(&m_target);
}

void toccata::VelocityPracticeControls::Render() {
    Component::Render();
}

void toccata::VelocityPracticeControls::Update() {
    m_target.SetBoundingBox(BoundingBox(m_boundingBox.Width() / 2, m_boundingBox.Height())
        .AlignLeft(m_boundingBox.Left()).AlignTop(m_boundingBox.Top()));
    m_target.SetMin(0.0);
    m_target.SetMax(128.0);
    m_target.SetStep(0.5);
    m_target.SetPrecision(4);
    m_target.SetTextHeight((double)m_target.GetBoundingBox().Height() * 0.9);
    m_target.SetWrap(true);

    m_threshold.SetBoundingBox(BoundingBox(m_boundingBox.Width() / 2, m_boundingBox.Height())
        .AlignLeft(m_target.GetBoundingBox().Right()).AlignTop(m_target.GetBoundingBox().Top()));
    m_threshold.SetMin(0.0);
    m_threshold.SetMax(128.0);
    m_threshold.SetStep(0.5);
    m_threshold.SetPrecision(4);
    m_threshold.SetTextHeight((double)m_threshold.GetBoundingBox().Height() * 0.9);
    m_threshold.SetWrap(true);
}