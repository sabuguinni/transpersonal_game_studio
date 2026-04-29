#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"
#include "Engine/GameInstance.h"

UAudioSystemManager::UAudioSystemManager()
{
    // Initialize default settings
    AudioSettings = FAudio_SoundSettings();
    CurrentMusicState = EAudio_MusicState::Calm;
    TargetMusicState = EAudio_MusicState::Calm;
    CurrentAmbienceType = EAudio_AmbienceType::Forest;
    CurrentFearLevel = 0.0f;
    ListenerPosition = FVector::ZeroVector;
    ListenerRotation = FRotator::ZeroRotator;
    
    // Initialize timers
    MusicTransitionTimer = 0.0f;
    MusicTransitionDuration = 2.0f;
    bIsMusicTransitioning = false;
    
    // Initialize components to null
    HeartbeatComponent = nullptr;
    BreathingComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio subsystem"));
    
    // Initialize all audio systems
    InitializeMusicSystem();
    InitializeAmbienceSystem();
    InitializePlayerAudio();
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio subsystem initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Shutting down audio subsystem"));
    
    // Stop all music
    StopAllMusic(0.5f);
    
    // Clean up components
    for (auto& Pair : MusicComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->Stop();
            Pair.Value->DestroyComponent();
        }
    }
    MusicComponents.Empty();
    
    for (auto& Pair : AmbienceComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->Stop();
            Pair.Value->DestroyComponent();
        }
    }
    AmbienceComponents.Empty();
    
    // Clean up player audio
    if (HeartbeatComponent && IsValid(HeartbeatComponent))
    {
        HeartbeatComponent->Stop();
        HeartbeatComponent->DestroyComponent();
        HeartbeatComponent = nullptr;
    }
    
    if (BreathingComponent && IsValid(BreathingComponent))
    {
        BreathingComponent->Stop();
        BreathingComponent->DestroyComponent();
        BreathingComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeMusicSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing music system"));
    
    // Create audio components for each music state
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Failed to get GameInstance"));
        return;
    }
    
    // Initialize music layers with default settings
    MusicLayers.Empty();
    
    // Add default music layers for each state
    FAudio_MusicLayer CalmLayer;
    CalmLayer.MusicState = EAudio_MusicState::Calm;
    CalmLayer.Volume = 0.5f;
    CalmLayer.FadeInTime = 3.0f;
    CalmLayer.FadeOutTime = 3.0f;
    MusicLayers.Add(CalmLayer);
    
    FAudio_MusicLayer TensionLayer;
    TensionLayer.MusicState = EAudio_MusicState::Tension;
    TensionLayer.Volume = 0.7f;
    TensionLayer.FadeInTime = 2.0f;
    TensionLayer.FadeOutTime = 2.0f;
    MusicLayers.Add(TensionLayer);
    
    FAudio_MusicLayer DangerLayer;
    DangerLayer.MusicState = EAudio_MusicState::Danger;
    DangerLayer.Volume = 0.9f;
    DangerLayer.FadeInTime = 1.0f;
    DangerLayer.FadeOutTime = 1.5f;
    MusicLayers.Add(DangerLayer);
    
    FAudio_MusicLayer CombatLayer;
    CombatLayer.MusicState = EAudio_MusicState::Combat;
    CombatLayer.Volume = 1.0f;
    CombatLayer.FadeInTime = 0.5f;
    CombatLayer.FadeOutTime = 2.0f;
    MusicLayers.Add(CombatLayer);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music system initialized with %d layers"), MusicLayers.Num());
}

void UAudioSystemManager::InitializeAmbienceSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing ambience system"));
    
    // Ambience components will be created on-demand when needed
    // This prevents unnecessary resource usage
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Ambience system initialized"));
}

void UAudioSystemManager::InitializePlayerAudio()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing player audio feedback"));
    
    // Player audio components will be created when first needed
    // This allows for proper world context
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Player audio feedback initialized"));
}

void UAudioSystemManager::SetMusicState(EAudio_MusicState NewState, float TransitionTime)
{
    if (NewState == CurrentMusicState && !bIsMusicTransitioning)
    {
        return; // Already in this state
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning music from %d to %d over %.2f seconds"), 
           (int32)CurrentMusicState, (int32)NewState, TransitionTime);
    
    TargetMusicState = NewState;
    MusicTransitionDuration = FMath::Max(0.1f, TransitionTime);
    MusicTransitionTimer = 0.0f;
    bIsMusicTransitioning = true;
    
    // Start the transition
    UpdateMusicTransition(0.0f);
}

void UAudioSystemManager::StopAllMusic(float FadeOutTime)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Stopping all music with %.2f second fade"));
    
    for (auto& Pair : MusicComponents)
    {
        if (Pair.Value && IsValid(Pair.Value) && Pair.Value->IsPlaying())
        {
            Pair.Value->FadeOut(FadeOutTime, 0.0f);
        }
    }
    
    bIsMusicTransitioning = false;
}

void UAudioSystemManager::SetAmbienceType(EAudio_AmbienceType NewType, float TransitionTime)
{
    if (NewType == CurrentAmbienceType)
    {
        return; // Already this type
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Changing ambience from %d to %d"), 
           (int32)CurrentAmbienceType, (int32)NewType);
    
    // Fade out current ambience
    if (AmbienceComponents.Contains(CurrentAmbienceType))
    {
        UAudioComponent* CurrentComponent = AmbienceComponents[CurrentAmbienceType];
        if (CurrentComponent && IsValid(CurrentComponent) && CurrentComponent->IsPlaying())
        {
            CurrentComponent->FadeOut(TransitionTime, 0.0f);
        }
    }
    
    CurrentAmbienceType = NewType;
    
    // Create and start new ambience if needed
    // This would typically load appropriate sound cues based on ambience type
}

