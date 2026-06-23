// AudioSystemManager.cpp — Transpersonal Game Studio
// Audio Agent #16 — PROD_CYCLE_AUTO_20260623_001
//
// Adaptive audio system for prehistoric survival game.
// Manages: zone-based ambient layers, dinosaur proximity audio,
// campfire crackling, quest stings, and day/night transitions.
//
// Sound references (Freesound.org):
//   ID 620324 — "Campfire crackling - Loop" (30s, loop)
//   ID 852107 — "Fireplace" (8.5s, crackling outdoor campfire)
//   ID 636708 — "FIREBurn_Burning Campfire.Crackling" (24s, intense)
//   ID 636709 — "FIREBurn_Burning Campfire.Crackling calm" (24s, calm)
//   ID 626277 — "FIREBurn_Burning Swaying fire" (21.7s, close)

#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Subsystem lifecycle
// ============================================================

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Set default footstep configs for each dino size class
    FAudio_DinoFootstepConfig SmallConfig;
    SmallConfig.DinoSize = EAudio_DinoSize::Small;
    SmallConfig.StepVolumeMultiplier = 0.4f;
    SmallConfig.RumbleRadius = 600.0f;
    SmallConfig.ScreenShakeIntensity = 0.1f;
    DinoFootstepConfigs.Add(SmallConfig);

    FAudio_DinoFootstepConfig MediumConfig;
    MediumConfig.DinoSize = EAudio_DinoSize::Medium;
    MediumConfig.StepVolumeMultiplier = 0.7f;
    MediumConfig.RumbleRadius = 1000.0f;
    MediumConfig.ScreenShakeIntensity = 0.35f;
    DinoFootstepConfigs.Add(MediumConfig);

    FAudio_DinoFootstepConfig LargeConfig;
    LargeConfig.DinoSize = EAudio_DinoSize::Large;
    LargeConfig.StepVolumeMultiplier = 1.0f;
    LargeConfig.RumbleRadius = 2000.0f;
    LargeConfig.ScreenShakeIntensity = 0.8f;
    DinoFootstepConfigs.Add(LargeConfig);

    CurrentZone = EAudio_Zone::Exploration;

    UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: Initialized with %d dino footstep configs"), DinoFootstepConfigs.Num());
}

void UAudioSystemManager::Deinitialize()
{
    StopAllAmbientLayers(0.5f);

    if (CampfireComponent && CampfireComponent->IsValidLowLevel())
    {
        CampfireComponent->Stop();
        CampfireComponent = nullptr;
    }

    Super::Deinitialize();
}

// ============================================================
// Zone management
// ============================================================

