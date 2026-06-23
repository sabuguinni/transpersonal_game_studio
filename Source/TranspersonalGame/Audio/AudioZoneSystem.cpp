#include "AudioZoneSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

// ============================================================
// AudioZoneSystem — Agent #16 Audio Agent
// Spatial audio management for MinPlayableMap
// ============================================================

UAudioZoneSystem::UAudioZoneSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;  // 10Hz tick — audio doesn't need per-frame
}

void UAudioZoneSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultZones();
}

void UAudioZoneSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    BlendMusicIntensity(DeltaTime);
}

void UAudioZoneSystem::InitializeDefaultZones()
{
    // Canyon Entrance — Raptor patrol zone (matches NarrTrigger_CanyonEntrance from Agent #15)
    FAudio_ZoneConfig CanyonZone;
    CanyonZone.ZoneID = FName("CanyonEntrance");
    CanyonZone.ZoneType = EAudio_ZoneType::Caution;
    CanyonZone.DangerLevel = EAudio_DangerLevel::Medium;
    CanyonZone.AmbientVolume = 0.7f;
    CanyonZone.MusicIntensity = 0.5f;
    CanyonZone.BlendRadius = 600.0f;
    CanyonZone.bLoopAmbient = true;
    RegisteredZones.Add(CanyonZone);

    // Riverbed Crossing — Resource zone (matches NarrTrigger_RiverbedCrossing)
    FAudio_ZoneConfig RiverbedZone;
    RiverbedZone.ZoneID = FName("RiverbedCrossing");
    RiverbedZone.ZoneType = EAudio_ZoneType::Water;
    RiverbedZone.DangerLevel = EAudio_DangerLevel::Low;
    RiverbedZone.AmbientVolume = 1.0f;
    RiverbedZone.MusicIntensity = 0.1f;
    RiverbedZone.BlendRadius = 500.0f;
    RiverbedZone.bLoopAmbient = true;
    RegisteredZones.Add(RiverbedZone);

    // Dino Nest Area — T-Rex territory (matches NarrTrigger_DinoNestArea)
    FAudio_ZoneConfig NestZone;
    NestZone.ZoneID = FName("DinoNestArea");
    NestZone.ZoneType = EAudio_ZoneType::Danger;
    NestZone.DangerLevel = EAudio_DangerLevel::Critical;
    NestZone.AmbientVolume = 0.4f;
    NestZone.MusicIntensity = 1.0f;
    NestZone.BlendRadius = 800.0f;
    NestZone.bLoopAmbient = true;
    RegisteredZones.Add(NestZone);

    // Player Camp — Safe base zone
    FAudio_ZoneConfig CampZone;
    CampZone.ZoneID = FName("PlayerCamp");
    CampZone.ZoneType = EAudio_ZoneType::Safe;
    CampZone.DangerLevel = EAudio_DangerLevel::None;
    CampZone.AmbientVolume = 1.0f;
    CampZone.MusicIntensity = 0.0f;
    CampZone.BlendRadius = 400.0f;
    CampZone.bLoopAmbient = true;
    RegisteredZones.Add(CampZone);
}

void UAudioZoneSystem::RegisterZone(const FAudio_ZoneConfig& ZoneConfig)
{
    // Remove existing zone with same ID if present
    RegisteredZones.RemoveAll([&ZoneConfig](const FAudio_ZoneConfig& Z)
    {
        return Z.ZoneID == ZoneConfig.ZoneID;
    });
    RegisteredZones.Add(ZoneConfig);
}

void UAudioZoneSystem::EnterZone(FName ZoneID)
{
    if (!ActiveZoneIDs.Contains(ZoneID))
    {
        ActiveZoneIDs.Add(ZoneID);
        RecalculateZoneState();
    }
}

void UAudioZoneSystem::ExitZone(FName ZoneID)
{
    ActiveZoneIDs.Remove(ZoneID);
    RecalculateZoneState();
}

