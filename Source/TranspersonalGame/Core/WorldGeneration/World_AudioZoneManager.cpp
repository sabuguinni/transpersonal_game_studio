#include "World_AudioZoneManager.h"
#include "Components/AudioComponent.h"
#include "Engine/AudioVolume.h"
#include "Sound/AmbientSound.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AWorld_AudioZoneManager::AWorld_AudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio components
    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    TimeOfDayAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TimeOfDayAudioComponent"));
    
    // Set default values
    CurrentBiomeAudio = EWorld_BiomeAudioType::Forest;
    CurrentWeatherIntensity = 0.0f;
    CurrentTimeOfDay = 12.0f; // Noon
    BiomeTransitionTime = 2.0f;
    WeatherAudioVolume = 0.3f;
    TimeOfDayAudioVolume = 0.2f;
}

void AWorld_AudioZoneManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default biome zones if none are set
    if (BiomeAudioZones.Num() == 0)
    {
        InitializeDefaultBiomeZones();
    }
    
    // Create audio zones
    InitializeBiomeAudioZones();
}

void AWorld_AudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update player audio zone based on location
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        UpdatePlayerAudioZone(PlayerLocation);
    }
    
    // Update audio zone volumes
    UpdateAudioZoneVolumes();
}

void AWorld_AudioZoneManager::InitializeBiomeAudioZones()
{
    // Clear existing audio volumes and sounds
    for (AAudioVolume* Volume : SpawnedAudioVolumes)
    {
        if (IsValid(Volume))
        {
            Volume->Destroy();
        }
    }
    SpawnedAudioVolumes.Empty();
    
    for (AAmbientSound* Sound : SpawnedAmbientSounds)
    {
        if (IsValid(Sound))
        {
            Sound->Destroy();
        }
    }
    SpawnedAmbientSounds.Empty();
    
    // Create audio zones for each biome
    for (const FWorld_BiomeAudioData& AudioData : BiomeAudioZones)
    {
        CreateAudioZone(AudioData);
    }
}

void AWorld_AudioZoneManager::CreateAudioZone(const FWorld_BiomeAudioData& AudioData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn audio volume
    AAudioVolume* AudioVolume = World->SpawnActor<AAudioVolume>(
        AAudioVolume::StaticClass(),
        AudioData.ZoneLocation,
        FRotator::ZeroRotator
    );
    
    if (AudioVolume)
    {
        // Set volume scale
        AudioVolume->SetActorScale3D(AudioData.ZoneScale);
        
        // Configure audio volume properties
        AudioVolume->Priority = 1.0f;
        AudioVolume->bEnabled = true;
        
        SpawnedAudioVolumes.Add(AudioVolume);
    }
    
    // Spawn ambient sound
    AAmbientSound* AmbientSound = World->SpawnActor<AAmbientSound>(
        AAmbientSound::StaticClass(),
        AudioData.ZoneLocation,
        FRotator::ZeroRotator
    );
    
    if (AmbientSound && AmbientSound->GetAudioComponent())
    {
        // Configure ambient sound
        AmbientSound->GetAudioComponent()->SetVolumeMultiplier(AudioData.AmbientVolume);
        AmbientSound->GetAudioComponent()->bAutoActivate = true;
        AmbientSound->GetAudioComponent()->Play();
        
        SpawnedAmbientSounds.Add(AmbientSound);
    }
}

void AWorld_AudioZoneManager::UpdatePlayerAudioZone(const FVector& PlayerLocation)
{
    EWorld_BiomeAudioType NewBiomeAudio = EWorld_BiomeAudioType::Forest;
    float ClosestDistance = FLT_MAX;
    
    // Find the closest biome audio zone
    for (const FWorld_BiomeAudioData& AudioData : BiomeAudioZones)
    {
        float Distance = CalculateDistanceToZone(PlayerLocation, AudioData);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            NewBiomeAudio = AudioData.BiomeType;
        }
    }
    
    // Trigger biome transition if changed
    if (NewBiomeAudio != CurrentBiomeAudio)
    {
        PlayBiomeTransitionSound(CurrentBiomeAudio, NewBiomeAudio);
        CrossfadeBiomeAudio(NewBiomeAudio);
        CurrentBiomeAudio = NewBiomeAudio;
    }
}

