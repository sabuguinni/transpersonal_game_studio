#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.6f);
    
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    // Clean up active audio components
    for (auto& Pair : ActiveAudioComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->Stop();
            Pair.Value->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent->DestroyComponent();
        CurrentAmbientComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_SoundManager::PlaySound(const FString& SoundName, FVector Location)
{
    if (!RegisteredSounds.Contains(SoundName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found: %s"), *SoundName);
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundName];
    
    // Stop existing instance if not looping
    if (!SoundEntry.bLoop && ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* ExistingComponent = ActiveAudioComponents[SoundName];
        if (ExistingComponent && IsValid(ExistingComponent))
        {
            ExistingComponent->Stop();
            ExistingComponent->DestroyComponent();
        }
        ActiveAudioComponents.Remove(SoundName);
    }
    
    // Create and play new audio component
    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry, Location);
    if (AudioComp)
    {
        ActiveAudioComponents.Add(SoundName, AudioComp);
        AudioComp->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing sound: %s"), *SoundName);
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    if (ActiveAudioComponents.Contains(SoundName))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundName];
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
        ActiveAudioComponents.Remove(SoundName);
        
        UE_LOG(LogTemp, Log, TEXT("Stopped sound: %s"), *SoundName);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (auto& Pair : ActiveAudioComponents)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            const FAudio_SoundEntry& SoundEntry = RegisteredSounds[Pair.Key];
            float CategoryVolume = CategoryVolumes.Contains(SoundEntry.Category) ? CategoryVolumes[SoundEntry.Category] : 1.0f;
            float FinalVolume = MasterVolume * CategoryVolume * SoundEntry.Volume;
            Pair.Value->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update all active audio components of this category
    for (auto& Pair : ActiveAudioComponents)
    {
        if (RegisteredSounds.Contains(Pair.Key))
        {
            const FAudio_SoundEntry& SoundEntry = RegisteredSounds[Pair.Key];
            if (SoundEntry.Category == Category && Pair.Value && IsValid(Pair.Value))
            {
                float CategoryVolume = CategoryVolumes[Category];
                float FinalVolume = MasterVolume * CategoryVolume * SoundEntry.Volume;
                Pair.Value->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundName);
}

void UAudio_SoundManager::PlayAmbientLoop(const FString& BiomeName)
{
    // Stop current ambient if playing
    StopAmbientLoop();
    
    FString AmbientSoundName = FString::Printf(TEXT("Ambient_%s"), *BiomeName);
    
    if (RegisteredSounds.Contains(AmbientSoundName))
    {
        const FAudio_SoundEntry& SoundEntry = RegisteredSounds[AmbientSoundName];
        CurrentAmbientComponent = CreateAudioComponent(SoundEntry, FVector::ZeroVector);
        if (CurrentAmbientComponent)
        {
            CurrentAmbientComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Playing ambient loop: %s"), *AmbientSoundName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Ambient sound not found for biome: %s"), *BiomeName);
    }
}

void UAudio_SoundManager::StopAmbientLoop()
{
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent->DestroyComponent();
        CurrentAmbientComponent = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Stopped ambient loop"));
    }
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundName = TEXT("Ambient_Forest");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bLoop = true;
    RegisterSound(ForestAmbient);
    
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundName = TEXT("TRex_Roar");
    TRexRoar.Category = EAudio_SoundCategory::SFX;
    TRexRoar.Volume = 1.0f;
    TRexRoar.bLoop = false;
    RegisterSound(TRexRoar);
    
    FAudio_SoundEntry Footsteps;
    Footsteps.SoundName = TEXT("Player_Footsteps");
    Footsteps.Category = EAudio_SoundCategory::SFX;
    Footsteps.Volume = 0.7f;
    Footsteps.bLoop = false;
    RegisterSound(Footsteps);
    
    FAudio_SoundEntry HeartBeat;
    HeartBeat.SoundName = TEXT("Fear_Heartbeat");
    HeartBeat.Category = EAudio_SoundCategory::SFX;
    HeartBeat.Volume = 0.8f;
    HeartBeat.bLoop = true;
    RegisterSound(HeartBeat);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (!AudioComp)
    {
        return nullptr;
    }
    
    // Load sound cue if available
    if (SoundEntry.SoundCue.IsValid())
    {
        AudioComp->SetSound(SoundEntry.SoundCue.LoadSynchronous());
    }
    
    // Set properties
    AudioComp->bAutoActivate = false;
    AudioComp->bIsUISound = (SoundEntry.Category == EAudio_SoundCategory::UI);
    AudioComp->SetWorldLocation(Location);
    
    // Calculate final volume
    float CategoryVolume = CategoryVolumes.Contains(SoundEntry.Category) ? CategoryVolumes[SoundEntry.Category] : 1.0f;
    float FinalVolume = MasterVolume * CategoryVolume * SoundEntry.Volume;
    AudioComp->SetVolumeMultiplier(FinalVolume);
    
    // Set looping
    if (SoundEntry.bLoop)
    {
        AudioComp->SetIntParameter(TEXT("bLoop"), 1);
    }
    
    return AudioComp;
}