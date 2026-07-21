#include "Audio_SoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Player, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.5f);
    
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized successfully"));
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    RegisteredSounds.Empty();
    ActiveSounds.Empty();
    CategoryVolumes.Empty();
    
    Super::Deinitialize();
}

UAudioComponent* UAudio_SoundManager::PlaySound2D(const FString& SoundName, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundCue.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found or failed to load: %s"), *SoundName);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    float FinalVolume = CalculateFinalVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        World,
        SoundEntry->SoundCue.Get(),
        FinalVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComp)
    {
        ActiveSounds.Add(SoundName, AudioComp);
        
        if (SoundEntry->FadeInTime > 0.0f)
        {
            AudioComp->FadeIn(SoundEntry->FadeInTime, FinalVolume);
        }
    }
    
    return AudioComp;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundName, FVector Location, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundCue.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found or failed to load: %s"), *SoundName);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    float FinalVolume = CalculateFinalVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        SoundEntry->SoundCue.Get(),
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComp)
    {
        ActiveSounds.Add(SoundName + TEXT("_") + FString::FromInt(FMath::Rand()), AudioComp);
        
        if (SoundEntry->FadeInTime > 0.0f)
        {
            AudioComp->FadeIn(SoundEntry->FadeInTime, FinalVolume);
        }
    }
    
    return AudioComp;
}

UAudioComponent* UAudio_SoundManager::PlaySoundAttached(const FString& SoundName, USceneComponent* AttachComponent, float VolumeMultiplier)
{
    if (!AttachComponent)
    {
        return nullptr;
    }
    
    FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundName);
    if (!SoundEntry || !SoundEntry->SoundCue.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sound not found or failed to load: %s"), *SoundName);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    float FinalVolume = CalculateFinalVolume(*SoundEntry, VolumeMultiplier);
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
        SoundEntry->SoundCue.Get(),
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        false,
        FinalVolume,
        SoundEntry->Pitch
    );
    
    if (AudioComp)
    {
        ActiveSounds.Add(SoundName + TEXT("_attached"), AudioComp);
        
        if (SoundEntry->FadeInTime > 0.0f)
        {
            AudioComp->FadeIn(SoundEntry->FadeInTime, FinalVolume);
        }
    }
    
    return AudioComp;
}

void UAudio_SoundManager::StopSound(const FString& SoundName)
{
    if (UAudioComponent** AudioCompPtr = ActiveSounds.Find(SoundName))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && IsValid(AudioComp))
        {
            FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundName);
            if (SoundEntry && SoundEntry->FadeOutTime > 0.0f)
            {
                AudioComp->FadeOut(SoundEntry->FadeOutTime, 0.0f);
            }
            else
            {
                AudioComp->Stop();
            }
        }
        ActiveSounds.Remove(SoundName);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
        }
    }
    ActiveSounds.Empty();
    
    if (CurrentAmbientLoop && IsValid(CurrentAmbientLoop))
    {
        CurrentAmbientLoop->Stop();
        CurrentAmbientLoop = nullptr;
    }
}

void UAudio_SoundManager::StopSoundsByCategory(EAudio_SoundCategory Category)
{
    TArray<FString> SoundsToRemove;
    
    for (auto& SoundPair : ActiveSounds)
    {
        FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundPair.Key);
        if (SoundEntry && SoundEntry->Category == Category)
        {
            if (SoundPair.Value && IsValid(SoundPair.Value))
            {
                SoundPair.Value->Stop();
            }
            SoundsToRemove.Add(SoundPair.Key);
        }
    }
    
    for (const FString& SoundName : SoundsToRemove)
    {
        ActiveSounds.Remove(SoundName);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundPair.Key);
            if (SoundEntry)
            {
                float NewVolume = CalculateFinalVolume(*SoundEntry, 1.0f);
                SoundPair.Value->SetVolumeMultiplier(NewVolume);
            }
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update active sounds in this category
    for (auto& SoundPair : ActiveSounds)
    {
        FAudio_SoundEntry* SoundEntry = FindSoundEntry(SoundPair.Key);
        if (SoundEntry && SoundEntry->Category == Category)
        {
            if (SoundPair.Value && IsValid(SoundPair.Value))
            {
                float NewVolume = CalculateFinalVolume(*SoundEntry, 1.0f);
                SoundPair.Value->SetVolumeMultiplier(NewVolume);
            }
        }
    }
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (const float* Volume = CategoryVolumes.Find(Category))
    {
        return *Volume;
    }
    return 1.0f;
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered sound: %s"), *SoundEntry.SoundName);
}

