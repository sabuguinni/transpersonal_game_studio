#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default audio state
    CurrentAudioState = FAudioState();
    
    // Initialize audio components
    InitializeAudioComponents();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialized"));
}

void UAudioSystemManager::Deinitialize()
{
    // Clean up audio components
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    for (auto& Component : AmbientAudioComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    if (WeatherAudioComponent && IsValid(WeatherAudioComponent))
    {
        WeatherAudioComponent->Stop();
    }
    
    if (PlayerAudioComponent && IsValid(PlayerAudioComponent))
    {
        PlayerAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create music audio component
    MusicAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoDestroy = false;
    }
    
    // Create ambient audio components (multiple layers)
    for (int32 i = 0; i < 4; ++i) // 4 ambient layers
    {
        UAudioComponent* AmbientComponent = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
        if (AmbientComponent)
        {
            AmbientComponent->bAutoDestroy = false;
            AmbientAudioComponents.Add(AmbientComponent);
        }
    }
    
    // Create weather audio component
    WeatherAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->bAutoDestroy = false;
    }
    
    // Create player audio component
    PlayerAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
    if (PlayerAudioComponent)
    {
        PlayerAudioComponent->bAutoDestroy = false;
    }
}

void UAudioSystemManager::UpdateAudioState(const FAudioState& NewState)
{
    FAudioState PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    // Check for significant changes that require audio updates
    bool bEnvironmentChanged = (PreviousState.Environment != NewState.Environment);
    bool bThreatChanged = (PreviousState.ThreatLevel != NewState.ThreatLevel);
    bool bTimeChanged = (PreviousState.TimeOfDay != NewState.TimeOfDay);
    bool bStressChanged = (FMath::Abs(PreviousState.PlayerStress - NewState.PlayerStress) > 0.1f);
    
    if (bEnvironmentChanged || bThreatChanged || bTimeChanged)
    {
        TriggerMusicTransition(3.0f); // Slower transition for major changes
        UpdateAmbientLayers();
    }
    else if (bStressChanged)
    {
        TriggerMusicTransition(1.0f); // Faster transition for stress changes
    }
    
    // Update weather audio
    UpdateWeatherAudio(NewState.WindIntensity, NewState.RainIntensity);
    
    // Update player heartbeat based on stress
    if (NewState.PlayerStress > 0.3f)
    {
        PlayPlayerHeartbeat(NewState.PlayerStress);
    }
}

void UAudioSystemManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentAudioState.ThreatLevel != NewThreatLevel)
    {
        CurrentAudioState.ThreatLevel = NewThreatLevel;
        
        // Adjust stress based on threat level
        switch (NewThreatLevel)
        {
            case EThreatLevel::Safe:
                CurrentAudioState.PlayerStress = FMath::Max(0.0f, CurrentAudioState.PlayerStress - 0.2f);
                break;
            case EThreatLevel::Cautious:
                CurrentAudioState.PlayerStress = FMath::Clamp(CurrentAudioState.PlayerStress + 0.1f, 0.2f, 1.0f);
                break;
            case EThreatLevel::Danger:
                CurrentAudioState.PlayerStress = FMath::Clamp(CurrentAudioState.PlayerStress + 0.3f, 0.5f, 1.0f);
                break;
            case EThreatLevel::Combat:
                CurrentAudioState.PlayerStress = FMath::Clamp(CurrentAudioState.PlayerStress + 0.5f, 0.8f, 1.0f);
                break;
            case EThreatLevel::Escape:
                CurrentAudioState.PlayerStress = 1.0f;
                break;
        }
        
        TriggerMusicTransition(1.5f);
    }
}

void UAudioSystemManager::SetEnvironment(EAudioEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.Environment != NewEnvironment)
    {
        CurrentAudioState.Environment = NewEnvironment;
        UpdateAmbientLayers();
        TriggerMusicTransition(4.0f); // Longer transition for environment changes
    }
}

void UAudioSystemManager::SetTimeOfDay(ETimeOfDay NewTimeOfDay)
{
    if (CurrentAudioState.TimeOfDay != NewTimeOfDay)
    {
        CurrentAudioState.TimeOfDay = NewTimeOfDay;
        UpdateAmbientLayers();
        TriggerMusicTransition(6.0f); // Very long transition for time changes
    }
}

