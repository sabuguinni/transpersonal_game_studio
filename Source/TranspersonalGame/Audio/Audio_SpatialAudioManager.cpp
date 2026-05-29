#include "Audio_SpatialAudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_SpatialAudioManager::UAudio_SpatialAudioManager()
{
    CurrentBiome = EAudio_BiomeType::Savana;
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_AudioCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_AudioCategory::Dinosaur, 0.8f);
    CategoryVolumes.Add(EAudio_AudioCategory::Player, 1.0f);
    CategoryVolumes.Add(EAudio_AudioCategory::Environment, 0.6f);
    CategoryVolumes.Add(EAudio_AudioCategory::Music, 0.5f);
    CategoryVolumes.Add(EAudio_AudioCategory::UI, 1.0f);
    CategoryVolumes.Add(EAudio_AudioCategory::Narration, 0.9f);
    CategoryVolumes.Add(EAudio_AudioCategory::Combat, 0.8f);
}

void UAudio_SpatialAudioManager::InitializeAudioSystem(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_SpatialAudioManager: World is null"));
        return;
    }

    InitializeBiomeAmbiences();
    InitializeDinosaurSounds();
    InitializeNarrationSounds();
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Audio system initialized"));
}

void UAudio_SpatialAudioManager::PlaySoundAtLocation(const FAudio_SoundEntry& SoundEntry, const FVector& Location)
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        return;
    }

    USoundWave* SoundWave = SoundEntry.SoundAsset.LoadSynchronous();
    if (!SoundWave)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_SpatialAudioManager: Failed to load sound: %s"), *SoundEntry.SoundName);
        return;
    }

    UAudioComponent* AudioComp = CreateAudioComponent(World, Location);
    if (AudioComp)
    {
        AudioComp->SetSound(SoundWave);
        
        float CategoryVolume = GetCategoryVolume(SoundEntry.Category);
        AudioComp->SetVolumeMultiplier(SoundEntry.Volume * CategoryVolume);
        AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
        
        AudioComp->Play();
        ActiveAudioComponents.Add(AudioComp);
        
        UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Playing sound %s at location %s"), 
               *SoundEntry.SoundName, *Location.ToString());
    }
}

void UAudio_SpatialAudioManager::PlayNarrationSound(const FString& SoundName, USoundWave* SoundWave)
{
    if (!SoundWave)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_SpatialAudioManager: Narration sound wave is null for %s"), *SoundName);
        return;
    }

    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        return;
    }

    // Stop any existing narration
    StopAllSoundsInCategory(EAudio_AudioCategory::Narration);

    // Play new narration as 2D sound (not spatialized)
    UGameplayStatics::PlaySound2D(World, SoundWave, GetCategoryVolume(EAudio_AudioCategory::Narration));
    
    // Store for future reference
    NarrationSounds.Add(SoundName, SoundWave);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Playing narration sound: %s"), *SoundName);
}

void UAudio_SpatialAudioManager::StopAllSoundsInCategory(EAudio_AudioCategory Category)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            // Note: In a full implementation, we'd track category per component
            // For now, stop all active components when stopping narration
            if (Category == EAudio_AudioCategory::Narration)
            {
                AudioComp->Stop();
                ActiveAudioComponents.RemoveAt(i);
            }
        }
        else
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
    
    CleanupInactiveComponents();
}

void UAudio_SpatialAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome, const FVector& PlayerLocation)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateBiomeAmbience(PlayerLocation);
        
        UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Biome changed to %d"), (int32)NewBiome);
    }
}

void UAudio_SpatialAudioManager::UpdateBiomeAmbience(const FVector& PlayerLocation)
{
    // Find current biome ambience
    for (const FAudio_BiomeAmbience& BiomeAmbience : BiomeAmbiences)
    {
        if (BiomeAmbience.BiomeType == CurrentBiome)
        {
            // Play ambient sounds for this biome
            for (const FAudio_SoundEntry& AmbientSound : BiomeAmbience.AmbientSounds)
            {
                // Offset sound location slightly from player
                FVector SoundLocation = PlayerLocation + FVector(
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(-500.0f, 500.0f),
                    0.0f
                );
                
                PlaySoundAtLocation(AmbientSound, SoundLocation);
            }
            break;
        }
    }
}

void UAudio_SpatialAudioManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Intensity)
{
    TSoftObjectPtr<USoundWave>* SoundPtr = DinosaurSounds.Find(DinosaurType);
    if (SoundPtr && SoundPtr->IsValid())
    {
        FAudio_SoundEntry DinoSound;
        DinoSound.SoundName = DinosaurType + TEXT("_Roar");
        DinoSound.SoundAsset = *SoundPtr;
        DinoSound.Category = EAudio_AudioCategory::Dinosaur;
        DinoSound.Volume = FMath::Clamp(Intensity, 0.1f, 2.0f);
        DinoSound.AttenuationRadius = 3000.0f; // Large radius for dinosaur sounds
        
        PlaySoundAtLocation(DinoSound, Location);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAudio_SpatialAudioManager: No sound found for dinosaur type: %s"), *DinosaurType);
    }
}

