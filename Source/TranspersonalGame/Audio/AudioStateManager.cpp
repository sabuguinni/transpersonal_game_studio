#include "AudioStateManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioStateManager, Log, All);

AAudioStateManager::AAudioStateManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudioComponent"));
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Attach audio components
    MusicAudioComponent->SetupAttachment(RootComponent);
    AmbienceAudioComponent->SetupAttachment(RootComponent);
    ThreatAudioComponent->SetupAttachment(RootComponent);
    
    // Initialize default values
    CurrentEmotionalState = EAudioEmotionalState::Exploration;
    CurrentThreatLevel = EThreatLevel::Safe;
    CurrentBiome = EBiomeType::DenseForest;
    CurrentStressLevel = 0.0f;
    TimeOfDay = 0.5f; // Noon
    
    LastMusicIntensity = 0.0f;
    LastAmbienceLevel = 1.0f;
    LastThreatLevel = 0.0f;
    
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    
    // Initialize default mix settings
    InitializeDefaultMixSettings();
}

void AAudioStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAudioStateManager, Log, TEXT("AudioStateManager initialized"));
    
    // Set initial audio state
    UpdateAudioMix();
    UpdateMetaSoundParameters();
    
    // Start with exploration ambience
    if (AmbienceAudioComponent && EnvironmentalAmbience)
    {
        AmbienceAudioComponent->SetSound(EnvironmentalAmbience);
        AmbienceAudioComponent->Play();
    }
}

void AAudioStateManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        float TransitionAlpha = FMath::Clamp(StateTransitionTimer / TargetMixSettings.TransitionTime, 0.0f, 1.0f);
        
        // Interpolate mix settings
        FAudioMixSettings InterpolatedSettings;
        InterpolatedSettings.MusicVolume = FMath::Lerp(CurrentMixSettings.MusicVolume, TargetMixSettings.MusicVolume, TransitionAlpha);
        InterpolatedSettings.AmbienceVolume = FMath::Lerp(CurrentMixSettings.AmbienceVolume, TargetMixSettings.AmbienceVolume, TransitionAlpha);
        InterpolatedSettings.DinosaurVolume = FMath::Lerp(CurrentMixSettings.DinosaurVolume, TargetMixSettings.DinosaurVolume, TransitionAlpha);
        InterpolatedSettings.PlayerVolume = FMath::Lerp(CurrentMixSettings.PlayerVolume, TargetMixSettings.PlayerVolume, TransitionAlpha);
        InterpolatedSettings.InteractiveVolume = FMath::Lerp(CurrentMixSettings.InteractiveVolume, TargetMixSettings.InteractiveVolume, TransitionAlpha);
        InterpolatedSettings.DialogueVolume = FMath::Lerp(CurrentMixSettings.DialogueVolume, TargetMixSettings.DialogueVolume, TransitionAlpha);
        
        ApplyMixSettings(InterpolatedSettings);
        
        if (TransitionAlpha >= 1.0f)
        {
            bIsTransitioning = false;
            CurrentMixSettings = TargetMixSettings;
        }
    }
    
    // Update threat level based on tracked predators
    CalculateThreatLevel();
    
    // Update MetaSound parameters
    UpdateMetaSoundParameters();
}

void AAudioStateManager::SetEmotionalState(EAudioEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        EAudioEmotionalState PreviousState = CurrentEmotionalState;
        CurrentEmotionalState = NewState;
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Audio emotional state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
        
        UpdateAudioMix();
        OnAudioStateChanged.Broadcast(NewState);
    }
}

void AAudioStateManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        EThreatLevel PreviousLevel = CurrentThreatLevel;
        CurrentThreatLevel = NewThreatLevel;
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Threat level changed from %d to %d"), (int32)PreviousLevel, (int32)NewThreatLevel);
        
        UpdateAudioMix();
        OnThreatLevelChanged.Broadcast(NewThreatLevel);
        
        // Play threat-specific audio cues
        HandleThreatLevelChange(NewThreatLevel);
    }
}

void AAudioStateManager::SetBiome(EBiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        EBiomeType PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Biome changed from %d to %d"), (int32)PreviousBiome, (int32)NewBiome);
        
        UpdateAudioMix();
        OnBiomeChanged.Broadcast(NewBiome);
        
        // Transition environmental ambience
        TransitionBiomeAmbience(NewBiome);
    }
}

