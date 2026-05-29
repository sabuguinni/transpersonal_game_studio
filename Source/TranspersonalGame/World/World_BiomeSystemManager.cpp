#include "World_BiomeSystemManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UWorld_BiomeSystemManager::UWorld_BiomeSystemManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetupDefaultBiomes();
}

void UWorld_BiomeSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing biome system"));
    InitializeBiomes();
    CreateWaterSystems();
    PopulateVegetation();
    SpawnBiomeSpecificDinosaurs();
}

void UWorld_BiomeSystemManager::SetupDefaultBiomes()
{
    BiomeRegions.Empty();
    
    // Savanna biome (center)
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0, 0, 0);
    SavannaBiome.Radius = 30000.0f;
    SavannaBiome.VegetationDensity = 0.3f;
    SavannaBiome.WaterCoverage = 0.05f;
    SavannaBiome.DinosaurSpecies = {"TRex", "Triceratops", "Parasaurolophus"};
    BiomeRegions.Add(SavannaBiome);
    
    // Forest biome (northwest)
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-45000, 40000, 0);
    ForestBiome.Radius = 25000.0f;
    ForestBiome.VegetationDensity = 0.8f;
    ForestBiome.WaterCoverage = 0.15f;
    ForestBiome.DinosaurSpecies = {"Velociraptor", "Ankylosaurus", "Protoceratops"};
    BiomeRegions.Add(ForestBiome);
    
    // Desert biome (southeast)
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(50000, -40000, 0);
    DesertBiome.Radius = 35000.0f;
    DesertBiome.VegetationDensity = 0.1f;
    DesertBiome.WaterCoverage = 0.02f;
    DesertBiome.DinosaurSpecies = {"Pachycephalo", "Tsintaosaurus"};
    BiomeRegions.Add(DesertBiome);
    
    // Wetlands biome (northeast)
    FWorld_BiomeData WetlandsBiome;
    WetlandsBiome.BiomeType = EWorld_BiomeType::Wetlands;
    WetlandsBiome.CenterLocation = FVector(30000, 50000, 0);
    WetlandsBiome.Radius = 20000.0f;
    WetlandsBiome.VegetationDensity = 0.6f;
    WetlandsBiome.WaterCoverage = 0.4f;
    WetlandsBiome.DinosaurSpecies = {"Brachiosaurus", "Parasaurolophus"};
    BiomeRegions.Add(WetlandsBiome);
}

void UWorld_BiomeSystemManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initialized %d biome regions"), BiomeRegions.Num());
    
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome: %s at location (%f, %f, %f) with radius %f"), 
            *UEnum::GetValueAsString(Biome.BiomeType),
            Biome.CenterLocation.X, Biome.CenterLocation.Y, Biome.CenterLocation.Z,
            Biome.Radius);
    }
}

EWorld_BiomeType UWorld_BiomeSystemManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Savanna;
    
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        float Distance = FVector::Dist(Location, Biome.CenterLocation);
        if (Distance < Biome.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FWorld_BiomeData UWorld_BiomeSystemManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    return FWorld_BiomeData(); // Return default if not found
}

void UWorld_BiomeSystemManager::CreateWaterSystems()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create main river connecting biomes
    CreateRiverSystem(FVector(-20000, 30000, 50), FVector(20000, -20000, 50));
    
    // Create wetlands water bodies
    for (int32 i = 0; i < 5; i++)
    {
        FVector WaterLocation = FVector(30000, 50000, 50) + FVector(
            FMath::RandRange(-15000, 15000),
            FMath::RandRange(-15000, 15000),
            0
        );
        
        CreateRiverSystem(WaterLocation, WaterLocation + FVector(5000, 0, 0));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Created %d water bodies"), WaterBodies.Num());
}

