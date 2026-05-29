#include "Audio_SpatialAudioManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

UAudio_SpatialAudioManager::UAudio_SpatialAudioManager()
{
    MaxConcurrentSounds = 32;
    GlobalVolumeMultiplier = 1.0f;
    OcclusionTraceDistance = 10000.0f;
    CurrentBiome = TEXT("Savana");
    PlayerLocation = FVector::ZeroVector;
    CurrentAmbienceComponent = nullptr;
    NarrationComponent = nullptr;
}

void UAudio_SpatialAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategoryVolumes();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager initialized"));
}

void UAudio_SpatialAudioManager::Deinitialize()
{
    // Clean up all active sounds
    for (UAudioComponent* Component : ActiveSounds)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    ActiveSounds.Empty();
    
    if (IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->Stop();
        CurrentAmbienceComponent = nullptr;
    }
    
    if (IsValid(NarrationComponent))
    {
        NarrationComponent->Stop();
        NarrationComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_SpatialAudioManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Player, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Narration, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
}

UAudioComponent* UAudio_SpatialAudioManager::PlaySoundAtLocation(USoundCue* SoundCue, const FVector& Location, 
    float Volume, float Pitch, bool bAutoDestroy)
{
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySoundAtLocation: SoundCue is null"));
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySoundAtLocation: World is null"));
        return nullptr;
    }
    
    // Check if we've reached max concurrent sounds
    CleanupFinishedSounds();
    if (ActiveSounds.Num() >= MaxConcurrentSounds)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaySoundAtLocation: Max concurrent sounds reached"));
        return nullptr;
    }
    
    // Calculate distance-based volume
    float Distance = FVector::Dist(Location, PlayerLocation);
    float DistanceVolume = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);
    float FinalVolume = Volume * DistanceVolume * GlobalVolumeMultiplier;
    
    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        World, SoundCue, Location, FRotator::ZeroRotator, FinalVolume, Pitch);
    
    if (AudioComponent)
    {
        ActiveSounds.Add(AudioComponent);
        UE_LOG(LogTemp, Log, TEXT("PlaySoundAtLocation: Spawned audio at %s, Volume: %f"), 
            *Location.ToString(), FinalVolume);
    }
    
    return AudioComponent;
}

UAudioComponent* UAudio_SpatialAudioManager::PlaySpatialSound(const FAudio_SpatialSound& SpatialSound)
{
    if (!CanPlaySound(SpatialSound.Priority, SpatialSound.Category))
    {
        return nullptr;
    }
    
    float CategoryVolume = GetCategoryVolume(SpatialSound.Category);
    float FinalVolume = SpatialSound.Volume * CategoryVolume;
    
    UAudioComponent* Component = PlaySoundAtLocation(SpatialSound.SoundCue, SpatialSound.Location, 
        FinalVolume, 1.0f, !SpatialSound.bIsLooping);
    
    return Component;
}

void UAudio_SpatialAudioManager::StopSoundsByCategory(EAudio_SoundCategory Category)
{
    for (int32 i = ActiveSounds.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Component = ActiveSounds[i];
        if (IsValid(Component))
        {
            // Note: In a full implementation, you'd store category info with each component
            Component->Stop();
            ActiveSounds.RemoveAt(i);
        }
    }
}

void UAudio_SpatialAudioManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
}

float UAudio_SpatialAudioManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* VolumePtr = CategoryVolumes.Find(Category);
    return VolumePtr ? *VolumePtr : 1.0f;
}

void UAudio_SpatialAudioManager::RegisterBiomeAmbience(const FAudio_BiomeAmbience& BiomeAmbience)
{
    RegisteredBiomes.Add(BiomeAmbience);
    UE_LOG(LogTemp, Log, TEXT("RegisterBiomeAmbience: Registered biome %s"), *BiomeAmbience.BiomeName);
}

void UAudio_SpatialAudioManager::UpdatePlayerLocation(const FVector& NewPlayerLocation)
{
    PlayerLocation = NewPlayerLocation;
    UpdateAmbienceBasedOnLocation();
}

void UAudio_SpatialAudioManager::TransitionToBiome(const FString& BiomeName, float TransitionTime)
{
    if (CurrentBiome == BiomeName)
    {
        return;
    }
    
    // Find the biome ambience
    const FAudio_BiomeAmbience* BiomeAmbience = RegisteredBiomes.FindByPredicate(
        [&BiomeName](const FAudio_BiomeAmbience& Biome)
        {
            return Biome.BiomeName == BiomeName;
        });
    
    if (!BiomeAmbience)
    {
        UE_LOG(LogTemp, Warning, TEXT("TransitionToBiome: Biome %s not found"), *BiomeName);
        return;
    }
    
    // Stop current ambience
    if (IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->FadeOut(TransitionTime, 0.0f);
    }
    
    // Start new ambience
    if (BiomeAmbience->AmbientSounds.Num() > 0)
    {
        USoundCue* NewAmbience = BiomeAmbience->AmbientSounds[0]; // Use first sound for now
        if (NewAmbience)
        {
            CurrentAmbienceComponent = PlaySoundAtLocation(NewAmbience, PlayerLocation, 
                BiomeAmbience->BaseVolume, 1.0f, false);
            if (CurrentAmbienceComponent)
            {
                CurrentAmbienceComponent->FadeIn(TransitionTime, BiomeAmbience->BaseVolume);
            }
        }
    }
    
    CurrentBiome = BiomeName;
    UE_LOG(LogTemp, Log, TEXT("TransitionToBiome: Transitioned to %s"), *BiomeName);
}

