#include "World_ProceduralVegetation.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/CollisionProfile.h"

DEFINE_LOG_CATEGORY_STATIC(LogProceduralVegetation, Log, All);

AWorld_ProceduralVegetation::AWorld_ProceduralVegetation()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create instanced static mesh components for different vegetation types
    TreeInstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TreeInstances"));
    TreeInstancedMesh->SetupAttachment(RootComponent);
    TreeInstancedMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    BushInstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BushInstances"));
    BushInstancedMesh->SetupAttachment(RootComponent);
    BushInstancedMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    GrassInstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GrassInstances"));
    GrassInstancedMesh->SetupAttachment(RootComponent);
    GrassInstancedMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    
    RockInstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RockInstances"));
    RockInstancedMesh->SetupAttachment(RootComponent);
    RockInstancedMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    // Initialize default values
    CurrentBiome = EWorld_BiomeType::Savana;
    VegetationDensity = 1.0f;
    SpawnRadius = 50000.0f;
    MinDistanceBetweenTrees = 500.0f;
    MinDistanceBetweenBushes = 200.0f;
    
    // Biome-specific density multipliers
    BiomeDensityMultipliers.Add(EWorld_BiomeType::Pantano, 1.5f);
    BiomeDensityMultipliers.Add(EWorld_BiomeType::Floresta, 2.0f);
    BiomeDensityMultipliers.Add(EWorld_BiomeType::Savana, 0.8f);
    BiomeDensityMultipliers.Add(EWorld_BiomeType::Deserto, 0.2f);
    BiomeDensityMultipliers.Add(EWorld_BiomeType::Montanha, 0.5f);
    
    // Initialize vegetation counts per biome
    TreeCountPerBiome.Add(EWorld_BiomeType::Pantano, 300);
    TreeCountPerBiome.Add(EWorld_BiomeType::Floresta, 500);
    TreeCountPerBiome.Add(EWorld_BiomeType::Savana, 150);
    TreeCountPerBiome.Add(EWorld_BiomeType::Deserto, 20);
    TreeCountPerBiome.Add(EWorld_BiomeType::Montanha, 100);
    
    BushCountPerBiome.Add(EWorld_BiomeType::Pantano, 200);
    BushCountPerBiome.Add(EWorld_BiomeType::Floresta, 400);
    BushCountPerBiome.Add(EWorld_BiomeType::Savana, 100);
    BushCountPerBiome.Add(EWorld_BiomeType::Deserto, 30);
    BushCountPerBiome.Add(EWorld_BiomeType::Montanha, 80);
    
    GrassCountPerBiome.Add(EWorld_BiomeType::Pantano, 800);
    GrassCountPerBiome.Add(EWorld_BiomeType::Floresta, 600);
    GrassCountPerBiome.Add(EWorld_BiomeType::Savana, 1000);
    GrassCountPerBiome.Add(EWorld_BiomeType::Deserto, 50);
    GrassCountPerBiome.Add(EWorld_BiomeType::Montanha, 200);
    
    RockCountPerBiome.Add(EWorld_BiomeType::Pantano, 50);
    RockCountPerBiome.Add(EWorld_BiomeType::Floresta, 100);
    RockCountPerBiome.Add(EWorld_BiomeType::Savana, 80);
    RockCountPerBiome.Add(EWorld_BiomeType::Deserto, 150);
    RockCountPerBiome.Add(EWorld_BiomeType::Montanha, 300);
}

void AWorld_ProceduralVegetation::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Procedural Vegetation System initialized"));
}

