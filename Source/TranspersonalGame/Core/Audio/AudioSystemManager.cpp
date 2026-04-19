#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
    SFXVolume = 1.0f;
    MusicVolume = 0.7f;
    VoiceVolume = 1.0f;
    AmbientVolume = 0.6f;
    CurrentEnvironment = EAudio_Environment::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentMusicIntensity = 0.5f;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio system"));
    
    // Initialize environment settings
    InitializeEnvironmentSettings();
    
    // Set up cleanup timer for inactive audio components
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimer, this, &UAudioSystemManager::CleanupInactiveComponents, 5.0f, true);
        World->GetTimerManager().SetTimer(AmbientUpdateTimer, this, &UAudioSystemManager::UpdateAmbientAudio, 2.0f, true);
    }
    
    // Create background music component
    BackgroundMusicComponent = CreateAudioComponent();
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->bAutoDestroy = false;
        BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    // Create ambient audio component
    AmbientAudioComponent = CreateAudioComponent();
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoDestroy = false;
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio system initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Deinitializing audio system"));
    
    // Stop all active audio
    StopAllSounds();
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimer);
        World->GetTimerManager().ClearTimer(AmbientUpdateTimer);
    }
    
    // Clean up components
    ActiveAudioComponents.Empty();
    BackgroundMusicComponent = nullptr;
    AmbientAudioComponent = nullptr;
    
    Super::Deinitialize();
}

void UAudioSystemManager::PlaySound(const FAudio_SoundEntry& SoundEntry, FVector Location)
{
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound cue in PlaySound"));
        return;
    }
    
    USoundCue* SoundCue = SoundEntry.SoundCue.LoadSynchronous();
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound cue"));
        return;
    }
    
    if (SoundEntry.b3D && Location != FVector::ZeroVector)
    {
        // Play 3D sound at location
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            SoundCue,
            Location,
            FRotator::ZeroRotator,
            SoundEntry.Volume * MasterVolume,
            SoundEntry.Pitch,
            0.0f,
            nullptr,
            nullptr,
            true
        );
        
        if (AudioComp)
        {
            ApplyVolumeSettings(AudioComp, SoundEntry.SoundType);
            ActiveAudioComponents.Add(AudioComp);
        }
    }
    else
    {
        // Play 2D sound
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
            GetWorld(),
            SoundCue,
            SoundEntry.Volume * MasterVolume,
            SoundEntry.Pitch,
            0.0f,
            nullptr,
            true
        );
        
        if (AudioComp)
        {
            ApplyVolumeSettings(AudioComp, SoundEntry.SoundType);
            ActiveAudioComponents.Add(AudioComp);
        }
    }
}

void UAudioSystemManager::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float Volume)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Null sound cue in PlaySoundAtLocation"));
        return;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        FRotator::ZeroRotator,
        Volume * SFXVolume * MasterVolume,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
    }
}

void UAudioSystemManager::StopAllSounds()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Stopping all sounds"));
    
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    
    if (BackgroundMusicComponent && IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->Stop();
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }
}

void UAudioSystemManager::StopSoundsByType(EAudio_SoundType SoundType)
{
    // Note: This is a simplified implementation
    // In a full system, we'd track sound types per component
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Stopping sounds by type: %d"), (int32)SoundType);
    
    switch (SoundType)
    {
        case EAudio_SoundType::Music:
            if (BackgroundMusicComponent && IsValid(BackgroundMusicComponent))
            {
                BackgroundMusicComponent->Stop();
            }
            break;
        case EAudio_SoundType::Ambient:
            if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
            {
                AmbientAudioComponent->Stop();
            }
            break;
        default:
            // For other types, we'd need to track them individually
            break;
    }
}

void UAudioSystemManager::SetEnvironment(EAudio_Environment NewEnvironment)
{
    if (CurrentEnvironment == NewEnvironment)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Changing environment from %d to %d"), (int32)CurrentEnvironment, (int32)NewEnvironment);
    
    CurrentEnvironment = NewEnvironment;
    UpdateAmbientAudio();
}

void UAudioSystemManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent || !IsValid(AmbientAudioComponent))
    {
        return;
    }
    
    // Get environment settings
    if (FAudio_EnvironmentSettings* EnvSettings = EnvironmentSettings.Find(CurrentEnvironment))
    {
        // Stop current ambient audio
        AmbientAudioComponent->Stop();
        
        // Play new ambient sounds based on environment
        if (EnvSettings->AmbientSounds.Num() > 0)
        {
            // For now, play the first ambient sound
            // In a full system, we'd blend multiple ambient sounds
            const FAudio_SoundEntry& AmbientSound = EnvSettings->AmbientSounds[0];
            if (AmbientSound.SoundCue.IsValid())
            {
                USoundCue* SoundCue = AmbientSound.SoundCue.LoadSynchronous();
                if (SoundCue)
                {
                    AmbientAudioComponent->SetSound(SoundCue);
                    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
                    AmbientAudioComponent->Play();
                }
            }
        }
    }
}

void UAudioSystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Changing threat level from %d to %d"), (int32)CurrentThreatLevel, (int32)NewThreatLevel);
    
    CurrentThreatLevel = NewThreatLevel;
    
    // Adjust music intensity based on threat level
    float NewIntensity = 0.5f;
    switch (NewThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            NewIntensity = 0.3f;
            break;
        case EAudio_ThreatLevel::Caution:
            NewIntensity = 0.5f;
            break;
        case EAudio_ThreatLevel::Danger:
            NewIntensity = 0.7f;
            break;
        case EAudio_ThreatLevel::Combat:
            NewIntensity = 0.9f;
            break;
        case EAudio_ThreatLevel::Panic:
            NewIntensity = 1.0f;
            break;
    }
    
    SetMusicIntensity(NewIntensity);
}