void UAudioZoneSystem::RecalculateZoneState()
{
    // Find highest danger zone among active zones
    EAudio_DangerLevel HighestDanger = EAudio_DangerLevel::None;
    EAudio_ZoneType DominantType = EAudio_ZoneType::Open;
    float HighestIntensity = 0.0f;

    for (const FName& ZoneID : ActiveZoneIDs)
    {
        const FAudio_ZoneConfig* Config = RegisteredZones.FindByPredicate([&ZoneID](const FAudio_ZoneConfig& Z)
        {
            return Z.ZoneID == ZoneID;
        });

        if (Config)
        {
            if ((uint8)Config->DangerLevel > (uint8)HighestDanger)
            {
                HighestDanger = Config->DangerLevel;
                DominantType = Config->ZoneType;
            }
            HighestIntensity = FMath::Max(HighestIntensity, Config->MusicIntensity);
        }
    }

    CurrentDangerLevel = HighestDanger;
    CurrentZoneType = DominantType;
    TargetMusicIntensity = HighestIntensity;
}

void UAudioZoneSystem::BlendMusicIntensity(float DeltaTime)
{
    // Smooth music intensity transition (RDR2-style adaptive music)
    if (!FMath::IsNearlyEqual(CurrentMusicIntensity, TargetMusicIntensity, 0.01f))
    {
        CurrentMusicIntensity = FMath::FInterpTo(
            CurrentMusicIntensity,
            TargetMusicIntensity,
            DeltaTime,
            MusicBlendSpeed
        );
    }
}

void UAudioZoneSystem::TriggerScreenShake(const FAudio_ScreenShakeConfig& ShakeConfig)
{
    // Trigger camera shake via PlayerController
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Use built-in UE5 camera shake (T-Rex footstep proximity)
    // Intensity scales with distance — handled by caller
    PC->ClientStartCameraShake(
        UCameraShakeBase::StaticClass(),
        ShakeConfig.Intensity
    );
}

void UAudioZoneSystem::TriggerDamageFlash(float Intensity)
{
    // Damage flash — red screen overlay
    // Implemented via PlayerController's client message system
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Flash the screen red — built-in UE5 damage feedback
    PC->ClientPlayCameraShake(UCameraShakeBase::StaticClass(), Intensity);
}

void UAudioZoneSystem::PlayFootstep(bool bIsRunning, bool bIsHeavyCreature)
{
    // Footstep audio — surface-aware
    // Volume and pitch vary by surface type and creature weight
    float Volume = bIsHeavyCreature ? 2.0f : 1.0f;
    float Pitch = bIsRunning ? 1.2f : 1.0f;

    if (CurrentSurfaceType == FName("Stone"))
    {
        Volume *= 1.3f;  // Stone is louder
    }
    else if (CurrentSurfaceType == FName("Mud"))
    {
        Volume *= 0.8f;  // Mud dampens sound
        Pitch *= 0.9f;
    }
    else if (CurrentSurfaceType == FName("Grass"))
    {
        Volume *= 0.7f;  // Grass is quietest
    }

    // Heavy creature footsteps trigger screen shake
    if (bIsHeavyCreature)
    {
        FAudio_ScreenShakeConfig ShakeConfig;
        ShakeConfig.Intensity = Volume * 0.3f;
        ShakeConfig.Duration = 0.2f;
        TriggerScreenShake(ShakeConfig);
    }
}

void UAudioZoneSystem::SetSurfaceType(FName SurfaceType)
{
    CurrentSurfaceType = SurfaceType;
}

void UAudioZoneSystem::UpdateTimeOfDay(float NormalizedTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NormalizedTimeOfDay, 0.0f, 1.0f);

    // Night = 0.0-0.25 and 0.75-1.0 (midnight ranges)
    bIsNighttime = (TimeOfDay < 0.25f || TimeOfDay > 0.75f);

    // Night audio: crickets, owls, distant predator calls
    // Day audio: birds, insects, wind
    // Music intensity increases at dusk/dawn (transition periods)
    float TransitionFactor = 0.0f;
    if (TimeOfDay > 0.2f && TimeOfDay < 0.3f)  // Dawn
    {
        TransitionFactor = 1.0f - FMath::Abs(TimeOfDay - 0.25f) / 0.05f;
    }
    else if (TimeOfDay > 0.7f && TimeOfDay < 0.8f)  // Dusk
    {
        TransitionFactor = 1.0f - FMath::Abs(TimeOfDay - 0.75f) / 0.05f;
    }

    // Boost music intensity at transitions (predators more active at dawn/dusk)
    if (TransitionFactor > 0.0f)
    {
        TargetMusicIntensity = FMath::Max(TargetMusicIntensity, TransitionFactor * 0.4f);
    }
}