void AWorld_ProceduralVegetation::GenerateVegetationForBiome(EWorld_BiomeType BiomeType, FVector BiomeCenter, float BiomeRadius)
{
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generating vegetation for biome %d at location %s with radius %f"), 
           (int32)BiomeType, *BiomeCenter.ToString(), BiomeRadius);
    
    CurrentBiome = BiomeType;
    
    // Clear existing vegetation instances
    ClearAllVegetation();
    
    // Get biome-specific vegetation counts
    int32 TreeCount = GetVegetationCountForBiome(BiomeType, EWorld_VegetationType::Tree);
    int32 BushCount = GetVegetationCountForBiome(BiomeType, EWorld_VegetationType::Bush);
    int32 GrassCount = GetVegetationCountForBiome(BiomeType, EWorld_VegetationType::Grass);
    int32 RockCount = GetVegetationCountForBiome(BiomeType, EWorld_VegetationType::Rock);
    
    // Generate vegetation instances
    GenerateTrees(BiomeCenter, BiomeRadius, TreeCount);
    GenerateBushes(BiomeCenter, BiomeRadius, BushCount);
    GenerateGrass(BiomeCenter, BiomeRadius, GrassCount);
    GenerateRocks(BiomeCenter, BiomeRadius, RockCount);
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generated %d trees, %d bushes, %d grass, %d rocks for biome %d"), 
           TreeCount, BushCount, GrassCount, RockCount, (int32)BiomeType);
}

void AWorld_ProceduralVegetation::GenerateTrees(FVector Center, float Radius, int32 Count)
{
    if (!TreeInstancedMesh || Count <= 0)
    {
        return;
    }
    
    TArray<FVector> TreePositions;
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector Position = GenerateRandomPositionInRadius(Center, Radius);
        
        // Check minimum distance from other trees
        bool bValidPosition = true;
        for (const FVector& ExistingPos : TreePositions)
        {
            if (FVector::Dist(Position, ExistingPos) < MinDistanceBetweenTrees)
            {
                bValidPosition = false;
                break;
            }
        }
        
        if (bValidPosition)
        {
            TreePositions.Add(Position);
            
            // Create transform for tree instance
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(Position);
            InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
            
            // Random scale variation
            float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
            InstanceTransform.SetScale3D(FVector(ScaleVariation));
            
            TreeInstancedMesh->AddInstance(InstanceTransform);
        }
    }
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generated %d tree instances"), TreePositions.Num());
}

void AWorld_ProceduralVegetation::GenerateBushes(FVector Center, float Radius, int32 Count)
{
    if (!BushInstancedMesh || Count <= 0)
    {
        return;
    }
    
    TArray<FVector> BushPositions;
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector Position = GenerateRandomPositionInRadius(Center, Radius);
        
        // Check minimum distance from other bushes
        bool bValidPosition = true;
        for (const FVector& ExistingPos : BushPositions)
        {
            if (FVector::Dist(Position, ExistingPos) < MinDistanceBetweenBushes)
            {
                bValidPosition = false;
                break;
            }
        }
        
        if (bValidPosition)
        {
            BushPositions.Add(Position);
            
            // Create transform for bush instance
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(Position);
            InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
            
            // Random scale variation
            float ScaleVariation = FMath::RandRange(0.7f, 1.3f);
            InstanceTransform.SetScale3D(FVector(ScaleVariation));
            
            BushInstancedMesh->AddInstance(InstanceTransform);
        }
    }
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generated %d bush instances"), BushPositions.Num());
}

void AWorld_ProceduralVegetation::GenerateGrass(FVector Center, float Radius, int32 Count)
{
    if (!GrassInstancedMesh || Count <= 0)
    {
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector Position = GenerateRandomPositionInRadius(Center, Radius);
        
        // Create transform for grass instance
        FTransform InstanceTransform;
        InstanceTransform.SetLocation(Position);
        InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
        
        // Random scale variation for grass
        float ScaleVariation = FMath::RandRange(0.5f, 1.5f);
        InstanceTransform.SetScale3D(FVector(ScaleVariation));
        
        GrassInstancedMesh->AddInstance(InstanceTransform);
    }
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generated %d grass instances"), Count);
}

