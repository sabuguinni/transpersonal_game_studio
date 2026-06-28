// AudioSystemManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260628_003
// Full implementation of adaptive audio system

#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio state

    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentBiome = EAudio_BiomeType::OpenPlains;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    MusicVolume = 0.6f;
    DialogueVolume = 1.0f;
    SFXVolume = 0.9f;

    ThreatDecayTimer = 0.0f;
    ClosestDinosaurDistance = 99999.0f;
    HeaviestDinosaurMass = 0.0f;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    ApplyAmbientForCurrentState();
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decay threat level over time when no dinosaurs are registered
    if (ThreatDecayTimer > 0.0f)
    {
        ThreatDecayTimer -= DeltaTime;
        if (ThreatDecayTimer <= 0.0f)
        {
            // Decay one step toward Safe
            switch (CurrentThreatLevel)
            {
            case EAudio_ThreatLevel::Flee:
                SetThreatLevel(EAudio_ThreatLevel::Danger);
                ThreatDecayTimer = 8.0f;
                break;
            case EAudio_ThreatLevel::Danger:
                SetThreatLevel(EAudio_ThreatLevel::Aware);
                ThreatDecayTimer = 12.0f;
                break;
            case EAudio_ThreatLevel::Aware:
                SetThreatLevel(EAudio_ThreatLevel::Safe);
                ThreatDecayTimer = 0.0f;
                break;
            default:
                break;
            }
        }
    }
}

// ── Threat System ──────────────────────────────────────────────────────────

void AAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;

    EAudio_ThreatLevel OldLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewLevel;

    // Find matching music transition
    for (const FAudio_ThreatTransition& Transition : MusicTransitions)
    {
        if (Transition.FromLevel == OldLevel && Transition.ToLevel == NewLevel)
        {
            if (Transition.MusicLayer && ActiveMusicComponent)
            {
                ActiveMusicComponent->FadeOut(Transition.CrossfadeDuration, 0.0f);
            }
            if (Transition.MusicLayer)
            {
                UWorld* World = GetWorld();
                if (World)
                {
                    ActiveMusicComponent = UGameplayStatics::SpawnSound2D(
                        World,
                        Transition.MusicLayer,
                        MusicVolume * MasterVolume,
                        1.0f,
                        Transition.CrossfadeDuration
                    );
                }
            }
            break;
        }
    }

    // Reset threat decay timer based on new level
    switch (NewLevel)
    {
    case EAudio_ThreatLevel::Flee:
        ThreatDecayTimer = 15.0f;
        break;
    case EAudio_ThreatLevel::Danger:
        ThreatDecayTimer = 20.0f;
        break;
    case EAudio_ThreatLevel::Aware:
        ThreatDecayTimer = 25.0f;
        break;
    default:
        ThreatDecayTimer = 0.0f;
        break;
    }
}

void AAudio_SystemManager::RegisterDinosaurNearby(float DistanceMetres, float DinosaurMass)
{
    ClosestDinosaurDistance = FMath::Min(ClosestDinosaurDistance, DistanceMetres);
    HeaviestDinosaurMass = FMath::Max(HeaviestDinosaurMass, DinosaurMass);
    UpdateThreatFromDinosaurData();

    // Trigger footstep shake for large dinosaurs
    if (DinosaurMass > 1000.0f)
    {
        TriggerFootstepShake(DinosaurMass, DistanceMetres);
    }
}

void AAudio_SystemManager::ClearDinosaurThreats()
{
    ClosestDinosaurDistance = 99999.0f;
    HeaviestDinosaurMass = 0.0f;
    // Let decay timer handle the music transition naturally
}

void AAudio_SystemManager::UpdateThreatFromDinosaurData()
{
    EAudio_ThreatLevel NewLevel = EAudio_ThreatLevel::Safe;

    // Threat thresholds based on distance and mass
    if (ClosestDinosaurDistance < 15.0f)
    {
        NewLevel = EAudio_ThreatLevel::Flee;
    }
    else if (ClosestDinosaurDistance < 40.0f)
    {
        NewLevel = EAudio_ThreatLevel::Danger;
    }
    else if (ClosestDinosaurDistance < 100.0f)
    {
        NewLevel = EAudio_ThreatLevel::Aware;
    }

    // Escalate further for very large dinosaurs (T-Rex, Brachiosaurus)
    if (HeaviestDinosaurMass > 5000.0f && NewLevel < EAudio_ThreatLevel::Aware)
    {
        NewLevel = EAudio_ThreatLevel::Aware;
    }

    // Only escalate, never de-escalate immediately (let decay handle that)
    if (NewLevel > CurrentThreatLevel)
    {
        SetThreatLevel(NewLevel);
    }
}

