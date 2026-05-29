#include "Audio_BiomeAudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "AudioDevice.h"

UAudio_BiomeAudioManager::UAudio_BiomeAudioManager()
{
    // Initialize default values
    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    CreatureVolume = 0.9f;
    MaxTransitionTime = 5.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    TransitionDuration = 3.0f;
    
    WeatherAudioComponent = nullptr;
    CreatureAudioComponent = nullptr;
    ThreatAudioComponent = nullptr;
}

void UAudio_BiomeAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Initializing biome audio system"));
    
    // Initialize default audio state
    CurrentAudioState.CurrentBiome = EBiomeType::Savanna;
    CurrentAudioState.TimeOfDay = 12.0f;
    CurrentAudioState.CurrentWeather = EWeatherType::Clear;
    CurrentAudioState.ThreatLevel = 0.0f;
    CurrentAudioState.NearbyDinosaurCount = 0;
    CurrentAudioState.bIsInCombat = false;
    
    TargetAudioState = CurrentAudioState;
    
    // Initialize biome audio data
    InitializeBiomeAudioData();
    
    // Create audio components
    CreateAudioComponents();
    
    // Load audio assets
    LoadBiomeAudioAssets();
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Initialization complete"));
}

void UAudio_BiomeAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Deinitializing"));
    
    // Stop all audio components
    FadeOutAllAmbient(0.5f);
    
    // Clear component references
    BiomeAudioComponents.Empty();
    WeatherAudioComponent = nullptr;
    CreatureAudioComponent = nullptr;
    ThreatAudioComponent = nullptr;
    
    Super::Deinitialize();
}

void UAudio_BiomeAudioManager::Tick(float DeltaTime)
{
    if (bIsTransitioning)
    {
        UpdateAudioTransition(DeltaTime);
    }
    
    // Update volume based on current state
    UpdateVolumeBasedOnState();
}

bool UAudio_BiomeAudioManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UAudio_BiomeAudioManager::TransitionToBiome(EBiomeType NewBiome, float TransitionTime)
{
    if (NewBiome == CurrentAudioState.CurrentBiome && !bIsTransitioning)
    {
        return; // Already in target biome
    }
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Transitioning to biome %d"), (int32)NewBiome);
    
    FAudio_DynamicAudioState NewState = CurrentAudioState;
    NewState.CurrentBiome = NewBiome;
    
    StartAudioTransition(NewState, FMath::Clamp(TransitionTime, 1.0f, MaxTransitionTime));
}

void UAudio_BiomeAudioManager::UpdateAudioState(const FAudio_DynamicAudioState& NewState)
{
    if (!bIsTransitioning)
    {
        CurrentAudioState = NewState;
        TargetAudioState = NewState;
        ApplyAudioState(CurrentAudioState, 1.0f);
    }
    else
    {
        TargetAudioState = NewState;
    }
}

void UAudio_BiomeAudioManager::SetTimeOfDay(float Hour)
{
    CurrentAudioState.TimeOfDay = FMath::Clamp(Hour, 0.0f, 24.0f);
    TargetAudioState.TimeOfDay = CurrentAudioState.TimeOfDay;
    
    UE_LOG(LogTemp, Verbose, TEXT("UAudio_BiomeAudioManager: Time of day set to %f"), Hour);
}

void UAudio_BiomeAudioManager::SetWeatherType(EWeatherType WeatherType)
{
    if (CurrentAudioState.CurrentWeather != WeatherType)
    {
        CurrentAudioState.CurrentWeather = WeatherType;
        TargetAudioState.CurrentWeather = WeatherType;
        
        UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Weather changed to %d"), (int32)WeatherType);
    }
}

void UAudio_BiomeAudioManager::SetThreatLevel(float ThreatLevel)
{
    float ClampedThreat = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    if (!FMath::IsNearlyEqual(CurrentAudioState.ThreatLevel, ClampedThreat, 0.05f))
    {
        CurrentAudioState.ThreatLevel = ClampedThreat;
        TargetAudioState.ThreatLevel = ClampedThreat;
        
        UE_LOG(LogTemp, Verbose, TEXT("UAudio_BiomeAudioManager: Threat level set to %f"), ClampedThreat);
    }
}

void UAudio_BiomeAudioManager::PlayBiomeAmbient(EBiomeType BiomeType)
{
    if (UAudioComponent** ComponentPtr = BiomeAudioComponents.Find(BiomeType))
    {
        if (UAudioComponent* Component = *ComponentPtr)
        {
            if (!Component->IsPlaying())
            {
                Component->Play();
                UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Started biome ambient for %d"), (int32)BiomeType);
            }
        }
    }
}

