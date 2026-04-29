#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "EditorLevelLibrary.h"
#include "Kismet/GameplayStatics.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    WorldSize = 8000.0f;
    MaxVegetationActors = 50;
    MaxRockFormations = 20;
    
    SetupDefaultBiomes();
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: BeginPlay - Initializing biome system"));
    InitializeBiomes();
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWorld_BiomeSystem::SetupDefaultBiomes()
{
    BiomeZones.Empty();
    
    // Forest Biome (North)
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(0, 2000, 100);
    ForestBiome.Radius = 1500.0f;
    ForestBiome.VegetationDensity = 0.8f;
    ForestBiome.WaterLevel = 20.0f;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.7f;
    BiomeZones.Add(ForestBiome);
    
    // Savanna Biome (South)
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0, -2000, 100);
    SavannaBiome.Radius = 1500.0f;
    SavannaBiome.VegetationDensity = 0.3f;
    SavannaBiome.WaterLevel = 0.0f;
    SavannaBiome.Temperature = 28.0f;
    SavannaBiome.Humidity = 0.4f;
    BiomeZones.Add(SavannaBiome);
    
    // Swamp Biome (West)
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-2000, 0, 50);
    SwampBiome.Radius = 1200.0f;
    SwampBiome.VegetationDensity = 0.6f;
    SwampBiome.WaterLevel = 80.0f;
    SwampBiome.Temperature = 26.0f;
    SwampBiome.Humidity = 0.9f;
    BiomeZones.Add(SwampBiome);
    
    // Desert Biome (East)
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(2000, 0, 150);
    DesertBiome.Radius = 1500.0f;
    DesertBiome.VegetationDensity = 0.1f;
    DesertBiome.WaterLevel = 0.0f;
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.2f;
    BiomeZones.Add(DesertBiome);
}

void UWorld_BiomeSystem::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Initializing %d biome zones"), BiomeZones.Num());
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        FString BiomeName;
        switch (Biome.BiomeType)
        {
            case EWorld_BiomeType::Forest:
                BiomeName = TEXT("Forest");
                break;
            case EWorld_BiomeType::Savanna:
                BiomeName = TEXT("Savanna");
                break;
            case EWorld_BiomeType::Swamp:
                BiomeName = TEXT("Swamp");
                break;
            case EWorld_BiomeType::Desert:
                BiomeName = TEXT("Desert");
                break;
            case EWorld_BiomeType::Mountain:
                BiomeName = TEXT("Mountain");
                break;
        }
        
        SpawnBiomeMarker(Biome.CenterLocation, BiomeName);
        SpawnBiomeVegetation(Biome.BiomeType);
        SpawnTerrainFeatures(Biome.BiomeType);
    }
    
    CreateWaterFeatures();
}

EWorld_BiomeType UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = FVector::Dist(Location, Biome.CenterLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    return FWorld_BiomeData();
}

void UWorld_BiomeSystem::SpawnBiomeVegetation(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    int32 VegetationCount = FMath::RoundToInt(BiomeData.VegetationDensity * MaxVegetationActors);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeData.Radius, BiomeData.Radius),
            FMath::RandRange(-BiomeData.Radius, BiomeData.Radius),
            0
        );
        
        FVector SpawnLocation = BiomeData.CenterLocation + RandomOffset;
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0, 360), 0);
        
        FString ActorName = FString::Printf(TEXT("%s_Vegetation_%d"), 
            *UEnum::GetValueAsString(BiomeData.BiomeType), i);
        
        SpawnVegetationActor(SpawnLocation, SpawnRotation, ActorName);
    }
}

void UWorld_BiomeSystem::SpawnTerrainFeatures(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    int32 RockCount = 5;
    if (BiomeType == EWorld_BiomeType::Desert)
    {
        RockCount = 8;
    }
    else if (BiomeType == EWorld_BiomeType::Swamp)
    {
        RockCount = 3;
    }
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeData.Radius * 0.7f, BiomeData.Radius * 0.7f),
            FMath::RandRange(-BiomeData.Radius * 0.7f, BiomeData.Radius * 0.7f),
            0
        );
        
        FVector SpawnLocation = BiomeData.CenterLocation + RandomOffset;
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0, 360), 0);
        
        FString ActorName = FString::Printf(TEXT("%s_Rock_%d"), 
            *UEnum::GetValueAsString(BiomeData.BiomeType), i);
        
        SpawnRockFormation(SpawnLocation, SpawnRotation, ActorName);
    }
}

void UWorld_BiomeSystem::CreateWaterFeatures()
{
    // Central Lake
    FWorld_TerrainFeature CentralLake;
    CentralLake.FeatureName = TEXT("CentralLake");
    CentralLake.Location = FVector(0, 0, 0);
    CentralLake.Scale = FVector(5, 5, 0.5f);
    TerrainFeatures.Add(CentralLake);
    
    // River from Forest to Swamp
    FWorld_TerrainFeature River;
    River.FeatureName = TEXT("ForestSwampRiver");
    River.Location = FVector(-1000, 1000, 10);
    River.Rotation = FRotator(0, 45, 0);
    River.Scale = FVector(10, 2, 0.3f);
    TerrainFeatures.Add(River);
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Created %d water features"), TerrainFeatures.Num());
}

float UWorld_BiomeSystem::GetBiomeInfluence(const FVector& Location, EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    float Distance = FVector::Dist(Location, BiomeData.CenterLocation);
    float Influence = FMath::Clamp(1.0f - (Distance / BiomeData.Radius), 0.0f, 1.0f);
    
    return Influence;
}

void UWorld_BiomeSystem::RegenerateBiome(EWorld_BiomeType BiomeType)
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Regenerating biome %s"), 
        *UEnum::GetValueAsString(BiomeType));
    
    SpawnBiomeVegetation(BiomeType);
    SpawnTerrainFeatures(BiomeType);
}

void UWorld_BiomeSystem::EditorGenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Editor generate all biomes"));
    InitializeBiomes();
}

void UWorld_BiomeSystem::SpawnBiomeMarker(const FVector& Location, const FString& BiomeName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // In runtime, we would spawn actual marker actors here
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeSystem: Biome marker %s at %s"), 
        *BiomeName, *Location.ToString());
}

AStaticMeshActor* UWorld_BiomeSystem::SpawnVegetationActor(const FVector& Location, const FRotator& Rotation, const FString& ActorName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // In runtime, we would spawn actual vegetation actors here
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Vegetation %s at %s"), 
        *ActorName, *Location.ToString());
    
    return nullptr;
}

AStaticMeshActor* UWorld_BiomeSystem::SpawnRockFormation(const FVector& Location, const FRotator& Rotation, const FString& ActorName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // In runtime, we would spawn actual rock formation actors here
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Rock formation %s at %s"), 
        *ActorName, *Location.ToString());
    
    return nullptr;
}