#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentConsciousnessState = EAudio_ConsciousnessState::Dormant;
    CurrentEnvironment = EAudio_EnvironmentType::PrimordialForest;
    ConsciousnessIntensity = 0.0f;
    EnvironmentIntensity = 0.7f;
    bIsInitialized = false;
    MasterVolume = 1.0f;
    
    // Initialize audio components as null - they'll be created during initialization
    ConsciousnessAudioComponent = nullptr;
    EnvironmentAudioComponent = nullptr;
    NarrationAudioComponent = nullptr;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Initializing AudioSystemManager..."));

    // Initialize consciousness layers
    InitializeConsciousnessLayers();
    
    // Initialize environment profiles
    InitializeEnvironmentProfiles();
    
    // Initialize narration library
    InitializeNarrationLibrary();

    // Create audio components
    UWorld* World = GetWorld();
    if (World)
    {
        // Create consciousness audio component
        ConsciousnessAudioComponent = NewObject<UAudioComponent>(this);
        if (ConsciousnessAudioComponent)
        {
            ConsciousnessAudioComponent->SetVolumeMultiplier(0.8f);
            ConsciousnessAudioComponent->bAutoActivate = false;
            UE_LOG(LogTemp, Log, TEXT("Consciousness audio component created"));
        }

        // Create environment audio component
        EnvironmentAudioComponent = NewObject<UAudioComponent>(this);
        if (EnvironmentAudioComponent)
        {
            EnvironmentAudioComponent->SetVolumeMultiplier(EnvironmentIntensity);
            EnvironmentAudioComponent->bAutoActivate = false;
            UE_LOG(LogTemp, Log, TEXT("Environment audio component created"));
        }

        // Create narration audio component
        NarrationAudioComponent = NewObject<UAudioComponent>(this);
        if (NarrationAudioComponent)
        {
            NarrationAudioComponent->SetVolumeMultiplier(0.9f);
            NarrationAudioComponent->bAutoActivate = false;
            UE_LOG(LogTemp, Log, TEXT("Narration audio component created"));
        }
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialization complete"));
}

void UAudioSystemManager::ShutdownAudioSystem()
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Shutting down AudioSystemManager..."));

    // Stop all audio components
    if (ConsciousnessAudioComponent && ConsciousnessAudioComponent->IsPlaying())
    {
        ConsciousnessAudioComponent->Stop();
    }
    
    if (EnvironmentAudioComponent && EnvironmentAudioComponent->IsPlaying())
    {
        EnvironmentAudioComponent->Stop();
    }
    
    if (NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }

    // Clear timers
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(ConsciousnessTransitionTimer);
        World->GetTimerManager().ClearTimer(EnvironmentTransitionTimer);
    }

    bIsInitialized = false;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager shutdown complete"));
}

void UAudioSystemManager::SetConsciousnessState(EAudio_ConsciousnessState NewState, float TransitionTime)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager not initialized"));
        return;
    }

    if (NewState == CurrentConsciousnessState)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Transitioning consciousness state from %d to %d over %.2f seconds"), 
           (int32)CurrentConsciousnessState, (int32)NewState, TransitionTime);

    EAudio_ConsciousnessState PreviousState = CurrentConsciousnessState;
    CurrentConsciousnessState = NewState;

    // Start transition
    UWorld* World = GetWorld();
    if (World && TransitionTime > 0.0f)
    {
        World->GetTimerManager().SetTimer(
            ConsciousnessTransitionTimer,
            this,
            &UAudioSystemManager::OnConsciousnessTransitionComplete,
            TransitionTime,
            false
        );
    }
    else
    {
        OnConsciousnessTransitionComplete();
    }

    UpdateConsciousnessAudio();
}

void UAudioSystemManager::UpdateConsciousnessIntensity(float NewIntensity, float BlendTime)
{
    if (!bIsInitialized)
    {
        return;
    }

    NewIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    ConsciousnessIntensity = NewIntensity;

    UE_LOG(LogTemp, Log, TEXT("Updating consciousness intensity to %.2f"), NewIntensity);

    if (ConsciousnessAudioComponent)
    {
        ConsciousnessAudioComponent->SetVolumeMultiplier(NewIntensity * 0.8f);
    }

    UpdateConsciousnessAudio();
}

void UAudioSystemManager::TransitionToEnvironment(EAudio_EnvironmentType NewEnvironment, float TransitionTime)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager not initialized"));
        return;
    }

    if (NewEnvironment == CurrentEnvironment)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Transitioning environment from %d to %d over %.2f seconds"), 
           (int32)CurrentEnvironment, (int32)NewEnvironment, TransitionTime);

    EAudio_EnvironmentType PreviousEnvironment = CurrentEnvironment;
    CurrentEnvironment = NewEnvironment;

    // Start transition
    UWorld* World = GetWorld();
    if (World && TransitionTime > 0.0f)
    {
        World->GetTimerManager().SetTimer(
            EnvironmentTransitionTimer,
            this,
            &UAudioSystemManager::OnEnvironmentTransitionComplete,
            TransitionTime,
            false
        );
    }
    else
    {
        OnEnvironmentTransitionComplete();
    }

    UpdateEnvironmentAudio();
}