void UAudioSystemManager::PlayAmbienceAtLocation(EAudio_AmbienceType AmbienceType, FVector Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing ambience type %d at location %s with radius %.2f"), 
           (int32)AmbienceType, *Location.ToString(), Radius);
    
    // This would create a 3D positioned ambience sound
    // Implementation would depend on available sound assets
}

void UAudioSystemManager::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Cannot play null sound cue"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No world context for playing sound"));
        return;
    }
    
    // Calculate volume based on distance
    float DistanceVolume = CalculateVolumeByDistance(Location, ListenerPosition, AudioSettings.MaxAudioDistance);
    float FinalVolume = VolumeMultiplier * DistanceVolume * AudioSettings.SFXVolume * AudioSettings.MasterVolume;
    
    UGameplayStatics::PlaySoundAtLocation(World, SoundCue, Location, FinalVolume);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Played sound at location %s with volume %.2f"), 
           *Location.ToString(), FinalVolume);
}

void UAudioSystemManager::PlayDinosaurSound(const FString& DinosaurType, FVector Location, float Intensity)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing dinosaur sound '%s' at location %s with intensity %.2f"), 
           *DinosaurType, *Location.ToString(), Intensity);
    
    // This would load and play appropriate dinosaur sounds based on type
    // Different sounds for T-Rex, Raptor, Brachiosaurus, etc.
    
    // For now, just log the request
    float Distance = FVector::Dist(Location, ListenerPosition);
    if (Distance < AudioSettings.MaxAudioDistance)
    {
        // Sound would be audible
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dinosaur sound audible at distance %.2f"), Distance);
    }
}

void UAudioSystemManager::SetPlayerFearLevel(float FearLevel)
{
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    
    if (FMath::Abs(CurrentFearLevel - FearLevel) > 0.01f)
    {
        CurrentFearLevel = FearLevel;
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Player fear level set to %.2f"), CurrentFearLevel);
        
        // Update player audio feedback based on fear level
        UpdatePlayerAudioFeedback(0.0f);
    }
}

void UAudioSystemManager::PlayPlayerHeartbeat(float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing player heartbeat with intensity %.2f"), Intensity);
    
    // This would play heartbeat sound with intensity-based pitch and volume
    // Higher intensity = faster, louder heartbeat
}

void UAudioSystemManager::PlayPlayerBreathing(float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing player breathing with intensity %.2f"), Intensity);
    
    // This would play breathing sound with intensity-based effects
    // Higher intensity = heavier, more labored breathing
}

void UAudioSystemManager::ApplyAudioSettings(const FAudio_SoundSettings& NewSettings)
{
    AudioSettings = NewSettings;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Applied new audio settings - Master: %.2f, Music: %.2f, SFX: %.2f"), 
           AudioSettings.MasterVolume, AudioSettings.MusicVolume, AudioSettings.SFXVolume);
    
    // Update all active audio components with new settings
    for (auto& Pair : MusicComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->SetVolumeMultiplier(AudioSettings.MusicVolume * AudioSettings.MasterVolume);
        }
    }
    
    for (auto& Pair : AmbienceComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->SetVolumeMultiplier(AudioSettings.AmbienceVolume * AudioSettings.MasterVolume);
        }
    }
}

float UAudioSystemManager::CalculateVolumeByDistance(FVector SourceLocation, FVector ListenerLocation, float MaxDistance) const
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f; // Too far to hear
    }
    
    // Linear falloff for now - could be improved with curves
    float VolumeMultiplier = 1.0f - (Distance / MaxDistance);
    return FMath::Clamp(VolumeMultiplier, 0.0f, 1.0f);
}

void UAudioSystemManager::UpdateListenerPosition(FVector NewPosition, FRotator NewRotation)
{
    ListenerPosition = NewPosition;
    ListenerRotation = NewRotation;
    
    // Update 3D audio listener position
    UWorld* World = GetWorld();
    if (World && World->GetAudioDeviceRaw())
    {
        World->GetAudioDeviceRaw()->SetListener(0, NewPosition, FVector::ForwardVector, FVector::UpVector);
    }
}

void UAudioSystemManager::UpdateMusicTransition(float DeltaTime)
{
    if (!bIsMusicTransitioning)
    {
        return;
    }
    
    MusicTransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(MusicTransitionTimer / MusicTransitionDuration, 0.0f, 1.0f);
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentMusicState = TargetMusicState;
        bIsMusicTransitioning = false;
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Music transition completed to state %d"), (int32)CurrentMusicState);
    }
    
    // Update volumes based on transition progress
    // This would fade between music layers smoothly
}

void UAudioSystemManager::UpdatePlayerAudioFeedback(float DeltaTime)
{
    // Update heartbeat and breathing based on current fear level
    if (CurrentFearLevel > 0.1f)
    {
        // Fear is high enough to trigger audio feedback
        float HeartbeatIntensity = FMath::Lerp(0.5f, 2.0f, CurrentFearLevel);
        float BreathingIntensity = FMath::Lerp(0.3f, 1.5f, CurrentFearLevel);
        
        // This would update the actual audio components
        // For now, just log the calculated values
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Fear %.2f -> Heartbeat %.2f, Breathing %.2f"), 
               CurrentFearLevel, HeartbeatIntensity, BreathingIntensity);
    }
}