void AAudioStateManager::SetStressLevel(float NewStressLevel)
{
    CurrentStressLevel = FMath::Clamp(NewStressLevel, 0.0f, 1.0f);
    
    // Stress affects music intensity and threat audio
    if (MusicAudioComponent)
    {
        float MusicIntensity = FMath::Lerp(0.3f, 1.0f, CurrentStressLevel);
        SetMusicIntensity(MusicIntensity);
    }
}

void AAudioStateManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    
    // Time of day affects ambient sounds and music mood
    UpdateTimeBasedAudio();
}

void AAudioStateManager::RegisterPredatorProximity(class ADinosaurCharacter* Predator, float Distance)
{
    if (Predator)
    {
        TrackedPredators.Add(Predator, Distance);
        UE_LOG(LogAudioStateManager, Log, TEXT("Registered predator proximity: %s at distance %f"), *Predator->GetName(), Distance);
    }
}

void AAudioStateManager::UnregisterPredatorProximity(class ADinosaurCharacter* Predator)
{
    if (Predator && TrackedPredators.Contains(Predator))
    {
        TrackedPredators.Remove(Predator);
        UE_LOG(LogAudioStateManager, Log, TEXT("Unregistered predator proximity: %s"), *Predator->GetName());
    }
}

void AAudioStateManager::TriggerPredatorAudio(class ADinosaurCharacter* Predator, FVector Location)
{
    if (ThreatAudioComponent && ThreatDetectionAudio)
    {
        // Play directional threat audio
        ThreatAudioComponent->SetWorldLocation(Location);
        ThreatAudioComponent->SetSound(ThreatDetectionAudio);
        ThreatAudioComponent->Play();
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Triggered predator audio at location: %s"), *Location.ToString());
    }
}

void AAudioStateManager::PlayMusicalStinger(class USoundCue* Stinger)
{
    if (Stinger)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Stinger, GetActorLocation());
        UE_LOG(LogAudioStateManager, Log, TEXT("Played musical stinger: %s"), *Stinger->GetName());
    }
}

void AAudioStateManager::SetMusicIntensity(float Intensity)
{
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (FMath::Abs(ClampedIntensity - LastMusicIntensity) > 0.01f)
    {
        LastMusicIntensity = ClampedIntensity;
        
        // Update MetaSound parameter
        if (MusicAudioComponent && AdaptiveMusicController)
        {
            MusicAudioComponent->SetFloatParameter(FName("Intensity"), ClampedIntensity);
        }
    }
}

void AAudioStateManager::CrossfadeToMusicState(FName StateName, float CrossfadeTime)
{
    if (MusicAudioComponent && AdaptiveMusicController)
    {
        // Trigger MetaSound state change
        MusicAudioComponent->SetTriggerParameter(FName("ChangeState"));
        MusicAudioComponent->SetStringParameter(FName("TargetState"), StateName.ToString());
        MusicAudioComponent->SetFloatParameter(FName("CrossfadeTime"), CrossfadeTime);
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Crossfading to music state: %s over %f seconds"), *StateName.ToString(), CrossfadeTime);
    }
}

void AAudioStateManager::UpdateWeatherAudio(float WindIntensity, float RainIntensity, float ThunderProbability)
{
    if (AmbienceAudioComponent && EnvironmentalAmbience)
    {
        // Update weather parameters in MetaSound
        AmbienceAudioComponent->SetFloatParameter(FName("WindIntensity"), WindIntensity);
        AmbienceAudioComponent->SetFloatParameter(FName("RainIntensity"), RainIntensity);
        AmbienceAudioComponent->SetFloatParameter(FName("ThunderProbability"), ThunderProbability);
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Updated weather audio - Wind: %f, Rain: %f, Thunder: %f"), WindIntensity, RainIntensity, ThunderProbability);
    }
}

void AAudioStateManager::PlayEnvironmentalOneShot(class USoundCue* Sound, FVector Location, float VolumeMultiplier)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, VolumeMultiplier);
        UE_LOG(LogAudioStateManager, Log, TEXT("Played environmental one-shot: %s at %s"), *Sound->GetName(), *Location.ToString());
    }
}

void AAudioStateManager::UpdateAudioMix()
{
    // Combine settings from different states
    FAudioMixSettings CombinedSettings = CalculateCombinedMixSettings();
    
    // Start transition to new settings
    TargetMixSettings = CombinedSettings;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = true;
}

