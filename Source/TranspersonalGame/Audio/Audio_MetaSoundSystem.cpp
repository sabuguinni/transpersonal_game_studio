#include "Audio_MetaSoundSystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UAudio_MetaSoundSystem::UAudio_MetaSoundSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default environment settings
    CurrentEnvironment.EnvironmentType = EAudio_EnvironmentType::Forest;
    CurrentEnvironment.AmbientVolume = 0.7f;
    CurrentEnvironment.WindIntensity = 0.5f;
    CurrentEnvironment.CreatureActivity = 0.3f;
    CurrentEnvironment.ThreatLevel = EAudio_ThreatLevel::Safe;

    MasterVolume = 1.0f;
    EnvironmentUpdateInterval = 2.0f;
    LastEnvironmentUpdate = 0.0f;
    bAmbientLoopActive = false;
}

void UAudio_MetaSoundSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    LoadDinosaurSoundProfiles();
    StartAmbientLoop();

    UE_LOG(LogTemp, Log, TEXT("Audio MetaSound System initialized successfully"));
}

void UAudio_MetaSoundSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastEnvironmentUpdate += DeltaTime;
    if (LastEnvironmentUpdate >= EnvironmentUpdateInterval)
    {
        UpdateEnvironmentAudio();
        LastEnvironmentUpdate = 0.0f;
    }
}

void UAudio_MetaSoundSystem::InitializeAudioComponents()
{
    if (!AmbientAudioComponent)
    {
        AmbientAudioComponent = NewObject<UAudioComponent>(this, TEXT("AmbientAudioComponent"));
        if (AmbientAudioComponent && GetOwner())
        {
            AmbientAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            AmbientAudioComponent->bAutoActivate = false;
        }
    }

    if (!NarrationAudioComponent)
    {
        NarrationAudioComponent = NewObject<UAudioComponent>(this, TEXT("NarrationAudioComponent"));
        if (NarrationAudioComponent && GetOwner())
        {
            NarrationAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            NarrationAudioComponent->bAutoActivate = false;
        }
    }
}

void UAudio_MetaSoundSystem::LoadDinosaurSoundProfiles()
{
    // Initialize default dinosaur sound profiles
    DinosaurSoundProfiles.Empty();

    // T-Rex Profile
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.DinosaurType = TEXT("TRex");
    TRexProfile.VolumeMultiplier = 1.5f;
    TRexProfile.PitchVariation = 0.15f;
    DinosaurSoundProfiles.Add(TRexProfile);

    // Raptor Profile
    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.DinosaurType = TEXT("Raptor");
    RaptorProfile.VolumeMultiplier = 0.8f;
    RaptorProfile.PitchVariation = 0.2f;
    DinosaurSoundProfiles.Add(RaptorProfile);

    // Brachiosaurus Profile
    FAudio_DinosaurSoundProfile BrachioProfile;
    BrachioProfile.DinosaurType = TEXT("Brachiosaurus");
    BrachioProfile.VolumeMultiplier = 2.0f;
    BrachioProfile.PitchVariation = 0.1f;
    DinosaurSoundProfiles.Add(BrachioProfile);

    // Triceratops Profile
    FAudio_DinosaurSoundProfile TriceratopsProfile;
    TriceratopsProfile.DinosaurType = TEXT("Triceratops");
    TriceratopsProfile.VolumeMultiplier = 1.2f;
    TriceratopsProfile.PitchVariation = 0.12f;
    DinosaurSoundProfiles.Add(TriceratopsProfile);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d dinosaur sound profiles"), DinosaurSoundProfiles.Num());
}

void UAudio_MetaSoundSystem::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment)
{
    if (CurrentEnvironment.EnvironmentType != NewEnvironment)
    {
        CurrentEnvironment.EnvironmentType = NewEnvironment;
        
        // Adjust environment-specific settings
        switch (NewEnvironment)
        {
            case EAudio_EnvironmentType::Forest:
                CurrentEnvironment.AmbientVolume = 0.8f;
                CurrentEnvironment.WindIntensity = 0.4f;
                CurrentEnvironment.CreatureActivity = 0.6f;
                break;
            case EAudio_EnvironmentType::Swamp:
                CurrentEnvironment.AmbientVolume = 0.9f;
                CurrentEnvironment.WindIntensity = 0.2f;
                CurrentEnvironment.CreatureActivity = 0.8f;
                break;
            case EAudio_EnvironmentType::Plains:
                CurrentEnvironment.AmbientVolume = 0.6f;
                CurrentEnvironment.WindIntensity = 0.8f;
                CurrentEnvironment.CreatureActivity = 0.4f;
                break;
            case EAudio_EnvironmentType::Desert:
                CurrentEnvironment.AmbientVolume = 0.4f;
                CurrentEnvironment.WindIntensity = 0.9f;
                CurrentEnvironment.CreatureActivity = 0.2f;
                break;
            case EAudio_EnvironmentType::Mountain:
                CurrentEnvironment.AmbientVolume = 0.5f;
                CurrentEnvironment.WindIntensity = 1.0f;
                CurrentEnvironment.CreatureActivity = 0.3f;
                break;
        }
        
        UpdateMetaSoundParameters();
        UE_LOG(LogTemp, Log, TEXT("Environment changed to: %s"), 
            *UEnum::GetValueAsString(NewEnvironment));
    }
}

