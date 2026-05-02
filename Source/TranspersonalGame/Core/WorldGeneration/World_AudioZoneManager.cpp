#include "World_AudioZoneManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

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
    WeatherAudioVolume = 0.7f;
    TimeOfDayAudioVolume = 0.3f;

    // Initialize default biome zones
    InitializeDefaultBiomeZones();
}

void AWorld_AudioZoneManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome audio zones in the world
    InitializeBiomeAudioZones();
}

void AWorld_AudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player location and update audio zones
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdatePlayerAudioZone(PlayerPawn->GetActorLocation());
    }

    // Update audio zone volumes based on distance
    UpdateAudioZoneVolumes();
}

void AWorld_AudioZoneManager::InitializeBiomeAudioZones()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioZoneManager: No world found"));
        return;
    }

    // Clear existing spawned actors
    for (AAudioVolume* Volume : SpawnedAudioVolumes)
    {
        if (Volume && IsValid(Volume))
        {
            Volume->Destroy();
        }
    }
    SpawnedAudioVolumes.Empty();

    for (AAmbientSound* Sound : SpawnedAmbientSounds)
    {
        if (Sound && IsValid(Sound))
        {
            Sound->Destroy();
        }
    }
    SpawnedAmbientSounds.Empty();

    // Create audio zones for each biome
    for (const FWorld_BiomeAudioData& ZoneData : BiomeAudioZones)
    {
        CreateAudioZone(ZoneData);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Initialized %d biome audio zones"), BiomeAudioZones.Num());
}

void AWorld_AudioZoneManager::CreateAudioZone(const FWorld_BiomeAudioData& AudioData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn audio volume
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    
    AAudioVolume* AudioVolume = World->SpawnActor<AAudioVolume>(
        AAudioVolume::StaticClass(),
        AudioData.ZoneLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (AudioVolume)
    {
        AudioVolume->SetActorScale3D(AudioData.ZoneScale);
        SpawnedAudioVolumes.Add(AudioVolume);

        // Set audio volume label based on biome type
        FString BiomeName;
        switch (AudioData.BiomeType)
        {
            case EWorld_BiomeAudioType::Forest:
                BiomeName = TEXT("Forest_Audio_Zone");
                break;
            case EWorld_BiomeAudioType::Swamp:
                BiomeName = TEXT("Swamp_Audio_Zone");
                break;
            case EWorld_BiomeAudioType::Savanna:
                BiomeName = TEXT("Savanna_Audio_Zone");
                break;
            case EWorld_BiomeAudioType::Desert:
                BiomeName = TEXT("Desert_Audio_Zone");
                break;
            case EWorld_BiomeAudioType::Mountain:
                BiomeName = TEXT("Mountain_Audio_Zone");
                break;
        }
        AudioVolume->SetActorLabel(BiomeName);
    }

    // Spawn ambient sound
    AAmbientSound* AmbientSound = World->SpawnActor<AAmbientSound>(
        AAmbientSound::StaticClass(),
        AudioData.ZoneLocation + FVector(0, 0, 100),
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (AmbientSound)
    {
        SpawnedAmbientSounds.Add(AmbientSound);
        
        // Set ambient sound properties
        if (AmbientSound->GetAudioComponent())
        {
            AmbientSound->GetAudioComponent()->SetVolumeMultiplier(AudioData.AmbientVolume);
            AmbientSound->GetAudioComponent()->bAutoActivate = true;
        }

        FString SoundName = BiomeName.Replace(TEXT("_Zone"), TEXT("_Ambient"));
        AmbientSound->SetActorLabel(SoundName);
    }
}

void AWorld_AudioZoneManager::UpdatePlayerAudioZone(const FVector& PlayerLocation)
{
    if (BiomeAudioZones.Num() == 0)
    {
        return;
    }

    // Find closest biome zone
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeAudioType ClosestBiome = CurrentBiomeAudio;

    for (const FWorld_BiomeAudioData& ZoneData : BiomeAudioZones)
    {
        float Distance = CalculateDistanceToZone(PlayerLocation, ZoneData);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = ZoneData.BiomeType;
        }
    }

    // Check if biome changed
    if (ClosestBiome != CurrentBiomeAudio)
    {
        EWorld_BiomeAudioType PreviousBiome = CurrentBiomeAudio;
        CurrentBiomeAudio = ClosestBiome;
        
        // Trigger biome transition
        CrossfadeBiomeAudio(CurrentBiomeAudio);
        PlayBiomeTransitionSound(PreviousBiome, CurrentBiomeAudio);
        
        UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Biome audio changed to %d"), (int32)CurrentBiomeAudio);
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

    float TargetIntensity = 0.0f;
    if (bIsStormy)
    {
        TargetIntensity = 1.0f;
    }
    else if (bIsRaining)
    {
        TargetIntensity = 0.5f;
    }

    CurrentWeatherIntensity = TargetIntensity;
    WeatherAudioComponent->SetVolumeMultiplier(TargetIntensity * WeatherAudioVolume);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Weather audio intensity set to %f"), TargetIntensity);
}

void AWorld_AudioZoneManager::SetTimeOfDayAudio(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 24.0f);
    
    if (!TimeOfDayAudioComponent)
    {
        return;
    }

    // Calculate night audio intensity (higher during night hours)
    float NightIntensity = 0.0f;
    if (CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay <= 6.0f)
    {
        // Night time (8 PM to 6 AM)
        NightIntensity = 1.0f;
    }
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
    {
        // Evening transition
        NightIntensity = (CurrentTimeOfDay - 18.0f) / 2.0f;
    }
    else if (CurrentTimeOfDay > 6.0f && CurrentTimeOfDay <= 8.0f)
    {
        // Morning transition
        NightIntensity = 1.0f - ((CurrentTimeOfDay - 6.0f) / 2.0f);
    }

    TimeOfDayAudioComponent->SetVolumeMultiplier(NightIntensity * TimeOfDayAudioVolume);
    
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Time of day audio set for %f hours (night intensity: %f)"), 
           CurrentTimeOfDay, NightIntensity);
}

