#include "World_DynamicBiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SceneComponent.h"

UWorld_DynamicBiomeGenerator::UWorld_DynamicBiomeGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    InitializeDefaultBiomeConfigurations();
}

void UWorld_DynamicBiomeGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome configurations with proper coordinates from memory
    InitializeDefaultBiomeConfigurations();
    
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: BeginPlay - Initialized with %d biomes"), BiomeConfigurations.Num());
}

void UWorld_DynamicBiomeGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    // Update biomes based on frequency
    if (LastUpdateTime >= UpdateFrequency)
    {
        OptimizeBiomePerformance();
        LastUpdateTime = 0.0f;
    }
}

void UWorld_DynamicBiomeGenerator::InitializeDefaultBiomeConfigurations()
{
    // Initialize biome configurations based on memory coordinates
    FWorld_BiomeConfiguration SavannaConfig;
    SavannaConfig.BiomeName = "Savanna";
    SavannaConfig.BiomeCenter = FVector2D(0, 0);
    SavannaConfig.BiomeRadius = 15000.0f;
    SavannaConfig.VegetationDensity = 0.3f;
    SavannaConfig.RockDensity = 0.2f;
    SavannaConfig.WaterPresence = 0.1f;
    SavannaConfig.TerrainRoughness = 0.2f;
    SavannaConfig.BiomeColor = FLinearColor(0.8f, 0.7f, 0.3f, 1.0f);
    SavannaConfig.VegetationTypes = {"AcaciaTree", "SavannaGrass", "SmallBush"};
    SavannaConfig.DinosaurSpecies = {"TRex", "Triceratops", "Raptor"};
    BiomeConfigurations.Add(EBiomeType::Savanna, SavannaConfig);

    FWorld_BiomeConfiguration SwampConfig;
    SwampConfig.BiomeName = "Swamp";
    SwampConfig.BiomeCenter = FVector2D(-50000, -45000);
    SwampConfig.BiomeRadius = 15000.0f;
    SwampConfig.VegetationDensity = 0.8f;
    SwampConfig.RockDensity = 0.1f;
    SwampConfig.WaterPresence = 0.7f;
    SwampConfig.TerrainRoughness = 0.3f;
    SwampConfig.BiomeColor = FLinearColor(0.2f, 0.4f, 0.2f, 1.0f);
    SwampConfig.VegetationTypes = {"CypressTree", "SwampGrass", "Moss", "Reeds"};
    SwampConfig.DinosaurSpecies = {"Spinosaurus", "Raptor", "Parasaurolophus"};
    BiomeConfigurations.Add(EBiomeType::Swamp, SwampConfig);

    FWorld_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeName = "Forest";
    ForestConfig.BiomeCenter = FVector2D(-45000, 40000);
    ForestConfig.BiomeRadius = 15000.0f;
    ForestConfig.VegetationDensity = 0.9f;
    ForestConfig.RockDensity = 0.3f;
    ForestConfig.WaterPresence = 0.3f;
    ForestConfig.TerrainRoughness = 0.5f;
    ForestConfig.BiomeColor = FLinearColor(0.1f, 0.6f, 0.1f, 1.0f);
    ForestConfig.VegetationTypes = {"OakTree", "PineTree", "Fern", "Undergrowth"};
    ForestConfig.DinosaurSpecies = {"Brachiosaurus", "Stegosaurus", "Raptor"};
    BiomeConfigurations.Add(EBiomeType::Forest, ForestConfig);

    FWorld_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeName = "Desert";
    DesertConfig.BiomeCenter = FVector2D(55000, 0);
    DesertConfig.BiomeRadius = 15000.0f;
    DesertConfig.VegetationDensity = 0.1f;
    DesertConfig.RockDensity = 0.6f;
    DesertConfig.WaterPresence = 0.05f;
    DesertConfig.TerrainRoughness = 0.4f;
    DesertConfig.BiomeColor = FLinearColor(0.9f, 0.7f, 0.4f, 1.0f);
    DesertConfig.VegetationTypes = {"Cactus", "DesertShrub", "PalmTree"};
    DesertConfig.DinosaurSpecies = {"Compsognathus", "Dilophosaurus", "Raptor"};
    BiomeConfigurations.Add(EBiomeType::Desert, DesertConfig);

    FWorld_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeName = "Mountain";
    MountainConfig.BiomeCenter = FVector2D(40000, 50000);
    MountainConfig.BiomeRadius = 15000.0f;
    MountainConfig.VegetationDensity = 0.4f;
    MountainConfig.RockDensity = 0.8f;
    MountainConfig.WaterPresence = 0.2f;
    MountainConfig.TerrainRoughness = 0.8f;
    MountainConfig.BiomeColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    MountainConfig.VegetationTypes = {"PineTree", "MountainGrass", "AlpineShrub"};
    MountainConfig.DinosaurSpecies = {"Pteranodon", "Ankylosaurus", "Raptor"};
    BiomeConfigurations.Add(EBiomeType::Mountain, MountainConfig);
}

