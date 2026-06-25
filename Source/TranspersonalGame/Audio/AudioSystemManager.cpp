#include "AudioSystemManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// AAudio_ZoneTrigger Implementation
// ============================================================

AAudio_ZoneTrigger::AAudio_ZoneTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(800.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
    AudioComp->SetupAttachment(RootComponent);
    AudioComp->bAutoActivate = false;
}

void AAudio_ZoneTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Apply config radius
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
    }

    // Start looping ambient if configured
    if (AmbientSound && AudioComp)
    {
        AudioComp->SetSound(AmbientSound);
        AudioComp->SetVolumeMultiplier(0.0f);
        if (ZoneConfig.bLooping)
        {
            AudioComp->Play();
        }
    }

    bCurrentlyActive = false;
}

void AAudio_ZoneTrigger::SetZoneActive(bool bActive)
{
    if (bCurrentlyActive == bActive) return;
    bCurrentlyActive = bActive;

    if (!AudioComp) return;

    if (bActive)
    {
        if (!AudioComp->IsPlaying())
        {
            AudioComp->Play();
        }
        AudioComp->AdjustVolume(ZoneConfig.FadeInTime, ZoneConfig.BaseVolume);
    }
    else
    {
        AudioComp->AdjustVolume(ZoneConfig.FadeOutTime, 0.0f);
    }
}

bool AAudio_ZoneTrigger::IsActiveForTimeOfDay(EAudio_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:  return bActiveAtDawn;
        case EAudio_TimeOfDay::Day:   return bActiveAtDay;
        case EAudio_TimeOfDay::Dusk:  return bActiveAtDusk;
        case EAudio_TimeOfDay::Night: return bActiveAtNight;
        default: return true;
    }
}

float AAudio_ZoneTrigger::GetVolumeForDistance(float Distance) const
{
    if (Distance <= 0.0f) return ZoneConfig.BaseVolume;
    float Radius = ZoneConfig.TriggerRadius;
    if (Distance >= Radius) return 0.0f;

    // Linear falloff from center to edge
    float Alpha = 1.0f - (Distance / Radius);
    return ZoneConfig.BaseVolume * Alpha;
}

// ============================================================
// UAudio_AdaptiveManager Implementation
// ============================================================

UAudio_AdaptiveManager::UAudio_AdaptiveManager()
{
    // Default dino sound profiles
    FAudio_DinoSoundProfile TRexProfile;
    TRexProfile.DinoSpecies = FName("TRex");
    TRexProfile.FootstepShakeRadius = 800.0f;
    TRexProfile.FootstepShakeIntensity = 2.5f;
    TRexProfile.RoarAudibleRadius = 4000.0f;
    TRexProfile.BreathingAudibleRadius = 500.0f;
    TRexProfile.VolumeMultiplier = 1.5f;
    DinoProfiles.Add(TRexProfile);

    FAudio_DinoSoundProfile RaptorProfile;
    RaptorProfile.DinoSpecies = FName("Raptor");
    RaptorProfile.FootstepShakeRadius = 300.0f;
    RaptorProfile.FootstepShakeIntensity = 0.8f;
    RaptorProfile.RoarAudibleRadius = 2000.0f;
    RaptorProfile.BreathingAudibleRadius = 200.0f;
    RaptorProfile.VolumeMultiplier = 1.0f;
    DinoProfiles.Add(RaptorProfile);

    FAudio_DinoSoundProfile BrachioProfile;
    BrachioProfile.DinoSpecies = FName("Brachiosaurus");
    BrachioProfile.FootstepShakeRadius = 1200.0f;
    BrachioProfile.FootstepShakeIntensity = 3.0f;
    BrachioProfile.RoarAudibleRadius = 5000.0f;
    BrachioProfile.BreathingAudibleRadius = 700.0f;
    BrachioProfile.VolumeMultiplier = 1.8f;
    DinoProfiles.Add(BrachioProfile);
}

void UAudio_AdaptiveManager::UpdateTimeOfDay(float GameHour)
{
    // 0-24 hour cycle
    if (GameHour >= 5.0f && GameHour < 7.0f)
        CurrentTimeOfDay = EAudio_TimeOfDay::Dawn;
    else if (GameHour >= 7.0f && GameHour < 18.0f)
        CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    else if (GameHour >= 18.0f && GameHour < 20.0f)
        CurrentTimeOfDay = EAudio_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = EAudio_TimeOfDay::Night;
}

void UAudio_AdaptiveManager::RegisterDinoProximity(FName DinoSpecies, float Distance)
{
    LastDinoProximityDistance = FMath::Min(LastDinoProximityDistance, Distance);

    // Escalate danger based on proximity
    for (const FAudio_DinoSoundProfile& Profile : DinoProfiles)
    {
        if (Profile.DinoSpecies == DinoSpecies)
        {
            if (Distance < Profile.FootstepShakeRadius)
            {
                CurrentDangerLevel = FMath::Clamp(
                    CurrentDangerLevel + (Profile.FootstepShakeIntensity * 0.1f),
                    0.0f, 1.0f
                );
            }
            break;
        }
    }
}

void UAudio_AdaptiveManager::TriggerDangerAlert(float Intensity)
{
    CurrentDangerLevel = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

float UAudio_AdaptiveManager::GetAmbientVolumeMultiplier() const
{
    // Danger suppresses ambient volume (tension effect)
    // At max danger, ambient drops to 20% — only heartbeat/danger music
    return FMath::Lerp(1.0f, 0.2f, CurrentDangerLevel);
}
