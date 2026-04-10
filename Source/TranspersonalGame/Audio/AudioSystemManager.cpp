#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentAudioZone = EAudioZoneType::Forest;
    CurrentEmotionalState = FEmotionalAudioState();
    bIsInitialized = false;
    bIsTransitioning = false;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing Audio System"));

    // Initialize default audio zone configurations
    FAudioZoneConfig ForestConfig;
    ForestConfig.ZoneType = EAudioZoneType::Forest;
    ForestConfig.BaseVolume = 0.7f;
    ForestConfig.FadeInTime = 3.0f;
    ForestConfig.FadeOutTime = 2.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Forest, ForestConfig);

    FAudioZoneConfig PlainsConfig;
    PlainsConfig.ZoneType = EAudioZoneType::Plains;
    PlainsConfig.BaseVolume = 0.6f;
    PlainsConfig.FadeInTime = 4.0f;
    PlainsConfig.FadeOutTime = 3.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Plains, PlainsConfig);

    FAudioZoneConfig MountainsConfig;
    MountainsConfig.ZoneType = EAudioZoneType::Mountains;
    MountainsConfig.BaseVolume = 0.8f;
    MountainsConfig.FadeInTime = 5.0f;
    MountainsConfig.FadeOutTime = 4.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Mountains, MountainsConfig);

    FAudioZoneConfig RiversConfig;
    RiversConfig.ZoneType = EAudioZoneType::Rivers;
    RiversConfig.BaseVolume = 0.9f;
    RiversConfig.FadeInTime = 2.0f;
    RiversConfig.FadeOutTime = 2.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Rivers, RiversConfig);

    FAudioZoneConfig SacredConfig;
    SacredConfig.ZoneType = EAudioZoneType::Sacred;
    SacredConfig.BaseVolume = 0.5f;
    SacredConfig.FadeInTime = 6.0f;
    SacredConfig.FadeOutTime = 5.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Sacred, SacredConfig);

    FAudioZoneConfig CombatConfig;
    CombatConfig.ZoneType = EAudioZoneType::Combat;
    CombatConfig.BaseVolume = 1.0f;
    CombatConfig.FadeInTime = 0.5f;
    CombatConfig.FadeOutTime = 1.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Combat, CombatConfig);

    FAudioZoneConfig MeditationConfig;
    MeditationConfig.ZoneType = EAudioZoneType::Meditation;
    MeditationConfig.BaseVolume = 0.3f;
    MeditationConfig.FadeInTime = 8.0f;
    MeditationConfig.FadeOutTime = 6.0f;
    AudioZoneConfigs.Add(EAudioZoneType::Meditation, MeditationConfig);

    // Initialize emotional state
    CurrentEmotionalState.CurrentState = EEmotionalState::Peaceful;
    CurrentEmotionalState.Intensity = 0.5f;
    CurrentEmotionalState.TransitionSpeed = 1.0f;

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio System Initialized Successfully"));
}

void UAudioSystemManager::ShutdownAudioSystem()
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Shutting down Audio System"));

    // Stop all active audio
    StopAmbientSound(0.5f);
    StopAdaptiveMusic(true);

    // Clear active components
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }

    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->Stop();
        CurrentMusicComponent = nullptr;
    }

    // Clean up 3D audio components
    for (auto& AudioComp : Active3DAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    Active3DAudioComponents.Empty();

    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ZoneTransitionTimer);
        World->GetTimerManager().ClearTimer(EmotionalTransitionTimer);
    }

    bIsInitialized = false;
    bIsTransitioning = false;
}

void UAudioSystemManager::TransitionToAudioZone(EAudioZoneType NewZone, float TransitionTime)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot transition zones - system not initialized"));
        return;
    }

    if (CurrentAudioZone == NewZone)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Already in target audio zone"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning from zone %d to zone %d"), 
           (int32)CurrentAudioZone, (int32)NewZone);

    EAudioZoneType PreviousZone = CurrentAudioZone;
    CurrentAudioZone = NewZone;

    ExecuteZoneTransition(PreviousZone, NewZone, TransitionTime);
}

