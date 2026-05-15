#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// AArch_ArchitecturalStructure Implementation
AArch_ArchitecturalStructure::AArch_ArchitecturalStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default structure data
    StructureData = FArch_StructureData();
    
    // Set default mesh reference (will be set via Blueprint or code)
    DefaultMesh = nullptr;

    // Set collision
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    StructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

void AArch_ArchitecturalStructure::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial structure data
    if (StructureMesh && DefaultMesh)
    {
        StructureMesh->SetStaticMesh(DefaultMesh);
        SetActorScale3D(StructureData.Scale);
    }
}

void AArch_ArchitecturalStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update structural integrity over time (weathering simulation)
    if (StructureData.StructuralIntegrity > 0.0f)
    {
        float WeatheringRate = 0.01f; // Very slow degradation
        StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - (WeatheringRate * DeltaTime));
    }
}

void AArch_ArchitecturalStructure::InitializeStructure(const FArch_StructureData& InStructureData)
{
    StructureData = InStructureData;
    
    SetActorLocation(StructureData.Location);
    SetActorRotation(StructureData.Rotation);
    SetActorScale3D(StructureData.Scale);
    
    UpdateMeshForBiome(StructureData.BiomeType);
}

void AArch_ArchitecturalStructure::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    
    // Update max occupants based on structure type
    switch (NewType)
    {
        case EArch_StructureType::CaveDwelling:
            StructureData.MaxOccupants = 6;
            break;
        case EArch_StructureType::ElevatedShelter:
            StructureData.MaxOccupants = 4;
            break;
        case EArch_StructureType::StonePillar:
            StructureData.MaxOccupants = 1;
            StructureData.bIsInteractable = false;
            break;
        case EArch_StructureType::WoodenPlatform:
            StructureData.MaxOccupants = 8;
            break;
        case EArch_StructureType::RockFormation:
            StructureData.MaxOccupants = 2;
            StructureData.bIsInteractable = false;
            break;
        default:
            StructureData.MaxOccupants = 4;
            break;
    }
}

EArch_StructureType AArch_ArchitecturalStructure::GetStructureType() const
{
    return StructureData.StructureType;
}

void AArch_ArchitecturalStructure::SetStructuralIntegrity(float NewIntegrity)
{
    StructureData.StructuralIntegrity = FMath::Clamp(NewIntegrity, 0.0f, 100.0f);
}

float AArch_ArchitecturalStructure::GetStructuralIntegrity() const
{
    return StructureData.StructuralIntegrity;
}

bool AArch_ArchitecturalStructure::CanAccommodateOccupants(int32 RequestedOccupants) const
{
    return RequestedOccupants <= StructureData.MaxOccupants && StructureData.StructuralIntegrity > 25.0f;
}

void AArch_ArchitecturalStructure::UpdateMeshForBiome(EBiomeType BiomeType)
{
    // This would typically load different meshes based on biome
    // For now, we'll use the default mesh but could scale or modify materials
    if (StructureMesh && DefaultMesh)
    {
        StructureMesh->SetStaticMesh(DefaultMesh);
        
        // Adjust scale based on biome characteristics
        FVector BiomeScale = StructureData.Scale;
        switch (BiomeType)
        {
            case EBiomeType::Swamp:
                BiomeScale *= 0.8f; // Smaller structures in swamp
                break;
            case EBiomeType::Mountain:
                BiomeScale *= 1.2f; // Larger structures in mountains
                break;
            case EBiomeType::Desert:
                BiomeScale *= 0.9f; // Compact structures in desert
                break;
            default:
                break;
        }
        SetActorScale3D(BiomeScale);
    }
}

// UArch_ArchitecturalStructureManager Implementation
UArch_ArchitecturalStructureManager::UArch_ArchitecturalStructureManager()
{
    ManagedStructures.Empty();
}

UArch_ArchitecturalStructureManager* UArch_ArchitecturalStructureManager::GetInstance(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }
    
    // Simple singleton pattern - in production, this would be a subsystem
    static UArch_ArchitecturalStructureManager* Instance = nullptr;
    if (!Instance)
    {
        Instance = NewObject<UArch_ArchitecturalStructureManager>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    
    return Instance;
}

