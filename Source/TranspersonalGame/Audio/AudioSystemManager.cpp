// AudioSystemManager.cpp
// Agent #16 — Audio Agent
// Implementation of adaptive audio system for prehistoric survival game

#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"

// ============================================================
// UAudio_SystemManager Implementation
// ============================================================

UAudio_SystemManager::UAudio_SystemManager()
{
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentZone = EAudio_ZoneType::OpenPlains;
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    AmbientVolume = 0.8f;
}

void UAudio_SystemManager::UpdateDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel)
    {
        return;
    }

    EAudio_DangerLevel PreviousLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewLevel;

    // Log danger transition for debugging
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Danger level changed from %d to %d"),
        (int32)PreviousLevel, (int32)NewLevel);

    // Music intensity is driven by GetMusicIntensity() which reads CurrentDangerLevel
    // MetaSound parameters would be updated here in full implementation
}

void UAudio_SystemManager::TransitionToZone(EAudio_ZoneType NewZone, float CrossfadeTime)
{
    if (CurrentZone == NewZone)
    {
        return;
    }

    EAudio_ZoneType PreviousZone = CurrentZone;
    CurrentZone = NewZone;

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Zone transition from %d to %d (crossfade: %.1fs)"),
        (int32)PreviousZone, (int32)NewZone, CrossfadeTime);

    // In full implementation: fade out previous zone ambient, fade in new zone ambient
    // MetaSound graph parameters would be updated here
}

void UAudio_SystemManager::TriggerDinoProximity(EAudio_DinoSpecies Species, float Distance)
{
    // Find matching proximity event config
    for (const FAudio_DinoProximityEvent& Event : DinoProximityEvents)
    {
        if (Event.Species == Species)
        {
            // Determine danger level based on distance
            if (Distance <= Event.DangerRadius)
            {
                UpdateDangerLevel(EAudio_DangerLevel::Imminent);
            }
            else if (Distance <= Event.ProximityRadius)
            {
                UpdateDangerLevel(EAudio_DangerLevel::Stalked);
            }

            // T-Rex causes ground rumble at close range
            if (Event.bCausesGroundRumble && Distance <= Event.ProximityRadius)
            {
                UE_LOG(LogTemp, Log, TEXT("AudioSystem: Ground rumble triggered for T-Rex at distance %.0f"),
                    Distance);
                // Screen shake and low-frequency rumble would be triggered here
            }

            return;
        }
    }

    // Generic fallback: update danger based on distance thresholds
    if (Distance <= 500.0f)
    {
        UpdateDangerLevel(EAudio_DangerLevel::Imminent);
    }
    else if (Distance <= 1500.0f)
    {
        UpdateDangerLevel(EAudio_DangerLevel::Stalked);
    }
    else if (Distance <= 3000.0f)
    {
        UpdateDangerLevel(EAudio_DangerLevel::Aware);
    }
}

bool UAudio_SystemManager::PlayVoiceLine(const FString& LineID)
{
    for (FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        if (Line.LineID == LineID)
        {
            if (Line.bIsPlayed)
            {
                UE_LOG(LogTemp, Warning, TEXT("AudioSystem: Voice line '%s' already played"), *LineID);
                return false;
            }

            Line.bIsPlayed = true;

            UE_LOG(LogTemp, Log, TEXT("AudioSystem: Playing voice line '%s' from speaker '%s'"),
                *LineID, *Line.SpeakerName);
            UE_LOG(LogTemp, Log, TEXT("AudioSystem: Audio URL: %s"), *Line.AudioURL);

            // In full implementation: stream audio from URL via HTTP audio component
            // Subtitle system would display Line.SubtitleText for Line.Duration seconds
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioSystem: Voice line '%s' not found in registry"), *LineID);
    return false;
}

void UAudio_SystemManager::RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine)
{
    // Check for duplicate
    for (const FAudio_VoiceLine& Existing : VoiceLineRegistry)
    {
        if (Existing.LineID == VoiceLine.LineID)
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioSystem: Voice line '%s' already registered"),
                *VoiceLine.LineID);
            return;
        }
    }

    VoiceLineRegistry.Add(VoiceLine);
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Registered voice line '%s' from '%s'"),
        *VoiceLine.LineID, *VoiceLine.SpeakerName);
}

