#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
    SFXVolume = 1.0f;
    MusicVolume = 1.0f;
    AmbientVolume = 1.0f;
    CurrentBiomeAmbient = nullptr;
    DayAmbientComponent = nullptr;
    NightAmbientComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialized"));
    
    LoadDefaultAudioAssets();
    
    // Set up cleanup timer for finished audio components
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UAudioSystemManager::CleanupFinishedComponents,
            1.0f,
            true
        );
    }
}

void UAudioSystemManager::Deinitialize()
{
    // Stop all active audio components
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    ActiveAudioComponents.Empty();
    
    // Clear proximity audio map
    ProximityAudioMap.Empty();
    
    // Stop biome ambient
    if (IsValid(CurrentBiomeAmbient))
    {
        CurrentBiomeAmbient->Stop();
        CurrentBiomeAmbient = nullptr;
    }
    
    // Stop day/night ambients
    if (IsValid(DayAmbientComponent))
    {
        DayAmbientComponent->Stop();
        DayAmbientComponent = nullptr;
    }
    
    if (IsValid(NightAmbientComponent))
    {
        NightAmbientComponent->Stop();
        NightAmbientComponent = nullptr;
    }
    
    Super::Deinitialize();
}

UAudioComponent* UAudioSystemManager::PlaySound2D(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, bool bPersistent)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager::PlaySound2D - Sound is null"));
        return nullptr;
    }
    
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(
            World,
            Sound,
            VolumeMultiplier * SFXVolume * MasterVolume,
            PitchMultiplier,
            0.0f,
            nullptr,
            bPersistent
        );
        
        if (AudioComponent)
        {
            ActiveAudioComponents.Add(AudioComponent);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::PlaySound2D - Sound played: %s"), *Sound->GetName());
        }
        
        return AudioComponent;
    }
    
    return nullptr;
}

UAudioComponent* UAudioSystemManager::PlaySoundAtLocation(const UObject* WorldContext, USoundBase* Sound, FVector Location, FRotator Rotation, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager::PlaySoundAtLocation - Sound is null"));
        return nullptr;
    }
    
    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        WorldContext,
        Sound,
        Location,
        Rotation,
        VolumeMultiplier * SFXVolume * MasterVolume,
        PitchMultiplier,
        StartTime
    );
    
    if (AudioComponent)
    {
        ActiveAudioComponents.Add(AudioComponent);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::PlaySoundAtLocation - Sound played at location: %s"), *Location.ToString());
    }
    
    return AudioComponent;
}

void UAudioSystemManager::RegisterProximityAudio(AActor* SourceActor, const FAudio_ProximityAudioData& AudioData)
{
    if (!SourceActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager::RegisterProximityAudio - SourceActor is null"));
        return;
    }
    
    ProximityAudioMap.Add(SourceActor, AudioData);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::RegisterProximityAudio - Registered proximity audio for: %s"), *SourceActor->GetName());
}

void UAudioSystemManager::UnregisterProximityAudio(AActor* SourceActor)
{
    if (!SourceActor)
    {
        return;
    }
    
    ProximityAudioMap.Remove(SourceActor);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::UnregisterProximityAudio - Unregistered proximity audio for: %s"), *SourceActor->GetName());
}

void UAudioSystemManager::UpdateProximityAudio(APawn* Listener)
{
    if (!Listener)
    {
        return;
    }
    
    FVector ListenerLocation = Listener->GetActorLocation();
    
    for (auto& ProximityPair : ProximityAudioMap)
    {
        if (AActor* SourceActor = ProximityPair.Key.Get())
        {
            const FAudio_ProximityAudioData& AudioData = ProximityPair.Value;
            
            float Distance = FVector::Dist(ListenerLocation, SourceActor->GetActorLocation());
            
            if (Distance <= AudioData.TriggerDistance)
            {
                if (USoundBase* ProximitySound = AudioData.ProximitySound.LoadSynchronous())
                {
                    float VolumeMultiplier = AudioData.VolumeMultiplier;
                    
                    if (AudioData.bUseDistanceAttenuation)
                    {
                        VolumeMultiplier *= CalculateDistanceAttenuation(Distance, AudioData.MaxDistance);
                    }
                    
                    PlaySoundAtLocation(
                        Listener,
                        ProximitySound,
                        SourceActor->GetActorLocation(),
                        FRotator::ZeroRotator,
                        VolumeMultiplier
                    );
                }
            }
        }
    }
}

void UAudioSystemManager::SetBiomeAmbient(EAudio_BiomeAmbient BiomeType, float FadeTime)
{
    // Stop current biome ambient
    if (IsValid(CurrentBiomeAmbient))
    {
        CurrentBiomeAmbient->FadeOut(FadeTime, 0.0f);
        CurrentBiomeAmbient = nullptr;
    }
    
    // Find and play new biome ambient
    if (TSoftObjectPtr<USoundBase>* BiomeSound = BiomeAmbientSounds.Find(BiomeType))
    {
        if (USoundBase* Sound = BiomeSound->LoadSynchronous())
        {
            CurrentBiomeAmbient = PlaySound2D(Sound, AmbientVolume, 1.0f, true);
            if (CurrentBiomeAmbient)
            {
                CurrentBiomeAmbient->FadeIn(FadeTime, AmbientVolume * MasterVolume);
                UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::SetBiomeAmbient - Switched to biome: %d"), (int32)BiomeType);
            }
        }
    }
}

