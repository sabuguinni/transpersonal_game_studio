#include "World_TerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

UWorld_TerrainGenerator::UWorld_TerrainGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;

    // Default terrain settings
    TerrainScale = 100000.0f;  // 100km x 100km world
    TerrainResolution = 512;
    MaxTerrainHeight = 2000.0f;
    MaxLODLevel = 3;
    LODDistance = 10000.0f;
    bEnablePerformanceOptimization = true;
    LODUpdateTimer = 0.0f;

    // Initialize default biome regions
    TerrainFeatures.Empty();
    
    // Savanna region (center)
    FWorld_TerrainFeature SavannaFeature;
    SavannaFeature.Location = FVector(0.0f, 0.0f, 0.0f);
    SavannaFeature.Scale = FVector(30000.0f, 30000.0f, 500.0f);
    SavannaFeature.BiomeType = EBiomeType::Savanna;
    SavannaFeature.Density = 0.8f;
    TerrainFeatures.Add(SavannaFeature);

    // Forest region (northwest)
    FWorld_TerrainFeature ForestFeature;
    ForestFeature.Location = FVector(-45000.0f, 40000.0f, 200.0f);
    ForestFeature.Scale = FVector(25000.0f, 25000.0f, 800.0f);
    ForestFeature.BiomeType = EBiomeType::Forest;
    ForestFeature.Density = 1.0f;
    TerrainFeatures.Add(ForestFeature);

    // Desert region (southeast)
    FWorld_TerrainFeature DesertFeature;
    DesertFeature.Location = FVector(50000.0f, -40000.0f, 100.0f);
    DesertFeature.Scale = FVector(20000.0f, 20000.0f, 300.0f);
    DesertFeature.BiomeType = EBiomeType::Desert;
    DesertFeature.Density = 0.6f;
    TerrainFeatures.Add(DesertFeature);

    // Initialize water bodies
    WaterBodies.Empty();

    // Main river through savanna
    FWorld_WaterBody MainRiver;
    MainRiver.Location = FVector(0.0f, 0.0f, 50.0f);
    MainRiver.Size = FVector2D(40000.0f, 500.0f);
    MainRiver.Depth = 150.0f;
    MainRiver.bIsRiver = true;
    WaterBodies.Add(MainRiver);

    // Forest lake
    FWorld_WaterBody ForestLake;
    ForestLake.Location = FVector(-45000.0f, 40000.0f, 150.0f);
    ForestLake.Size = FVector2D(3000.0f, 3000.0f);
    ForestLake.Depth = 200.0f;
    ForestLake.bIsRiver = false;
    WaterBodies.Add(ForestLake);
}

void UWorld_TerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: BeginPlay - Starting terrain generation"));
    
    // Generate terrain automatically on start
    GenerateBaseTerrain();
    CreateBiomeRegions();
    GenerateWaterBodies();
    AddTerrainFeatures();
}

void UWorld_TerrainGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnablePerformanceOptimization)
    {
        UpdateTerrainLOD(DeltaTime);
    }
}

void UWorld_TerrainGenerator::GenerateBaseTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Generating base terrain"));

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_TerrainGenerator: No valid world found"));
        return;
    }

    // Create terrain chunks for each biome
    for (const FWorld_TerrainFeature& Feature : TerrainFeatures)
    {
        CreateTerrainMesh(Feature.Location, Feature.Scale, Feature.BiomeType);
    }

    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Base terrain generation complete"));
}

void UWorld_TerrainGenerator::CreateBiomeRegions()
{
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Creating biome regions"));

    // Biome regions are created as part of terrain generation
    // This method can be extended for additional biome-specific features
    
    for (const FWorld_TerrainFeature& Feature : TerrainFeatures)
    {
        FString BiomeName = UEnum::GetValueAsString(Feature.BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Created %s biome at %s"), 
               *BiomeName, *Feature.Location.ToString());
    }
}

void UWorld_TerrainGenerator::GenerateWaterBodies()
{
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Generating water bodies"));

    for (const FWorld_WaterBody& WaterConfig : WaterBodies)
    {
        CreateWaterPlane(WaterConfig);
    }

    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Water body generation complete"));
}

void UWorld_TerrainGenerator::AddTerrainFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Adding terrain features"));

    // Terrain features are integrated into the main terrain generation
    // This method can be extended for additional decorative elements
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Terrain features added"));
}

void UWorld_TerrainGenerator::OptimizeTerrainLOD()
{
    if (!bEnablePerformanceOptimization)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("World_TerrainGenerator: Optimizing terrain LOD"));

    for (AStaticMeshActor* TerrainActor : GeneratedTerrainActors)
    {
        if (!TerrainActor || !IsValid(TerrainActor))
        {
            continue;
        }

        float DistanceToPlayer = CalculateDistanceToPlayer(TerrainActor->GetActorLocation());
        
        UStaticMeshComponent* MeshComp = TerrainActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Set LOD based on distance
            int32 LODLevel = FMath::Clamp(
                FMath::FloorToInt(DistanceToPlayer / LODDistance), 
                0, 
                MaxLODLevel
            );
            
            MeshComp->SetForcedLodModel(LODLevel + 1);
        }
    }
}