void AWorld_ProceduralVegetation::GenerateRocks(FVector Center, float Radius, int32 Count)
{
    if (!RockInstancedMesh || Count <= 0)
    {
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector Position = GenerateRandomPositionInRadius(Center, Radius);
        
        // Create transform for rock instance
        FTransform InstanceTransform;
        InstanceTransform.SetLocation(Position);
        InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(
            FMath::RandRange(-15.0f, 15.0f),  // Slight pitch variation
            FMath::RandRange(-15.0f, 15.0f),  // Slight roll variation
            FMath::RandRange(0.0f, 360.0f)    // Random yaw
        )));
        
        // Random scale variation for rocks
        float ScaleVariation = FMath::RandRange(0.6f, 2.0f);
        InstanceTransform.SetScale3D(FVector(ScaleVariation));
        
        RockInstancedMesh->AddInstance(InstanceTransform);
    }
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Generated %d rock instances"), Count);
}

FVector AWorld_ProceduralVegetation::GenerateRandomPositionInRadius(FVector Center, float Radius)
{
    // Generate random position within circular area
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return Center + RandomOffset;
}

int32 AWorld_ProceduralVegetation::GetVegetationCountForBiome(EWorld_BiomeType BiomeType, EWorld_VegetationType VegType)
{
    float DensityMultiplier = BiomeDensityMultipliers.Contains(BiomeType) ? 
                              BiomeDensityMultipliers[BiomeType] : 1.0f;
    
    int32 BaseCount = 0;
    
    switch (VegType)
    {
        case EWorld_VegetationType::Tree:
            BaseCount = TreeCountPerBiome.Contains(BiomeType) ? TreeCountPerBiome[BiomeType] : 100;
            break;
        case EWorld_VegetationType::Bush:
            BaseCount = BushCountPerBiome.Contains(BiomeType) ? BushCountPerBiome[BiomeType] : 50;
            break;
        case EWorld_VegetationType::Grass:
            BaseCount = GrassCountPerBiome.Contains(BiomeType) ? GrassCountPerBiome[BiomeType] : 200;
            break;
        case EWorld_VegetationType::Rock:
            BaseCount = RockCountPerBiome.Contains(BiomeType) ? RockCountPerBiome[BiomeType] : 30;
            break;
    }
    
    return FMath::RoundToInt(BaseCount * DensityMultiplier * VegetationDensity);
}

void AWorld_ProceduralVegetation::ClearAllVegetation()
{
    if (TreeInstancedMesh)
    {
        TreeInstancedMesh->ClearInstances();
    }
    
    if (BushInstancedMesh)
    {
        BushInstancedMesh->ClearInstances();
    }
    
    if (GrassInstancedMesh)
    {
        GrassInstancedMesh->ClearInstances();
    }
    
    if (RockInstancedMesh)
    {
        RockInstancedMesh->ClearInstances();
    }
    
    UE_LOG(LogProceduralVegetation, Log, TEXT("Cleared all vegetation instances"));
}

void AWorld_ProceduralVegetation::SetVegetationDensity(float NewDensity)
{
    VegetationDensity = FMath::Clamp(NewDensity, 0.0f, 3.0f);
    UE_LOG(LogProceduralVegetation, Log, TEXT("Vegetation density set to %f"), VegetationDensity);
}

void AWorld_ProceduralVegetation::RegenerateVegetationForCurrentBiome()
{
    FVector CurrentCenter = GetActorLocation();
    GenerateVegetationForBiome(CurrentBiome, CurrentCenter, SpawnRadius);
}

int32 AWorld_ProceduralVegetation::GetTotalVegetationInstanceCount() const
{
    int32 TotalCount = 0;
    
    if (TreeInstancedMesh)
    {
        TotalCount += TreeInstancedMesh->GetInstanceCount();
    }
    
    if (BushInstancedMesh)
    {
        TotalCount += BushInstancedMesh->GetInstanceCount();
    }
    
    if (GrassInstancedMesh)
    {
        TotalCount += GrassInstancedMesh->GetInstanceCount();
    }
    
    if (RockInstancedMesh)
    {
        TotalCount += RockInstancedMesh->GetInstanceCount();
    }
    
    return TotalCount;
}