#include "Audio_MetaSoundManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio components
    InitializeAudioComponents();
    
    // Set default states
    CurrentMusicState = EAudio_MusicState::Calm;
    CurrentAmbientZone = EAudio_AmbientZone::Forest;
    
    // Set default volumes
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    AmbientVolume = 0.5f;
    SFXVolume = 1.0f;
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    LoadDefaultSoundLayers();
    UpdateVolumeSettings();
    
    // Start with default music and ambient
    SetMusicState(CurrentMusicState);
    SetAmbientZone(CurrentAmbientZone);
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsFadingMusic)
    {
        ProcessMusicFade();
    }
    
    if (bIsFadingAmbient)
    {
        ProcessAmbientFade();
    }
}

void AAudio_MetaSoundManager::InitializeAudioComponents()
{
    // Create music audio component
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume);
    }
    
    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
    }
    
    // Create SFX audio component
    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    if (SFXAudioComponent)
    {
        SFXAudioComponent->bAutoActivate = false;
        SFXAudioComponent->SetVolumeMultiplier(SFXVolume);
    }
}

void AAudio_MetaSoundManager::LoadDefaultSoundLayers()
{
    // Initialize empty sound layers - to be populated via Blueprint or data assets
    for (int32 i = 0; i < (int32)EAudio_MusicState::Danger + 1; i++)
    {
        EAudio_MusicState State = (EAudio_MusicState)i;
        MusicLayers.Add(State, FAudio_SoundLayer());
    }
    
    for (int32 i = 0; i < (int32)EAudio_AmbientZone::Mountain + 1; i++)
    {
        EAudio_AmbientZone Zone = (EAudio_AmbientZone)i;
        AmbientLayers.Add(Zone, FAudio_SoundLayer());
    }
}

void AAudio_MetaSoundManager::UpdateVolumeSettings()
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * AmbientVolume);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetVolumeMultiplier(MasterVolume * SFXVolume);
    }
}

void AAudio_MetaSoundManager::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState == NewState) return;
    
    CurrentMusicState = NewState;
    
    if (MusicLayers.Contains(NewState))
    {
        const FAudio_SoundLayer& Layer = MusicLayers[NewState];
        if (Layer.SoundAsset && MusicAudioComponent)
        {
            MusicAudioComponent->Stop();
            MusicAudioComponent->SetSound(Layer.SoundAsset);
            MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume * Layer.Volume);
            MusicAudioComponent->Play();
        }
    }
}

void AAudio_MetaSoundManager::SetAmbientZone(EAudio_AmbientZone NewZone)
{
    if (CurrentAmbientZone == NewZone) return;
    
    CurrentAmbientZone = NewZone;
    
    if (AmbientLayers.Contains(NewZone))
    {
        const FAudio_SoundLayer& Layer = AmbientLayers[NewZone];
        if (Layer.SoundAsset && AmbientAudioComponent)
        {
            AmbientAudioComponent->Stop();
            AmbientAudioComponent->SetSound(Layer.SoundAsset);
            AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * AmbientVolume * Layer.Volume);
            AmbientAudioComponent->Play();
        }
    }
}

void AAudio_MetaSoundManager::PlaySFX(USoundBase* SoundEffect, float Volume)
{
    if (SoundEffect && SFXAudioComponent)
    {
        SFXAudioComponent->SetSound(SoundEffect);
        SFXAudioComponent->SetVolumeMultiplier(MasterVolume * SFXVolume * Volume);
        SFXAudioComponent->Play();
    }
}

void AAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateVolumeSettings();
}

void AAudio_MetaSoundManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateVolumeSettings();
}

void AAudio_MetaSoundManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateVolumeSettings();
}

void AAudio_MetaSoundManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateVolumeSettings();
}

void AAudio_MetaSoundManager::FadeToMusicState(EAudio_MusicState NewState, float FadeTime)
{
    if (CurrentMusicState == NewState || !MusicAudioComponent) return;
    
    // Setup fade parameters
    MusicFadeStartVolume = MusicAudioComponent->VolumeMultiplier;
    MusicFadeTargetVolume = 0.0f;
    MusicFadeCurrentTime = 0.0f;
    MusicFadeDuration = FadeTime * 0.5f; // Half time to fade out
    bIsFadingMusic = true;
    
    // Store the target state for after fade
    EAudio_MusicState TargetState = NewState;
    
    // Set timer to switch music at halfway point
    GetWorldTimerManager().SetTimer(MusicFadeTimer, [this, TargetState]()
    {
        SetMusicState(TargetState);
        // Start fade in
        MusicFadeStartVolume = 0.0f;
        if (MusicLayers.Contains(TargetState))
        {
            MusicFadeTargetVolume = MasterVolume * MusicVolume * MusicLayers[TargetState].Volume;
        }
        MusicFadeCurrentTime = 0.0f;
    }, MusicFadeDuration, false);
}

void AAudio_MetaSoundManager::FadeToAmbientZone(EAudio_AmbientZone NewZone, float FadeTime)
{
    if (CurrentAmbientZone == NewZone || !AmbientAudioComponent) return;
    
    // Setup fade parameters
    AmbientFadeStartVolume = AmbientAudioComponent->VolumeMultiplier;
    AmbientFadeTargetVolume = 0.0f;
    AmbientFadeCurrentTime = 0.0f;
    AmbientFadeDuration = FadeTime * 0.5f; // Half time to fade out
    bIsFadingAmbient = true;
    
    // Store the target zone for after fade
    EAudio_AmbientZone TargetZone = NewZone;
    
    // Set timer to switch ambient at halfway point
    GetWorldTimerManager().SetTimer(AmbientFadeTimer, [this, TargetZone]()
    {
        SetAmbientZone(TargetZone);
        // Start fade in
        AmbientFadeStartVolume = 0.0f;
        if (AmbientLayers.Contains(TargetZone))
        {
            AmbientFadeTargetVolume = MasterVolume * AmbientVolume * AmbientLayers[TargetZone].Volume;
        }
        AmbientFadeCurrentTime = 0.0f;
    }, AmbientFadeDuration, false);
}

void AAudio_MetaSoundManager::ProcessMusicFade()
{
    if (!MusicAudioComponent || MusicFadeDuration <= 0.0f)
    {
        bIsFadingMusic = false;
        return;
    }
    
    MusicFadeCurrentTime += GetWorld()->GetDeltaSeconds();
    float Alpha = FMath::Clamp(MusicFadeCurrentTime / MusicFadeDuration, 0.0f, 1.0f);
    
    float CurrentVolume = FMath::Lerp(MusicFadeStartVolume, MusicFadeTargetVolume, Alpha);
    MusicAudioComponent->SetVolumeMultiplier(CurrentVolume);
    
    if (Alpha >= 1.0f)
    {
        bIsFadingMusic = false;
    }
}

void AAudio_MetaSoundManager::ProcessAmbientFade()
{
    if (!AmbientAudioComponent || AmbientFadeDuration <= 0.0f)
    {
        bIsFadingAmbient = false;
        return;
    }
    
    AmbientFadeCurrentTime += GetWorld()->GetDeltaSeconds();
    float Alpha = FMath::Clamp(AmbientFadeCurrentTime / AmbientFadeDuration, 0.0f, 1.0f);
    
    float CurrentVolume = FMath::Lerp(AmbientFadeStartVolume, AmbientFadeTargetVolume, Alpha);
    AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
    
    if (Alpha >= 1.0f)
    {
        bIsFadingAmbient = false;
    }
}