void UWorld_DynamicBiomeGenerator::GenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Starting generation of all biomes"));
    
    for (auto& BiomePair : BiomeConfigurations)
    {
        GenerateBiome(BiomePair.Value);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Completed generation of all biomes"));
}

void UWorld_DynamicBiomeGenerator::GenerateBiome(const FWorld_BiomeConfiguration& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Generating biome %s at (%f, %f)"), 
           *BiomeConfig.BiomeName, BiomeConfig.BiomeCenter.X, BiomeConfig.BiomeCenter.Y);
    
    // Calculate vegetation count based on density
    int32 VegetationCount = FMath::RoundToInt(BiomeConfig.VegetationDensity * 200.0f);
    int32 DinosaurCount = FMath::RoundToInt(20.0f * (BiomeConfig.VegetationDensity + 0.1f));
    
    PopulateBiomeWithVegetation(BiomeConfig, VegetationCount);
    PopulateBiomeWithDinosaurs(BiomeConfig, DinosaurCount);
    CreateTerrainFeatures(BiomeConfig);
    CreateWaterBodies(BiomeConfig);
}

void UWorld_DynamicBiomeGenerator::PopulateBiomeWithVegetation(const FWorld_BiomeConfiguration& BiomeConfig, int32 VegetationCount)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_DynamicBiomeGenerator: No valid world found"));
        return;
    }
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector Location = GetRandomLocationInBiome(BiomeConfig);
        
        if (BiomeConfig.VegetationTypes.Num() > 0)
        {
            int32 TypeIndex = FMath::RandRange(0, BiomeConfig.VegetationTypes.Num() - 1);
            SpawnVegetationActor(Location, BiomeConfig.VegetationTypes[TypeIndex]);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Spawned %d vegetation actors in %s"), 
           VegetationCount, *BiomeConfig.BiomeName);
}

void UWorld_DynamicBiomeGenerator::PopulateBiomeWithDinosaurs(const FWorld_BiomeConfiguration& BiomeConfig, int32 DinosaurCount)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (int32 i = 0; i < DinosaurCount; i++)
    {
        FVector Location = GetRandomLocationInBiome(BiomeConfig);
        Location.Z = 100.0f; // Ensure dinosaurs spawn above ground
        
        if (BiomeConfig.DinosaurSpecies.Num() > 0)
        {
            int32 SpeciesIndex = FMath::RandRange(0, BiomeConfig.DinosaurSpecies.Num() - 1);
            SpawnDinosaurActor(Location, BiomeConfig.DinosaurSpecies[SpeciesIndex]);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Spawned %d dinosaur actors in %s"), 
           DinosaurCount, *BiomeConfig.BiomeName);
}

void UWorld_DynamicBiomeGenerator::CreateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig)
{
    int32 RockCount = FMath::RoundToInt(BiomeConfig.RockDensity * 50.0f);
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector Location = GetRandomLocationInBiome(BiomeConfig);
        float Scale = FMath::RandRange(0.5f, 2.0f);
        CreateRockFormation(Location, Scale);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Created %d rock formations in %s"), 
           RockCount, *BiomeConfig.BiomeName);
}

void UWorld_DynamicBiomeGenerator::CreateWaterBodies(const FWorld_BiomeConfiguration& BiomeConfig)
{
    if (BiomeConfig.WaterPresence > 0.1f)
    {
        int32 WaterBodyCount = FMath::RoundToInt(BiomeConfig.WaterPresence * 10.0f);
        
        for (int32 i = 0; i < WaterBodyCount; i++)
        {
            FVector Location = GetRandomLocationInBiome(BiomeConfig);
            FVector Scale = FVector(FMath::RandRange(10.0f, 50.0f), FMath::RandRange(10.0f, 50.0f), 1.0f);
            CreateWaterPlane(Location, Scale);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Created %d water bodies in %s"), 
               WaterBodyCount, *BiomeConfig.BiomeName);
    }
}

FVector UWorld_DynamicBiomeGenerator::GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig) const
{
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.BiomeRadius);
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    
    float X = BiomeConfig.BiomeCenter.X + RandomRadius * FMath::Cos(RandomAngle);
    float Y = BiomeConfig.BiomeCenter.Y + RandomRadius * FMath::Sin(RandomAngle);
    float Z = FMath::RandRange(0.0f, 500.0f * BiomeConfig.TerrainRoughness);
    
    return FVector(X, Y, Z);
}

void UWorld_DynamicBiomeGenerator::SpawnVegetationActor(const FVector& Location, const FString& VegetationType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create a basic static mesh actor for vegetation
    AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (VegetationActor)
    {
        VegetationActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *VegetationType, FMath::RandRange(1000, 9999)));
        VegetationActor->Tags.Add(FName("Vegetation"));
        VegetationActor->Tags.Add(FName(*VegetationType));
    }
}