void UAudioSystemManager::StopBiomeAmbient(float FadeTime)
{
    if (IsValid(CurrentBiomeAmbient))
    {
        CurrentBiomeAmbient->FadeOut(FadeTime, 0.0f);
        CurrentBiomeAmbient = nullptr;
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::StopBiomeAmbient - Stopped biome ambient"));
    }
}

void UAudioSystemManager::PlayDamageAudio(float DamageAmount, FVector HitLocation)
{
    if (USoundBase* DamageSound = DamageAudioSound.LoadSynchronous())
    {
        float VolumeMultiplier = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);
        float PitchMultiplier = FMath::RandRange(0.8f, 1.2f);
        
        PlaySoundAtLocation(
            GetWorld(),
            DamageSound,
            HitLocation,
            FRotator::ZeroRotator,
            VolumeMultiplier,
            PitchMultiplier
        );
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::PlayDamageAudio - Damage: %f at location: %s"), DamageAmount, *HitLocation.ToString());
    }
}

void UAudioSystemManager::PlayTRexFootstepAudio(FVector TRexLocation, float Intensity)
{
    if (USoundBase* FootstepSound = TRexFootstepSound.LoadSynchronous())
    {
        float VolumeMultiplier = Intensity * 1.5f;
        float PitchMultiplier = FMath::RandRange(0.7f, 0.9f);
        
        PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            TRexLocation,
            FRotator::ZeroRotator,
            VolumeMultiplier,
            PitchMultiplier
        );
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::PlayTRexFootstepAudio - T-Rex footstep at: %s"), *TRexLocation.ToString());
    }
}

void UAudioSystemManager::TransitionToDayAudio(float TransitionTime)
{
    // Fade out night ambient
    if (IsValid(NightAmbientComponent))
    {
        NightAmbientComponent->FadeOut(TransitionTime, 0.0f);
    }
    
    // Fade in day ambient
    if (USoundBase* DaySound = DayAmbientSound.LoadSynchronous())
    {
        DayAmbientComponent = PlaySound2D(DaySound, 0.0f, 1.0f, true);
        if (DayAmbientComponent)
        {
            DayAmbientComponent->FadeIn(TransitionTime, AmbientVolume * MasterVolume);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::TransitionToDayAudio - Transitioning to day audio"));
        }
    }
}

void UAudioSystemManager::TransitionToNightAudio(float TransitionTime)
{
    // Fade out day ambient
    if (IsValid(DayAmbientComponent))
    {
        DayAmbientComponent->FadeOut(TransitionTime, 0.0f);
    }
    
    // Fade in night ambient
    if (USoundBase* NightSound = NightAmbientSound.LoadSynchronous())
    {
        NightAmbientComponent = PlaySound2D(NightSound, 0.0f, 1.0f, true);
        if (NightAmbientComponent)
        {
            NightAmbientComponent->FadeIn(TransitionTime, AmbientVolume * MasterVolume);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::TransitionToNightAudio - Transitioning to night audio"));
        }
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::SetMasterVolume - Volume set to: %f"), MasterVolume);
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::SetSFXVolume - Volume set to: %f"), SFXVolume);
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::SetMusicVolume - Volume set to: %f"), MusicVolume);
}

void UAudioSystemManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::SetAmbientVolume - Volume set to: %f"), AmbientVolume);
}

void UAudioSystemManager::CleanupFinishedComponents()
{
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Component)
    {
        return !IsValid(Component) || !Component->IsPlaying();
    });
}

void UAudioSystemManager::LoadDefaultAudioAssets()
{
    // Load default damage audio (placeholder path - will be replaced with real assets)
    DamageAudioSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button")));
    
    // Load default T-Rex footstep audio (placeholder path)
    TRexFootstepSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button")));
    
    // Load default day/night ambients (placeholder paths)
    DayAmbientSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button")));
    NightAmbientSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button")));
    
    // Load biome ambient sounds (placeholder paths)
    BiomeAmbientSounds.Add(EAudio_BiomeAmbient::Savana, TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button"))));
    BiomeAmbientSounds.Add(EAudio_BiomeAmbient::Forest, TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button"))));
    BiomeAmbientSounds.Add(EAudio_BiomeAmbient::Desert, TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button"))));
    BiomeAmbientSounds.Add(EAudio_BiomeAmbient::Mountain, TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button"))));
    BiomeAmbientSounds.Add(EAudio_BiomeAmbient::Swamp, TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button"))));
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager::LoadDefaultAudioAssets - Default audio assets loaded"));
}

float UAudioSystemManager::CalculateDistanceAttenuation(float Distance, float MaxDistance) const
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear attenuation for now - can be made more sophisticated
    return 1.0f - (Distance / MaxDistance);
}