void UAudioSystemManager::SetEnvironmentIntensity(float Intensity, float BlendTime)
{
    if (!bIsInitialized)
    {
        return;
    }

    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    EnvironmentIntensity = Intensity;

    UE_LOG(LogTemp, Log, TEXT("Setting environment intensity to %.2f"), Intensity);

    if (EnvironmentAudioComponent)
    {
        EnvironmentAudioComponent->SetVolumeMultiplier(Intensity);
    }

    UpdateEnvironmentAudio();
}

void UAudioSystemManager::PlayNarrationLine(const FString& NarrationKey, bool bInterruptCurrent)
{
    if (!bIsInitialized || !NarrationAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager not initialized or narration component missing"));
        return;
    }

    // Check if we have the narration in our library
    if (!NarrationLibrary.Contains(NarrationKey))
    {
        UE_LOG(LogTemp, Warning, TEXT("Narration key '%s' not found in library"), *NarrationKey);
        return;
    }

    // Stop current narration if interrupting
    if (bInterruptCurrent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }

    // Load and play the narration
    TSoftObjectPtr<USoundWave> NarrationSound = NarrationLibrary[NarrationKey];
    if (NarrationSound.IsValid())
    {
        NarrationAudioComponent->SetSound(NarrationSound.Get());
        NarrationAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("Playing narration: %s"), *NarrationKey);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load narration sound for key: %s"), *NarrationKey);
    }
}

void UAudioSystemManager::StopNarration(float FadeOutTime)
{
    if (!NarrationAudioComponent || !NarrationAudioComponent->IsPlaying())
    {
        return;
    }

    if (FadeOutTime > 0.0f)
    {
        NarrationAudioComponent->FadeOut(FadeOutTime, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("Fading out narration over %.2f seconds"), FadeOutTime);
    }
    else
    {
        NarrationAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("Stopping narration immediately"));
    }
}

void UAudioSystemManager::UpdateMetaSoundParameter(const FString& ParameterName, float Value)
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Updating MetaSound parameter '%s' to %.2f"), *ParameterName, Value);

    // Update parameter on consciousness MetaSound
    if (ConsciousnessAudioComponent && ConsciousnessAudioComponent->GetSound())
    {
        // MetaSound parameter updates would go here
        // This requires the MetaSound to have exposed parameters
    }

    // Update parameter on environment MetaSound
    if (EnvironmentAudioComponent && EnvironmentAudioComponent->GetSound())
    {
        // MetaSound parameter updates would go here
    }
}

void UAudioSystemManager::TriggerMetaSoundEvent(const FString& EventName)
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Triggering MetaSound event: %s"), *EventName);

    // MetaSound event triggers would go here
    // This requires the MetaSound to have exposed trigger inputs
}

void UAudioSystemManager::InitializeConsciousnessLayers()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing consciousness audio layers..."));

    // Initialize each consciousness state with default values
    // These would be configured with actual MetaSound assets in a real implementation
    
    FAudio_ConsciousnessLayer DormantLayer;
    DormantLayer.State = EAudio_ConsciousnessState::Dormant;
    DormantLayer.Intensity = 0.1f;
    DormantLayer.TransitionSpeed = 0.5f;
    ConsciousnessLayers.Add(EAudio_ConsciousnessState::Dormant, DormantLayer);

    FAudio_ConsciousnessLayer AwakeningLayer;
    AwakeningLayer.State = EAudio_ConsciousnessState::Awakening;
    AwakeningLayer.Intensity = 0.3f;
    AwakeningLayer.TransitionSpeed = 1.0f;
    ConsciousnessLayers.Add(EAudio_ConsciousnessState::Awakening, AwakeningLayer);

    FAudio_ConsciousnessLayer AwareLayer;
    AwareLayer.State = EAudio_ConsciousnessState::Aware;
    AwareLayer.Intensity = 0.6f;
    AwareLayer.TransitionSpeed = 1.5f;
    ConsciousnessLayers.Add(EAudio_ConsciousnessState::Aware, AwareLayer);

    FAudio_ConsciousnessLayer TranscendentLayer;
    TranscendentLayer.State = EAudio_ConsciousnessState::Transcendent;
    TranscendentLayer.Intensity = 0.8f;
    TranscendentLayer.TransitionSpeed = 2.0f;
    ConsciousnessLayers.Add(EAudio_ConsciousnessState::Transcendent, TranscendentLayer);

    FAudio_ConsciousnessLayer UnityLayer;
    UnityLayer.State = EAudio_ConsciousnessState::Unity;
    UnityLayer.Intensity = 1.0f;
    UnityLayer.TransitionSpeed = 3.0f;
    ConsciousnessLayers.Add(EAudio_ConsciousnessState::Unity, UnityLayer);

    UE_LOG(LogTemp, Log, TEXT("Consciousness layers initialized with %d entries"), ConsciousnessLayers.Num());
}

