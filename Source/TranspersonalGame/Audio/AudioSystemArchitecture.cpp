#include "AudioSystemArchitecture.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentEmotionalState = EEmotionalState::Calm;
    CurrentEnvironment = EEnvironmentType::DenseForest;
    bInSuspiciousSilence = false;
    SilenceTimer = 0.0f;
    
    MasterMusicComponent = nullptr;
    EnvironmentComponent = nullptr;
    TensionComponent = nullptr;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Initializing Jurassic Survival Audio System"));
    
    // Create master audio components
    if (UWorld* World = GetWorld())
    {
        // Master music component for adaptive soundtrack
        MasterMusicComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (MasterMusicComponent)
        {
            MasterMusicComponent->bAutoDestroy = false;
            MasterMusicComponent->SetVolumeMultiplier(0.7f); // Base music volume
        }

        // Environment ambient component
        EnvironmentComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (EnvironmentComponent)
        {
            EnvironmentComponent->bAutoDestroy = false;
            EnvironmentComponent->SetVolumeMultiplier(0.5f);
        }

        // Tension layer component
        TensionComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (TensionComponent)
        {
            TensionComponent->bAutoDestroy = false;
            TensionComponent->SetVolumeMultiplier(0.0f); // Start silent
        }
    }

    // Initialize default emotional state
    SetEmotionalState(EEmotionalState::Calm, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Initialization complete"));
}

void UAudioSystemManager::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (CurrentEmotionalState == NewState)
    {
        return; // Already in this state
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio System: Transitioning from %d to %d over %.2f seconds"), 
           (int32)CurrentEmotionalState, (int32)NewState, TransitionTime);

    EEmotionalState PreviousState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;

    // Handle special state transitions
    switch (NewState)
    {
        case EEmotionalState::Terror:
            // Immediate response for terror - no gradual transition
            TransitionTime = FMath::Min(TransitionTime, 0.5f);
            break;
            
        case EEmotionalState::Isolation:
            // Trigger suspicious silence
            TriggerSuspiciousSilence(15.0f);
            break;
            
        case EEmotionalState::Wonder:
            // Break any existing silence
            if (bInSuspiciousSilence)
            {
                BreakSilence(false);
            }
            break;
    }

    TransitionToEmotionalState(NewState, TransitionTime);
}

void UAudioSystemManager::SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime)
{
    if (CurrentEnvironment == NewEnvironment)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio System: Environment transition from %d to %d"), 
           (int32)CurrentEnvironment, (int32)NewEnvironment);

    CurrentEnvironment = NewEnvironment;
    
    // Environment changes should be gradual to maintain immersion
    // Implementation would involve MetaSound parameter changes
    if (EnvironmentComponent && EnvironmentComponent->IsPlaying())
    {
        // Fade out current environment
        EnvironmentComponent->FadeOut(TransitionTime, 0.0f);
        
        // Schedule fade in of new environment
        FTimerHandle EnvironmentTransitionTimer;
        GetWorld()->GetTimerManager().SetTimer(EnvironmentTransitionTimer, 
            [this]()
            {
                // Load and play new environment audio
                UpdateMusicLayers();
            }, 
            TransitionTime, false);
    }
}

void UAudioSystemManager::UpdateThreatProximity(float DistanceToNearestThreat, bool bIsBeingHunted)
{
    // Dynamic threat response based on proximity
    if (bIsBeingHunted)
    {
        SetEmotionalState(EEmotionalState::Terror, 0.2f);
        return;
    }

    // Distance-based emotional state calculation
    if (DistanceToNearestThreat < 500.0f) // Very close
    {
        SetEmotionalState(EEmotionalState::Danger, 1.0f);
    }
    else if (DistanceToNearestThreat < 1500.0f) // Nearby
    {
        SetEmotionalState(EEmotionalState::Tension, 2.0f);
    }
    else if (DistanceToNearestThreat > 5000.0f) // Very far or no threats
    {
        // Check if we should enter isolation state
        if (CurrentEmotionalState != EEmotionalState::Wonder)
        {
            SetEmotionalState(EEmotionalState::Isolation, 4.0f);
        }
    }

    // Update tension layer volume based on proximity
    if (TensionComponent)
    {
        float TensionVolume = FMath::Clamp(1.0f - (DistanceToNearestThreat / 2000.0f), 0.0f, 0.8f);
        TensionComponent->SetVolumeMultiplier(TensionVolume);
    }
}