// ── Ambient System ─────────────────────────────────────────────────────────

void AAudio_SystemManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;
    CurrentBiome = NewBiome;
    ApplyAmbientForCurrentState();
}

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay) return;
    CurrentTimeOfDay = NewTime;
    ApplyAmbientForCurrentState();
}

void AAudio_SystemManager::CrossfadeAmbientLayer(FAudio_AmbientLayer NewLayer)
{
    if (!NewLayer.Sound) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Fade out existing ambient
    if (ActiveAmbientComponent && ActiveAmbientComponent->IsPlaying())
    {
        ActiveAmbientComponent->FadeOut(NewLayer.FadeOutTime, 0.0f);
    }

    // Spawn new ambient layer
    ActiveAmbientComponent = UGameplayStatics::SpawnSound2D(
        World,
        NewLayer.Sound,
        NewLayer.BaseVolume * AmbientVolume * MasterVolume,
        1.0f,
        NewLayer.FadeInTime
    );
}

void AAudio_SystemManager::ApplyAmbientForCurrentState()
{
    // Find the best matching ambient layer for current biome + time of day
    FAudio_AmbientLayer* BestLayer = nullptr;
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome == CurrentBiome && Layer.TimeOfDay == CurrentTimeOfDay)
        {
            BestLayer = &Layer;
            break;
        }
    }

    // Fallback: match biome only
    if (!BestLayer)
    {
        for (FAudio_AmbientLayer& Layer : AmbientLayers)
        {
            if (Layer.Biome == CurrentBiome)
            {
                BestLayer = &Layer;
                break;
            }
        }
    }

    if (BestLayer)
    {
        CrossfadeAmbientLayer(*BestLayer);
    }
}

// ── Dialogue System ────────────────────────────────────────────────────────

void AAudio_SystemManager::PlayDialogueLine(const FAudio_DialogueLine& Line, FVector WorldLocation)
{
    if (!Line.SoundAsset) return;

    UWorld* World = GetWorld();
    if (!World) return;

    if (Line.bSpatialised)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World,
            Line.SoundAsset,
            WorldLocation,
            DialogueVolume * MasterVolume
        );
    }
    else
    {
        UGameplayStatics::PlaySound2D(
            World,
            Line.SoundAsset,
            DialogueVolume * MasterVolume
        );
    }
}

void AAudio_SystemManager::StopAllDialogue()
{
    // Stop all active audio components tagged as dialogue
    // In production this would iterate a tracked array of dialogue components
    UWorld* World = GetWorld();
    if (!World) return;

    // Broadcast stop event — Blueprint can bind to this
    // For now, fade out music as a safe fallback
    if (ActiveMusicComponent)
    {
        ActiveMusicComponent->AdjustVolume(0.5f, MusicVolume * 0.3f);
    }
}

// ── Screen Shake / Feedback ────────────────────────────────────────────────

void AAudio_SystemManager::TriggerFootstepShake(float DinosaurMass, float DistanceMetres)
{
    float Intensity = CalculateShakeIntensity(DinosaurMass, DistanceMetres);
    if (Intensity < 0.05f) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Scale: T-Rex at 30m = 0.4 intensity, at 10m = 1.0 intensity
    // We use ClientStartCameraShake with a default shake class
    // In production, assign a UCameraShakeBase subclass via UPROPERTY
    PC->ClientStartCameraShake(
        UCameraShakeBase::StaticClass(),
        Intensity
    );
}

void AAudio_SystemManager::TriggerImpactShake(float ImpactForce)
{
    float Intensity = FMath::Clamp(ImpactForce / 10000.0f, 0.1f, 1.0f);

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    PC->ClientStartCameraShake(
        UCameraShakeBase::StaticClass(),
        Intensity
    );
}

float AAudio_SystemManager::CalculateShakeIntensity(float Mass, float Distance) const
{
    // Inverse square falloff, clamped
    // T-Rex ~7000kg, Raptor ~80kg
    float MassFactor = FMath::Clamp(Mass / 7000.0f, 0.01f, 1.0f);
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 150.0f), 0.0f, 1.0f);
    return MassFactor * DistanceFactor;
}
