#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TriggerVolume.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create biome marker mesh component
    BiomeMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BiomeMarkerMesh"));
    BiomeMarkerMesh->SetupAttachment(RootComponent);

    // Create audio component for biome ambient sounds
    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudioComponent"));
    BiomeAudioComponent->SetupAttachment(RootComponent);
    BiomeAudioComponent->bAutoActivate = false;

    // Set default values
    MaxVegetationPerBiome = 100;
    BiomeTransitionBlendDistance = 200.0f;
    bEnablePerformanceOptimization = true;
    LODDistance = 5000.0f;

    // Load default sphere mesh for biome markers
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        BiomeMarkerMesh->SetStaticMesh(SphereMeshAsset.Object);
        BiomeMarkerMesh->SetWorldScale3D(FVector(5.0f, 5.0f, 5.0f));
    }
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: BeginPlay started"));
    
    // Initialize biomes if not already set up
    if (BiomeZones.Num() == 0)
    {
        CreateDefaultBiomes();
    }
    
    InitializeBiomes();
    SpawnBiomeMarkers();
    SetupAudioZones();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initialization complete with %d biomes"), BiomeZones.Num());
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnablePerformanceOptimization)
    {
        // Get player location for performance optimization
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            OptimizeBiomePerformance(PlayerPawn->GetActorLocation());
        }
    }
}

void AWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initializing biomes"));
    
    for (FWorld_BiomeData& Biome : BiomeZones)
    {
        ApplyBiomeSpecificSettings(Biome.BiomeType);
        GenerateVegetationForBiome(Biome.BiomeType);
    }
}

void AWorld_BiomeManager::CreateDefaultBiomes()
{
    // Forest biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.Center = FVector(2000.0f, 0.0f, 100.0f);
    ForestBiome.Radius = 1500.0f;
    ForestBiome.BiomeColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.0f);
    ForestBiome.VegetationDensity = 0.8f;
    ForestBiome.TemperatureModifier = -5.0f;
    ForestBiome.HumidityModifier = 20.0f;
    ForestBiome.AudioZones.Add(TEXT("forest_ambient"));
    BiomeZones.Add(ForestBiome);

    // Plains biome
    FWorld_BiomeData PlainsBiome;
    PlainsBiome.BiomeType = EWorld_BiomeType::Plains;
    PlainsBiome.Center = FVector(-2000.0f, 2000.0f, 50.0f);
    PlainsBiome.Radius = 1200.0f;
    PlainsBiome.BiomeColor = FLinearColor(0.8f, 0.8f, 0.2f, 1.0f);
    PlainsBiome.VegetationDensity = 0.2f;
    PlainsBiome.TemperatureModifier = 5.0f;
    PlainsBiome.HumidityModifier = -10.0f;
    PlainsBiome.AudioZones.Add(TEXT("plains_wind"));
    BiomeZones.Add(PlainsBiome);

    // Rocky biome
    FWorld_BiomeData RockyBiome;
    RockyBiome.BiomeType = EWorld_BiomeType::Rocky;
    RockyBiome.Center = FVector(0.0f, -2500.0f, 200.0f);
    RockyBiome.Radius = 1000.0f;
    RockyBiome.BiomeColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    RockyBiome.VegetationDensity = 0.1f;
    RockyBiome.TemperatureModifier = 0.0f;
    RockyBiome.HumidityModifier = -15.0f;
    RockyBiome.AudioZones.Add(TEXT("rocky_echoes"));
    BiomeZones.Add(RockyBiome);

    // Wetlands biome
    FWorld_BiomeData WetlandsBiome;
    WetlandsBiome.BiomeType = EWorld_BiomeType::Wetlands;
    WetlandsBiome.Center = FVector(-1500.0f, -1500.0f, 0.0f);
    WetlandsBiome.Radius = 800.0f;
    WetlandsBiome.BiomeColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
    WetlandsBiome.VegetationDensity = 0.4f;
    WetlandsBiome.TemperatureModifier = -2.0f;
    WetlandsBiome.HumidityModifier = 30.0f;
    WetlandsBiome.AudioZones.Add(TEXT("water_sounds"));
    BiomeZones.Add(WetlandsBiome);

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Created %d default biomes"), BiomeZones.Num());
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;

    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = FVector::Dist(Location, Biome.Center);
        if (Distance <= Biome.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData AWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default if not found
    return FWorld_BiomeData();
}

