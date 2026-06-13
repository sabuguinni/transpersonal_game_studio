#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    PlacementRadius = 10000.0f;
    MaxStructuresPerBiome = 15;
    MinDistanceBetweenStructures = 2000.0f;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: BeginPlay - Initializing structure management"));
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update structure degradation over time
    UpdateStructureDegradation(DeltaTime);
}

bool AArch_StructureManager::PlaceStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    // Check if location is valid
    if (!IsLocationValidForPlacement(Location, StructureType))
    {
        // Try to find a nearby valid location
        FVector ValidLocation = FindNearestValidPlacement(Location, StructureType);
        if (ValidLocation == FVector::ZeroVector)
        {
            UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: No valid placement location found"));
            return false;
        }
        Location = ValidLocation;
    }

    // Check structure count limit
    if (ManagedStructures.Num() >= MaxStructuresPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Maximum structures reached for biome"));
        return false;
    }

    // Create structure data
    FArch_StructureData NewStructure;
    NewStructure.StructureName = FString::Printf(TEXT("Structure_%d"), ManagedStructures.Num() + 1);
    NewStructure.StructureType = StructureType;
    NewStructure.PlacementLocation = Location;
    NewStructure.PlacementRotation = Rotation;
    NewStructure.DegradationLevel = FMath::RandRange(0.1f, 0.4f); // Random initial wear
    NewStructure.bIsAccessible = true;

    // Add to managed structures
    ManagedStructures.Add(NewStructure);

    // Spawn the actual mesh
    SpawnStructureMesh(NewStructure);

    UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: Structure placed at %s"), *Location.ToString());
    return true;
}

void AArch_StructureManager::RemoveStructure(int32 StructureIndex)
{
    if (ManagedStructures.IsValidIndex(StructureIndex))
    {
        ManagedStructures.RemoveAt(StructureIndex);
        UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: Structure removed at index %d"), StructureIndex);
    }
}

TArray<FArch_StructureData> AArch_StructureManager::GetStructuresInRadius(FVector CenterLocation, float Radius)
{
    TArray<FArch_StructureData> NearbyStructures;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        float Distance = FVector::Dist(CenterLocation, Structure.PlacementLocation);
        if (Distance <= Radius)
        {
            NearbyStructures.Add(Structure);
        }
    }
    
    return NearbyStructures;
}

void AArch_StructureManager::UpdateStructureDegradation(float DeltaTime)
{
    // Slowly degrade structures over time
    float DegradationRate = 0.00001f; // Very slow degradation
    
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        Structure.DegradationLevel = FMath::Clamp(Structure.DegradationLevel + (DegradationRate * DeltaTime), 0.0f, 1.0f);
        
        // Mark as inaccessible if heavily degraded
        if (Structure.DegradationLevel > 0.8f)
        {
            Structure.bIsAccessible = false;
        }
    }
}

void AArch_StructureManager::GenerateStructuresInBiome()
{
    UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Generating structures in biome"));
    
    // Clear existing structures
    ClearAllStructures();
    
    // Generate random structures within placement radius
    int32 StructuresToGenerate = FMath::RandRange(5, MaxStructuresPerBiome);
    
    for (int32 i = 0; i < StructuresToGenerate; i++)
    {
        // Random location within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(1000.0f, PlacementRadius);
        
        FVector RandomLocation = GetActorLocation() + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Random structure type
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
        
        // Random rotation
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        PlaceStructure(RandomType, RandomLocation, RandomRotation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: Generated %d structures"), ManagedStructures.Num());
}

void AArch_StructureManager::ClearAllStructures()
{
    ManagedStructures.Empty();
    UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: All structures cleared"));
}

bool AArch_StructureManager::IsLocationValidForPlacement(FVector Location, EArch_StructureType StructureType)
{
    // Check minimum distance from other structures
    for (const FArch_StructureData& ExistingStructure : ManagedStructures)
    {
        float Distance = FVector::Dist(Location, ExistingStructure.PlacementLocation);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    // Additional validation could include terrain checks, water proximity, etc.
    return true;
}

FVector AArch_StructureManager::FindNearestValidPlacement(FVector DesiredLocation, EArch_StructureType StructureType)
{
    // Try locations in expanding circles around the desired location
    for (float Radius = 500.0f; Radius <= 5000.0f; Radius += 500.0f)
    {
        for (int32 i = 0; i < 8; i++)
        {
            float Angle = (i * 45.0f) * PI / 180.0f;
            FVector TestLocation = DesiredLocation + FVector(
                FMath::Cos(Angle) * Radius,
                FMath::Sin(Angle) * Radius,
                0.0f
            );
            
            if (IsLocationValidForPlacement(TestLocation, StructureType))
            {
                return TestLocation;
            }
        }
    }
    
    return FVector::ZeroVector; // No valid location found
}

void AArch_StructureManager::SpawnStructureMesh(const FArch_StructureData& StructureData)
{
    // Create a static mesh component for visualization
    UStaticMeshComponent* StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("StructureMesh_%d"), ManagedStructures.Num())
    );
    
    if (StructureMesh)
    {
        StructureMesh->SetupAttachment(RootComponent);
        StructureMesh->SetWorldLocation(StructureData.PlacementLocation);
        StructureMesh->SetWorldRotation(StructureData.PlacementRotation);
        
        // Apply degradation-based scaling
        float Scale = 1.0f - (StructureData.DegradationLevel * 0.3f);
        StructureMesh->SetWorldScale3D(FVector(Scale));
        
        UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: Structure mesh spawned"));
    }
}