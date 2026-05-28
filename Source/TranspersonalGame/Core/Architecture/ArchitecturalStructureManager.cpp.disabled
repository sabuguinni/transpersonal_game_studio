#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UArchitecturalStructureManager::UArchitecturalStructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    StructureSpawnRadius = 10000.0f;
    MaxStructuresPerBiome = 20;
    bAutoGenerateStructures = false;
}

void UArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeStructureSystem();
}

void UArchitecturalStructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UArchitecturalStructureManager::InitializeStructureSystem()
{
    if (bAutoGenerateStructures)
    {
        // Generate structures for different biomes
        GenerateStructuresForBiome(EBiomeType::Savana, 5);
        GenerateStructuresForBiome(EBiomeType::Forest, 3);
        GenerateStructuresForBiome(EBiomeType::Desert, 4);
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager initialized with %d managed structures"), ManagedStructures.Num());
}

void UArchitecturalStructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn structure: World is null"));
        return;
    }

    AActor* NewStructure = CreateStructureActor(StructureType, Location, Rotation);
    if (NewStructure)
    {
        FArch_StructureData StructureData;
        StructureData.StructureType = StructureType;
        StructureData.Location = Location;
        StructureData.Rotation = Rotation;
        StructureData.WeatheringLevel = FMath::RandRange(0.3f, 0.8f);
        StructureData.bHasMossGrowth = FMath::RandBool();

        ManagedStructures.Add(StructureData);
        SpawnedStructureActors.Add(NewStructure);

        SetupStructureAppearance(NewStructure, StructureData);

        UE_LOG(LogTemp, Warning, TEXT("Spawned architectural structure at location: %s"), *Location.ToString());
    }
}

void UArchitecturalStructureManager::GenerateStructuresForBiome(EBiomeType BiomeType, int32 StructureCount)
{
    FVector BiomeCenter;
    
    // Get biome center coordinates from SharedTypes
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeCenter = FVector(0, 0, 100);
            break;
        case EBiomeType::Forest:
            BiomeCenter = FVector(-45000, 40000, 100);
            break;
        case EBiomeType::Desert:
            BiomeCenter = FVector(55000, 0, 100);
            break;
        case EBiomeType::Mountain:
            BiomeCenter = FVector(40000, 50000, 200);
            break;
        case EBiomeType::Swamp:
            BiomeCenter = FVector(-50000, -45000, 50);
            break;
        default:
            BiomeCenter = FVector::ZeroVector;
            break;
    }

    for (int32 i = 0; i < StructureCount; i++)
    {
        // Random offset within biome area
        FVector RandomOffset = FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-50.0f, 50.0f)
        );

        FVector SpawnLocation = BiomeCenter + RandomOffset;
        FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);

        // Choose structure type based on biome
        EArch_StructureType StructureType;
        switch (BiomeType)
        {
            case EBiomeType::Savana:
                StructureType = (i % 2 == 0) ? EArch_StructureType::StonePillar : EArch_StructureType::StoneCircle;
                break;
            case EBiomeType::Forest:
                StructureType = EArch_StructureType::RockShelter;
                break;
            case EBiomeType::Desert:
                StructureType = EArch_StructureType::AncientRuin;
                break;
            case EBiomeType::Mountain:
                StructureType = EArch_StructureType::CaveDwelling;
                break;
            default:
                StructureType = EArch_StructureType::StonePillar;
                break;
        }

        SpawnStructureAtLocation(StructureType, SpawnLocation, RandomRotation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated %d structures for biome at center: %s"), StructureCount, *BiomeCenter.ToString());
}

AActor* UArchitecturalStructureManager::CreateStructureActor(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Create a StaticMeshActor as base
    AStaticMeshActor* StructureActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn StaticMeshActor for structure"));
        return nullptr;
    }

    // Set appropriate label based on structure type
    FString StructureLabel;
    switch (StructureType)
    {
        case EArch_StructureType::StonePillar:
            StructureLabel = TEXT("Arch_StonePillar");
            break;
        case EArch_StructureType::CaveDwelling:
            StructureLabel = TEXT("Arch_CaveDwelling");
            break;
        case EArch_StructureType::StoneCircle:
            StructureLabel = TEXT("Arch_StoneCircle");
            break;
        case EArch_StructureType::RockShelter:
            StructureLabel = TEXT("Arch_RockShelter");
            break;
        case EArch_StructureType::AncientRuin:
            StructureLabel = TEXT("Arch_AncientRuin");
            break;
        default:
            StructureLabel = TEXT("Arch_Unknown");
            break;
    }

    StructureActor->SetActorLabel(StructureLabel);
    return StructureActor;
}

void UArchitecturalStructureManager::SetupStructureAppearance(AActor* StructureActor, const FArch_StructureData& StructureData)
{
    if (!StructureActor)
    {
        return;
    }

    // Apply scale based on structure data
    StructureActor->SetActorScale3D(StructureData.Scale);

    // Apply weathering effects (placeholder implementation)
    if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(StructureActor))
    {
        UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Future: Apply weathering materials based on WeatheringLevel
            // Future: Add moss growth effects if bHasMossGrowth is true
        }
    }
}

void UArchitecturalStructureManager::ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringAmount)
{
    if (!StructureActor)
    {
        return;
    }

    // Find the structure data for this actor
    for (FArch_StructureData& StructureData : ManagedStructures)
    {
        if (StructureData.Location.Equals(StructureActor->GetActorLocation(), 10.0f))
        {
            StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel + WeatheringAmount, 0.0f, 1.0f);
            SetupStructureAppearance(StructureActor, StructureData);
            break;
        }
    }
}

TArray<AActor*> UArchitecturalStructureManager::GetAllManagedStructures() const
{
    return SpawnedStructureActors;
}

void UArchitecturalStructureManager::ClearAllStructures()
{
    for (AActor* StructureActor : SpawnedStructureActors)
    {
        if (IsValid(StructureActor))
        {
            StructureActor->Destroy();
        }
    }

    SpawnedStructureActors.Empty();
    ManagedStructures.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Cleared all managed architectural structures"));
}