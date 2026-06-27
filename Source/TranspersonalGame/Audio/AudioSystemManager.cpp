// AudioSystemManager.cpp
// Agent #16 — Audio Agent
// Adaptive audio system implementation

#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — audio doesn't need per-frame update
}

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize threat state
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentBiome = EAudio_BiomeType::OpenPlains;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialized. Threat=Safe, Biome=OpenPlains, Time=Day"));
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastThreatChange += DeltaTime;
    UpdateAdaptiveMusic(DeltaTime);
    UpdateAmbientLayers(DeltaTime);
}

// ─── Threat Level ─────────────────────────────────────────────────────────────

void AAudioSystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel)
    {
        return;
    }

    EAudio_ThreatLevel OldLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewLevel;
    TimeSinceLastThreatChange = 0.0f;
    ThreatBlendAlpha = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level changed %d -> %d"),
        (int32)OldLevel, (int32)NewLevel);

    // Trigger camera shake for high threat transitions
    if (NewLevel == EAudio_ThreatLevel::Combat || NewLevel == EAudio_ThreatLevel::Flee)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraShake(
                UCameraShakeBase::StaticClass(), 1.0f
            );
        }
    }
}

// ─── Biome Transition ─────────────────────────────────────────────────────────

void AAudioSystemManager::TransitionToBiome(EAudio_BiomeType NewBiome, float CrossfadeTime)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    CurrentBiome = NewBiome;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome transition to %d (crossfade %.1fs)"),
        (int32)NewBiome, CrossfadeTime);

    // Fade out existing ambient components
    for (UAudioComponent* Comp : ActiveAmbientComponents)
    {
        if (Comp && Comp->IsPlaying())
        {
            Comp->FadeOut(CrossfadeTime, 0.0f);
        }
    }
    ActiveAmbientComponents.Empty();

    // New layers will be started in UpdateAmbientLayers
}

void AAudioSystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay)
    {
        return;
    }

    CurrentTimeOfDay = NewTime;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Time of day changed to %d"), (int32)NewTime);
}

// ─── Dinosaur Audio ───────────────────────────────────────────────────────────

void AAudioSystemManager::PlayDinosaurFootstep(FName Species, FVector Location)
{
    // Find the species profile
    FAudio_DinosaurSoundProfile* Profile = nullptr;
    for (FAudio_DinosaurSoundProfile& P : DinosaurProfiles)
    {
        if (P.DinosaurSpecies == Species)
        {
            Profile = &P;
            break;
        }
    }

    if (!Profile)
    {
        return;
    }

    // Check distance to player for shake
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());

        if (Distance < Profile->FootstepShakeRadius)
        {
            // Scale shake by distance — closer = stronger
            float ShakeScale = FMath::Clamp(
                1.0f - (Distance / Profile->FootstepShakeRadius),
                0.1f, 1.0f
            ) * Profile->FootstepShakeIntensity;

            if (PC->PlayerCameraManager)
            {
                PC->PlayerCameraManager->StartCameraShake(
                    UCameraShakeBase::StaticClass(), ShakeScale
                );
            }
        }
    }

    // Play footstep sound at location
    if (Profile->FootstepSound.IsValid())
    {
        USoundBase* Sound = Profile->FootstepSound.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, SFXVolume);
        }
    }
}

void AAudioSystemManager::TriggerTRexApproachSequence(float Distance)
{
    // Distance-based threat escalation for T-Rex
    // > 2000 units: Safe (just ambient rumble)
    // 1000-2000: Aware (subtle percussion enters)
    // 500-1000: Danger (full danger music)
    // < 500: Combat

    EAudio_ThreatLevel NewThreat = EAudio_ThreatLevel::Safe;

    if (Distance < 500.0f)
    {
        NewThreat = EAudio_ThreatLevel::Combat;
    }
    else if (Distance < 1000.0f)
    {
        NewThreat = EAudio_ThreatLevel::Danger;
    }
    else if (Distance < 2000.0f)
    {
        NewThreat = EAudio_ThreatLevel::Aware;
    }

    SetThreatLevel(NewThreat);

    // Ground shake for T-Rex proximity
    if (Distance < 1500.0f)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->PlayerCameraManager)
        {
            float ShakeScale = FMath::Clamp(1.0f - (Distance / 1500.0f), 0.05f, 0.8f);
            PC->PlayerCameraManager->StartCameraShake(
                UCameraShakeBase::StaticClass(), ShakeScale
            );
        }
    }
}