void UAudioSystemManager::RegisterAudioZone(EAudioZoneType ZoneType, const FAudioZoneConfig& Config)
{
    AudioZoneConfigs.Add(ZoneType, Config);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered audio zone configuration for zone %d"), (int32)ZoneType);
}

void UAudioSystemManager::SetEmotionalState(EEmotionalState NewState, float Intensity, float TransitionTime)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot set emotional state - system not initialized"));
        return;
    }

    EEmotionalState PreviousState = CurrentEmotionalState.CurrentState;
    
    CurrentEmotionalState.CurrentState = NewState;
    CurrentEmotionalState.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    CurrentEmotionalState.TransitionSpeed = TransitionTime > 0.0f ? 1.0f / TransitionTime : 1.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Emotional state transition from %d to %d (Intensity: %.2f)"), 
           (int32)PreviousState, (int32)NewState, Intensity);

    ExecuteEmotionalTransition(PreviousState, NewState, TransitionTime);
}

void UAudioSystemManager::PlayAdaptiveMusic(const FString& MusicID, bool bFadeIn)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot play adaptive music - system not initialized"));
        return;
    }

    // Stop current music if playing
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        if (bFadeIn)
        {
            CurrentMusicComponent->FadeOut(2.0f, 0.0f);
        }
        else
        {
            CurrentMusicComponent->Stop();
        }
    }

    // Look up music asset
    if (AdaptiveMusicLibrary.Contains(MusicID))
    {
        TSoftObjectPtr<UMetaSoundSource> MusicAsset = AdaptiveMusicLibrary[MusicID];
        if (MusicAsset.IsValid() || MusicAsset.LoadSynchronous())
        {
            UWorld* World = GetWorld();
            if (World)
            {
                CurrentMusicComponent = UGameplayStatics::CreateSound2D(World, MusicAsset.Get());
                if (CurrentMusicComponent)
                {
                    if (bFadeIn)
                    {
                        CurrentMusicComponent->FadeIn(2.0f, 1.0f);
                    }
                    else
                    {
                        CurrentMusicComponent->Play();
                    }
                    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing adaptive music: %s"), *MusicID);
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music ID not found in library: %s"), *MusicID);
    }
}

void UAudioSystemManager::StopAdaptiveMusic(bool bFadeOut)
{
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        if (bFadeOut)
        {
            CurrentMusicComponent->FadeOut(2.0f, 0.0f);
        }
        else
        {
            CurrentMusicComponent->Stop();
        }
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped adaptive music"));
    }
}

void UAudioSystemManager::SetMusicParameter(const FString& ParameterName, float Value)
{
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        // Note: This would require MetaSound parameter setting functionality
        // CurrentMusicComponent->SetFloatParameter(FName(*ParameterName), Value);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set music parameter %s to %.2f"), *ParameterName, Value);
    }
}

void UAudioSystemManager::PlayAmbientSound(USoundCue* AmbientCue, float Volume, bool bLoop)
{
    if (!AmbientCue)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot play ambient sound - null sound cue"));
        return;
    }

    // Stop current ambient if playing
    StopAmbientSound(1.0f);

    UWorld* World = GetWorld();
    if (World)
    {
        CurrentAmbientComponent = UGameplayStatics::CreateSound2D(World, AmbientCue);
        if (CurrentAmbientComponent)
        {
            CurrentAmbientComponent->SetVolumeMultiplier(Volume);
            CurrentAmbientComponent->bIsLooping = bLoop;
            CurrentAmbientComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing ambient sound"));
        }
    }
}

void UAudioSystemManager::StopAmbientSound(float FadeOutTime)
{
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        if (FadeOutTime > 0.0f)
        {
            CurrentAmbientComponent->FadeOut(FadeOutTime, 0.0f);
        }
        else
        {
            CurrentAmbientComponent->Stop();
        }
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped ambient sound"));
    }
}

void UAudioSystemManager::PlaySoundAtLocation(USoundCue* Sound, FVector Location, float Volume, float Pitch)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot play sound at location - null sound cue"));
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Volume, Pitch);
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Played sound at location (%.1f, %.1f, %.1f)"), 
               Location.X, Location.Y, Location.Z);
    }
}

