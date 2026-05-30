#include "Arch_StructureManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default structure database
    StructureDatabase.Empty();
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-populate structures if database is empty
    if (StructureDatabase.Num() == 0)
    {
        PopulateAllBiomesWithStructures();
    }
}

void AArch_StructureManager::CreateStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType, FVector Offset)
{
    FVector BiomeCenter = GetBiomeCoordinates(BiomeType);
    FVector StructureLocation = BiomeCenter + Offset;

    switch (StructureType)
    {
        case EArch_StructureType::StoneCircle:
            CreateStoneCircle(StructureLocation);
            break;
        case EArch_StructureType::Platform:
            CreatePlatform(StructureLocation);
            break;
        case EArch_StructureType::Archway:
            CreateArchway(StructureLocation);
            break;
        default:
            CreatePlatform(StructureLocation);
            break;
    }

    // Record in database
    FArch_StructureData NewStructure;
    NewStructure.StructureType = StructureType;
    NewStructure.Location = StructureLocation;
    NewStructure.BiomeType = BiomeType;
    StructureDatabase.Add(NewStructure);
}

void AArch_StructureManager::CreateStoneCircle(FVector CenterLocation, float Radius, int32 NumStones)
{
    for (int32 i = 0; i < NumStones; i++)
    {
        float Angle = i * (360.0f / NumStones);
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector StoneOffset = FVector(
            Radius * FMath::Cos(RadianAngle),
            Radius * FMath::Sin(RadianAngle),
            0.0f
        );
        
        FVector StoneLocation = CenterLocation + StoneOffset;
        StoneLocation.Z += FMath::RandRange(0.0f, 200.0f);
        
        FRotator StoneRotation = FRotator(
            FMath::RandRange(-10.0f, 10.0f),
            Angle,
            FMath::RandRange(-5.0f, 5.0f)
        );
        
        FVector StoneScale = FVector(0.8f, 0.8f, FMath::RandRange(2.0f, 4.0f));
        
        UStaticMeshComponent* StoneComponent = CreateStructureComponent(StoneLocation, StoneRotation, StoneScale);
        if (StoneComponent)
        {
            StoneComponent->SetMobility(EComponentMobility::Static);
        }
    }
}

void AArch_StructureManager::CreatePlatform(FVector Location, FVector PlatformScale)
{
    FVector PlatformLocation = Location;
    PlatformLocation.Z = 50.0f;
    
    UStaticMeshComponent* PlatformComponent = CreateStructureComponent(PlatformLocation, FRotator::ZeroRotator, PlatformScale);
    if (PlatformComponent)
    {
        PlatformComponent->SetMobility(EComponentMobility::Static);
    }
}

void AArch_StructureManager::CreateArchway(FVector Location, FRotator Rotation)
{
    // Create two pillars and a top beam
    FVector LeftPillarLoc = Location + FVector(-200.0f, 0.0f, 0.0f);
    FVector RightPillarLoc = Location + FVector(200.0f, 0.0f, 0.0f);
    FVector BeamLoc = Location + FVector(0.0f, 0.0f, 400.0f);
    
    FVector PillarScale = FVector(1.0f, 1.0f, 4.0f);
    FVector BeamScale = FVector(4.0f, 1.0f, 0.5f);
    
    CreateStructureComponent(LeftPillarLoc, Rotation, PillarScale);
    CreateStructureComponent(RightPillarLoc, Rotation, PillarScale);
    CreateStructureComponent(BeamLoc, Rotation, BeamScale);
}

void AArch_StructureManager::PopulateAllBiomesWithStructures()
{
    // Create structures in each biome
    CreateStructureAtBiome(EBiomeType::Savanna, EArch_StructureType::StoneCircle);
    CreateStructureAtBiome(EBiomeType::Swamp, EArch_StructureType::Platform, FVector(0.0f, 0.0f, 100.0f));
    CreateStructureAtBiome(EBiomeType::Forest, EArch_StructureType::Archway);
    CreateStructureAtBiome(EBiomeType::Desert, EArch_StructureType::StoneCircle, FVector(1000.0f, 0.0f, 0.0f));
    CreateStructureAtBiome(EBiomeType::Mountain, EArch_StructureType::Platform, FVector(0.0f, 1000.0f, 200.0f));
}

FVector AArch_StructureManager::GetBiomeCoordinates(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 0.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 0.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 0.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 0.0f);
        default:
            return FVector::ZeroVector;
    }
}

void AArch_StructureManager::GenerateStructuresInEditor()
{
    StructureDatabase.Empty();
    PopulateAllBiomesWithStructures();
}

UStaticMeshComponent* AArch_StructureManager::CreateStructureComponent(FVector Location, FRotator Rotation, FVector Scale)
{
    UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("StructureMesh_%d"), StructureDatabase.Num()));
    
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(RootComponent);
        MeshComponent->SetWorldLocation(Location);
        MeshComponent->SetWorldRotation(Rotation);
        MeshComponent->SetWorldScale3D(Scale);
        
        // Apply default cube mesh if available
        if (DefaultStoneMesh)
        {
            MeshComponent->SetStaticMesh(DefaultStoneMesh);
        }
        
        // Apply weathered material
        if (StoneMaterial)
        {
            MeshComponent->SetMaterial(0, StoneMaterial);
        }
    }
    
    return MeshComponent;
}

void AArch_StructureManager::ApplyWeatheredMaterial(UStaticMeshComponent* MeshComponent, EBiomeType BiomeType)
{
    if (!MeshComponent) return;
    
    UMaterialInterface* MaterialToApply = StoneMaterial;
    
    // Choose material based on biome
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
        case EBiomeType::Forest:
            MaterialToApply = WeatheredStoneMaterial;
            break;
        default:
            MaterialToApply = StoneMaterial;
            break;
    }
    
    if (MaterialToApply)
    {
        MeshComponent->SetMaterial(0, MaterialToApply);
    }
}