void UAudioSystemManager::PlayBackgroundMusic(USoundCue* MusicCue, bool bFadeIn)
{
    if (!MusicCue || !BackgroundMusicComponent || !IsValid(BackgroundMusicComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid music cue or component in PlayBackgroundMusic"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing background music"));
    
    BackgroundMusicComponent->SetSound(MusicCue);
    BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * CurrentMusicIntensity);
    
    if (bFadeIn)
    {
        BackgroundMusicComponent->FadeIn(2.0f, MusicVolume * MasterVolume * CurrentMusicIntensity);
    }
    else
    {
        BackgroundMusicComponent->Play();
    }
}

void UAudioSystemManager::StopBackgroundMusic(bool bFadeOut)
{
    if (!BackgroundMusicComponent || !IsValid(BackgroundMusicComponent))
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Stopping background music"));
    
    if (bFadeOut)
    {
        BackgroundMusicComponent->FadeOut(2.0f, 0.0f);
    }
    else
    {
        BackgroundMusicComponent->Stop();
    }
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    CurrentMusicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (BackgroundMusicComponent && IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * CurrentMusicIntensity);
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Master volume set to %f"), MasterVolume);
    
    // Update all active components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            // Note: In a full system, we'd track the original volume and sound type
            AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * MasterVolume);
        }
    }
    
    if (BackgroundMusicComponent && IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * CurrentMusicIntensity);
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: SFX volume set to %f"), SFXVolume);
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music volume set to %f"), MusicVolume);
    
    if (BackgroundMusicComponent && IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * CurrentMusicIntensity);
    }
}

void UAudioSystemManager::SetVoiceVolume(float Volume)
{
    VoiceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Voice volume set to %f"), VoiceVolume);
}

float UAudioSystemManager::GetCurrentAudioLevel()
{
    // Simplified implementation - returns current music intensity as audio level
    return CurrentMusicIntensity;
}

bool UAudioSystemManager::IsLocationAudible(FVector Location, float MaxDistance)
{
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            return Distance <= MaxDistance;
        }
    }
    return false;
}

void UAudioSystemManager::InitializeEnvironmentSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing environment settings"));
    
    // Initialize default environment settings
    // In a full system, these would be loaded from data assets
    
    FAudio_EnvironmentSettings ForestSettings;
    ForestSettings.Environment = EAudio_Environment::Forest;
    ForestSettings.ReverbAmount = 0.4f;
    ForestSettings.EchoDelay = 0.15f;
    EnvironmentSettings.Add(EAudio_Environment::Forest, ForestSettings);
    
    FAudio_EnvironmentSettings PlainsSettings;
    PlainsSettings.Environment = EAudio_Environment::Plains;
    PlainsSettings.ReverbAmount = 0.1f;
    PlainsSettings.EchoDelay = 0.05f;
    EnvironmentSettings.Add(EAudio_Environment::Plains, PlainsSettings);
    
    FAudio_EnvironmentSettings SwampSettings;
    SwampSettings.Environment = EAudio_Environment::Swamp;
    SwampSettings.ReverbAmount = 0.6f;
    SwampSettings.EchoDelay = 0.25f;
    EnvironmentSettings.Add(EAudio_Environment::Swamp, SwampSettings);
    
    FAudio_EnvironmentSettings MountainSettings;
    MountainSettings.Environment = EAudio_Environment::Mountains;
    MountainSettings.ReverbAmount = 0.8f;
    MountainSettings.EchoDelay = 0.4f;
    EnvironmentSettings.Add(EAudio_Environment::Mountains, MountainSettings);
    
    FAudio_EnvironmentSettings CaveSettings;
    CaveSettings.Environment = EAudio_Environment::Cave;
    CaveSettings.ReverbAmount = 1.0f;
    CaveSettings.EchoDelay = 0.6f;
    EnvironmentSettings.Add(EAudio_Environment::Cave, CaveSettings);
    
    FAudio_EnvironmentSettings RiverSettings;
    RiverSettings.Environment = EAudio_Environment::River;
    RiverSettings.ReverbAmount = 0.3f;
    RiverSettings.EchoDelay = 0.1f;
    EnvironmentSettings.Add(EAudio_Environment::River, RiverSettings);
}

void UAudioSystemManager::CleanupInactiveComponents()
{
    // Remove inactive audio components from tracking
    ActiveAudioComponents.RemoveAll([](UAudioComponent* AudioComp)
    {
        return !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent()
{
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
        if (AudioComp)
        {
            AudioComp->bAutoDestroy = true;
            AudioComp->bStopWhenOwnerDestroyed = false;
            return AudioComp;
        }
    }
    return nullptr;
}

void UAudioSystemManager::ApplyVolumeSettings(UAudioComponent* AudioComponent, EAudio_SoundType SoundType)
{
    if (!AudioComponent || !IsValid(AudioComponent))
    {
        return;
    }
    
    float TypeVolume = 1.0f;
    switch (SoundType)
    {
        case EAudio_SoundType::SFX:
            TypeVolume = SFXVolume;
            break;
        case EAudio_SoundType::Music:
            TypeVolume = MusicVolume;
            break;
        case EAudio_SoundType::Voice:
            TypeVolume = VoiceVolume;
            break;
        case EAudio_SoundType::Ambient:
            TypeVolume = AmbientVolume;
            break;
        case EAudio_SoundType::UI:
            TypeVolume = SFXVolume; // UI uses SFX volume
            break;
    }
    
    float FinalVolume = AudioComponent->VolumeMultiplier * TypeVolume * MasterVolume;
    AudioComponent->SetVolumeMultiplier(FinalVolume);
}