void UAudioSystemManager::TriggerMusicTransition(float TransitionTime)
{
    if (!MusicAudioComponent || !AdaptiveMusicMetaSound) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMusicTransitionTime < 1.0f) return; // Prevent rapid transitions
    
    LastMusicTransitionTime = CurrentTime;
    
    // Update MetaSound parameters
    UpdateMusicParameters();
    
    // If not playing, start the adaptive music
    if (!MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->Play();
    }
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicAudioComponent) return;
    
    // Convert enums to float parameters for MetaSound
    float EnvironmentParam = static_cast<float>(CurrentAudioState.Environment) / 10.0f;
    float ThreatParam = static_cast<float>(CurrentAudioState.ThreatLevel) / 5.0f;
    float TimeParam = static_cast<float>(CurrentAudioState.TimeOfDay) / 7.0f;
    
    // Set MetaSound parameters
    MusicAudioComponent->SetFloatParameter(FName("Environment"), EnvironmentParam);
    MusicAudioComponent->SetFloatParameter(FName("ThreatLevel"), ThreatParam);
    MusicAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeParam);
    MusicAudioComponent->SetFloatParameter(FName("PlayerStress"), CurrentAudioState.PlayerStress);
    MusicAudioComponent->SetFloatParameter(FName("DinosaurProximity"), CurrentAudioState.DinosaurProximity);
    MusicAudioComponent->SetBoolParameter(FName("PlayerHidden"), CurrentAudioState.bPlayerHidden);
    MusicAudioComponent->SetFloatParameter(FName("WindIntensity"), CurrentAudioState.WindIntensity);
}

void UAudioSystemManager::UpdateAmbientLayers()
{
    // Update ambient sound layers based on environment and time
    for (int32 i = 0; i < AmbientAudioComponents.Num(); ++i)
    {
        if (!AmbientAudioComponents[i]) continue;
        
        // Each layer handles different aspects of the ambient soundscape
        switch (i)
        {
            case 0: // Base environment layer
                UpdateEnvironmentAmbient(AmbientAudioComponents[i]);
                break;
            case 1: // Wildlife layer
                UpdateWildlifeAmbient(AmbientAudioComponents[i]);
                break;
            case 2: // Vegetation layer (wind through trees, etc.)
                UpdateVegetationAmbient(AmbientAudioComponents[i]);
                break;
            case 3: // Distance layer (far-off sounds)
                UpdateDistanceAmbient(AmbientAudioComponents[i]);
                break;
        }
    }
}

void UAudioSystemManager::UpdateEnvironmentAmbient(UAudioComponent* Component)
{
    if (!Component) return;
    
    // Select appropriate ambient sound based on environment
    UMetaSoundSource* AmbientSound = nullptr;
    if (EnvironmentAmbientSounds.Contains(CurrentAudioState.Environment))
    {
        AmbientSound = EnvironmentAmbientSounds[CurrentAudioState.Environment];
    }
    
    if (AmbientSound && Component->GetSound() != AmbientSound)
    {
        Component->SetSound(AmbientSound);
        if (!Component->IsPlaying())
        {
            Component->Play();
        }
    }
    
    // Adjust volume based on time of day
    float TimeVolume = 1.0f;
    switch (CurrentAudioState.TimeOfDay)
    {
        case ETimeOfDay::Dawn:
        case ETimeOfDay::Dusk:
            TimeVolume = 0.7f;
            break;
        case ETimeOfDay::Night:
        case ETimeOfDay::DeepNight:
            TimeVolume = 0.5f;
            break;
        default:
            TimeVolume = 1.0f;
            break;
    }
    
    Component->SetVolumeMultiplier(TimeVolume);
}