void AWorld_AudioZoneManager::PlayBiomeTransitionSound(EWorld_BiomeAudioType FromBiome, EWorld_BiomeAudioType ToBiome)
{
    // Play a transition sound effect when moving between biomes
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Playing biome transition from %d to %d"), 
           (int32)FromBiome, (int32)ToBiome);
    
    // This would play a specific transition sound based on biome types
    // For now, just log the transition
}

void AWorld_AudioZoneManager::UpdateAudioZoneVolumes()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Update volume for each ambient sound based on distance
    for (int32 i = 0; i < SpawnedAmbientSounds.Num() && i < BiomeAudioZones.Num(); ++i)
    {
        AAmbientSound* AmbientSound = SpawnedAmbientSounds[i];
        if (!AmbientSound || !AmbientSound->GetAudioComponent())
        {
            continue;
        }

        const FWorld_BiomeAudioData& ZoneData = BiomeAudioZones[i];
        float Distance = CalculateDistanceToZone(PlayerLocation, ZoneData);
        
        // Calculate volume based on distance
        float VolumeMultiplier = 1.0f;
        if (Distance > ZoneData.AudioFadeDistance)
        {
            VolumeMultiplier = 0.0f;
        }
        else if (Distance > ZoneData.AudioFadeDistance * 0.5f)
        {
            float FadeRatio = (Distance - (ZoneData.AudioFadeDistance * 0.5f)) / (ZoneData.AudioFadeDistance * 0.5f);
            VolumeMultiplier = 1.0f - FadeRatio;
        }

        VolumeMultiplier *= ZoneData.AmbientVolume;
        AmbientSound->GetAudioComponent()->SetVolumeMultiplier(VolumeMultiplier);
    }
}

void AWorld_AudioZoneManager::CrossfadeBiomeAudio(EWorld_BiomeAudioType NewBiome)
{
    // Implement crossfading between biome audio
    // This would smoothly transition between different ambient sounds
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Crossfading to biome audio %d"), (int32)NewBiome);
}

float AWorld_AudioZoneManager::CalculateDistanceToZone(const FVector& PlayerLocation, const FWorld_BiomeAudioData& ZoneData) const
{
    return FVector::Dist(PlayerLocation, ZoneData.ZoneLocation);
}

void AWorld_AudioZoneManager::InitializeDefaultBiomeZones()
{
    BiomeAudioZones.Empty();

    // Forest biome
    FWorld_BiomeAudioData ForestZone;
    ForestZone.BiomeType = EWorld_BiomeAudioType::Forest;
    ForestZone.ZoneLocation = FVector(2000, 0, 100);
    ForestZone.ZoneScale = FVector(20, 20, 10);
    ForestZone.AmbientVolume = 0.6f;
    ForestZone.AudioFadeDistance = 2500.0f;
    BiomeAudioZones.Add(ForestZone);

    // Swamp biome
    FWorld_BiomeAudioData SwampZone;
    SwampZone.BiomeType = EWorld_BiomeAudioType::Swamp;
    SwampZone.ZoneLocation = FVector(-2000, 2000, 50);
    SwampZone.ZoneScale = FVector(18, 18, 8);
    SwampZone.AmbientVolume = 0.7f;
    SwampZone.AudioFadeDistance = 2000.0f;
    BiomeAudioZones.Add(SwampZone);

    // Savanna biome
    FWorld_BiomeAudioData SavannaZone;
    SavannaZone.BiomeType = EWorld_BiomeAudioType::Savanna;
    SavannaZone.ZoneLocation = FVector(0, -2000, 150);
    SavannaZone.ZoneScale = FVector(25, 25, 12);
    SavannaZone.AmbientVolume = 0.5f;
    SavannaZone.AudioFadeDistance = 3000.0f;
    BiomeAudioZones.Add(SavannaZone);

    // Desert biome
    FWorld_BiomeAudioData DesertZone;
    DesertZone.BiomeType = EWorld_BiomeAudioType::Desert;
    DesertZone.ZoneLocation = FVector(3000, 3000, 200);
    DesertZone.ZoneScale = FVector(22, 22, 15);
    DesertZone.AmbientVolume = 0.4f;
    DesertZone.AudioFadeDistance = 2800.0f;
    BiomeAudioZones.Add(DesertZone);

    // Mountain biome
    FWorld_BiomeAudioData MountainZone;
    MountainZone.BiomeType = EWorld_BiomeAudioType::Mountain;
    MountainZone.ZoneLocation = FVector(-3000, -3000, 400);
    MountainZone.ZoneScale = FVector(20, 20, 20);
    MountainZone.AmbientVolume = 0.5f;
    MountainZone.AudioFadeDistance = 3500.0f;
    BiomeAudioZones.Add(MountainZone);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Initialized %d default biome zones"), BiomeAudioZones.Num());
}