void AWorld_BiomeManager::SetBiomeAudio(EWorld_BiomeType BiomeType, const FString& AudioZoneName)
{
    for (FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            Biome.AudioZones.AddUnique(AudioZoneName);
            break;
        }
    }
}

void AWorld_BiomeManager::GenerateVegetationForBiome(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    if (BiomeData.VegetationDensity <= 0.0f) return;

    int32 VegetationCount = FMath::RoundToInt(MaxVegetationPerBiome * BiomeData.VegetationDensity);
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Generating %d vegetation items for biome %d"), 
           VegetationCount, (int32)BiomeType);

    // This would spawn vegetation actors - simplified for now
    for (int32 i = 0; i < VegetationCount; ++i)
    {
        FVector VegLocation = GetRandomLocationInBiome(BiomeType);
        // TODO: Spawn actual vegetation actors here
    }
}

void AWorld_BiomeManager::CreateBiomeTransitionZone(const FVector& Location1, const FVector& Location2)
{
    FVector MidPoint = (Location1 + Location2) * 0.5f;
    float Distance = FVector::Dist(Location1, Location2);
    
    // Create transition zone logic
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Creating transition zone at %s"), 
           *MidPoint.ToString());
}

void AWorld_BiomeManager::OptimizeBiomePerformance(const FVector& PlayerLocation)
{
    // Performance optimization based on distance to player
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float DistanceToPlayer = FVector::Dist(PlayerLocation, Biome.Center);
        
        // Adjust LOD based on distance
        if (DistanceToPlayer > LODDistance)
        {
            // Reduce detail for distant biomes
        }
    }
}

void AWorld_BiomeManager::UpdateWeatherForBiome(EWorld_BiomeType BiomeType, float Temperature, float Humidity)
{
    FWorld_BiomeData* BiomeData = nullptr;
    for (FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            BiomeData = &Biome;
            break;
        }
    }

    if (BiomeData)
    {
        BiomeData->TemperatureModifier = Temperature;
        BiomeData->HumidityModifier = Humidity;
        
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Updated weather for biome %d - Temp: %f, Humidity: %f"), 
               (int32)BiomeType, Temperature, Humidity);
    }
}

float AWorld_BiomeManager::GetDistanceToBiome(const FVector& Location, EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    return FVector::Dist(Location, BiomeData.Center);
}

bool AWorld_BiomeManager::IsLocationInBiome(const FVector& Location, EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    float Distance = FVector::Dist(Location, BiomeData.Center);
    return Distance <= BiomeData.Radius;
}

void AWorld_BiomeManager::SpawnBiomeMarkers()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Spawning biome markers"));
    
    // This would spawn visual markers for each biome
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        // Biome markers would be spawned here
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Marker for biome %d at %s"), 
               (int32)Biome.BiomeType, *Biome.Center.ToString());
    }
}

void AWorld_BiomeManager::SetupAudioZones()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Setting up audio zones"));
    
    // Setup audio components and triggers for each biome
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        for (const FString& AudioZone : Biome.AudioZones)
        {
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Audio zone '%s' for biome %d"), 
                   *AudioZone, (int32)Biome.BiomeType);
        }
    }
}

FVector AWorld_BiomeManager::GetRandomLocationInBiome(EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Generate random point within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeData.Radius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = FMath::RandRange(-50.0f, 50.0f);
    
    return BiomeData.Center + RandomOffset;
}

void AWorld_BiomeManager::ApplyBiomeSpecificSettings(EWorld_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Applying forest settings"));
            break;
        case EWorld_BiomeType::Plains:
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Applying plains settings"));
            break;
        case EWorld_BiomeType::Rocky:
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Applying rocky settings"));
            break;
        case EWorld_BiomeType::Wetlands:
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Applying wetlands settings"));
            break;
        default:
            break;
    }
}