FVector UWorld_TerrainGenerator::GetBiomeCenter(EBiomeType BiomeType) const
{
    for (const FWorld_TerrainFeature& Feature : TerrainFeatures)
    {
        if (Feature.BiomeType == BiomeType)
        {
            return Feature.Location;
        }
    }
    
    return FVector::ZeroVector;
}

float UWorld_TerrainGenerator::GetTerrainHeightAtLocation(const FVector& Location) const
{
    // Simple height calculation based on biome
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            return 200.0f + FMath::Sin(Location.X * 0.001f) * 100.0f;
        case EBiomeType::Desert:
            return 100.0f + FMath::Cos(Location.Y * 0.0008f) * 50.0f;
        case EBiomeType::Savanna:
        default:
            return FMath::Sin(Location.X * 0.0005f) * FMath::Cos(Location.Y * 0.0005f) * 150.0f;
    }
}

EBiomeType UWorld_TerrainGenerator::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;

    for (const FWorld_TerrainFeature& Feature : TerrainFeatures)
    {
        float Distance = FVector::Dist2D(Location, Feature.Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Feature.BiomeType;
        }
    }

    return ClosestBiome;
}

void UWorld_TerrainGenerator::CreateTerrainMesh(const FVector& Location, const FVector& Size, EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create terrain actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* TerrainActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (TerrainActor)
    {
        // Set terrain mesh (using cube as placeholder)
        UStaticMeshComponent* MeshComp = TerrainActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Load default cube mesh
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                MeshComp->SetWorldScale3D(Size / 100.0f);  // Scale cube to terrain size
                
                // Apply biome-specific material
                ApplyBiomeMaterial(TerrainActor, BiomeType);
            }
        }

        // Set actor label
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        TerrainActor->SetActorLabel(FString::Printf(TEXT("Terrain_%s_%d"), *BiomeName, GeneratedTerrainActors.Num()));
        
        GeneratedTerrainActors.Add(TerrainActor);
        
        UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Created terrain mesh for %s at %s"), 
               *BiomeName, *Location.ToString());
    }
}

void UWorld_TerrainGenerator::CreateWaterPlane(const FWorld_WaterBody& WaterConfig)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        WaterConfig.Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (WaterActor)
    {
        UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Load plane mesh for water
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
            if (PlaneMesh)
            {
                MeshComp->SetStaticMesh(PlaneMesh);
                
                // Scale water plane
                FVector WaterScale = FVector(
                    WaterConfig.Size.X / 100.0f,
                    WaterConfig.Size.Y / 100.0f,
                    1.0f
                );
                MeshComp->SetWorldScale3D(WaterScale);
                
                // Set blue-tinted material for water
                UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
                if (WaterMaterial)
                {
                    MeshComp->SetMaterial(0, WaterMaterial);
                }
            }
        }

        // Set water actor label
        FString WaterType = WaterConfig.bIsRiver ? TEXT("River") : TEXT("Lake");
        WaterActor->SetActorLabel(FString::Printf(TEXT("Water_%s_%d"), *WaterType, GeneratedWaterActors.Num()));
        
        GeneratedWaterActors.Add(WaterActor);
        
        UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Created water body (%s) at %s"), 
               *WaterType, *WaterConfig.Location.ToString());
    }
}

void UWorld_TerrainGenerator::ApplyBiomeMaterial(AStaticMeshActor* TerrainActor, EBiomeType BiomeType)
{
    if (!TerrainActor)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = TerrainActor->GetStaticMeshComponent();
    if (!MeshComp)
    {
        return;
    }

    // Apply different colored materials for different biomes
    UMaterialInterface* BiomeMaterial = nullptr;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            // Green material for forest
            BiomeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EBiomeType::Desert:
            // Yellow/sand material for desert
            BiomeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EBiomeType::Savanna:
        default:
            // Brown/grass material for savanna
            BiomeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
    }

    if (BiomeMaterial)
    {
        MeshComp->SetMaterial(0, BiomeMaterial);
    }
}

void UWorld_TerrainGenerator::UpdateTerrainLOD(float DeltaTime)
{
    LODUpdateTimer += DeltaTime;
    
    if (LODUpdateTimer >= LOD_UPDATE_INTERVAL)
    {
        OptimizeTerrainLOD();
        LODUpdateTimer = 0.0f;
    }
}

float UWorld_TerrainGenerator::CalculateDistanceToPlayer(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FLT_MAX;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return FLT_MAX;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return FLT_MAX;
    }

    return FVector::Dist(Location, PlayerPawn->GetActorLocation());
}