#include "Shrink.hpp"

#include "Globals.hpp"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/Window.hpp>
#include <hyprland/src/managers/AnimationManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

void CShrink::init(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::init(pHandle, "shrink");
  addConfigValue(pHandle, "shrink_percentage", Hyprlang::FLOAT{0.5f});
}

void CShrink::setup(HANDLE pHandle, std::string animationName) {
  IFocusAnimation::setup(pHandle, animationName);
  static const auto *shrinkPercentage =
      (Hyprlang::FLOAT *const *)(getConfigValue(pHandle, "shrink_percentage")
                                     ->getDataStaticPtr());
  g_fShrinkPercentage = **shrinkPercentage;
  hyprfocus_log(LOG, "Shrink percentage: {}", g_fShrinkPercentage);
}

void CShrink::onWindowFocus(CWindow *pWindow, HANDLE pHandle) {
  std::string currentAnimStyle =
      pWindow->m_vRealSize.getConfig()->internalStyle;
  hyprfocus_log(LOG, "Current animation style: {}", currentAnimStyle);
  if ((currentAnimStyle == "popout" || currentAnimStyle == "popin") &&
      pWindow->m_vRealSize.isBeingAnimated()) {
    Debug::log(LOG, "Shrink: Window is already being animated, skipping");
    return;
  }

  IFocusAnimation::onWindowFocus(pWindow, pHandle);

  pWindow->m_vRealSize.setConfig(&m_sFocusOutAnimConfig);
  pWindow->m_vRealPosition.setConfig(&m_sFocusOutAnimConfig);

  m_sShrinkAnimation.registerVar();
  m_sShrinkAnimation.create(1.0f, &m_sFocusInAnimConfig, pWindow,
                            AVARDAMAGE_ENTIRE);
  m_sShrinkAnimation = g_fShrinkPercentage;

  m_sShrinkAnimation.setUpdateCallback([this, pWindow](void *pShrinkAnimation) {
    const auto GOALPOS = pWindow->m_vRealPosition.goal();
    const auto GOALSIZE = pWindow->m_vRealSize.goal();

    const auto *PANIMATION = (CAnimatedVariable<float> *)pShrinkAnimation;

    pWindow->m_vRealSize.setValue(GOALSIZE * PANIMATION->value());
    pWindow->m_vRealPosition.setValue(GOALPOS + GOALSIZE / 2.f -
                                      pWindow->m_vRealSize.value() / 2.f);
  });

  m_sShrinkAnimation.setCallbackOnEnd([this, pWindow](void *pShrinkAnimation) {
    ((CAnimatedVariable<float> *)pShrinkAnimation)->resetAllCallbacks();
  });
}
