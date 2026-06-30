// AudioSystemManager.cpp
// Audio Agent #16 — PROD_CYCLE_AUTO_20260630_007
// Adaptive audio system: danger-driven music, ambient zones, dialogue voice binding

#include "AudioSystemManager.h"
#include "AudioDevice.h"
#include "Sound/SoundBase.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_AdaptiveMusicComponent
// ─────────────────────────────────────────────────────────────────────────────

UAudio_AdaptiveMusicComponent::UAudio_AdaptiveMusicComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentTimeOfDay   = EAudio_TimeOfDay::Day;
    DangerUpdateInterval = 2.0f;
    bMusicSystemActive = false;
    MasterVolume = 1.0f;
    MusicVolume  = 0.8f;
    SFXVolume    = 1.0f;
    AmbienceVolume = 0.9f;
    DangerBlendSpeed = 1.5f;
    TargetDangerWeight = 0.0f;
    CurrentDangerWeight = 0.0f;
}

void UAudio_AdaptiveMusicComponent::BeginPlay()
{
    Super::BeginPlay();
    StartAdaptiveMusic();
}

void UAudio_AdaptiveMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Smoothly blend danger music weight
    CurrentDangerWeight = FMath::FInterpTo(CurrentDangerWeight, TargetDangerWeight, DeltaTime, DangerBlendSpeed);
}

void UAudio_AdaptiveMusicComponent::StartAdaptiveMusic()
{
    if (bMusicSystemActive) return;
    bMusicSystemActive = true;

    UWorld* World = GetWorld();
    if (!World) return;

    // Start periodic danger evaluation
    World->GetTimerManager().SetTimer(
        DangerUpdateTimer,
        this,
        &UAudio_AdaptiveMusicComponent::EvaluateDangerLevel,
        DangerUpdateInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Adaptive music started — danger polling every %.1fs"), DangerUpdateInterval);
}

void UAudio_AdaptiveMusicComponent::StopAdaptiveMusic()
{
    bMusicSystemActive = false;
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(DangerUpdateTimer);
    }
}

void UAudio_AdaptiveMusicComponent::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;

    EAudio_DangerLevel OldLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewLevel;

    // Map danger level to blend weight
    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:       TargetDangerWeight = 0.0f;  break;
        case EAudio_DangerLevel::Aware:      TargetDangerWeight = 0.33f; break;
        case EAudio_DangerLevel::Threatened: TargetDangerWeight = 0.66f; break;
        case EAudio_DangerLevel::Critical:   TargetDangerWeight = 1.0f;  break;
        default:                             TargetDangerWeight = 0.0f;  break;
    }

    OnDangerLevelChanged.Broadcast(OldLevel, NewLevel);
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Danger level changed %d → %d (blend target: %.2f)"),
        (int32)OldLevel, (int32)NewLevel, TargetDangerWeight);
}

void UAudio_AdaptiveMusicComponent::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
    OnTimeOfDayChanged.Broadcast(NewTime);
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Time of day changed to %d"), (int32)NewTime);
}

void UAudio_AdaptiveMusicComponent::EvaluateDangerLevel()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Scan registered danger zones
    EAudio_DangerLevel HighestDanger = EAudio_DangerLevel::Safe;

    for (const FAudio_DangerZone& Zone : RegisteredDangerZones)
    {
        float DistSq = FVector::DistSquared(PlayerLoc, Zone.ZoneCenter);
        float RadiusSq = Zone.DangerRadius * Zone.DangerRadius;

        if (DistSq <= RadiusSq)
        {
            if ((int32)Zone.DangerLevel > (int32)HighestDanger)
            {
                HighestDanger = Zone.DangerLevel;
            }
        }
    }

    SetDangerLevel(HighestDanger);
}

void UAudio_AdaptiveMusicComponent::RegisterDangerZone(const FAudio_DangerZone& Zone)
{
    RegisteredDangerZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Registered danger zone '%s' at (%.0f, %.0f, %.0f) r=%.0f"),
        *Zone.ZoneName.ToString(), Zone.ZoneCenter.X, Zone.ZoneCenter.Y, Zone.ZoneCenter.Z, Zone.DangerRadius);
}