void UAudioSystemManager::SetAudioZone(EAudio_Zone NewZone)
{
    if (NewZone == CurrentZone)
    {
        return;
    }

    EAudio_Zone PreviousZone = CurrentZone;
    CurrentZone = NewZone;

    // Broadcast zone change for Blueprint listeners (music system, UI, etc.)
    OnZoneChanged.Broadcast(NewZone);

    // Handle night/day transitions automatically
    if (NewZone == EAudio_Zone::Night)
    {
        TransitionToNightAudio(8.0f);
    }
    else if (PreviousZone == EAudio_Zone::Night)
    {
        TransitionToDayAudio(8.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: Zone changed %d -> %d"),
        static_cast<int32>(PreviousZone), static_cast<int32>(NewZone));
}

// ============================================================
// Ambient layers
// ============================================================

void UAudioSystemManager::PlayAmbientLayer(const FAudio_AmbientLayer& Layer)
{
    if (!Layer.SoundAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudioSystemManager::PlayAmbientLayer — SoundAsset is null"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Play 2D ambient (non-positional) with fade in
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        World,
        Layer.SoundAsset,
        Layer.BaseVolume * AmbientVolume * MasterVolume,
        1.0f,
        0.0f,
        nullptr,
        false,
        true
    );

    if (AudioComp)
    {
        AudioComp->FadeIn(Layer.FadeInTime, Layer.BaseVolume * AmbientVolume * MasterVolume);
        ActiveAmbientComponents.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: Ambient layer started — %s"), *Layer.SoundAsset->GetName());
    }
}

void UAudioSystemManager::StopAllAmbientLayers(float FadeOutTime)
{
    for (UAudioComponent* Comp : ActiveAmbientComponents)
    {
        if (Comp && Comp->IsValidLowLevel() && Comp->IsPlaying())
        {
            Comp->FadeOut(FadeOutTime, 0.0f);
        }
    }
    ActiveAmbientComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: All ambient layers stopped (fade %.1fs)"), FadeOutTime);
}

// ============================================================
// Campfire audio
// ============================================================

void UAudioSystemManager::PlayCampfireAudio(FVector WorldLocation, float Radius)
{
    // Campfire sound is positional — attenuates with distance
    // Asset reference: Freesound ID 620324 "Campfire crackling - Loop"
    // In production, SoundAsset would be loaded from content browser.
    // For now, log the intent and position for Blueprint wiring.

    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Campfire audio requested at (%.0f, %.0f, %.0f) radius=%.0f — wire to Freesound 620324"),
        WorldLocation.X, WorldLocation.Y, WorldLocation.Z, Radius);

    // If we already have a campfire component, stop it first
    if (CampfireComponent && CampfireComponent->IsValidLowLevel())
    {
        CampfireComponent->Stop();
        CampfireComponent = nullptr;
    }
}

void UAudioSystemManager::StopCampfireAudio()
{
    if (CampfireComponent && CampfireComponent->IsValidLowLevel())
    {
        CampfireComponent->FadeOut(1.5f, 0.0f);
        CampfireComponent = nullptr;
    }
}

// ============================================================
// Dinosaur proximity audio
// ============================================================

void UAudioSystemManager::NotifyDinoFootstep(EAudio_DinoSize DinoSize, FVector StepLocation, AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        return;
    }

    // Find matching config
    const FAudio_DinoFootstepConfig* Config = nullptr;
    for (const FAudio_DinoFootstepConfig& Cfg : DinoFootstepConfigs)
    {
        if (Cfg.DinoSize == DinoSize)
        {
            Config = &Cfg;
            break;
        }
    }

    if (!Config)
    {
        return;
    }

    float Distance = FVector::Dist(StepLocation, PlayerActor->GetActorLocation());

    if (Distance <= Config->RumbleRadius)
    {
        // Notify Blueprint listeners — they handle screen shake and audio playback
        OnDinoNearby.Broadcast(DinoSize, Distance);

        // Volume scales with proximity (closer = louder)
        float ProximityFactor = 1.0f - (Distance / Config->RumbleRadius);
        float FinalVolume = Config->StepVolumeMultiplier * ProximityFactor * SFXVolume * MasterVolume;

        UE_LOG(LogTemp, Verbose,
            TEXT("UAudioSystemManager: Dino footstep size=%d dist=%.0f vol=%.2f shake=%.2f"),
            static_cast<int32>(DinoSize), Distance, FinalVolume,
            Config->ScreenShakeIntensity * ProximityFactor);
    }
}

void UAudioSystemManager::PlayDinoVocalization(EAudio_DinoSize DinoSize, FVector DinoLocation)
{
    // Vocalization type depends on size:
    // Small (Raptor): high-pitched bark/screech — territorial warning
    // Medium (Carnotaurus): deep grunt/bellow — aggression
    // Large (T-Rex): subsonic roar — dominance display

    FString SizeStr;
    switch (DinoSize)
    {
        case EAudio_DinoSize::Small:  SizeStr = TEXT("Raptor-class bark"); break;
        case EAudio_DinoSize::Medium: SizeStr = TEXT("Carnotaurus-class bellow"); break;
        case EAudio_DinoSize::Large:  SizeStr = TEXT("T-Rex subsonic roar"); break;
    }

    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Dino vocalization — %s at (%.0f, %.0f, %.0f)"),
        *SizeStr, DinoLocation.X, DinoLocation.Y, DinoLocation.Z);
}

