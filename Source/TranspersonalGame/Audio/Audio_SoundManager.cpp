#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    CurrentAmbientComponent = nullptr;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }
    
    ActiveAudioComponents.Empty();
    RegisteredSounds.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.2f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.1f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Weather, 0.7f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Initialize placeholder sound entries for prehistoric game
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundID = TEXT("TRex_Roar");
    TRexRoar.Volume = 1.5f;
    TRexRoar.AttenuationRadius = 2000.0f;
    RegisteredSounds.Add(TRexRoar.SoundID, TRexRoar);
    
    FAudio_SoundEntry RaptorCall;
    RaptorCall.SoundID = TEXT("Raptor_Call");
    RaptorCall.Volume = 1.0f;
    RaptorCall.AttenuationRadius = 1500.0f;
    RegisteredSounds.Add(RaptorCall.SoundID, RaptorCall);
    
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundID = TEXT("Forest_Ambient");
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bLoop = true;
    RegisteredSounds.Add(ForestAmbient.SoundID, ForestAmbient);
    
    FAudio_SoundEntry HeavyFootsteps;
    HeavyFootsteps.SoundID = TEXT("Heavy_Footsteps");
    HeavyFootsteps.Volume = 0.8f;
    HeavyFootsteps.AttenuationRadius = 800.0f;
    RegisteredSounds.Add(HeavyFootsteps.SoundID, HeavyFootsteps);
    
    FAudio_SoundEntry CampfireCrackle;
    CampfireCrackle.SoundID = TEXT("Campfire_Crackle");
    CampfireCrackle.Volume = 0.5f;
    CampfireCrackle.bLoop = true;
    CampfireCrackle.AttenuationRadius = 500.0f;
    RegisteredSounds.Add(CampfireCrackle.SoundID, CampfireCrackle);
}

void UAudio_SoundManager::PlaySound2D(const FString& SoundID, float VolumeMultiplier)
{
    if (!RegisteredSounds.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
    
    if (SoundEntry.SoundWave.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), SoundEntry.SoundWave.Get(), 
            SoundEntry.Volume * VolumeMultiplier * MasterVolume, SoundEntry.Pitch);
    }
    else if (SoundEntry.SoundCue.IsValid())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), SoundEntry.SoundCue.Get(), 
            SoundEntry.Volume * VolumeMultiplier * MasterVolume, SoundEntry.Pitch);
    }
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundID, const FVector& Location, float VolumeMultiplier)
{
    if (!RegisteredSounds.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound ID not found: %s"), *SoundID);
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
    
    if (SoundEntry.SoundWave.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEntry.SoundWave.Get(), Location,
            SoundEntry.Volume * VolumeMultiplier * MasterVolume, SoundEntry.Pitch, 0.0f, nullptr, nullptr, true);
    }
    else if (SoundEntry.SoundCue.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEntry.SoundCue.Get(), Location,
            SoundEntry.Volume * VolumeMultiplier * MasterVolume, SoundEntry.Pitch, 0.0f, nullptr, nullptr, true);
    }
}

void UAudio_SoundManager::PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!RegisteredSounds.Contains(SoundID) || !IsValid(AttachComponent))
    {
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry);
    
    if (AudioComp)
    {
        AudioComp->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepWorldTransform);
        AudioComp->SetVolumeMultiplier(SoundEntry.Volume * VolumeMultiplier * MasterVolume);
        AudioComp->Play();
        
        ActiveAudioComponents.Add(SoundID, AudioComp);
    }
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (ActiveAudioComponents.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundID];
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
        ActiveAudioComponents.Remove(SoundID);
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
    MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 2.0f));
}

void UAudio_SoundManager::RegisterSound(const FString& SoundID, USoundWave* SoundWave, EAudio_SoundCategory Category)
{
    if (!IsValid(SoundWave))
    {
        return;
    }
    
    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.SoundWave = SoundWave;
    
    RegisteredSounds.Add(SoundID, NewEntry);
}

void UAudio_SoundManager::RegisterSoundCue(const FString& SoundID, USoundCue* SoundCue, EAudio_SoundCategory Category)
{
    if (!IsValid(SoundCue))
    {
        return;
    }
    
    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.SoundCue = SoundCue;
    
    RegisteredSounds.Add(SoundID, NewEntry);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundID) const
{
    if (ActiveAudioComponents.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundID];
        return IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location)
{
    FString SoundID = DinosaurType + TEXT("_Roar");
    PlaySoundAtLocation(SoundID, Location, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Playing dinosaur roar: %s at location %s"), *SoundID, *Location.ToString());
}

void UAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, const FVector& Location, float Weight)
{
    FString SoundID = TEXT("Footstep_") + SurfaceType;
    float VolumeMultiplier = FMath::Clamp(Weight, 0.1f, 2.0f);
    
    PlaySoundAtLocation(SoundID, Location, VolumeMultiplier);
}

void UAudio_SoundManager::PlayAmbientLoop(const FString& BiomeType)
{
    StopAmbientLoop();
    
    FString SoundID = BiomeType + TEXT("_Ambient");
    
    if (RegisteredSounds.Contains(SoundID))
    {
        const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
        CurrentAmbientComponent = CreateAudioComponent(SoundEntry);
        
        if (CurrentAmbientComponent)
        {
            CurrentAmbientComponent->SetVolumeMultiplier(SoundEntry.Volume * MasterVolume);
            CurrentAmbientComponent->Play();
        }
    }
}

void UAudio_SoundManager::StopAmbientLoop()
{
    if (CurrentAmbientComponent && IsValid(CurrentAmbientComponent))
    {
        CurrentAmbientComponent->Stop();
        CurrentAmbientComponent = nullptr;
    }
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    
    if (SoundEntry.SoundWave.IsValid())
    {
        AudioComp->SetSound(SoundEntry.SoundWave.Get());
    }
    else if (SoundEntry.SoundCue.IsValid())
    {
        AudioComp->SetSound(SoundEntry.SoundCue.Get());
    }
    
    AudioComp->bAutoActivate = false;
    AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
    
    return AudioComp;
}