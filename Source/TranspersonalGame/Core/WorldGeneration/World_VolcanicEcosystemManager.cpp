#include "World_VolcanicEcosystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"

AWorld_VolcanicEcosystemManager::AWorld_VolcanicEcosystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create volcanic mesh component
    VolcanicMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolcanicMeshComponent"));
    VolcanicMeshComponent->SetupAttachment(RootComponent);

    // Create particle system component for volcanic smoke
    VolcanicSmokeComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VolcanicSmokeComponent"));
    VolcanicSmokeComponent->SetupAttachment(RootComponent);

    // Create audio component for volcanic sounds
    VolcanicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicAudioComponent"));
    VolcanicAudioComponent->SetupAttachment(RootComponent);

    // Initialize default values
    VolcanicActivityLevel = 0.5f;
    EruptionProbability = 0.01f;
    bVolcanicWeatherEnabled = true;
    MaxHotSprings = 15;
    MaxGeyserFields = 8;
    GeothermalDensity = 0.3f;
    VolcanicUpdateTimer = 0.0f;
    bEffectsActive = false;

    // Initialize default volcanic zones
    FWorld_VolcanicZoneData DefaultZone;
    DefaultZone.ZoneCenter = FVector(8000.0f, 8000.0f, 0.0f);
    DefaultZone.ZoneRadius = 3000.0f;
    DefaultZone.TemperatureIncrease = 30.0f;
    DefaultZone.ToxicGasLevel = 0.4f;
    DefaultZone.bIsActiveVolcano = true;
    VolcanicZones.Add(DefaultZone);
}

void AWorld_VolcanicEcosystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_VolcanicEcosystemManager: BeginPlay started"));
    
    // Initialize volcanic ecosystem
    InitializeVolcanicZones();
    SpawnGeothermalFeatures();
    PlayVolcanicAmbientSound();
    StartVolcanicParticleEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("World_VolcanicEcosystemManager: Initialization complete"));
}

void AWorld_VolcanicEcosystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    VolcanicUpdateTimer += DeltaTime;
    
    // Update volcanic activity every 5 seconds
    if (VolcanicUpdateTimer >= 5.0f)
    {
        UpdateVolcanicActivity(DeltaTime);
        UpdateVolcanicZoneEffects();
        ProcessGeothermalActivity();
        
        if (bVolcanicWeatherEnabled)
        {
            HandleVolcanicWeatherEffects();
        }
        
        VolcanicUpdateTimer = 0.0f;
    }
}

void AWorld_VolcanicEcosystemManager::InitializeVolcanicZones()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing %d volcanic zones"), VolcanicZones.Num());
    
    for (int32 i = 0; i < VolcanicZones.Num(); i++)
    {
        FWorld_VolcanicZoneData& Zone = VolcanicZones[i];
        
        // Validate zone parameters
        if (Zone.ZoneRadius <= 0.0f)
        {
            Zone.ZoneRadius = 2000.0f;
        }
        
        if (Zone.TemperatureIncrease <= 0.0f)
        {
            Zone.TemperatureIncrease = 20.0f;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Volcanic Zone %d: Center(%s), Radius(%.1f), Temp(+%.1f)"), 
               i, *Zone.ZoneCenter.ToString(), Zone.ZoneRadius, Zone.TemperatureIncrease);
    }
}