void AAudioStateManager::CalculateThreatLevel()
{
    float ClosestPredatorDistance = FLT_MAX;
    int32 PredatorCount = 0;
    
    // Clean up invalid predator references
    TArray<ADinosaurCharacter*> InvalidPredators;
    for (auto& PredatorPair : TrackedPredators)
    {
        if (!IsValid(PredatorPair.Key))
        {
            InvalidPredators.Add(PredatorPair.Key);
        }
        else
        {
            PredatorCount++;
            ClosestPredatorDistance = FMath::Min(ClosestPredatorDistance, PredatorPair.Value);
        }
    }
    
    // Remove invalid references
    for (ADinosaurCharacter* InvalidPredator : InvalidPredators)
    {
        TrackedPredators.Remove(InvalidPredator);
    }
    
    // Calculate threat level based on proximity and count
    EThreatLevel NewThreatLevel = EThreatLevel::Safe;
    
    if (PredatorCount > 0)
    {
        if (ClosestPredatorDistance < 500.0f) // Critical range
        {
            NewThreatLevel = EThreatLevel::Critical;
        }
        else if (ClosestPredatorDistance < 1000.0f) // Danger range
        {
            NewThreatLevel = EThreatLevel::Danger;
        }
        else if (ClosestPredatorDistance < 2000.0f) // Awareness range
        {
            NewThreatLevel = EThreatLevel::Awareness;
        }
    }
    
    // Multiple predators increase threat level
    if (PredatorCount > 2 && NewThreatLevel < EThreatLevel::Critical)
    {
        NewThreatLevel = static_cast<EThreatLevel>(static_cast<int32>(NewThreatLevel) + 1);
    }
    
    SetThreatLevel(NewThreatLevel);
}

void AAudioStateManager::UpdateMetaSoundParameters()
{
    // Update all MetaSound parameters based on current state
    if (MusicAudioComponent && AdaptiveMusicController)
    {
        MusicAudioComponent->SetFloatParameter(FName("EmotionalState"), static_cast<float>(CurrentEmotionalState));
        MusicAudioComponent->SetFloatParameter(FName("ThreatLevel"), static_cast<float>(CurrentThreatLevel));
        MusicAudioComponent->SetFloatParameter(FName("StressLevel"), CurrentStressLevel);
        MusicAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeOfDay);
    }
    
    if (AmbienceAudioComponent && EnvironmentalAmbience)
    {
        AmbienceAudioComponent->SetFloatParameter(FName("BiomeType"), static_cast<float>(CurrentBiome));
        AmbienceAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeOfDay);
        AmbienceAudioComponent->SetFloatParameter(FName("ThreatLevel"), static_cast<float>(CurrentThreatLevel));
    }
    
    if (ThreatAudioComponent && ThreatDetectionAudio)
    {
        ThreatAudioComponent->SetFloatParameter(FName("ThreatLevel"), static_cast<float>(CurrentThreatLevel));
        ThreatAudioComponent->SetFloatParameter(FName("PredatorCount"), static_cast<float>(TrackedPredators.Num()));
    }
}

void AAudioStateManager::InitializeDefaultMixSettings()
{
    // Default mix settings for emotional states
    FAudioMixSettings ExplorationMix;
    ExplorationMix.MusicVolume = 0.7f;
    ExplorationMix.AmbienceVolume = 1.0f;
    ExplorationMix.DinosaurVolume = 0.8f;
    ExplorationMix.PlayerVolume = 1.0f;
    ExplorationMix.InteractiveVolume = 1.0f;
    ExplorationMix.DialogueVolume = 1.0f;
    ExplorationMix.TransitionTime = 3.0f;
    EmotionalStateMixSettings.Add(EAudioEmotionalState::Exploration, ExplorationMix);
    
    FAudioMixSettings PreyMix;
    PreyMix.MusicVolume = 1.0f;
    PreyMix.AmbienceVolume = 0.6f;
    PreyMix.DinosaurVolume = 1.2f;
    PreyMix.PlayerVolume = 0.8f;
    PreyMix.InteractiveVolume = 0.7f;
    PreyMix.DialogueVolume = 1.0f;
    PreyMix.TransitionTime = 1.0f;
    EmotionalStateMixSettings.Add(EAudioEmotionalState::Prey, PreyMix);
    
    FAudioMixSettings DangerMix;
    DangerMix.MusicVolume = 1.2f;
    DangerMix.AmbienceVolume = 0.4f;
    DangerMix.DinosaurVolume = 1.5f;
    DangerMix.PlayerVolume = 0.6f;
    DangerMix.InteractiveVolume = 0.5f;
    DangerMix.DialogueVolume = 1.0f;
    DangerMix.TransitionTime = 0.5f;
    EmotionalStateMixSettings.Add(EAudioEmotionalState::Danger, DangerMix);
    
    // Initialize other state mix settings...
    // (Observation, Domestication states would be added here)
}