void UAudio_SpatialAudioManager::RegisterDinosaurActor(AActor* DinosaurActor, const FString& DinosaurType)
{
    if (!DinosaurActor)
    {
        return;
    }

    // In a full implementation, we'd store actor references and update their audio dynamically
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Registered dinosaur actor %s of type %s"), 
           *DinosaurActor->GetName(), *DinosaurType);
}

void UAudio_SpatialAudioManager::SetCategoryVolume(EAudio_AudioCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Set category %d volume to %f"), (int32)Category, Volume);
}

float UAudio_SpatialAudioManager::GetCategoryVolume(EAudio_AudioCategory Category) const
{
    const float* VolumePtr = CategoryVolumes.Find(Category);
    return VolumePtr ? *VolumePtr : 1.0f;
}

float UAudio_SpatialAudioManager::CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance) const
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear attenuation
    return 1.0f - (Distance / MaxDistance);
}

bool UAudio_SpatialAudioManager::IsLocationOccluded(const FVector& SoundLocation, const FVector& ListenerLocation) const
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        SoundLocation,
        ListenerLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return bHit;
}

void UAudio_SpatialAudioManager::InitializeBiomeAmbiences()
{
    // Savana biome
    FAudio_BiomeAmbience SavanaAmbience;
    SavanaAmbience.BiomeType = EAudio_BiomeType::Savana;
    SavanaAmbience.BaseVolume = 0.6f;
    SavanaAmbience.FadeDistance = 1500.0f;
    
    FAudio_SoundEntry SavanaWind;
    SavanaWind.SoundName = TEXT("SavanaWind");
    SavanaWind.Category = EAudio_AudioCategory::Ambient;
    SavanaWind.Volume = 0.4f;
    SavanaWind.AttenuationRadius = 2000.0f;
    SavanaAmbience.AmbientSounds.Add(SavanaWind);
    
    BiomeAmbiences.Add(SavanaAmbience);

    // Forest biome
    FAudio_BiomeAmbience ForestAmbience;
    ForestAmbience.BiomeType = EAudio_BiomeType::Forest;
    ForestAmbience.BaseVolume = 0.7f;
    ForestAmbience.FadeDistance = 1000.0f;
    
    FAudio_SoundEntry ForestBirds;
    ForestBirds.SoundName = TEXT("ForestBirds");
    ForestBirds.Category = EAudio_AudioCategory::Ambient;
    ForestBirds.Volume = 0.5f;
    ForestBirds.AttenuationRadius = 1500.0f;
    ForestAmbience.AmbientSounds.Add(ForestBirds);
    
    BiomeAmbiences.Add(ForestAmbience);

    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Initialized %d biome ambiences"), BiomeAmbiences.Num());
}

void UAudio_SpatialAudioManager::InitializeDinosaurSounds()
{
    // These would be loaded from actual sound assets in a full implementation
    // For now, we register the sound names for future asset loading
    
    DinosaurSounds.Add(TEXT("TRex"), nullptr);
    DinosaurSounds.Add(TEXT("Velociraptor"), nullptr);
    DinosaurSounds.Add(TEXT("Triceratops"), nullptr);
    DinosaurSounds.Add(TEXT("Brachiosaurus"), nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Initialized %d dinosaur sound entries"), DinosaurSounds.Num());
}

void UAudio_SpatialAudioManager::InitializeNarrationSounds()
{
    // Initialize with the narration sounds from Agent #15
    NarrationSounds.Add(TEXT("TribalElder_Warning"), nullptr);
    NarrationSounds.Add(TEXT("Shaman_Discovery"), nullptr);
    NarrationSounds.Add(TEXT("ForestNarrator"), nullptr);
    NarrationSounds.Add(TEXT("TRexEncounter"), nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_SpatialAudioManager: Initialized %d narration sound entries"), NarrationSounds.Num());
}

void UAudio_SpatialAudioManager::CleanupInactiveComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!AudioComp || !AudioComp->IsValidLowLevel() || !AudioComp->IsPlaying())
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UAudio_SpatialAudioManager::CreateAudioComponent(UWorld* World, const FVector& Location)
{
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (AudioComp)
    {
        AudioComp->SetWorldLocation(Location);
        AudioComp->bAutoDestroy = true;
        AudioComp->bStopWhenOwnerDestroyed = true;
        AudioComp->AttenuationSettings = nullptr; // Use default attenuation
    }
    
    return AudioComp;
}