void UAudioSystemManager::TriggerSuspiciousSilence(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Triggering suspicious silence for %.2f seconds"), Duration);
    
    bInSuspiciousSilence = true;
    SilenceTimer = Duration;

    // Fade out all ambient sounds
    if (EnvironmentComponent && EnvironmentComponent->IsPlaying())
    {
        EnvironmentComponent->FadeOut(2.0f, 0.0f);
    }

    if (MasterMusicComponent && MasterMusicComponent->IsPlaying())
    {
        MasterMusicComponent->FadeOut(3.0f, 0.1f); // Keep very quiet music
    }

    // Set timer for silence break
    GetWorld()->GetTimerManager().SetTimer(SilenceTimerHandle, 
        this, &UAudioSystemManager::HandleSilenceTimeout, Duration, false);
}

void UAudioSystemManager::BreakSilence(bool bWithThreat)
{
    if (!bInSuspiciousSilence)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio System: Breaking silence (threat: %s)"), 
           bWithThreat ? TEXT("true") : TEXT("false"));

    bInSuspiciousSilence = false;
    GetWorld()->GetTimerManager().ClearTimer(SilenceTimerHandle);

    if (bWithThreat)
    {
        // Sudden audio burst - something dangerous revealed
        SetEmotionalState(EEmotionalState::Danger, 0.1f);
    }
    else
    {
        // Gentle return to ambient
        SetEmotionalState(EEmotionalState::Calm, 2.0f);
    }
}

void UAudioSystemManager::RegisterDinosaurAudio(class ADinosaurCharacter* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }

    // Create dedicated audio component for this dinosaur
    SpawnDinosaurAudioComponent(Dinosaur);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Registered dinosaur audio"));
}

void UAudioSystemManager::UpdateDinosaurBehaviorAudio(class ADinosaurCharacter* Dinosaur, EDinosaurBehaviorState BehaviorState)
{
    if (!Dinosaur)
    {
        return;
    }

    // Find the audio component for this dinosaur and update its behavior
    // This would integrate with the dinosaur's AI system to play appropriate sounds
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Updated dinosaur behavior audio to state %d"), (int32)BehaviorState);
}

void UAudioSystemManager::TransitionToEmotionalState(EEmotionalState TargetState, float TransitionTime)
{
    // This is where the MetaSound parameters would be updated
    // Each emotional state has different parameter values for:
    // - Music intensity
    // - Harmonic tension
    // - Rhythm complexity
    // - Filter cutoffs
    // - Reverb settings

    UpdateMusicLayers();
}

void UAudioSystemManager::UpdateMusicLayers()
{
    // Update MetaSound parameters based on current state
    // This would involve setting parameters on the MetaSound assets
    
    if (MasterMusicComponent)
    {
        // Example parameter updates (would be actual MetaSound parameters)
        switch (CurrentEmotionalState)
        {
            case EEmotionalState::Calm:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 0.2f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 0.3f);
                break;
                
            case EEmotionalState::Tension:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 0.6f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 0.5f);
                break;
                
            case EEmotionalState::Danger:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 0.8f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 0.8f);
                break;
                
            case EEmotionalState::Terror:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 1.0f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 1.0f);
                break;
                
            case EEmotionalState::Wonder:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 0.1f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 0.7f);
                break;
                
            case EEmotionalState::Isolation:
                MasterMusicComponent->SetFloatParameter(FName("Tension"), 0.4f);
                MasterMusicComponent->SetFloatParameter(FName("Intensity"), 0.1f);
                break;
        }
    }
}

void UAudioSystemManager::HandleSilenceTimeout()
{
    // Silence has lasted too long - something should happen
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Silence timeout - breaking with potential threat"));
    
    // 70% chance of threat, 30% chance of false alarm
    bool bThreatRevealed = FMath::RandRange(0.0f, 1.0f) < 0.7f;
    BreakSilence(bThreatRevealed);
}

void UAudioSystemManager::SpawnDinosaurAudioComponent(class ADinosaurCharacter* Dinosaur)
{
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* DinosaurAudio = UGameplayStatics::SpawnSoundAttached(
            nullptr, // Will be set based on dinosaur type
            Dinosaur->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            true, // Stop when owner is destroyed
            1.0f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true // Auto destroy
        );

        if (DinosaurAudio)
        {
            DinosaurAudioComponents.Add(DinosaurAudio);
        }
    }
}