#include "World_BiomeTerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

AWorld_BiomeTerrainGenerator::AWorld_BiomeTerrainGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bAutoGenerateOnBeginPlay = false;
    bUseProceduralPlacement = true;
    RandomSeed = 12345;
    
    SetupBiomeConfigs();
    InitializeDefaultAssets();
}

void AWorld_BiomeTerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        GenerateAllBiomeTerrain();
    }
}

void AWorld_BiomeTerrainGenerator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeTerrainGenerator::SetupBiomeConfigs()
{
    BiomeConfigs.Empty();
    
    // Savana biome
    FWorld_BiomeTerrainConfig SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.BiomeRadius = 15000.0f;
    SavanaConfig.TerrainFeatureCount = 75;
    SavanaConfig.MinTerrainScale = 1.0f;
    SavanaConfig.MaxTerrainScale = 2.5f;
    BiomeConfigs.Add(SavanaConfig);
    
    // Pantano biome
    FWorld_BiomeTerrainConfig PantanoConfig;
    PantanoConfig.BiomeType = EBiomeType::Pantano;
    PantanoConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 50.0f);
    PantanoConfig.BiomeRadius = 12000.0f;
    PantanoConfig.TerrainFeatureCount = 60;
    PantanoConfig.MinTerrainScale = 0.8f;
    PantanoConfig.MaxTerrainScale = 2.0f;
    BiomeConfigs.Add(PantanoConfig);
    
    // Floresta biome
    FWorld_BiomeTerrainConfig FlorestaConfig;
    FlorestaConfig.BiomeType = EBiomeType::Floresta;
    FlorestaConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 150.0f);
    FlorestaConfig.BiomeRadius = 18000.0f;
    FlorestaConfig.TerrainFeatureCount = 100;
    FlorestaConfig.MinTerrainScale = 1.2f;
    FlorestaConfig.MaxTerrainScale = 3.5f;
    BiomeConfigs.Add(FlorestaConfig);
    
    // Deserto biome
    FWorld_BiomeTerrainConfig DesertoConfig;
    DesertoConfig.BiomeType = EBiomeType::Deserto;
    DesertoConfig.CenterLocation = FVector(55000.0f, 0.0f, 80.0f);
    DesertoConfig.BiomeRadius = 14000.0f;
    DesertoConfig.TerrainFeatureCount = 45;
    DesertoConfig.MinTerrainScale = 0.6f;
    DesertoConfig.MaxTerrainScale = 2.8f;
    BiomeConfigs.Add(DesertoConfig);
    
    // Montanha biome
    FWorld_BiomeTerrainConfig MontanhaConfig;
    MontanhaConfig.BiomeType = EBiomeType::Montanha;
    MontanhaConfig.CenterLocation = FVector(40000.0f, 50000.0f, 300.0f);
    MontanhaConfig.BiomeRadius = 16000.0f;
    MontanhaConfig.TerrainFeatureCount = 80;
    MontanhaConfig.MinTerrainScale = 2.0f;
    MontanhaConfig.MaxTerrainScale = 5.0f;
    BiomeConfigs.Add(MontanhaConfig);
}

void AWorld_BiomeTerrainGenerator::InitializeDefaultAssets()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        DefaultTerrainMesh = CubeMeshFinder.Object;
    }
}

void AWorld_BiomeTerrainGenerator::GenerateAllBiomeTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating terrain for all biomes"));
    
    // Set random seed for consistent generation
    FMath::RandInit(RandomSeed);
    
    for (const FWorld_BiomeTerrainConfig& Config : BiomeConfigs)
    {
        GenerateBiomeTerrain(Config.BiomeType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Completed terrain generation for %d biomes"), BiomeConfigs.Num());
}

void AWorld_BiomeTerrainGenerator::GenerateBiomeTerrain(EBiomeType BiomeType)
{
    // Find the config for this biome
    const FWorld_BiomeTerrainConfig* Config = nullptr;
    for (const FWorld_BiomeTerrainConfig& BiomeConfig : BiomeConfigs)
    {
        if (BiomeConfig.BiomeType == BiomeType)
        {
            Config = &BiomeConfig;
            break;
        }
    }
    
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No config found for biome type"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for terrain generation"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generating %d terrain features for biome at %s"), 
           Config->TerrainFeatureCount, *Config->CenterLocation.ToString());
    
    // Generate terrain features
    for (int32 i = 0; i < Config->TerrainFeatureCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(*Config);
        SpawnTerrainFeature(SpawnLocation, BiomeType, i);
    }
}