void UAudioSystemManager::InitializeEnvironmentProfiles()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing environment audio profiles..."));

    // Initialize each environment type with default values
    
    FAudio_EnvironmentProfile ForestProfile;
    ForestProfile.EnvironmentType = EAudio_EnvironmentType::PrimordialForest;
    ForestProfile.BaseVolume = 0.7f;
    ForestProfile.FadeInTime = 3.0f;
    ForestProfile.FadeOutTime = 2.0f;
    EnvironmentProfiles.Add(EAudio_EnvironmentType::PrimordialForest, ForestProfile);

    FAudio_EnvironmentProfile GroveProfile;
    GroveProfile.EnvironmentType = EAudio_EnvironmentType::SacredGrove;
    GroveProfile.BaseVolume = 0.6f;
    GroveProfile.FadeInTime = 4.0f;
    GroveProfile.FadeOutTime = 3.0f;
    EnvironmentProfiles.Add(EAudio_EnvironmentType::SacredGrove, GroveProfile);

    FAudio_EnvironmentProfile RiverProfile;
    RiverProfile.EnvironmentType = EAudio_EnvironmentType::AncientRiver;
    RiverProfile.BaseVolume = 0.8f;
    RiverProfile.FadeInTime = 2.5f;
    RiverProfile.FadeOutTime = 2.5f;
    EnvironmentProfiles.Add(EAudio_EnvironmentType::AncientRiver, RiverProfile);

    FAudio_EnvironmentProfile CaveProfile;
    CaveProfile.EnvironmentType = EAudio_EnvironmentType::MysticCave;
    CaveProfile.BaseVolume = 0.5f;
    CaveProfile.FadeInTime = 5.0f;
    CaveProfile.FadeOutTime = 4.0f;
    EnvironmentProfiles.Add(EAudio_EnvironmentType::MysticCave, CaveProfile);

    FAudio_EnvironmentProfile PlainProfile;
    PlainProfile.EnvironmentType = EAudio_EnvironmentType::SpiritualPlain;
    PlainProfile.BaseVolume = 0.9f;
    PlainProfile.FadeInTime = 3.5f;
    PlainProfile.FadeOutTime = 2.0f;
    EnvironmentProfiles.Add(EAudio_EnvironmentType::SpiritualPlain, PlainProfile);

    UE_LOG(LogTemp, Log, TEXT("Environment profiles initialized with %d entries"), EnvironmentProfiles.Num());
}

void UAudioSystemManager::InitializeNarrationLibrary()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing narration library..."));

    // Initialize with placeholder entries
    // In a real implementation, these would reference actual sound assets
    NarrationLibrary.Add(TEXT("Intro_Consciousness"), nullptr);
    NarrationLibrary.Add(TEXT("Guide_Awakening"), nullptr);
    NarrationLibrary.Add(TEXT("Wisdom_Ancient"), nullptr);
    NarrationLibrary.Add(TEXT("Transformation_Begin"), nullptr);
    NarrationLibrary.Add(TEXT("Unity_Achievement"), nullptr);

    UE_LOG(LogTemp, Log, TEXT("Narration library initialized with %d entries"), NarrationLibrary.Num());
}

void UAudioSystemManager::UpdateConsciousnessAudio()
{
    if (!bIsInitialized || !ConsciousnessAudioComponent)
    {
        return;
    }

    // Get the current consciousness layer
    if (ConsciousnessLayers.Contains(CurrentConsciousnessState))
    {
        const FAudio_ConsciousnessLayer& Layer = ConsciousnessLayers[CurrentConsciousnessState];
        
        // Update audio component based on layer settings
        float TargetVolume = Layer.Intensity * ConsciousnessIntensity * MasterVolume;
        ConsciousnessAudioComponent->SetVolumeMultiplier(TargetVolume);

        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated consciousness audio - State: %d, Volume: %.2f"), 
               (int32)CurrentConsciousnessState, TargetVolume);
    }
}

void UAudioSystemManager::UpdateEnvironmentAudio()
{
    if (!bIsInitialized || !EnvironmentAudioComponent)
    {
        return;
    }

    // Get the current environment profile
    if (EnvironmentProfiles.Contains(CurrentEnvironment))
    {
        const FAudio_EnvironmentProfile& Profile = EnvironmentProfiles[CurrentEnvironment];
        
        // Update audio component based on profile settings
        float TargetVolume = Profile.BaseVolume * EnvironmentIntensity * MasterVolume;
        EnvironmentAudioComponent->SetVolumeMultiplier(TargetVolume);

        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated environment audio - Type: %d, Volume: %.2f"), 
               (int32)CurrentEnvironment, TargetVolume);
    }
}

void UAudioSystemManager::OnConsciousnessTransitionComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Consciousness transition complete - Current state: %d"), (int32)CurrentConsciousnessState);
    UpdateConsciousnessAudio();
}

void UAudioSystemManager::OnEnvironmentTransitionComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Environment transition complete - Current environment: %d"), (int32)CurrentEnvironment);
    UpdateEnvironmentAudio();
}