void UAudio_BiomeAudioManager::StopBiomeAmbient(EBiomeType BiomeType)
{
    if (UAudioComponent** ComponentPtr = BiomeAudioComponents.Find(BiomeType))
    {
        if (UAudioComponent* Component = *ComponentPtr)
        {
            if (Component->IsPlaying())
            {
                Component->Stop();
                UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Stopped biome ambient for %d"), (int32)BiomeType);
            }
        }
    }
}

void UAudio_BiomeAudioManager::FadeOutAllAmbient(float FadeTime)
{
    for (auto& ComponentPair : BiomeAudioComponents)
    {
        if (UAudioComponent* Component = ComponentPair.Value)
        {
            if (Component->IsPlaying())
            {
                Component->FadeOut(FadeTime, 0.0f);
            }
        }
    }
    
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->FadeOut(FadeTime, 0.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Fading out all ambient audio"));
}

void UAudio_BiomeAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Master volume set to %f"), MasterVolume);
}

void UAudio_BiomeAudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Ambient volume set to %f"), AmbientVolume);
}

void UAudio_BiomeAudioManager::SetCreatureVolume(float Volume)
{
    CreatureVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Creature volume set to %f"), CreatureVolume);
}

void UAudio_BiomeAudioManager::InitializeBiomeAudioData()
{
    // Initialize audio data for each biome type
    for (int32 i = 0; i < (int32)EBiomeType::MAX; ++i)
    {
        EBiomeType BiomeType = (EBiomeType)i;
        FAudio_BiomeAudioData& AudioData = BiomeAudioData.FindOrAdd(BiomeType);
        
        // Set biome-specific defaults
        switch (BiomeType)
        {
            case EBiomeType::Savanna:
                AudioData.BaseVolume = 0.7f;
                AudioData.FadeInTime = 3.0f;
                AudioData.FadeOutTime = 2.0f;
                break;
                
            case EBiomeType::Forest:
                AudioData.BaseVolume = 0.8f;
                AudioData.FadeInTime = 4.0f;
                AudioData.FadeOutTime = 3.0f;
                break;
                
            case EBiomeType::Swamp:
                AudioData.BaseVolume = 0.6f;
                AudioData.FadeInTime = 5.0f;
                AudioData.FadeOutTime = 4.0f;
                break;
                
            case EBiomeType::Desert:
                AudioData.BaseVolume = 0.5f;
                AudioData.FadeInTime = 2.0f;
                AudioData.FadeOutTime = 1.5f;
                break;
                
            case EBiomeType::Mountain:
                AudioData.BaseVolume = 0.6f;
                AudioData.FadeInTime = 3.5f;
                AudioData.FadeOutTime = 2.5f;
                break;
                
            default:
                AudioData.BaseVolume = 0.7f;
                AudioData.FadeInTime = 3.0f;
                AudioData.FadeOutTime = 2.0f;
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Initialized audio data for %d biomes"), BiomeAudioData.Num());
}

void UAudio_BiomeAudioManager::CreateAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_BiomeAudioManager: No world available for audio components"));
        return;
    }
    
    // Create biome-specific audio components
    for (auto& AudioDataPair : BiomeAudioData)
    {
        EBiomeType BiomeType = AudioDataPair.Key;
        
        UAudioComponent* AudioComponent = NewObject<UAudioComponent>(this);
        if (AudioComponent)
        {
            AudioComponent->bAutoActivate = false;
            AudioComponent->bIsUISound = false;
            AudioComponent->VolumeMultiplier = AudioDataPair.Value.BaseVolume;
            
            BiomeAudioComponents.Add(BiomeType, AudioComponent);
            
            UE_LOG(LogTemp, Verbose, TEXT("UAudio_BiomeAudioManager: Created audio component for biome %d"), (int32)BiomeType);
        }
    }
    
    // Create weather audio component
    WeatherAudioComponent = NewObject<UAudioComponent>(this);
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->bAutoActivate = false;
        WeatherAudioComponent->bIsUISound = false;
        WeatherAudioComponent->VolumeMultiplier = 0.6f;
    }
    
    // Create creature audio component
    CreatureAudioComponent = NewObject<UAudioComponent>(this);
    if (CreatureAudioComponent)
    {
        CreatureAudioComponent->bAutoActivate = false;
        CreatureAudioComponent->bIsUISound = false;
        CreatureAudioComponent->VolumeMultiplier = CreatureVolume;
    }
    
    // Create threat audio component
    ThreatAudioComponent = NewObject<UAudioComponent>(this);
    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->bAutoActivate = false;
        ThreatAudioComponent->bIsUISound = false;
        ThreatAudioComponent->VolumeMultiplier = 0.8f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Created %d audio components"), BiomeAudioComponents.Num() + 3);
}