void UAudio_SpatialAudioManager::PlayDinosaurSound(USoundCue* DinosaurSound, const FVector& DinosaurLocation, 
    float ThreatLevel)
{
    if (!DinosaurSound)
    {
        return;
    }
    
    float ThreatVolume = FMath::Clamp(ThreatLevel, 0.5f, 2.0f);
    float CategoryVolume = GetCategoryVolume(EAudio_SoundCategory::Dinosaur);
    float FinalVolume = ThreatVolume * CategoryVolume;
    
    UAudioComponent* Component = PlaySoundAtLocation(DinosaurSound, DinosaurLocation, FinalVolume);
    
    UE_LOG(LogTemp, Log, TEXT("PlayDinosaurSound: Played at %s, ThreatLevel: %f"), 
        *DinosaurLocation.ToString(), ThreatLevel);
}

void UAudio_SpatialAudioManager::PlayFootstepSound(const FVector& Location, float CreatureSize)
{
    // In a full implementation, you'd have different footstep sounds based on size
    float SizeVolume = FMath::Clamp(CreatureSize * 0.5f, 0.1f, 1.5f);
    
    // For now, just log the footstep
    UE_LOG(LogTemp, Log, TEXT("PlayFootstepSound: Size %f at %s"), CreatureSize, *Location.ToString());
}

void UAudio_SpatialAudioManager::PlayNarration(USoundCue* NarrationSound, bool bPauseOtherAudio)
{
    if (!NarrationSound)
    {
        return;
    }
    
    if (bPauseOtherAudio)
    {
        // Pause ambient and music
        SetCategoryVolume(EAudio_SoundCategory::Ambient, 0.3f);
        SetCategoryVolume(EAudio_SoundCategory::Music, 0.2f);
    }
    
    // Stop previous narration
    if (IsValid(NarrationComponent))
    {
        NarrationComponent->Stop();
    }
    
    float NarrationVolume = GetCategoryVolume(EAudio_SoundCategory::Narration);
    NarrationComponent = PlaySoundAtLocation(NarrationSound, PlayerLocation, NarrationVolume);
    
    UE_LOG(LogTemp, Log, TEXT("PlayNarration: Started narration"));
}

void UAudio_SpatialAudioManager::StopNarration()
{
    if (IsValid(NarrationComponent))
    {
        NarrationComponent->Stop();
        NarrationComponent = nullptr;
    }
    
    // Restore other audio volumes
    SetCategoryVolume(EAudio_SoundCategory::Ambient, 0.8f);
    SetCategoryVolume(EAudio_SoundCategory::Music, 0.6f);
    
    UE_LOG(LogTemp, Log, TEXT("StopNarration: Stopped narration"));
}

float UAudio_SpatialAudioManager::CalculateOcclusion(const FVector& SourceLocation, const FVector& ListenerLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 1.0f;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        SourceLocation,
        ListenerLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Simple occlusion calculation
        float Distance = FVector::Dist(SourceLocation, ListenerLocation);
        float OcclusionFactor = FMath::Clamp(Distance / OcclusionTraceDistance, 0.3f, 1.0f);
        return OcclusionFactor;
    }
    
    return 1.0f; // No occlusion
}

bool UAudio_SpatialAudioManager::CanPlaySound(int32 Priority, EAudio_SoundCategory Category)
{
    CleanupFinishedSounds();
    
    if (ActiveSounds.Num() < MaxConcurrentSounds)
    {
        return true;
    }
    
    // Check if we can replace a lower priority sound
    // For now, just check count
    return ActiveSounds.Num() < MaxConcurrentSounds;
}

void UAudio_SpatialAudioManager::CleanupFinishedSounds()
{
    for (int32 i = ActiveSounds.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Component = ActiveSounds[i];
        if (!IsValid(Component) || !Component->IsPlaying())
        {
            ActiveSounds.RemoveAt(i);
        }
    }
}

FString UAudio_SpatialAudioManager::DetermineBiomeFromLocation(const FVector& Location)
{
    // Biome detection based on coordinates from memory
    if (Location.X >= -10000 && Location.X <= 10000 && Location.Y >= -10000 && Location.Y <= 10000)
    {
        return TEXT("Savana");
    }
    else if (Location.X >= -60000 && Location.X <= -40000 && Location.Y >= -55000 && Location.Y <= -35000)
    {
        return TEXT("Pantano");
    }
    else if (Location.X >= -55000 && Location.X <= -35000 && Location.Y >= 30000 && Location.Y <= 50000)
    {
        return TEXT("Floresta");
    }
    else if (Location.X >= 45000 && Location.X <= 65000 && Location.Y >= -10000 && Location.Y <= 10000)
    {
        return TEXT("Deserto");
    }
    else if (Location.X >= 30000 && Location.X <= 50000 && Location.Y >= 40000 && Location.Y <= 60000)
    {
        return TEXT("Montanha");
    }
    
    return TEXT("Savana"); // Default
}

void UAudio_SpatialAudioManager::UpdateAmbienceBasedOnLocation()
{
    FString NewBiome = DetermineBiomeFromLocation(PlayerLocation);
    if (NewBiome != CurrentBiome)
    {
        TransitionToBiome(NewBiome, 2.0f);
    }
}