void UAudio_AdaptiveMusicComponent::UnregisterDangerZone(FName ZoneName)
{
    RegisteredDangerZones.RemoveAll([&ZoneName](const FAudio_DangerZone& Z) {
        return Z.ZoneName == ZoneName;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_DialogueVoiceComponent
// ─────────────────────────────────────────────────────────────────────────────

UAudio_DialogueVoiceComponent::UAudio_DialogueVoiceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsPlayingDialogue = false;
    DialogueVolume = 1.0f;
    DialogueFadeInTime = 0.1f;
    DialogueFadeOutTime = 0.3f;
    ActiveAudioComponent = nullptr;
}

void UAudio_DialogueVoiceComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_DialogueVoiceComponent::RegisterVoiceLine(FName SpeakerName, int32 LineIndex, USoundBase* AudioAsset)
{
    if (!AudioAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioDialogue: Cannot register null audio for %s line %d"), *SpeakerName.ToString(), LineIndex);
        return;
    }

    FAudio_VoiceLineKey Key;
    Key.SpeakerName = SpeakerName;
    Key.LineIndex   = LineIndex;

    VoiceLineRegistry.Add(Key, AudioAsset);
    UE_LOG(LogTemp, Log, TEXT("AudioDialogue: Registered voice line — %s[%d]"), *SpeakerName.ToString(), LineIndex);
}

void UAudio_DialogueVoiceComponent::PlayDialogueLine(FName SpeakerName, int32 LineIndex)
{
    FAudio_VoiceLineKey Key;
    Key.SpeakerName = SpeakerName;
    Key.LineIndex   = LineIndex;

    USoundBase** FoundSound = VoiceLineRegistry.Find(Key);
    if (!FoundSound || !(*FoundSound))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioDialogue: No voice line registered for %s[%d]"), *SpeakerName.ToString(), LineIndex);
        OnDialogueLineStarted.Broadcast(SpeakerName, LineIndex);
        return;
    }

    StopCurrentDialogue();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    ActiveAudioComponent = UGameplayStatics::SpawnSoundAttached(
        *FoundSound,
        Owner->GetRootComponent(),
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::SnapToTarget,
        true,
        DialogueVolume
    );

    if (ActiveAudioComponent)
    {
        bIsPlayingDialogue = true;
        ActiveSpeakerName = SpeakerName;
        ActiveLineIndex   = LineIndex;

        // Bind completion callback
        ActiveAudioComponent->OnAudioFinished.AddDynamic(this, &UAudio_DialogueVoiceComponent::OnDialogueFinished);
        OnDialogueLineStarted.Broadcast(SpeakerName, LineIndex);

        UE_LOG(LogTemp, Log, TEXT("AudioDialogue: Playing %s[%d]"), *SpeakerName.ToString(), LineIndex);
    }
}

void UAudio_DialogueVoiceComponent::StopCurrentDialogue()
{
    if (ActiveAudioComponent && bIsPlayingDialogue)
    {
        ActiveAudioComponent->FadeOut(DialogueFadeOutTime, 0.0f);
        bIsPlayingDialogue = false;
    }
}

void UAudio_DialogueVoiceComponent::OnDialogueFinished()
{
    bIsPlayingDialogue = false;
    OnDialogueLineFinished.Broadcast(ActiveSpeakerName, ActiveLineIndex);
    UE_LOG(LogTemp, Log, TEXT("AudioDialogue: Finished %s[%d]"), *ActiveSpeakerName.ToString(), ActiveLineIndex);
}

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_AmbientZoneComponent
// ─────────────────────────────────────────────────────────────────────────────

UAudio_AmbientZoneComponent::UAudio_AmbientZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ZoneRadius = 800.0f;
    FadeDistance = 200.0f;
    bPlayerInZone = false;
    CurrentVolume = 0.0f;
    TargetVolume  = 0.0f;
    VolumeBlendSpeed = 2.0f;
    ActiveAmbientComponent = nullptr;
}

void UAudio_AmbientZoneComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start ambient sound at zero volume
    if (AmbientSound)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            ActiveAmbientComponent = UGameplayStatics::SpawnSoundAttached(
                AmbientSound,
                Owner->GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                EAttachLocation::SnapToTarget,
                true,
                0.0f  // Start silent, fade in when player enters
            );
        }
    }
}

void UAudio_AmbientZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), Owner->GetActorLocation());

    // Calculate target volume based on distance
    if (Dist <= ZoneRadius - FadeDistance)
    {
        TargetVolume = 1.0f;
        if (!bPlayerInZone)
        {
            bPlayerInZone = true;
            OnPlayerEnteredZone.Broadcast();
        }
    }
    else if (Dist <= ZoneRadius)
    {
        float FadeAlpha = 1.0f - ((Dist - (ZoneRadius - FadeDistance)) / FadeDistance);
        TargetVolume = FMath::Clamp(FadeAlpha, 0.0f, 1.0f);
    }
    else
    {
        TargetVolume = 0.0f;
        if (bPlayerInZone)
        {
            bPlayerInZone = false;
            OnPlayerExitedZone.Broadcast();
        }
    }

    // Smooth volume blend
    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, VolumeBlendSpeed);

    if (ActiveAmbientComponent)
    {
        ActiveAmbientComponent->SetVolumeMultiplier(CurrentVolume);
    }
}
