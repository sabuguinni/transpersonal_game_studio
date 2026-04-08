#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MetasoundSource.h"
#include "AudioParameterControllerInterface.h"
#include "Kismet/GameplayStatics.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default audio state
    CurrentAudioState.CurrentEmotion = EEmotionalState::Calm;
    CurrentAudioState.Environment = EEnvironmentType::DenseForest;
    CurrentAudioState.ThreatLevel = 0.0f;
    CurrentAudioState.TimeOfDay = 0.5f; // Start at noon
    CurrentAudioState.NearbyDinosaurs = 0;
    CurrentAudioState.bPlayerHidden = false;
    CurrentAudioState.PlayerMovementSpeed = 0.0f;

    InitializeMetaSounds();
}

void UAudioSystemManager::Deinitialize()
{
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }

    // Clear parameter controllers
    MusicParameterController.Reset();
    AmbientParameterController.Reset();

    Super::Deinitialize();
}

void UAudioSystemManager::InitializeMetaSounds()
{
    if (!GetWorld())
    {
        return;
    }

    // Create audio components for music and ambient
    MusicAudioComponent = UGameplayStatics::CreateSound2D(GetWorld(), nullptr);
    AmbientAudioComponent = UGameplayStatics::CreateSound2D(GetWorld(), nullptr);

    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoDestroy = false;
        MusicAudioComponent->SetVolumeMultiplier(0.7f); // Start with lower music volume
    }

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoDestroy = false;
        AmbientAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UAudioSystemManager::UpdateAudioState(const FAudioStateData& NewState)
{
    FAudioStateData PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Check for significant changes that require parameter updates
    bool bNeedsUpdate = false;

    if (PreviousState.CurrentEmotion != NewState.CurrentEmotion ||
        PreviousState.ThreatLevel != NewState.ThreatLevel ||
        PreviousState.Environment != NewState.Environment ||
        PreviousState.TimeOfDay != NewState.TimeOfDay)
    {
        bNeedsUpdate = true;
    }

    if (bNeedsUpdate)
    {
        UpdateMusicParameters();
        UpdateAmbientParameters();
    }
}

void UAudioSystemManager::SetEmotionalState(EEmotionalState NewEmotion, float TransitionTime)
{
    if (CurrentAudioState.CurrentEmotion == NewEmotion)
    {
        return;
    }

    CurrentAudioState.CurrentEmotion = NewEmotion;
    CalculateEmotionalTransition(NewEmotion, TransitionTime);
}

void UAudioSystemManager::SetThreatLevel(float NewThreatLevel, float TransitionTime)
{
    NewThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 1.0f);
    
    if (FMath::IsNearlyEqual(CurrentAudioState.ThreatLevel, NewThreatLevel, 0.01f))
    {
        return;
    }

    CurrentAudioState.ThreatLevel = NewThreatLevel;
    
    // Immediate parameter update for threat changes
    UpdateMusicParameters();
}

void UAudioSystemManager::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.Environment == NewEnvironment)
    {
        return;
    }

    CurrentAudioState.Environment = NewEnvironment;
    UpdateAmbientParameters();
}

void UAudioSystemManager::OnDinosaurSpotted(class ADinosaurCharacter* Dinosaur, float Distance)
{
    if (!Dinosaur)
    {
        return;
    }

    // Add to tracked dinosaurs
    NearbyDinosaurs.AddUnique(Dinosaur);
    CurrentAudioState.NearbyDinosaurs = NearbyDinosaurs.Num();

    // Calculate threat based on dinosaur type and distance
    float ThreatContribution = CalculateThreatFromDinosaurs();
    SetThreatLevel(ThreatContribution, 0.5f);

    // Trigger specific audio cues based on dinosaur type
    // This would be expanded with specific dinosaur audio logic
}

void UAudioSystemManager::OnDinosaurLost(class ADinosaurCharacter* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }

    NearbyDinosaurs.RemoveAll([Dinosaur](const TWeakObjectPtr<class ADinosaurCharacter>& WeakDino)
    {
        return !WeakDino.IsValid() || WeakDino.Get() == Dinosaur;
    });

    CurrentAudioState.NearbyDinosaurs = NearbyDinosaurs.Num();

    // Recalculate threat
    float ThreatContribution = CalculateThreatFromDinosaurs();
    SetThreatLevel(ThreatContribution, 1.0f);
}

void UAudioSystemManager::OnPlayerHiding(bool bIsHiding)
{
    CurrentAudioState.bPlayerHidden = bIsHiding;
    
    // When hiding, reduce music volume and change ambient parameters
    if (MusicAudioComponent)
    {
        float TargetVolume = bIsHiding ? 0.3f : 0.7f;
        MusicAudioComponent->SetVolumeMultiplier(TargetVolume);
    }

    UpdateAmbientParameters();
}

void UAudioSystemManager::PlayAdaptiveMusic()
{
    if (!AdaptiveMusicMetaSound || !MusicAudioComponent)
    {
        return;
    }

    MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
    MusicAudioComponent->Play();

    // Initialize parameter controller for real-time control
    if (MusicAudioComponent->GetAudioComponentID() != 0)
    {
        MusicParameterController = Audio::FParameterControllerInterface::CreateParameterController(
            MusicAudioComponent->GetAudioComponentID()
        );
    }

    UpdateMusicParameters();
}