EWorld_BiomeAudioType AWorld_AudioZoneManager::GetCurrentBiomeAudio() const
{
    return CurrentBiomeAudio;
}

void AWorld_AudioZoneManager::SetWeatherAudio(bool bIsRaining, bool bIsStormy)
{
    if (!WeatherAudioComponent)
    {
        return;
    }
    
    if (bIsStormy)
    {
        CurrentWeatherIntensity = 1.0f;
        WeatherAudioComponent->SetVolumeMultiplier(WeatherAudioVolume * 1.5f);
    }
    else if (bIsRaining)
    {
        CurrentWeatherIntensity = 0.6f;
        WeatherAudioComponent->SetVolumeMultiplier(WeatherAudioVolume);
    }
    else
    {
        CurrentWeatherIntensity = 0.0f;
        WeatherAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void AWorld_AudioZoneManager::SetTimeOfDayAudio(float TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    
    if (!TimeOfDayAudioComponent)
    {
        return;
    }
    
    // Adjust audio based on time of day
    float NightVolume = 0.0f;
    
    if (TimeOfDay >= 20.0f || TimeOfDay <= 6.0f) // Night time
    {
        NightVolume = TimeOfDayAudioVolume;
    }
    else if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f) // Evening transition
    {
        float TransitionFactor = (TimeOfDay - 18.0f) / 2.0f;
        NightVolume = TimeOfDayAudioVolume * TransitionFactor;
    }
    else if (TimeOfDay > 6.0f && TimeOfDay <= 8.0f) // Morning transition
    {
        float TransitionFactor = 1.0f - ((TimeOfDay - 6.0f) / 2.0f);
        NightVolume = TimeOfDayAudioVolume * TransitionFactor;
    }
    
    TimeOfDayAudioComponent->SetVolumeMultiplier(NightVolume);
}

void AWorld_AudioZoneManager::PlayBiomeTransitionSound(EWorld_BiomeAudioType FromBiome, EWorld_BiomeAudioType ToBiome)
{
    // Log biome transition for debugging
    UE_LOG(LogTemp, Log, TEXT("Biome audio transition: %d -> %d"), 
           static_cast<int32>(FromBiome), static_cast<int32>(ToBiome));
    
    // Here you would play specific transition sounds based on biome changes
    // For now, we'll just log the transition
}

void AWorld_AudioZoneManager::UpdateAudioZoneVolumes()
{
    // Update volumes of spawned ambient sounds based on distance and settings
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    for (int32 i = 0; i < SpawnedAmbientSounds.Num() && i < BiomeAudioZones.Num(); ++i)
    {
        AAmbientSound* AmbientSound = SpawnedAmbientSounds[i];
        const FWorld_BiomeAudioData& AudioData = BiomeAudioZones[i];
        
        if (IsValid(AmbientSound) && AmbientSound->GetAudioComponent())
        {
            float Distance = CalculateDistanceToZone(PlayerLocation, AudioData);
            float VolumeMultiplier = 1.0f;
            
            if (Distance > AudioData.AudioFadeDistance)
            {
                VolumeMultiplier = 0.0f;
            }
            else if (Distance > AudioData.AudioFadeDistance * 0.5f)
            {
                float FadeRange = AudioData.AudioFadeDistance * 0.5f;
                float FadeDistance = Distance - (AudioData.AudioFadeDistance * 0.5f);
                VolumeMultiplier = 1.0f - (FadeDistance / FadeRange);
            }
            
            float FinalVolume = AudioData.AmbientVolume * VolumeMultiplier;
            AmbientSound->GetAudioComponent()->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void AWorld_AudioZoneManager::CrossfadeBiomeAudio(EWorld_BiomeAudioType NewBiome)
{
    // Implement crossfade logic between biome audio
    // This would typically involve fading out old audio and fading in new audio
    // For now, we'll implement a simple immediate transition
    
    CurrentBiomeAudio = NewBiome;
}

float AWorld_AudioZoneManager::CalculateDistanceToZone(const FVector& PlayerLocation, const FWorld_BiomeAudioData& ZoneData) const
{
    // Calculate distance from player to the edge of the audio zone
    FVector ZoneMin = ZoneData.ZoneLocation - (ZoneData.ZoneScale * 50.0f); // Assuming 50 unit scale factor
    FVector ZoneMax = ZoneData.ZoneLocation + (ZoneData.ZoneScale * 50.0f);
    
    FVector ClosestPoint;
    ClosestPoint.X = FMath::Clamp(PlayerLocation.X, ZoneMin.X, ZoneMax.X);
    ClosestPoint.Y = FMath::Clamp(PlayerLocation.Y, ZoneMin.Y, ZoneMax.Y);
    ClosestPoint.Z = FMath::Clamp(PlayerLocation.Z, ZoneMin.Z, ZoneMax.Z);
    
    return FVector::Dist(PlayerLocation, ClosestPoint);
}

void AWorld_AudioZoneManager::InitializeDefaultBiomeZones()
{
    BiomeAudioZones.Empty();
    
    // Forest biome zone
    FWorld_BiomeAudioData ForestZone;
    ForestZone.BiomeType = EWorld_BiomeAudioType::Forest;
    ForestZone.ZoneLocation = FVector(-1000.0f, 0.0f, 100.0f);
    ForestZone.ZoneScale = FVector(20.0f, 40.0f, 10.0f);
    ForestZone.AmbientVolume = 0.6f;
    ForestZone.AudioFadeDistance = 2500.0f;
    BiomeAudioZones.Add(ForestZone);
    
    // Swamp biome zone
    FWorld_BiomeAudioData SwampZone;
    SwampZone.BiomeType = EWorld_BiomeAudioType::Swamp;
    SwampZone.ZoneLocation = FVector(1000.0f, -1000.0f, 0.0f);
    SwampZone.ZoneScale = FVector(20.0f, 20.0f, 8.0f);
    SwampZone.AmbientVolume = 0.5f;
    SwampZone.AudioFadeDistance = 2000.0f;
    BiomeAudioZones.Add(SwampZone);
    
    // Savanna biome zone
    FWorld_BiomeAudioData SavannaZone;
    SavannaZone.BiomeType = EWorld_BiomeAudioType::Savanna;
    SavannaZone.ZoneLocation = FVector(1000.0f, 1000.0f, 50.0f);
    SavannaZone.ZoneScale = FVector(20.0f, 20.0f, 5.0f);
    SavannaZone.AmbientVolume = 0.4f;
    SavannaZone.AudioFadeDistance = 3000.0f;
    BiomeAudioZones.Add(SavannaZone);
    
    // Desert biome zone
    FWorld_BiomeAudioData DesertZone;
    DesertZone.BiomeType = EWorld_BiomeAudioType::Desert;
    DesertZone.ZoneLocation = FVector(-1000.0f, 1000.0f, 80.0f);
    DesertZone.ZoneScale = FVector(20.0f, 20.0f, 8.0f);
    DesertZone.AmbientVolume = 0.3f;
    DesertZone.AudioFadeDistance = 2500.0f;
    BiomeAudioZones.Add(DesertZone);
    
    // Mountain biome zone
    FWorld_BiomeAudioData MountainZone;
    MountainZone.BiomeType = EWorld_BiomeAudioType::Mountain;
    MountainZone.ZoneLocation = FVector(0.0f, 0.0f, 300.0f);
    MountainZone.ZoneScale = FVector(15.0f, 15.0f, 15.0f);
    MountainZone.AmbientVolume = 0.4f;
    MountainZone.AudioFadeDistance = 2000.0f;
    BiomeAudioZones.Add(MountainZone);
}