bool UAudio_SoundManager::IsSoundRegistered(const FString& SoundName) const
{
    return RegisteredSounds.Contains(SoundName);
}

void UAudio_SoundManager::PlayDinosaurRoar(const FString& DinosaurType, FVector Location)
{
    FString SoundName = TEXT("Roar_") + DinosaurType;
    PlaySoundAtLocation(SoundName, Location, 1.0f);
}

void UAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, FVector Location)
{
    FString SoundName = TEXT("Footstep_") + SurfaceType;
    PlaySoundAtLocation(SoundName, Location, 0.8f);
}

void UAudio_SoundManager::StartAmbientLoop(const FString& BiomeType)
{
    StopAmbientLoop();
    
    FString SoundName = TEXT("Ambient_") + BiomeType;
    CurrentAmbientLoop = PlaySound2D(SoundName, 1.0f);
}

void UAudio_SoundManager::StopAmbientLoop()
{
    if (CurrentAmbientLoop && IsValid(CurrentAmbientLoop))
    {
        CurrentAmbientLoop->FadeOut(2.0f, 0.0f);
        CurrentAmbientLoop = nullptr;
    }
}

FAudio_SoundEntry* UAudio_SoundManager::FindSoundEntry(const FString& SoundName)
{
    return RegisteredSounds.Find(SoundName);
}

float UAudio_SoundManager::CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry, float VolumeMultiplier) const
{
    float CategoryVolume = GetCategoryVolume(SoundEntry.Category);
    return SoundEntry.Volume * CategoryVolume * MasterVolume * VolumeMultiplier;
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Register default prehistoric sounds
    FAudio_SoundEntry Entry;
    
    // Dinosaur roars
    Entry.SoundName = TEXT("Roar_TRex");
    Entry.Category = EAudio_SoundCategory::Dinosaur;
    Entry.Volume = 1.0f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Roar_Raptor");
    Entry.Volume = 0.8f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Roar_Triceratops");
    Entry.Volume = 0.9f;
    RegisterSound(Entry);
    
    // Ambient sounds
    Entry.Category = EAudio_SoundCategory::Ambient;
    Entry.bLooping = true;
    Entry.FadeInTime = 3.0f;
    Entry.FadeOutTime = 2.0f;
    
    Entry.SoundName = TEXT("Ambient_Forest");
    Entry.Volume = 0.6f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Ambient_Plains");
    Entry.Volume = 0.5f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Ambient_River");
    Entry.Volume = 0.7f;
    RegisterSound(Entry);
    
    // Environment sounds
    Entry.Category = EAudio_SoundCategory::Environment;
    Entry.bLooping = false;
    Entry.FadeInTime = 0.0f;
    Entry.FadeOutTime = 0.0f;
    
    Entry.SoundName = TEXT("Footstep_Dirt");
    Entry.Volume = 0.4f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Footstep_Grass");
    Entry.Volume = 0.3f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Footstep_Stone");
    Entry.Volume = 0.6f;
    RegisterSound(Entry);
    
    Entry.SoundName = TEXT("Fire_Crackling");
    Entry.Volume = 0.5f;
    Entry.bLooping = true;
    RegisterSound(Entry);
    
    UE_LOG(LogTemp, Log, TEXT("Default prehistoric sounds registered"));
}

void UAudio_SoundManager::CleanupFinishedSounds()
{
    TArray<FString> SoundsToRemove;
    
    for (auto& SoundPair : ActiveSounds)
    {
        if (!SoundPair.Value || !IsValid(SoundPair.Value) || !SoundPair.Value->IsPlaying())
        {
            SoundsToRemove.Add(SoundPair.Key);
        }
    }
    
    for (const FString& SoundName : SoundsToRemove)
    {
        ActiveSounds.Remove(SoundName);
    }
}