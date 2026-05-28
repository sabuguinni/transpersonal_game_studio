#include "WorldGenerationManager.h"
#include "TerrainGenerator.h"
#include "BiomeManager.h"
#include "RiverGenerator.h"
#include "SettlementPlacer.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

AWorldGenerationManager::AWorldGenerationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create components
    TerrainGenerator = CreateDefaultSubobject<UWorld_TerrainGenerator>(TEXT("TerrainGenerator"));
    BiomeManager = CreateDefaultSubobject<UWorld_BiomeManager>(TEXT("BiomeManager"));
    RiverGenerator = CreateDefaultSubobject<UWorld_RiverGenerator>(TEXT("RiverGenerator"));
    SettlementPlacer = CreateDefaultSubobject<UWorld_SettlementPlacer>(TEXT("SettlementPlacer"));
    
    // Default configuration
    WorldSeed = 12345;
    WorldSize = FVector2D(50000.0f, 50000.0f);
    bAutoGenerate = false;
    bGenerateTerrain = true;
    bGenerateBiomes = true;
    bGenerateRivers = true;
    bGenerateSettlements = true;
    bWorldGenerated = false;
}

void AWorldGenerationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    if (bAutoGenerate)
    {
        GenerateWorld();
    }
}

void AWorldGenerationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update generation components if needed
    if (TerrainGenerator)
    {
        TerrainGenerator->UpdateGeneration(DeltaTime);
    }
}

void AWorldGenerationManager::GenerateWorld()
{
    if (!ValidateConfiguration())
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: Invalid configuration, cannot generate world"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Starting world generation..."));
    
    SetupRandomSeed();
    ClearWorld();
    
    // Generate in sequence
    if (bGenerateTerrain)
    {
        GenerateTerrain();
    }
    
    if (bGenerateBiomes)
    {
        GenerateBiomes();
    }
    
    if (bGenerateRivers)
    {
        GenerateRivers();
    }
    
    if (bGenerateSettlements)
    {
        GenerateSettlements();
    }
    
    bWorldGenerated = true;
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: World generation completed"));
}

void AWorldGenerationManager::GenerateTerrain()
{
    if (!TerrainGenerator)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: TerrainGenerator is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Generating terrain..."));
    TerrainGenerator->GenerateTerrain(TerrainParams, WorldSize);
}

void AWorldGenerationManager::GenerateBiomes()
{
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: BiomeManager is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Generating biomes..."));
    BiomeManager->GenerateBiomes(BiomeConfigTable, WorldSize);
}

void AWorldGenerationManager::GenerateRivers()
{
    if (!RiverGenerator)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: RiverGenerator is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Generating rivers..."));
    RiverGenerator->GenerateRivers(RiverParams, WorldSize);
}

void AWorldGenerationManager::GenerateSettlements()
{
    if (!SettlementPlacer)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: SettlementPlacer is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Generating settlements..."));
    GeneratedSettlements = SettlementPlacer->GenerateSettlements(WorldSize, 5);
}

void AWorldGenerationManager::ClearWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Clearing existing world data..."));
    
    if (TerrainGenerator)
    {
        TerrainGenerator->ClearTerrain();
    }
    
    if (BiomeManager)
    {
        BiomeManager->ClearBiomes();
    }
    
    if (RiverGenerator)
    {
        RiverGenerator->ClearRivers();
    }
    
    if (SettlementPlacer)
    {
        SettlementPlacer->ClearSettlements();
    }
    
    GeneratedSettlements.Empty();
    bWorldGenerated = false;
}

EWorld_BiomeType AWorldGenerationManager::GetBiomeAtLocation(const FVector& Location) const
{
    if (!BiomeManager)
    {
        return EWorld_BiomeType::Forest;
    }
    
    return BiomeManager->GetBiomeAtLocation(Location);
}

float AWorldGenerationManager::GetElevationAtLocation(const FVector& Location) const
{
    if (!TerrainGenerator)
    {
        return 0.0f;
    }
    
    return TerrainGenerator->GetElevationAtLocation(Location);
}

bool AWorldGenerationManager::IsLocationNearWater(const FVector& Location, float SearchRadius) const
{
    if (!RiverGenerator)
    {
        return false;
    }
    
    return RiverGenerator->IsLocationNearWater(Location, SearchRadius);
}

TArray<FWorld_SettlementData> AWorldGenerationManager::GetNearbySettlements(const FVector& Location, float SearchRadius) const
{
    TArray<FWorld_SettlementData> NearbySettlements;
    
    for (const FWorld_SettlementData& Settlement : GeneratedSettlements)
    {
        float Distance = FVector::Dist(Location, Settlement.Location);
        if (Distance <= SearchRadius)
        {
            NearbySettlements.Add(Settlement);
        }
    }
    
    return NearbySettlements;
}

void AWorldGenerationManager::InitializeComponents()
{
    if (TerrainGenerator)
    {
        TerrainGenerator->Initialize();
    }
    
    if (BiomeManager)
    {
        BiomeManager->Initialize();
    }
    
    if (RiverGenerator)
    {
        RiverGenerator->Initialize();
    }
    
    if (SettlementPlacer)
    {
        SettlementPlacer->Initialize();
    }
}

void AWorldGenerationManager::SetupRandomSeed()
{
    FMath::RandInit(WorldSeed);
    UKismetMathLibrary::SetRandomStreamSeed(FRandomStream(), WorldSeed);
    UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Random seed set to %d"), WorldSeed);
}

bool AWorldGenerationManager::ValidateConfiguration() const
{
    if (WorldSize.X <= 0 || WorldSize.Y <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: Invalid world size"));
        return false;
    }
    
    if (!TerrainGenerator || !BiomeManager || !RiverGenerator || !SettlementPlacer)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: Missing required components"));
        return false;
    }
    
    return true;
}