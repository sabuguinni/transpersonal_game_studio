#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio components
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    RootComponent = AmbienceAudioComponent;
    AmbienceAudioComponent->bAutoActivate = false;
    
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;
    
    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    SFXAudioComponent->SetupAttachment(RootComponent);
    SFXAudioComponent->bAutoActivate = false;
    
    // Set default values
    CurrentMasterVolume = 1.0f;
    CurrentMusicIntensity = EAudio_IntensityLevel::Calm;
    bIsFadingAmbience = false;
    bIsFadingMusic = false;
    
    // Initialize default ambience settings
    CurrentAmbienceSettings.AmbienceType = EAudio_AmbienceType::Forest;
    CurrentAmbienceSettings.Volume = 0.5f;
    CurrentAmbienceSettings.FadeInTime = 2.0f;
    CurrentAmbienceSettings.FadeOutTime = 2.0f;
    CurrentAmbienceSettings.bLooping = true;
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    LoadDefaultAudioAssets();
    
    // Start with default forest ambience
    SetAmbienceType(EAudio_AmbienceType::Forest);
    SetMusicIntensity(EAudio_IntensityLevel::Calm);
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateDynamicParameters();
}

void AAudio_MetaSoundManager::InitializeAudioComponents()
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(CurrentAmbienceSettings.Volume * CurrentMasterVolume);
        AmbienceAudioComponent->bOverrideAttenuation = true;
        AmbienceAudioComponent->AttenuationOverrides.bAttenuate = false;
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.3f * CurrentMasterVolume);
        MusicAudioComponent->bOverrideAttenuation = true;
        MusicAudioComponent->AttenuationOverrides.bAttenuate = false;
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(0.8f * CurrentMasterVolume);
        SFXAudioComponent->bOverrideAttenuation = true;
        SFXAudioComponent->AttenuationOverrides.bAttenuate = false;
    }
}

void AAudio_MetaSoundManager::LoadDefaultAudioAssets()
{
    // Note: In a real implementation, these would be loaded from content browser paths
    // For now, we initialize empty maps that can be populated in Blueprint or at runtime
    
    AmbienceSounds.Empty();
    MusicTracks.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Default audio assets loaded"));
}

void AAudio_MetaSoundManager::SetAmbienceType(EAudio_AmbienceType NewAmbienceType)
{
    if (CurrentAmbienceSettings.AmbienceType == NewAmbienceType && AmbienceAudioComponent && AmbienceAudioComponent->IsPlaying())
    {
        return; // Already playing this ambience type
    }
    
    CurrentAmbienceSettings.AmbienceType = NewAmbienceType;
    
    // Find the appropriate sound for this ambience type
    USoundBase** FoundSound = AmbienceSounds.Find(NewAmbienceType);
    if (FoundSound && *FoundSound && AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetSound(*FoundSound);
        AmbienceAudioComponent->FadeIn(CurrentAmbienceSettings.FadeInTime, CurrentAmbienceSettings.Volume * CurrentMasterVolume);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Set ambience type to %d"), (int32)NewAmbienceType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: No sound found for ambience type %d"), (int32)NewAmbienceType);
    }
}

void AAudio_MetaSoundManager::SetMusicIntensity(EAudio_IntensityLevel NewIntensity)
{
    if (CurrentMusicIntensity == NewIntensity)
    {
        return; // Already at this intensity
    }
    
    CurrentMusicIntensity = NewIntensity;
    
    // Find the appropriate music track for this intensity
    USoundBase** FoundMusic = MusicTracks.Find(NewIntensity);
    if (FoundMusic && *FoundMusic && MusicAudioComponent)
    {
        MusicAudioComponent->SetSound(*FoundMusic);
        MusicAudioComponent->FadeIn(2.0f, 0.3f * CurrentMasterVolume);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Set music intensity to %d"), (int32)NewIntensity);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: No music found for intensity %d"), (int32)NewIntensity);
    }
    
    // Update MetaSound parameters if available
    if (AdaptiveMusicMetaSound)
    {
        UpdateMetaSoundParameter(TEXT("Intensity"), (float)NewIntensity);
    }
}

void AAudio_MetaSoundManager::PlaySFX(USoundBase* SFXSound, float Volume)
{
    if (!SFXSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Attempted to play null SFX sound"));
        return;
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetSound(SFXSound);
        SFXAudioComponent->SetVolumeMultiplier(Volume * CurrentMasterVolume);
        SFXAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing SFX at volume %f"), Volume);
    }
}

void AAudio_MetaSoundManager::FadeToAmbience(EAudio_AmbienceType NewAmbienceType, float FadeTime)
{
    if (bIsFadingAmbience)
    {
        return; // Already fading
    }
    
    bIsFadingAmbience = true;
    
    // Fade out current ambience
    if (AmbienceAudioComponent && AmbienceAudioComponent->IsPlaying())
    {
        AmbienceAudioComponent->FadeOut(FadeTime, 0.0f);
        
        // Set timer to start new ambience after fade out
        FTimerHandle FadeTimer;
        GetWorld()->GetTimerManager().SetTimer(FadeTimer, [this, NewAmbienceType]()
        {
            SetAmbienceType(NewAmbienceType);
            bIsFadingAmbience = false;
        }, FadeTime, false);
    }
    else
    {
        // No current ambience, start new one immediately
        SetAmbienceType(NewAmbienceType);
        bIsFadingAmbience = false;
    }
}

void AAudio_MetaSoundManager::StopAllAudio()
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->Stop();
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: All audio stopped"));
}

void AAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    CurrentMasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all audio component volumes
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(CurrentAmbienceSettings.Volume * CurrentMasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.3f * CurrentMasterVolume);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(0.8f * CurrentMasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Master volume set to %f"), CurrentMasterVolume);
}

void AAudio_MetaSoundManager::UpdateMetaSoundParameter(const FString& ParameterName, float Value)
{
    // Update dynamic ambience MetaSound
    if (DynamicAmbienceMetaSound && AmbienceAudioComponent)
    {
        // Note: This would require MetaSound parameter interface implementation
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated MetaSound parameter %s to %f"), *ParameterName, Value);
    }
    
    // Update adaptive music MetaSound
    if (AdaptiveMusicMetaSound && MusicAudioComponent)
    {
        // Note: This would require MetaSound parameter interface implementation
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated music MetaSound parameter %s to %f"), *ParameterName, Value);
    }
}

void AAudio_MetaSoundManager::TriggerMetaSoundEvent(const FString& EventName)
{
    if (DynamicAmbienceMetaSound || AdaptiveMusicMetaSound)
    {
        // Note: This would require MetaSound event triggering implementation
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Triggered MetaSound event %s"), *EventName);
    }
}

void AAudio_MetaSoundManager::UpdateDynamicParameters()
{
    // Update parameters based on game state
    // This could include time of day, weather, player stress level, etc.
    
    // Example: Update ambience intensity based on nearby threats
    if (GetWorld())
    {
        // In a real implementation, this would query the game state for threats, weather, etc.
        float ThreatLevel = 0.0f; // Placeholder
        float TimeOfDay = 0.5f;   // Placeholder (0.0 = midnight, 0.5 = noon)
        
        UpdateMetaSoundParameter(TEXT("ThreatLevel"), ThreatLevel);
        UpdateMetaSoundParameter(TEXT("TimeOfDay"), TimeOfDay);
    }
}

void AAudio_MetaSoundManager::OnAmbienceFadeComplete()
{
    bIsFadingAmbience = false;
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Ambience fade completed"));
}

void AAudio_MetaSoundManager::OnMusicFadeComplete()
{
    bIsFadingMusic = false;
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Music fade completed"));
}