void UAudio_BiomeAudioManager::UpdateAudioTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        CompleteAudioTransition();
        return;
    }
    
    // Apply transition curve
    float BlendWeight = CalculateTransitionCurve(TransitionProgress);
    
    // Blend between current and target states
    FAudio_DynamicAudioState BlendedState;
    BlendedState.CurrentBiome = (BlendWeight > 0.5f) ? TargetAudioState.CurrentBiome : CurrentAudioState.CurrentBiome;
    BlendedState.TimeOfDay = FMath::Lerp(CurrentAudioState.TimeOfDay, TargetAudioState.TimeOfDay, BlendWeight);
    BlendedState.CurrentWeather = (BlendWeight > 0.5f) ? TargetAudioState.CurrentWeather : CurrentAudioState.CurrentWeather;
    BlendedState.ThreatLevel = FMath::Lerp(CurrentAudioState.ThreatLevel, TargetAudioState.ThreatLevel, BlendWeight);
    BlendedState.NearbyDinosaurCount = FMath::RoundToInt(FMath::Lerp((float)CurrentAudioState.NearbyDinosaurCount, (float)TargetAudioState.NearbyDinosaurCount, BlendWeight));
    BlendedState.bIsInCombat = (BlendWeight > 0.5f) ? TargetAudioState.bIsInCombat : CurrentAudioState.bIsInCombat;
    
    ApplyAudioState(BlendedState, BlendWeight);
}

void UAudio_BiomeAudioManager::ApplyAudioState(const FAudio_DynamicAudioState& AudioState, float BlendWeight)
{
    // Apply biome ambient audio
    PlayBiomeAmbient(AudioState.CurrentBiome);
    
    // Stop other biome ambients
    for (auto& ComponentPair : BiomeAudioComponents)
    {
        if (ComponentPair.Key != AudioState.CurrentBiome)
        {
            StopBiomeAmbient(ComponentPair.Key);
        }
    }
    
    // Update volume based on threat level and time of day
    UpdateVolumeBasedOnState();
}

void UAudio_BiomeAudioManager::UpdateVolumeBasedOnState()
{
    float TimeMultiplier = 1.0f;
    
    // Reduce ambient volume at night (20:00 - 06:00)
    if (CurrentAudioState.TimeOfDay >= 20.0f || CurrentAudioState.TimeOfDay <= 6.0f)
    {
        TimeMultiplier = 0.6f;
    }
    
    // Increase threat audio based on threat level
    float ThreatMultiplier = FMath::Lerp(0.2f, 1.0f, CurrentAudioState.ThreatLevel);
    
    // Apply volume to current biome
    if (UAudioComponent** ComponentPtr = BiomeAudioComponents.Find(CurrentAudioState.CurrentBiome))
    {
        if (UAudioComponent* Component = *ComponentPtr)
        {
            float FinalVolume = MasterVolume * AmbientVolume * TimeMultiplier;
            Component->SetVolumeMultiplier(FinalVolume);
        }
    }
    
    // Apply threat audio volume
    if (ThreatAudioComponent)
    {
        float ThreatVolume = MasterVolume * ThreatMultiplier;
        ThreatAudioComponent->SetVolumeMultiplier(ThreatVolume);
    }
}

void UAudio_BiomeAudioManager::LoadBiomeAudioAssets()
{
    // Note: In a real implementation, you would load actual sound assets here
    // For now, we'll use placeholder logic
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Audio asset loading placeholder - implement with real sound cues"));
    
    // Example of how to load a sound cue:
    // USoundCue* SavannaCue = LoadSoundCueAsset(TEXT("/Game/Audio/Biomes/Savanna_Ambient_Cue"));
    // if (SavannaCue && BiomeAudioComponents.Contains(EBiomeType::Savanna))
    // {
    //     BiomeAudioComponents[EBiomeType::Savanna]->SetSound(SavannaCue);
    // }
}

USoundCue* UAudio_BiomeAudioManager::LoadSoundCueAsset(const FString& AssetPath)
{
    return LoadObject<USoundCue>(nullptr, *AssetPath);
}

void UAudio_BiomeAudioManager::StartAudioTransition(const FAudio_DynamicAudioState& NewState, float Duration)
{
    TargetAudioState = NewState;
    TransitionDuration = Duration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Started audio transition (duration: %f)"), Duration);
}

void UAudio_BiomeAudioManager::CompleteAudioTransition()
{
    CurrentAudioState = TargetAudioState;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    
    ApplyAudioState(CurrentAudioState, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_BiomeAudioManager: Audio transition completed"));
}

float UAudio_BiomeAudioManager::CalculateTransitionCurve(float Progress) const
{
    // Smooth step curve for natural audio transitions
    return Progress * Progress * (3.0f - 2.0f * Progress);
}