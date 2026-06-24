// AudioZoneManager.cpp
// Adaptive ambient audio zone system for prehistoric survival game
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260624_004

#include "AudioZoneManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio fade
    CurrentFadeAlpha = 0.0f;
    bFadingIn = false;
}

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();

    if (bIsActive)
    {
        bFadingIn = true;
    }
}

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive)
    {
        return;
    }

    // Fade in/out logic
    if (bFadingIn && CurrentFadeAlpha < 1.0f)
    {
        float FadeSpeed = (ZoneConfig.FadeInTime > 0.0f) ? (1.0f / ZoneConfig.FadeInTime) : 1.0f;
        CurrentFadeAlpha = FMath::Clamp(CurrentFadeAlpha + DeltaTime * FadeSpeed, 0.0f, 1.0f);
    }
    else if (!bFadingIn && CurrentFadeAlpha > 0.0f)
    {
        float FadeSpeed = (ZoneConfig.FadeOutTime > 0.0f) ? (1.0f / ZoneConfig.FadeOutTime) : 1.0f;
        CurrentFadeAlpha = FMath::Clamp(CurrentFadeAlpha - DeltaTime * FadeSpeed, 0.0f, 1.0f);
    }
}

void AAudioZoneManager::ActivateZone()
{
    bIsActive = true;
    bFadingIn = true;
}

void AAudioZoneManager::DeactivateZone()
{
    bIsActive = false;
    bFadingIn = false;
}

EAudio_ZoneType AAudioZoneManager::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

float AAudioZoneManager::GetMusicIntensity() const
{
    return ZoneConfig.MusicIntensity * CurrentFadeAlpha;
}