AArch_ArchitecturalStructure* UArch_ArchitecturalStructureManager::SpawnStructure(UWorld* World, const FArch_StructureData& StructureData)
{
    if (!World)
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AArch_ArchitecturalStructure* NewStructure = World->SpawnActor<AArch_ArchitecturalStructure>(
        AArch_ArchitecturalStructure::StaticClass(),
        StructureData.Location,
        StructureData.Rotation,
        SpawnParams
    );
    
    if (NewStructure)
    {
        NewStructure->InitializeStructure(StructureData);
        ManagedStructures.Add(NewStructure);
        
        // Set a descriptive label
        FString StructureLabel = FString::Printf(TEXT("Arch_Structure_%s_%s"), 
            *UEnum::GetValueAsString(StructureData.StructureType),
            *UEnum::GetValueAsString(StructureData.BiomeType));
        NewStructure->SetActorLabel(StructureLabel);
    }
    
    return NewStructure;
}

void UArch_ArchitecturalStructureManager::SpawnStructuresForBiome(UWorld* World, EBiomeType BiomeType, int32 StructureCount)
{
    if (!World || StructureCount <= 0)
    {
        return;
    }
    
    FVector BiomeCenter = GetBiomeBaseLocation(BiomeType);
    float SpawnRadius = 5000.0f; // 5km radius around biome center
    
    for (int32 i = 0; i < StructureCount; i++)
    {
        // Generate random location within biome radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnLocation.Z = 100.0f; // Default height above ground
        
        FArch_StructureData NewStructureData = GenerateStructureDataForBiome(BiomeType, SpawnLocation);
        SpawnStructure(World, NewStructureData);
    }
}

TArray<AArch_ArchitecturalStructure*> UArch_ArchitecturalStructureManager::GetStructuresInRadius(UWorld* World, FVector Center, float Radius)
{
    TArray<AArch_ArchitecturalStructure*> StructuresInRadius;
    
    for (auto& WeakStructure : ManagedStructures)
    {
        if (WeakStructure.IsValid())
        {
            AArch_ArchitecturalStructure* Structure = WeakStructure.Get();
            if (Structure && FVector::Dist(Structure->GetActorLocation(), Center) <= Radius)
            {
                StructuresInRadius.Add(Structure);
            }
        }
    }
    
    return StructuresInRadius;
}

void UArch_ArchitecturalStructureManager::UpdateAllStructures(UWorld* World, float DeltaTime)
{
    // Clean up invalid references
    ManagedStructures.RemoveAll([](const TWeakObjectPtr<AArch_ArchitecturalStructure>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    // Update all valid structures
    for (auto& WeakStructure : ManagedStructures)
    {
        if (WeakStructure.IsValid())
        {
            // Structures update themselves via Tick, but we could add manager-level updates here
        }
    }
}

int32 UArch_ArchitecturalStructureManager::GetTotalStructureCount(UWorld* World) const
{
    int32 ValidCount = 0;
    for (const auto& WeakStructure : ManagedStructures)
    {
        if (WeakStructure.IsValid())
        {
            ValidCount++;
        }
    }
    return ValidCount;
}

FArch_StructureData UArch_ArchitecturalStructureManager::GenerateStructureDataForBiome(EBiomeType BiomeType, FVector BaseLocation)
{
    FArch_StructureData StructureData;
    StructureData.BiomeType = BiomeType;
    StructureData.Location = BaseLocation;
    StructureData.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    StructureData.StructureType = GetPreferredStructureTypeForBiome(BiomeType);
    
    // Randomize scale slightly
    float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
    StructureData.Scale = FVector(ScaleVariation);
    
    // Set random structural integrity
    StructureData.StructuralIntegrity = FMath::RandRange(60.0f, 100.0f);
    
    return StructureData;
}

FVector UArch_ArchitecturalStructureManager::GetBiomeBaseLocation(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

EArch_StructureType UArch_ArchitecturalStructureManager::GetPreferredStructureTypeForBiome(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return EArch_StructureType::ElevatedShelter;
        case EBiomeType::Swamp:
            return EArch_StructureType::ElevatedShelter; // Keep dry
        case EBiomeType::Forest:
            return EArch_StructureType::WoodenPlatform;
        case EBiomeType::Desert:
            return EArch_StructureType::CaveDwelling; // Shade and protection
        case EBiomeType::Mountain:
            return EArch_StructureType::CaveDwelling; // Natural caves
        default:
            return EArch_StructureType::CaveDwelling;
    }
}