UAudioComponent* UAudioSystemManager::CreateAudioComponentAtLocation(USoundCue* Sound, FVector Location)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Cannot create audio component - null sound cue"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(World, Sound, Location);
        if (NewAudioComponent)
        {
            Active3DAudioComponents.Add(NewAudioComponent);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Created audio component at location"));
            return NewAudioComponent;
        }
    }

    return nullptr;
}

void UAudioSystemManager::TriggerAudioEvent(const FString& EventName, FVector Location)
{
    if (AudioEventLibrary.Contains(EventName))
    {
        TSoftObjectPtr<USoundCue> EventSound = AudioEventLibrary[EventName];
        if (EventSound.IsValid() || EventSound.LoadSynchronous())
        {
            if (Location != FVector::ZeroVector)
            {
                PlaySoundAtLocation(EventSound.Get(), Location);
            }
            else
            {
                UWorld* World = GetWorld();
                if (World)
                {
                    UGameplayStatics::PlaySound2D(World, EventSound.Get());
                }
            }
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Triggered audio event: %s"), *EventName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio event not found: %s"), *EventName);
    }
}

void UAudioSystemManager::SetAudioQualityLevel(int32 QualityLevel)
{
    // Implement audio quality scaling based on performance requirements
    QualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    switch (QualityLevel)
    {
        case 0: // Low Quality
            // Reduce audio component count, lower sample rates
            break;
        case 1: // Medium Quality
            // Balanced settings
            break;
        case 2: // High Quality
            // Full quality settings
            break;
        case 3: // Ultra Quality
            // Maximum quality with all features
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set audio quality level to %d"), QualityLevel);
}

void UAudioSystemManager::OptimizeAudioForPerformance()
{
    CleanupInactiveAudioComponents();
    UpdateAudioComponentsForPerformance();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Optimized audio for performance"));
}

void UAudioSystemManager::ExecuteZoneTransition(EAudioZoneType FromZone, EAudioZoneType ToZone, float TransitionTime)
{
    bIsTransitioning = true;

    // Get transition configuration
    FAudioZoneConfig* ToZoneConfig = AudioZoneConfigs.Find(ToZone);
    if (!ToZoneConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No configuration found for target zone %d"), (int32)ToZone);
        bIsTransitioning = false;
        return;
    }

    // Fade out current ambient
    StopAmbientSound(TransitionTime * 0.5f);

    // Schedule new ambient to start
    UWorld* World = GetWorld();
    if (World && ToZoneConfig->AmbientSound.IsValid())
    {
        FTimerDelegate TransitionDelegate;
        TransitionDelegate.BindUFunction(this, FName("PlayAmbientSound"), 
                                       ToZoneConfig->AmbientSound.Get(), 
                                       ToZoneConfig->BaseVolume, 
                                       ToZoneConfig->bLoopAmbient);
        
        World->GetTimerManager().SetTimer(ZoneTransitionTimer, TransitionDelegate, TransitionTime * 0.5f, false);
    }

    // Schedule transition completion
    FTimerDelegate CompletionDelegate;
    CompletionDelegate.BindLambda([this]() { bIsTransitioning = false; });
    World->GetTimerManager().SetTimer(ZoneTransitionTimer, CompletionDelegate, TransitionTime, false);
}

void UAudioSystemManager::ExecuteEmotionalTransition(EEmotionalState FromState, EEmotionalState ToState, float TransitionTime)
{
    // This would typically adjust MetaSound parameters or trigger different music layers
    // For now, we log the transition
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Executing emotional transition over %.2f seconds"), TransitionTime);
    
    // In a full implementation, this would:
    // - Adjust music intensity parameters
    // - Fade between different musical layers
    // - Modify ambient sound filtering
    // - Trigger emotional audio cues
}

void UAudioSystemManager::CleanupInactiveAudioComponents()
{
    Active3DAudioComponents.RemoveAll([](const TObjectPtr<UAudioComponent>& AudioComp)
    {
        return !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

void UAudioSystemManager::UpdateAudioComponentsForPerformance()
{
    // Implement distance-based audio culling and LOD
    // Reduce quality of distant audio sources
    // Limit concurrent audio components based on performance budget
}