void UAudio_MetaSoundSystem::UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentEnvironment.ThreatLevel != NewThreatLevel)
    {
        CurrentEnvironment.ThreatLevel = NewThreatLevel;
        
        // Adjust audio intensity based on threat level
        float ThreatMultiplier = 1.0f;
        switch (NewThreatLevel)
        {
            case EAudio_ThreatLevel::Safe:
                ThreatMultiplier = 0.8f;
                break;
            case EAudio_ThreatLevel::Cautious:
                ThreatMultiplier = 1.0f;
                break;
            case EAudio_ThreatLevel::Dangerous:
                ThreatMultiplier = 1.3f;
                break;
            case EAudio_ThreatLevel::Lethal:
                ThreatMultiplier = 1.6f;
                break;
        }
        
        SetMetaSoundParameter(TEXT("ThreatLevel"), static_cast<float>(NewThreatLevel));
        SetMetaSoundParameter(TEXT("ThreatMultiplier"), ThreatMultiplier);
        
        UE_LOG(LogTemp, Log, TEXT("Threat level updated to: %s"), 
            *UEnum::GetValueAsString(NewThreatLevel));
    }
}

void UAudio_MetaSoundSystem::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType)
{
    FAudio_DinosaurSoundProfile* Profile = GetDinosaurProfile(DinosaurType);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur sound profile not found: %s"), *DinosaurType);
        return;
    }

    USoundWave* SoundToPlay = nullptr;
    
    if (SoundType == TEXT("Idle"))
    {
        SoundToPlay = Profile->IdleSound;
    }
    else if (SoundType == TEXT("Alert"))
    {
        SoundToPlay = Profile->AlertSound;
    }
    else if (SoundType == TEXT("Attack"))
    {
        SoundToPlay = Profile->AttackSound;
    }
    else if (SoundType == TEXT("Footstep"))
    {
        SoundToPlay = Profile->FootstepSound;
    }

    if (SoundToPlay && GetWorld())
    {
        float FinalVolume = Profile->VolumeMultiplier * MasterVolume;
        float PitchVariation = FMath::RandRange(-Profile->PitchVariation, Profile->PitchVariation);
        
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, 
            GetOwner()->GetActorLocation(), FinalVolume, 1.0f + PitchVariation);
        
        UE_LOG(LogTemp, Log, TEXT("Playing %s sound for %s dinosaur"), *SoundType, *DinosaurType);
    }
}

void UAudio_MetaSoundSystem::PlayNarrationAudio(const FString& AudioURL, float Volume)
{
    if (NarrationAudioComponent)
    {
        // For now, log the audio URL - in production this would load and play the audio
        UE_LOG(LogTemp, Log, TEXT("Playing narration audio: %s at volume: %f"), *AudioURL, Volume);
        
        // TODO: Implement actual audio streaming from URL
        // This would require additional audio streaming components
    }
}

void UAudio_MetaSoundSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * CurrentEnvironment.AmbientVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Master volume set to: %f"), MasterVolume);
}

void UAudio_MetaSoundSystem::StartAmbientLoop()
{
    if (AmbientAudioComponent && !bAmbientLoopActive)
    {
        bAmbientLoopActive = true;
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * CurrentEnvironment.AmbientVolume);
        
        // TODO: Set appropriate ambient sound based on environment
        // AmbientAudioComponent->SetSound(EnvironmentMetaSound);
        // AmbientAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Ambient audio loop started"));
    }
}

void UAudio_MetaSoundSystem::StopAmbientLoop()
{
    if (AmbientAudioComponent && bAmbientLoopActive)
    {
        bAmbientLoopActive = false;
        AmbientAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("Ambient audio loop stopped"));
    }
}

void UAudio_MetaSoundSystem::UpdateMetaSoundParameters()
{
    SetMetaSoundParameter(TEXT("EnvironmentType"), static_cast<float>(CurrentEnvironment.EnvironmentType));
    SetMetaSoundParameter(TEXT("AmbientVolume"), CurrentEnvironment.AmbientVolume);
    SetMetaSoundParameter(TEXT("WindIntensity"), CurrentEnvironment.WindIntensity);
    SetMetaSoundParameter(TEXT("CreatureActivity"), CurrentEnvironment.CreatureActivity);
    SetMetaSoundParameter(TEXT("ThreatLevel"), static_cast<float>(CurrentEnvironment.ThreatLevel));
}

void UAudio_MetaSoundSystem::SetMetaSoundParameter(const FString& ParameterName, float Value)
{
    // TODO: Implement MetaSound parameter setting when MetaSound assets are available
    UE_LOG(LogTemp, Verbose, TEXT("MetaSound parameter %s set to %f"), *ParameterName, Value);
}

void UAudio_MetaSoundSystem::UpdateEnvironmentAudio()
{
    // Periodically update environment-based audio parameters
    if (bAmbientLoopActive)
    {
        // Add subtle variations to ambient audio
        float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.1f;
        float VariedVolume = CurrentEnvironment.AmbientVolume + TimeVariation;
        
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * VariedVolume);
        }
    }
}

FAudio_DinosaurSoundProfile* UAudio_MetaSoundSystem::GetDinosaurProfile(const FString& DinosaurType)
{
    for (FAudio_DinosaurSoundProfile& Profile : DinosaurSoundProfiles)
    {
        if (Profile.DinosaurType == DinosaurType)
        {
            return &Profile;
        }
    }
    return nullptr;
}