void UAudioSystemManager::PlayDinosaurSound(class ADinosaurCharacter* Dinosaur, FName SoundType)
{
    if (!Dinosaur) return;
    
    USoundCue* DinosaurSound = nullptr;
    if (DinosaurSounds.Contains(SoundType))
    {
        DinosaurSound = DinosaurSounds[SoundType];
    }
    
    if (DinosaurSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DinosaurSound,
            Dinosaur->GetActorLocation(),
            1.0f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Sound attenuation
            nullptr, // Sound concurrency
            Dinosaur // Owner
        );
    }
}

void UAudioSystemManager::RegisterDinosaurProximity(float Distance, float ThreatWeight)
{
    // Calculate proximity influence on audio state
    float ProximityInfluence = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f); // 2000 units max range
    ProximityInfluence *= ThreatWeight;
    
    ProximityWeights.Add(ProximityInfluence);
    
    // Keep only recent proximity data
    if (ProximityWeights.Num() > 10)
    {
        ProximityWeights.RemoveAt(0);
    }
    
    // Calculate average proximity
    float AverageProximity = 0.0f;
    for (float Weight : ProximityWeights)
    {
        AverageProximity += Weight;
    }
    AverageProximity /= FMath::Max(1, ProximityWeights.Num());
    
    CurrentAudioState.DinosaurProximity = AverageProximity;
    
    // Update stress based on proximity
    if (AverageProximity > 0.5f)
    {
        CurrentAudioState.PlayerStress = FMath::Clamp(
            CurrentAudioState.PlayerStress + (AverageProximity * 0.1f),
            0.0f, 1.0f
        );
    }
}

void UAudioSystemManager::PlayPlayerFootstep(FVector Location, FName SurfaceType)
{
    // Play appropriate footstep sound based on surface type
    // This would be expanded with a database of surface-specific sounds
    FString SoundName = FString::Printf(TEXT("Footstep_%s"), *SurfaceType.ToString());
    
    // For now, just play a generic footstep
    // In full implementation, this would select from a variety of footstep sounds
}

void UAudioSystemManager::PlayPlayerHeartbeat(float Intensity)
{
    if (!PlayerAudioComponent || !HeartbeatMetaSound) return;
    
    if (PlayerAudioComponent->GetSound() != HeartbeatMetaSound)
    {
        PlayerAudioComponent->SetSound(HeartbeatMetaSound);
    }
    
    // Set heartbeat parameters
    PlayerAudioComponent->SetFloatParameter(FName("Intensity"), Intensity);
    PlayerAudioComponent->SetFloatParameter(FName("Rate"), FMath::Lerp(60.0f, 140.0f, Intensity)); // BPM
    
    if (!PlayerAudioComponent->IsPlaying() && Intensity > 0.3f)
    {
        PlayerAudioComponent->Play();
    }
    else if (PlayerAudioComponent->IsPlaying() && Intensity <= 0.1f)
    {
        PlayerAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void UAudioSystemManager::UpdateWeatherAudio(float WindIntensity, float RainIntensity)
{
    if (!WeatherAudioComponent) return;
    
    CurrentAudioState.WindIntensity = WindIntensity;
    CurrentAudioState.RainIntensity = RainIntensity;
    
    // Determine which weather MetaSound to use
    UMetaSoundSource* WeatherSound = nullptr;
    if (RainIntensity > 0.1f)
    {
        WeatherSound = RainMetaSound;
    }
    else if (WindIntensity > 0.2f)
    {
        WeatherSound = WindMetaSound;
    }
    
    if (WeatherSound)
    {
        if (WeatherAudioComponent->GetSound() != WeatherSound)
        {
            WeatherAudioComponent->SetSound(WeatherSound);
        }
        
        // Set weather parameters
        WeatherAudioComponent->SetFloatParameter(FName("WindIntensity"), WindIntensity);
        WeatherAudioComponent->SetFloatParameter(FName("RainIntensity"), RainIntensity);
        
        if (!WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Play();
        }
    }
    else if (WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->FadeOut(3.0f, 0.0f);
    }
}

void UAudioSystemManager::PlayEnvironmentalSound(FVector Location, FName SoundType, float Volume)
{
    // Play one-shot environmental sounds (branch breaking, water splash, etc.)
    // This would be expanded with a comprehensive environmental sound database
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(FName("MusicIntensity"), Intensity);
    }
}