void AWorld_BiomeTerrainGenerator::SpawnTerrainFeature(const FVector& Location, EBiomeType BiomeType, int32 FeatureIndex)
{
    UWorld* World = GetWorld();
    if (!World || !DefaultTerrainMesh)
    {
        return;
    }
    
    // Spawn static mesh actor
    AStaticMeshActor* TerrainActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, GetRandomRotationForTerrain());
    
    if (TerrainActor)
    {
        // Set the mesh
        UStaticMeshComponent* MeshComp = TerrainActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(DefaultTerrainMesh);
            
            // Set scale based on biome
            float Scale = GetTerrainScaleForBiome(BiomeType);
            FVector ScaleVector = FVector(Scale, Scale, Scale * 0.6f); // Make terrain features flatter
            TerrainActor->SetActorScale3D(ScaleVector);
        }
        
        // Set actor label for identification
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        BiomeName = BiomeName.Replace(TEXT("EBiomeType::"), TEXT(""));
        TerrainActor->SetActorLabel(FString::Printf(TEXT("%s_Terrain_%d"), *BiomeName, FeatureIndex));
        
        // Track spawned actors
        SpawnedTerrainActors.Add(TerrainActor);
    }
}

FVector AWorld_BiomeTerrainGenerator::GetRandomLocationInBiome(const FWorld_BiomeTerrainConfig& Config)
{
    // Generate random point within biome radius
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Config.BiomeRadius);
    
    FVector Offset;
    Offset.X = FMath::Cos(Angle) * Distance;
    Offset.Y = FMath::Sin(Angle) * Distance;
    Offset.Z = FMath::RandRange(-100.0f, 200.0f); // Some height variation
    
    return Config.CenterLocation + Offset;
}

float AWorld_BiomeTerrainGenerator::GetTerrainScaleForBiome(EBiomeType BiomeType)
{
    const FWorld_BiomeTerrainConfig* Config = nullptr;
    for (const FWorld_BiomeTerrainConfig& BiomeConfig : BiomeConfigs)
    {
        if (BiomeConfig.BiomeType == BiomeType)
        {
            Config = &BiomeConfig;
            break;
        }
    }
    
    if (Config)
    {
        return FMath::RandRange(Config->MinTerrainScale, Config->MaxTerrainScale);
    }
    
    return 1.0f;
}

FRotator AWorld_BiomeTerrainGenerator::GetRandomRotationForTerrain()
{
    return FRotator(
        FMath::RandRange(-15.0f, 15.0f), // Small pitch variation
        FMath::RandRange(0.0f, 360.0f),  // Random yaw
        FMath::RandRange(-10.0f, 10.0f)  // Small roll variation
    );
}

void AWorld_BiomeTerrainGenerator::ClearBiomeTerrain(EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FString BiomeName = UEnum::GetValueAsString(BiomeType);
    BiomeName = BiomeName.Replace(TEXT("EBiomeType::"), TEXT(""));
    
    // Remove actors that match this biome
    for (int32 i = SpawnedTerrainActors.Num() - 1; i >= 0; i--)
    {
        if (SpawnedTerrainActors[i] && SpawnedTerrainActors[i]->GetActorLabel().Contains(BiomeName))
        {
            SpawnedTerrainActors[i]->Destroy();
            SpawnedTerrainActors.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared terrain for biome: %s"), *BiomeName);
}

void AWorld_BiomeTerrainGenerator::ClearAllTerrain()
{
    for (AActor* Actor : SpawnedTerrainActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    SpawnedTerrainActors.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all generated terrain"));
}

void AWorld_BiomeTerrainGenerator::EditorGenerateTerrain()
{
    GenerateAllBiomeTerrain();
}

void AWorld_BiomeTerrainGenerator::EditorClearTerrain()
{
    ClearAllTerrain();
}