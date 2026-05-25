#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    AmbientAudioComponent = nullptr;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::DinosaurRoars, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Weather, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Narration, 0.9f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric survival sounds
    FAudio_SoundEntry TRexRoar;
    TRexRoar.Category = EAudio_SoundCategory::DinosaurRoars;
    TRexRoar.Volume = 1.0f;
    TRexRoar.Pitch = 1.0f;
    TRexRoar.bLooping = false;
    RegisterSound(TEXT("TRexRoar"), TRexRoar);
    
    FAudio_SoundEntry RaptorGrowl;
    RaptorGrowl.Category = EAudio_SoundCategory::DinosaurRoars;
    RaptorGrowl.Volume = 0.8f;
    RaptorGrowl.Pitch = 1.2f;
    RaptorGrowl.bLooping = false;
    RegisterSound(TEXT("RaptorGrowl"), RaptorGrowl);
    
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.5f;
    ForestAmbient.Pitch = 1.0f;
    ForestAmbient.bLooping = true;
    RegisterSound(TEXT("ForestAmbient"), ForestAmbient);
    
    FAudio_SoundEntry FootstepDirt;
    FootstepDirt.Category = EAudio_SoundCategory::Footsteps;
    FootstepDirt.Volume = 0.6f;
    FootstepDirt.Pitch = 1.0f;
    FootstepDirt.bLooping = false;
    RegisterSound(TEXT("FootstepDirt"), FootstepDirt);
}

void UAudio_SoundManager::PlaySound(const FString& SoundName, const FVector& Location)
{
    PlaySoundAtLocation(SoundName, Location, 1.0f);
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundName, const FVector& Location, float VolumeMultiplier)
{
    if (!RegisteredSounds.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundName);
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundName];
    
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sound cue for: %s"), *SoundName);
        return;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry, Location);
    if (AudioComp)
    {
        float FinalVolume = SoundEntry.Volume * VolumeMultiplier * MasterVolume;
        if (CategoryVolumes.Contains(SoundEntry.Category))
        {
            FinalVolume *= CategoryVolumes[SoundEntry.Category];
        }
        
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
        AudioComp->Play();
        
        if (!SoundEntry.bLooping)
        {
            ActiveAudioComponents.Add(SoundName, AudioComp);
        }
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
        ActiveAudioComponents.Remove(SoundName);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UAudio_SoundManager::RegisterSound(const FString& SoundName, const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundName, SoundEntry);
}

void UAudio_SoundManager::PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location)
{
    FString SoundName = DinosaurType + TEXT("Roar");
    PlaySoundAtLocation(SoundName, Location, 1.0f);
}

void UAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, const FVector& Location)
{
    FString SoundName = TEXT("Footstep") + SurfaceType;
    PlaySoundAtLocation(SoundName, Location, 0.8f);
}

void UAudio_SoundManager::PlayAmbientLoop(const FString& BiomeName)
{
    StopAmbientLoop();
    
    FString AmbientSoundName = BiomeName + TEXT("Ambient");
    if (RegisteredSounds.Contains(AmbientSoundName))
    {
        const FAudio_SoundEntry& SoundEntry = RegisteredSounds[AmbientSoundName];
        AmbientAudioComponent = CreateAudioComponent(SoundEntry, FVector::ZeroVector);
        
        if (AmbientAudioComponent)
        {
            float FinalVolume = SoundEntry.Volume * MasterVolume;
            if (CategoryVolumes.Contains(EAudio_SoundCategory::Ambient))
            {
                FinalVolume *= CategoryVolumes[EAudio_SoundCategory::Ambient];
            }
            
            AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
            AmbientAudioComponent->Play();
        }
    }
}

void UAudio_SoundManager::StopAmbientLoop()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry.SoundCue.Get(),
        Location,
        FRotator::ZeroRotator,
        1.0f,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        false
    );
    
    return AudioComp;
}