void UWorld_BiomeSystemManager::CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create water plane
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>();
    if (WaterActor)
    {
        FVector MidPoint = (StartLocation + EndLocation) * 0.5f;
        float Distance = FVector::Dist(StartLocation, EndLocation);
        
        WaterActor->SetActorLocation(MidPoint);
        WaterActor->SetActorScale3D(FVector(Distance / 100.0f, 10.0f, 1.0f));
        WaterActor->SetActorLabel(FString::Printf(TEXT("WaterBody_%d"), WaterBodies.Num()));
        
        WaterBodies.Add(WaterActor);
        
        UE_LOG(LogTemp, Log, TEXT("Created water body: %s"), *WaterActor->GetActorLabel());
    }
}

void UWorld_BiomeSystemManager::PopulateVegetation()
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        SpawnVegetationInBiome(Biome);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Populated %d vegetation actors"), VegetationActors.Num());
}

void UWorld_BiomeSystemManager::SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 VegetationCount = FMath::RoundToInt(BiomeData.VegetationDensity * 20.0f);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = BiomeData.CenterLocation + FVector(
            FMath::RandRange(-BiomeData.Radius * 0.8f, BiomeData.Radius * 0.8f),
            FMath::RandRange(-BiomeData.Radius * 0.8f, BiomeData.Radius * 0.8f),
            100.0f
        );
        
        AStaticMeshActor* VegActor = World->SpawnActor<AStaticMeshActor>();
        if (VegActor)
        {
            // Scale based on biome type
            FVector Scale = FVector(2.0f, 2.0f, 6.0f);
            if (BiomeData.BiomeType == EWorld_BiomeType::Forest)
            {
                Scale = FVector(3.0f, 3.0f, 12.0f);
            }
            else if (BiomeData.BiomeType == EWorld_BiomeType::Desert)
            {
                Scale = FVector(1.0f, 1.0f, 3.0f);
            }
            
            VegActor->SetActorLocation(SpawnLocation);
            VegActor->SetActorScale3D(Scale);
            VegActor->SetActorLabel(FString::Printf(TEXT("%s_Vegetation_%d"), 
                *UEnum::GetValueAsString(BiomeData.BiomeType), i));
            
            VegetationActors.Add(VegActor);
        }
    }
}

void UWorld_BiomeSystemManager::SpawnBiomeSpecificDinosaurs()
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        SpawnDinosaursInBiome(Biome);
    }
}

void UWorld_BiomeSystemManager::SpawnDinosaursInBiome(const FWorld_BiomeData& BiomeData)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (int32 i = 0; i < BiomeData.DinosaurSpecies.Num() && i < 3; i++)
    {
        FVector SpawnLocation = BiomeData.CenterLocation + FVector(
            FMath::RandRange(-BiomeData.Radius * 0.5f, BiomeData.Radius * 0.5f),
            FMath::RandRange(-BiomeData.Radius * 0.5f, BiomeData.Radius * 0.5f),
            200.0f
        );
        
        UE_LOG(LogTemp, Warning, TEXT("Spawning %s in %s biome at (%f, %f, %f)"), 
            *BiomeData.DinosaurSpecies[i],
            *UEnum::GetValueAsString(BiomeData.BiomeType),
            SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
    }
}

float UWorld_BiomeSystemManager::CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& BiomeData) const
{
    float Distance = FVector::Dist(Location, BiomeData.CenterLocation);
    if (Distance >= BiomeData.Radius) return 0.0f;
    
    return 1.0f - (Distance / BiomeData.Radius);
}

void UWorld_BiomeSystemManager::DebugDrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        FColor BiomeColor = FColor::Green;
        switch (Biome.BiomeType)
        {
            case EWorld_BiomeType::Savanna: BiomeColor = FColor::Yellow; break;
            case EWorld_BiomeType::Forest: BiomeColor = FColor::Green; break;
            case EWorld_BiomeType::Desert: BiomeColor = FColor::Orange; break;
            case EWorld_BiomeType::Wetlands: BiomeColor = FColor::Blue; break;
            case EWorld_BiomeType::Mountains: BiomeColor = FColor::Red; break;
        }
        
        DrawDebugCircle(World, Biome.CenterLocation, Biome.Radius, 32, BiomeColor, true, 60.0f, 0, 100.0f);
    }
}