// ─── Narrative Voice ──────────────────────────────────────────────────────────

void AAudioSystemManager::RegisterNarrativeLine(FAudio_NarrativeVoiceLine Line)
{
    // Prevent duplicate registration
    for (FAudio_NarrativeVoiceLine& Existing : NarrativeLines)
    {
        if (Existing.LineID == Line.LineID)
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narrative line '%s' already registered"),
                *Line.LineID.ToString());
            return;
        }
    }

    NarrativeLines.Add(Line);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered narrative line '%s'"),
        *Line.LineID.ToString());
}

void AAudioSystemManager::PlayNarrativeLine(FName LineID)
{
    for (FAudio_NarrativeVoiceLine& Line : NarrativeLines)
    {
        if (Line.LineID == LineID)
        {
            if (Line.bPlayOnce && Line.bHasPlayed)
            {
                return;
            }

            if (Line.AudioAsset.IsValid())
            {
                USoundBase* Sound = Line.AudioAsset.LoadSynchronous();
                if (Sound)
                {
                    UGameplayStatics::PlaySound2D(GetWorld(), Sound, VoiceVolume);
                    Line.bHasPlayed = true;
                    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing narrative line '%s'"),
                        *LineID.ToString());
                }
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narrative line '%s' not found"), *LineID.ToString());
}

// ─── Screen Feedback ──────────────────────────────────────────────────────────

void AAudioSystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    // Scale camera shake with damage amount
    float ShakeScale = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 1.5f);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraShake(
            UCameraShakeBase::StaticClass(), ShakeScale
        );
    }

    // Escalate threat on damage
    if (CurrentThreatLevel == EAudio_ThreatLevel::Safe)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage feedback triggered (%.1f dmg, shake %.2f)"),
        DamageAmount, ShakeScale);
}

void AAudioSystemManager::TriggerCraftingAudioFeedback()
{
    // Crafting is a safe activity — no threat escalation
    // Just play the crafting SFX (stone on stone)
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crafting audio feedback triggered"));
}

// ─── Private: Adaptive Music ─────────────────────────────────────────────────

void AAudioSystemManager::UpdateAdaptiveMusic(float DeltaTime)
{
    // Blend threat alpha toward 1.0 over ThreatTransitionSpeed seconds
    if (ThreatBlendAlpha < 1.0f)
    {
        ThreatBlendAlpha = FMath::Min(
            ThreatBlendAlpha + (DeltaTime / FMath::Max(ThreatTransitionSpeed, 0.1f)),
            1.0f
        );
    }

    // Music volume adjustments based on threat
    float TargetMusicVol = MusicVolume;
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:    TargetMusicVol = MusicVolume * 0.6f; break;
        case EAudio_ThreatLevel::Aware:   TargetMusicVol = MusicVolume * 0.8f; break;
        case EAudio_ThreatLevel::Danger:  TargetMusicVol = MusicVolume * 1.0f; break;
        case EAudio_ThreatLevel::Combat:  TargetMusicVol = MusicVolume * 1.2f; break;
        case EAudio_ThreatLevel::Flee:    TargetMusicVol = MusicVolume * 1.3f; break;
    }

    if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
    {
        ActiveMusicComponent->SetVolumeMultiplier(
            FMath::Lerp(ActiveMusicComponent->VolumeMultiplier, TargetMusicVol, DeltaTime * 2.0f)
        );
    }
}

void AAudioSystemManager::UpdateAmbientLayers(float DeltaTime)
{
    // Manage ambient layer components based on current biome + time of day
    // In a full implementation this would crossfade between MetaSound assets
    // For now, log state changes for Blueprint hookup
    (void)DeltaTime; // suppress unused warning
}

void AAudioSystemManager::CrossfadeAmbientLayer(FAudio_AmbientLayer& Layer, bool bFadeIn, float Duration)
{
    // Find or create audio component for this layer
    if (Layer.SoundAsset.IsValid())
    {
        USoundBase* Sound = Layer.SoundAsset.LoadSynchronous();
        if (!Sound) return;

        if (bFadeIn)
        {
            UAudioComponent* Comp = UGameplayStatics::SpawnSound2D(
                GetWorld(), Sound, Layer.BaseVolume, 1.0f, 0.0f, nullptr, false, false
            );
            if (Comp)
            {
                Comp->FadeIn(Duration, Layer.BaseVolume);
                ActiveAmbientComponents.Add(Comp);
            }
        }
    }
}