FAudioMixSettings AAudioStateManager::CalculateCombinedMixSettings()
{
    FAudioMixSettings CombinedSettings;
    
    // Start with emotional state settings
    if (EmotionalStateMixSettings.Contains(CurrentEmotionalState))
    {
        CombinedSettings = EmotionalStateMixSettings[CurrentEmotionalState];
    }
    
    // Modify based on threat level
    if (ThreatLevelMixSettings.Contains(CurrentThreatLevel))
    {
        FAudioMixSettings ThreatSettings = ThreatLevelMixSettings[CurrentThreatLevel];
        CombinedSettings.MusicVolume *= ThreatSettings.MusicVolume;
        CombinedSettings.DinosaurVolume *= ThreatSettings.DinosaurVolume;
        CombinedSettings.TransitionTime = FMath::Min(CombinedSettings.TransitionTime, ThreatSettings.TransitionTime);
    }
    
    // Modify based on biome
    if (BiomeMixSettings.Contains(CurrentBiome))
    {
        FAudioMixSettings BiomeSettings = BiomeMixSettings[CurrentBiome];
        CombinedSettings.AmbienceVolume *= BiomeSettings.AmbienceVolume;
    }
    
    return CombinedSettings;
}

void AAudioStateManager::ApplyMixSettings(const FAudioMixSettings& Settings)
{
    // Apply volume settings to audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(Settings.MusicVolume);
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(Settings.AmbienceVolume);
    }
    
    // Apply to global audio classes if available
    if (UWorld* World = GetWorld())
    {
        if (FAudioDevice* AudioDevice = World->GetAudioDevice().GetAudioDevice())
        {
            // Set audio class volumes (would need audio class references)
            // AudioDevice->SetClassVolume(DinosaurAudioClass, Settings.DinosaurVolume);
            // AudioDevice->SetClassVolume(PlayerAudioClass, Settings.PlayerVolume);
            // etc.
        }
    }
}

void AAudioStateManager::HandleThreatLevelChange(EThreatLevel NewThreatLevel)
{
    switch (NewThreatLevel)
    {
        case EThreatLevel::Critical:
            // Play immediate danger stinger
            CrossfadeToMusicState(FName("CriticalDanger"), 0.2f);
            break;
            
        case EThreatLevel::Danger:
            // Transition to danger music
            CrossfadeToMusicState(FName("Danger"), 1.0f);
            break;
            
        case EThreatLevel::Awareness:
            // Subtle tension music
            CrossfadeToMusicState(FName("Tension"), 2.0f);
            break;
            
        case EThreatLevel::Safe:
            // Return to exploration music
            CrossfadeToMusicState(FName("Exploration"), 3.0f);
            break;
    }
}

void AAudioStateManager::TransitionBiomeAmbience(EBiomeType NewBiome)
{
    if (AmbienceAudioComponent && EnvironmentalAmbience)
    {
        // Trigger biome transition in MetaSound
        AmbienceAudioComponent->SetTriggerParameter(FName("ChangeBiome"));
        AmbienceAudioComponent->SetFloatParameter(FName("TargetBiome"), static_cast<float>(NewBiome));
        
        UE_LOG(LogAudioStateManager, Log, TEXT("Transitioning to biome ambience: %d"), static_cast<int32>(NewBiome));
    }
}

void AAudioStateManager::UpdateTimeBasedAudio()
{
    // Calculate time-based audio parameters
    float DayNightCycle = FMath::Sin(TimeOfDay * PI); // 0 at midnight, 1 at noon
    float NightIntensity = 1.0f - DayNightCycle;
    
    if (AmbienceAudioComponent && EnvironmentalAmbience)
    {
        AmbienceAudioComponent->SetFloatParameter(FName("DayNightCycle"), DayNightCycle);
        AmbienceAudioComponent->SetFloatParameter(FName("NightIntensity"), NightIntensity);
    }
    
    if (MusicAudioComponent && AdaptiveMusicController)
    {
        MusicAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeOfDay);
    }
}