void AWorld_VolcanicEcosystemManager::SpawnGeothermalFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("Spawning geothermal features..."));
    
    // Clear existing features
    GeothermalFeatures.Empty();
    
    // Spawn hot springs
    for (int32 i = 0; i < MaxHotSprings; i++)
    {
        FVector Location = FindOptimalGeothermalLocation();
        if (IsValidGeothermalLocation(Location))
        {
            CreateGeothermalVent(Location, EWorld_GeothermalType::HotSpring);
        }
    }
    
    // Spawn geyser fields
    for (int32 i = 0; i < MaxGeyserFields; i++)
    {
        FVector Location = FindOptimalGeothermalLocation();
        if (IsValidGeothermalLocation(Location))
        {
            CreateGeothermalVent(Location, EWorld_GeothermalType::GeyserField);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d geothermal features"), GeothermalFeatures.Num());
}

void AWorld_VolcanicEcosystemManager::UpdateVolcanicActivity(float DeltaTime)
{
    // Check for potential eruptions
    for (int32 i = 0; i < VolcanicZones.Num(); i++)
    {
        FWorld_VolcanicZoneData& Zone = VolcanicZones[i];
        
        if (Zone.bIsActiveVolcano)
        {
            float EruptionChance = EruptionProbability * VolcanicActivityLevel * DeltaTime;
            float RandomValue = FMath::FRand();
            
            if (RandomValue < EruptionChance)
            {
                TriggerVolcanicEruption(i);
            }
        }
    }
}

bool AWorld_VolcanicEcosystemManager::IsLocationInVolcanicZone(const FVector& Location, float& TemperatureModifier) const
{
    TemperatureModifier = 0.0f;
    
    for (const FWorld_VolcanicZoneData& Zone : VolcanicZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        
        if (Distance <= Zone.ZoneRadius)
        {
            // Calculate temperature modifier based on distance from center
            float DistanceRatio = 1.0f - (Distance / Zone.ZoneRadius);
            TemperatureModifier = Zone.TemperatureIncrease * DistanceRatio;
            return true;
        }
    }
    
    return false;
}

void AWorld_VolcanicEcosystemManager::TriggerVolcanicEruption(int32 ZoneIndex)
{
    if (!VolcanicZones.IsValidIndex(ZoneIndex))
    {
        return;
    }
    
    const FWorld_VolcanicZoneData& Zone = VolcanicZones[ZoneIndex];
    
    UE_LOG(LogTemp, Warning, TEXT("VOLCANIC ERUPTION triggered at zone %d (%s)"), 
           ZoneIndex, *Zone.ZoneCenter.ToString());
    
    // Increase volcanic activity temporarily
    VolcanicActivityLevel = FMath::Clamp(VolcanicActivityLevel + 0.3f, 0.0f, 1.0f);
    
    // Enhanced particle effects during eruption
    if (VolcanicSmokeComponent && VolcanicSmokeComponent->IsActive())
    {
        VolcanicSmokeComponent->SetFloatParameter(TEXT("IntensityMultiplier"), 3.0f);
    }
}

void AWorld_VolcanicEcosystemManager::CreateGeothermalVent(const FVector& Location, EWorld_GeothermalType VentType)
{
    FWorld_GeothermalFeature NewFeature;
    NewFeature.Location = Location;
    NewFeature.FeatureType = VentType;
    
    switch (VentType)
    {
        case EWorld_GeothermalType::HotSpring:
            NewFeature.Intensity = FMath::RandRange(0.5f, 1.0f);
            NewFeature.EffectRadius = FMath::RandRange(500.0f, 1000.0f);
            NewFeature.bIsHazardous = false;
            break;
            
        case EWorld_GeothermalType::GeyserField:
            NewFeature.Intensity = FMath::RandRange(0.7f, 1.0f);
            NewFeature.EffectRadius = FMath::RandRange(800.0f, 1500.0f);
            NewFeature.bIsHazardous = true;
            break;
            
        case EWorld_GeothermalType::SulfurVent:
            NewFeature.Intensity = FMath::RandRange(0.3f, 0.8f);
            NewFeature.EffectRadius = FMath::RandRange(300.0f, 800.0f);
            NewFeature.bIsHazardous = true;
            break;
            
        default:
            NewFeature.Intensity = 0.5f;
            NewFeature.EffectRadius = 500.0f;
            NewFeature.bIsHazardous = false;
            break;
    }
    
    GeothermalFeatures.Add(NewFeature);
    
    UE_LOG(LogTemp, Log, TEXT("Created geothermal vent: Type(%d), Location(%s), Intensity(%.2f)"), 
           (int32)VentType, *Location.ToString(), NewFeature.Intensity);
}

float AWorld_VolcanicEcosystemManager::GetToxicGasLevelAtLocation(const FVector& Location) const
{
    float TotalToxicity = 0.0f;
    
    // Check volcanic zones
    for (const FWorld_VolcanicZoneData& Zone : VolcanicZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        
        if (Distance <= Zone.ZoneRadius)
        {
            float DistanceRatio = 1.0f - (Distance / Zone.ZoneRadius);
            TotalToxicity += Zone.ToxicGasLevel * DistanceRatio;
        }
    }
    
    // Check geothermal features
    for (const FWorld_GeothermalFeature& Feature : GeothermalFeatures)
    {
        if (Feature.bIsHazardous)
        {
            float Distance = FVector::Dist(Location, Feature.Location);
            
            if (Distance <= Feature.EffectRadius)
            {
                float DistanceRatio = 1.0f - (Distance / Feature.EffectRadius);
                TotalToxicity += 0.2f * Feature.Intensity * DistanceRatio;
            }
        }
    }
    
    return FMath::Clamp(TotalToxicity, 0.0f, 1.0f);
}

TArray<FVector> AWorld_VolcanicEcosystemManager::GetNearbyGeothermalFeatures(const FVector& Location, float SearchRadius) const
{
    TArray<FVector> NearbyFeatures;
    
    for (const FWorld_GeothermalFeature& Feature : GeothermalFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        
        if (Distance <= SearchRadius)
        {
            NearbyFeatures.Add(Feature.Location);
        }
    }
    
    return NearbyFeatures;
}

void AWorld_VolcanicEcosystemManager::PlayVolcanicAmbientSound()
{
    if (VolcanicAudioComponent)
    {
        VolcanicAudioComponent->SetVolumeMultiplier(VolcanicActivityLevel);
        
        if (!VolcanicAudioComponent->IsPlaying())
        {
            VolcanicAudioComponent->Play();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Volcanic ambient sound started"));
    }
}

void AWorld_VolcanicEcosystemManager::StartVolcanicParticleEffects()
{
    if (VolcanicSmokeComponent)
    {
        VolcanicSmokeComponent->SetActive(true);
        bEffectsActive = true;
        
        UE_LOG(LogTemp, Log, TEXT("Volcanic particle effects started"));
    }
}

void AWorld_VolcanicEcosystemManager::StopVolcanicEffects()
{
    if (VolcanicSmokeComponent && VolcanicSmokeComponent->IsActive())
    {
        VolcanicSmokeComponent->SetActive(false);
    }
    
    if (VolcanicAudioComponent && VolcanicAudioComponent->IsPlaying())
    {
        VolcanicAudioComponent->Stop();
    }
    
    bEffectsActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic effects stopped"));
}

void AWorld_VolcanicEcosystemManager::UpdateVolcanicZoneEffects()
{
    // Update particle intensity based on activity level
    if (VolcanicSmokeComponent && bEffectsActive)
    {
        float IntensityMultiplier = 0.5f + (VolcanicActivityLevel * 1.5f);
        VolcanicSmokeComponent->SetFloatParameter(TEXT("IntensityMultiplier"), IntensityMultiplier);
    }
    
    // Update audio volume based on activity
    if (VolcanicAudioComponent)
    {
        float VolumeLevel = 0.3f + (VolcanicActivityLevel * 0.7f);
        VolcanicAudioComponent->SetVolumeMultiplier(VolumeLevel);
    }
}

void AWorld_VolcanicEcosystemManager::ProcessGeothermalActivity()
{
    // Randomly activate geysers
    for (FWorld_GeothermalFeature& Feature : GeothermalFeatures)
    {
        if (Feature.FeatureType == EWorld_GeothermalType::GeyserField)
        {
            float ActivationChance = 0.05f * Feature.Intensity;
            
            if (FMath::FRand() < ActivationChance)
            {
                UE_LOG(LogTemp, Log, TEXT("Geyser activated at %s"), *Feature.Location.ToString());
                // Could spawn temporary particle effect here
            }
        }
    }
}

void AWorld_VolcanicEcosystemManager::HandleVolcanicWeatherEffects()
{
    // Reduce volcanic activity over time
    if (VolcanicActivityLevel > 0.1f)
    {
        VolcanicActivityLevel = FMath::Max(0.1f, VolcanicActivityLevel - 0.02f);
    }
    
    // Weather effects could modify temperature and air quality
    // This would integrate with weather system when available
}

FVector AWorld_VolcanicEcosystemManager::FindOptimalGeothermalLocation() const
{
    // Find location near volcanic zones but not too close to existing features
    FVector OptimalLocation = FVector::ZeroVector;
    
    if (VolcanicZones.Num() > 0)
    {
        // Pick a random volcanic zone
        int32 ZoneIndex = FMath::RandRange(0, VolcanicZones.Num() - 1);
        const FWorld_VolcanicZoneData& Zone = VolcanicZones[ZoneIndex];
        
        // Generate location within zone but not at center
        float Distance = FMath::RandRange(Zone.ZoneRadius * 0.3f, Zone.ZoneRadius * 0.8f);
        float Angle = FMath::RandRange(0.0f, 360.0f);
        
        OptimalLocation = Zone.ZoneCenter + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
    }
    else
    {
        // Fallback to random location
        OptimalLocation = FVector(
            FMath::RandRange(-10000.0f, 10000.0f),
            FMath::RandRange(-10000.0f, 10000.0f),
            0.0f
        );
    }
    
    return OptimalLocation;
}

bool AWorld_VolcanicEcosystemManager::IsValidGeothermalLocation(const FVector& Location) const
{
    // Check minimum distance from existing features
    const float MinDistance = 1000.0f;
    
    for (const FWorld_GeothermalFeature& Feature : GeothermalFeatures)
    {
        if (FVector::Dist(Location, Feature.Location) < MinDistance)
        {
            return false;
        }
    }
    
    return true;
}