// ============================================================
// Quest audio stings
// ============================================================

void UAudioSystemManager::PlayQuestStartSting(FName QuestID)
{
    // Find matching sting config
    for (const FAudio_QuestSting& Sting : QuestStings)
    {
        if (Sting.QuestID == QuestID && Sting.StartSting)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                UGameplayStatics::PlaySound2D(
                    World,
                    Sting.StartSting,
                    Sting.StingVolume * MasterVolume
                );
                UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: Quest start sting played — %s"), *QuestID.ToString());
            }
            return;
        }
    }

    // No asset assigned yet — log for Blueprint wiring
    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Quest start sting for '%s' — no asset assigned, wire in Blueprint"),
        *QuestID.ToString());
}

void UAudioSystemManager::PlayQuestCompleteSting(FName QuestID)
{
    for (const FAudio_QuestSting& Sting : QuestStings)
    {
        if (Sting.QuestID == QuestID && Sting.CompleteSting)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                UGameplayStatics::PlaySound2D(
                    World,
                    Sting.CompleteSting,
                    Sting.StingVolume * MasterVolume
                );
                UE_LOG(LogTemp, Log, TEXT("UAudioSystemManager: Quest complete sting played — %s"), *QuestID.ToString());
            }
            return;
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Quest complete sting for '%s' — no asset assigned, wire in Blueprint"),
        *QuestID.ToString());
}

// ============================================================
// Survival state audio
// ============================================================

void UAudioSystemManager::UpdateSurvivalAudio(float HealthNormalized, float StaminaNormalized, float FearNormalized)
{
    // Audio feedback for survival stats:
    // Low health (<0.25): heartbeat intensifies, breathing becomes laboured
    // Low stamina (<0.2): heavy breathing, footsteps become heavier
    // High fear (>0.7): tension drone increases, ambient sounds become more threatening

    // Determine appropriate zone based on survival state
    EAudio_Zone TargetZone = CurrentZone;

    if (FearNormalized > 0.8f || HealthNormalized < 0.15f)
    {
        TargetZone = EAudio_Zone::Combat;
    }
    else if (FearNormalized > 0.5f || HealthNormalized < 0.35f)
    {
        TargetZone = EAudio_Zone::Danger;
    }

    if (TargetZone != CurrentZone)
    {
        SetAudioZone(TargetZone);
    }

    UE_LOG(LogTemp, Verbose,
        TEXT("UAudioSystemManager: Survival audio — HP=%.2f STA=%.2f FEAR=%.2f zone=%d"),
        HealthNormalized, StaminaNormalized, FearNormalized, static_cast<int32>(CurrentZone));
}

// ============================================================
// Day/Night audio transitions
// ============================================================

void UAudioSystemManager::TransitionToNightAudio(float TransitionDuration)
{
    // Night audio profile:
    // - Daytime insects/birds fade out
    // - Cricket/nocturnal insect ambience fades in
    // - Distant predator calls become more frequent
    // - Music shifts to minor key, slower tempo

    StopAllAmbientLayers(TransitionDuration * 0.5f);

    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Transitioning to night audio (%.1fs) — wire nocturnal ambience in Blueprint"),
        TransitionDuration);
}

void UAudioSystemManager::TransitionToDayAudio(float TransitionDuration)
{
    // Day audio profile:
    // - Nocturnal ambience fades out
    // - Birds, wind, daytime insects fade in
    // - Music shifts to major key, more active

    StopAllAmbientLayers(TransitionDuration * 0.5f);

    UE_LOG(LogTemp, Log,
        TEXT("UAudioSystemManager: Transitioning to day audio (%.1fs) — wire diurnal ambience in Blueprint"),
        TransitionDuration);
}