float UAudio_SystemManager::GetMusicIntensity() const
{
    return DangerLevelToIntensity(CurrentDangerLevel);
}

bool UAudio_SystemManager::HasVoiceLinePlayed(const FString& LineID) const
{
    for (const FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        if (Line.LineID == LineID)
        {
            return Line.bIsPlayed;
        }
    }
    return false;
}

void UAudio_SystemManager::ResetVoiceLines()
{
    for (FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        Line.bIsPlayed = false;
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: All voice lines reset"));
}

float UAudio_SystemManager::GetDangerLevelFloat() const
{
    return DangerLevelToIntensity(CurrentDangerLevel);
}

float UAudio_SystemManager::DangerLevelToIntensity(EAudio_DangerLevel Level) const
{
    switch (Level)
    {
        case EAudio_DangerLevel::Safe:     return 0.0f;
        case EAudio_DangerLevel::Aware:    return 0.25f;
        case EAudio_DangerLevel::Stalked:  return 0.5f;
        case EAudio_DangerLevel::Imminent: return 0.75f;
        case EAudio_DangerLevel::Combat:   return 1.0f;
        default:                           return 0.0f;
    }
}

// ============================================================
// AAudio_AmbientZoneActor Implementation
// ============================================================

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ZoneAudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    ZoneType = EAudio_ZoneType::OpenPlains;
    ZoneRadius = 3000.0f;
    CrossfadeTime = 2.0f;
    Priority = 0;
    bPlayerInZone = false;
    DistanceToPlayer = 0.0f;
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Start ambient sound if assigned
    if (ZoneAmbientSound && AudioComponent)
    {
        AudioComponent->SetSound(ZoneAmbientSound);
        AudioComponent->SetVolumeMultiplier(0.0f); // Start silent, fade in when player enters
    }
}

void AAudio_AmbientZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckPlayerDistance();
}

void AAudio_AmbientZoneActor::CheckPlayerDistance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    bool bWasInZone = bPlayerInZone;
    bPlayerInZone = (DistanceToPlayer <= ZoneRadius);

    // Trigger zone enter/exit events
    if (bPlayerInZone && !bWasInZone)
    {
        OnPlayerEnterZone();
    }
    else if (!bPlayerInZone && bWasInZone)
    {
        OnPlayerExitZone();
    }

    // Smooth volume based on distance (fade at zone edge)
    if (AudioComponent && ZoneAmbientSound)
    {
        float EdgeFadeDistance = ZoneRadius * 0.2f; // 20% of radius for fade
        float TargetVolume = 0.0f;

        if (bPlayerInZone)
        {
            float FadeStart = ZoneRadius - EdgeFadeDistance;
            if (DistanceToPlayer > FadeStart)
            {
                TargetVolume = 1.0f - ((DistanceToPlayer - FadeStart) / EdgeFadeDistance);
            }
            else
            {
                TargetVolume = 1.0f;
            }
        }

        AudioComponent->SetVolumeMultiplier(TargetVolume);
    }
}

void AAudio_AmbientZoneActor::OnPlayerEnterZone()
{
    bPlayerInZone = true;

    if (AudioComponent && ZoneAmbientSound && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone '%s' (type: %d)"),
        *GetName(), (int32)ZoneType);
}

void AAudio_AmbientZoneActor::OnPlayerExitZone()
{
    bPlayerInZone = false;

    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(CrossfadeTime, 0.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone '%s' (type: %d)"),
        *GetName(), (int32)ZoneType);
}