void UWorld_DynamicBiomeGenerator::SpawnDinosaurActor(const FVector& Location, const FString& DinosaurSpecies)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create a basic static mesh actor for dinosaur placeholder
    AStaticMeshActor* DinosaurActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (DinosaurActor)
    {
        DinosaurActor->SetActorLabel(FString::Printf(TEXT("%s_%d"), *DinosaurSpecies, FMath::RandRange(1000, 9999)));
        DinosaurActor->Tags.Add(FName("Dinosaur"));
        DinosaurActor->Tags.Add(FName(*DinosaurSpecies));
    }
}

void UWorld_DynamicBiomeGenerator::CreateRockFormation(const FVector& Location, float Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (RockActor)
    {
        RockActor->SetActorScale3D(FVector(Scale));
        RockActor->SetActorLabel(FString::Printf(TEXT("Rock_%d"), FMath::RandRange(1000, 9999)));
        RockActor->Tags.Add(FName("Rock"));
        RockActor->Tags.Add(FName("TerrainFeature"));
    }
}

void UWorld_DynamicBiomeGenerator::CreateWaterPlane(const FVector& Location, const FVector& Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (WaterActor)
    {
        WaterActor->SetActorScale3D(Scale);
        WaterActor->SetActorLabel(FString::Printf(TEXT("Water_%d"), FMath::RandRange(1000, 9999)));
        WaterActor->Tags.Add(FName("Water"));
        WaterActor->Tags.Add(FName("WaterBody"));
    }
}

void UWorld_DynamicBiomeGenerator::RefreshBiome(EBiomeType BiomeType)
{
    if (BiomeConfigurations.Contains(BiomeType))
    {
        ClearBiome(BiomeType);
        GenerateBiome(BiomeConfigurations[BiomeType]);
    }
}

void UWorld_DynamicBiomeGenerator::ClearBiome(EBiomeType BiomeType)
{
    if (BiomeActors.Contains(BiomeType))
    {
        for (AActor* Actor : BiomeActors[BiomeType])
        {
            if (IsValid(Actor))
            {
                Actor->Destroy();
            }
        }
        BiomeActors[BiomeType].Empty();
    }
}

EBiomeType UWorld_DynamicBiomeGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FWorld_BiomeConfiguration& Config = BiomePair.Value;
        float Distance = FVector2D::Distance(Location2D, Config.BiomeCenter);
        
        if (Distance <= Config.BiomeRadius)
        {
            return BiomePair.Key;
        }
    }
    
    return EBiomeType::Savanna; // Default fallback
}

float UWorld_DynamicBiomeGenerator::GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EBiomeType BiomeType) const
{
    if (!BiomeConfigurations.Contains(BiomeType))
    {
        return 0.0f;
    }
    
    const FWorld_BiomeConfiguration& Config = BiomeConfigurations[BiomeType];
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    float Distance = FVector2D::Distance(Location2D, Config.BiomeCenter);
    
    if (Distance >= Config.BiomeRadius)
    {
        return 0.0f;
    }
    
    return 1.0f - (Distance / Config.BiomeRadius);
}

void UWorld_DynamicBiomeGenerator::OptimizeBiomePerformance()
{
    // Simple performance optimization - could be expanded
    for (auto& BiomeActorPair : BiomeActors)
    {
        TArray<AActor*>& Actors = BiomeActorPair.Value;
        
        // Remove destroyed actors
        Actors.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
        
        // Limit actor count per biome
        if (Actors.Num() > MaxActorsPerBiome)
        {
            int32 ExcessActors = Actors.Num() - MaxActorsPerBiome;
            for (int32 i = 0; i < ExcessActors; i++)
            {
                if (IsValid(Actors.Last()))
                {
                    Actors.Last()->Destroy();
                }
                Actors.RemoveAt(Actors.Num() - 1);
            }
        }
    }
}

FWorld_BiomeConfiguration UWorld_DynamicBiomeGenerator::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (BiomeConfigurations.Contains(BiomeType))
    {
        return BiomeConfigurations[BiomeType];
    }
    
    return FWorld_BiomeConfiguration();
}

void UWorld_DynamicBiomeGenerator::SetBiomeConfiguration(EBiomeType BiomeType, const FWorld_BiomeConfiguration& NewConfig)
{
    BiomeConfigurations.Add(BiomeType, NewConfig);
}

TArray<AActor*> UWorld_DynamicBiomeGenerator::GetActorsInBiome(EBiomeType BiomeType) const
{
    if (BiomeActors.Contains(BiomeType))
    {
        return BiomeActors[BiomeType];
    }
    
    return TArray<AActor*>();
}

bool UWorld_DynamicBiomeGenerator::IsLocationInBiome(const FVector& Location, const FWorld_BiomeConfiguration& BiomeConfig) const
{
    FVector2D Location2D(Location.X, Location.Y);
    float Distance = FVector2D::Distance(Location2D, BiomeConfig.BiomeCenter);
    return Distance <= BiomeConfig.BiomeRadius;
}

void UWorld_DynamicBiomeGenerator::SetLODDistances(float NearDistance, float FarDistance)
{
    MaxRenderDistance = FarDistance;
    UE_LOG(LogTemp, Warning, TEXT("World_DynamicBiomeGenerator: Updated LOD distances - Near: %f, Far: %f"), NearDistance, FarDistance);
}