void UAudioSystemManager::StopAdaptiveMusic(float FadeTime)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FadeTime, 0.0f);
    }

    MusicParameterController.Reset();
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicParameterController.IsValid())
    {
        return;
    }

    // Map emotional state to musical parameters
    float EmotionalIntensity = 0.0f;
    float MusicalTension = 0.0f;
    float Dissonance = 0.0f;

    switch (CurrentAudioState.CurrentEmotion)
    {
        case EEmotionalState::Calm:
            EmotionalIntensity = 0.2f;
            MusicalTension = 0.1f;
            Dissonance = 0.0f;
            break;
        case EEmotionalState::Tension:
            EmotionalIntensity = 0.5f;
            MusicalTension = 0.6f;
            Dissonance = 0.3f;
            break;
        case EEmotionalState::Fear:
            EmotionalIntensity = 0.8f;
            MusicalTension = 0.8f;
            Dissonance = 0.6f;
            break;
        case EEmotionalState::Panic:
            EmotionalIntensity = 1.0f;
            MusicalTension = 1.0f;
            Dissonance = 0.9f;
            break;
        case EEmotionalState::Wonder:
            EmotionalIntensity = 0.6f;
            MusicalTension = 0.2f;
            Dissonance = 0.1f;
            break;
        case EEmotionalState::Melancholy:
            EmotionalIntensity = 0.4f;
            MusicalTension = 0.3f;
            Dissonance = 0.2f;
            break;
    }

    // Apply threat level modulation
    float ThreatModulation = CurrentAudioState.ThreatLevel;
    EmotionalIntensity = FMath::Lerp(EmotionalIntensity, 1.0f, ThreatModulation * 0.5f);
    MusicalTension = FMath::Lerp(MusicalTension, 1.0f, ThreatModulation);
    Dissonance = FMath::Lerp(Dissonance, 1.0f, ThreatModulation * 0.7f);

    // Send parameters to MetaSound
    MusicParameterController->SetFloatParameter(FName("EmotionalIntensity"), EmotionalIntensity);
    MusicParameterController->SetFloatParameter(FName("MusicalTension"), MusicalTension);
    MusicParameterController->SetFloatParameter(FName("Dissonance"), Dissonance);
    MusicParameterController->SetFloatParameter(FName("ThreatLevel"), CurrentAudioState.ThreatLevel);
    MusicParameterController->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay);
}

void UAudioSystemManager::UpdateAmbientParameters()
{
    if (!AmbientParameterController.IsValid())
    {
        return;
    }

    // Environment-specific ambient parameters
    float ForestDensity = 0.0f;
    float WaterProximity = 0.0f;
    float CaveReverb = 0.0f;

    switch (CurrentAudioState.Environment)
    {
        case EEnvironmentType::DenseForest:
            ForestDensity = 1.0f;
            break;
        case EEnvironmentType::OpenPlains:
            ForestDensity = 0.2f;
            break;
        case EEnvironmentType::Riverside:
            ForestDensity = 0.6f;
            WaterProximity = 1.0f;
            break;
        case EEnvironmentType::Cave:
            ForestDensity = 0.0f;
            CaveReverb = 1.0f;
            break;
        case EEnvironmentType::DinosaurNest:
            ForestDensity = 0.8f;
            break;
    }

    // Send parameters to ambient MetaSound
    AmbientParameterController->SetFloatParameter(FName("ForestDensity"), ForestDensity);
    AmbientParameterController->SetFloatParameter(FName("WaterProximity"), WaterProximity);
    AmbientParameterController->SetFloatParameter(FName("CaveReverb"), CaveReverb);
    AmbientParameterController->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay);
    AmbientParameterController->SetFloatParameter(FName("ThreatLevel"), CurrentAudioState.ThreatLevel);
    AmbientParameterController->SetBoolParameter(FName("PlayerHidden"), CurrentAudioState.bPlayerHidden);
}

void UAudioSystemManager::CalculateEmotionalTransition(EEmotionalState TargetEmotion, float TransitionTime)
{
    // This would implement smooth transitions between emotional states
    // For now, we do immediate transitions, but this could be expanded
    // to create gradual parameter interpolation over time
    
    GetWorld()->GetTimerManager().ClearTimer(EmotionTransitionTimer);
    
    if (TransitionTime > 0.0f)
    {
        // Set up timer for gradual transition
        GetWorld()->GetTimerManager().SetTimer(
            EmotionTransitionTimer,
            [this]() { UpdateMusicParameters(); },
            0.1f, // Update every 100ms during transition
            true
        );
        
        // Clear timer after transition completes
        GetWorld()->GetTimerManager().SetTimer(
            EmotionTransitionTimer,
            [this]() { GetWorld()->GetTimerManager().ClearTimer(EmotionTransitionTimer); },
            TransitionTime,
            false
        );
    }
    else
    {
        UpdateMusicParameters();
    }
}

float UAudioSystemManager::CalculateThreatFromDinosaurs() const
{
    if (NearbyDinosaurs.Num() == 0)
    {
        return 0.0f;
    }

    float TotalThreat = 0.0f;
    int32 ValidDinosaurs = 0;

    for (const TWeakObjectPtr<class ADinosaurCharacter>& WeakDino : NearbyDinosaurs)
    {
        if (WeakDino.IsValid())
        {
            // This would be expanded to consider:
            // - Dinosaur type (predator vs herbivore)
            // - Distance from player
            // - Dinosaur's current behavior state
            // - Player visibility to dinosaur
            
            ValidDinosaurs++;
            TotalThreat += 0.3f; // Base threat per dinosaur
        }
    }

    // Cap threat at 1.0 and apply diminishing returns
    float NormalizedThreat = FMath::Min(TotalThreat, 1.0f);
    return FMath::Pow(NormalizedThreat, 0.7f); // Slight diminishing returns
}