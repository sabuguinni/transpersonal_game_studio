#include "ArchitecturalManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArchitecturalManager::AArchitecturalManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxStructuresPerBiome = 50;
    MinDistanceBetweenStructures = 1000.0f;

    // Initialize structure count per biome
    StructureCountPerBiome.Add(EBiomeType::Savanna, 0);
    StructureCountPerBiome.Add(EBiomeType::Swamp, 0);
    StructureCountPerBiome.Add(EBiomeType::Forest, 0);
    StructureCountPerBiome.Add(EBiomeType::Desert, 0);
    StructureCountPerBiome.Add(EBiomeType::Mountain, 0);
}

void AArchitecturalManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: BeginPlay - Initializing architectural system"));
    
    // Initialize structure database if empty
    if (StructureDatabase.Num() == 0)
    {
        GenerateArchitecturalLayout();
    }
}

void AArchitecturalManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update structure degradation over time
    UpdateStructureDegradation(DeltaTime);
}

void AArchitecturalManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    if (!IsLocationValid(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Invalid location for structure spawn"));
        return;
    }

    // Create structure data
    FArch_StructureData NewStructure;
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.Rotation = Rotation;
    NewStructure.Scale = FVector(1.0f, 1.0f, 1.0f);
    NewStructure.DegradationLevel = FMath::RandRange(0.1f, 0.8f);
    NewStructure.bHasVegetationOvergrowth = FMath::RandBool();

    // Add to database
    StructureDatabase.Add(NewStructure);

    // Spawn the actual mesh
    AActor* SpawnedActor = SpawnStructureMesh(StructureType, Location, Rotation);
    if (SpawnedActor)
    {
        FString StructureName = FString::Printf(TEXT("Arch_%s_%d"), 
            *UEnum::GetValueAsString(StructureType), StructureDatabase.Num());
        SpawnedActor->SetActorLabel(StructureName);
        
        UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Spawned %s at %s"), 
            *StructureName, *Location.ToString());
    }
}

void AArchitecturalManager::SpawnStructuresInBiome(EBiomeType BiomeType, int32 Count)
{
    int32 CurrentCount = StructureCountPerBiome.FindRef(BiomeType);
    
    if (CurrentCount >= MaxStructuresPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Biome %s already at max structures (%d)"), 
            *UEnum::GetValueAsString(BiomeType), MaxStructuresPerBiome);
        return;
    }

    int32 ActualCount = FMath::Min(Count, MaxStructuresPerBiome - CurrentCount);
    
    for (int32 i = 0; i < ActualCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(
            FMath::RandRange(1, static_cast<int32>(EArch_StructureType::Ruins)));
        
        SpawnStructureAtLocation(RandomType, SpawnLocation);
        StructureCountPerBiome[BiomeType]++;
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Spawned %d structures in %s biome"), 
        ActualCount, *UEnum::GetValueAsString(BiomeType));
}

void AArchitecturalManager::ClearAllStructures()
{
    StructureDatabase.Empty();
    
    // Reset biome counts
    for (auto& Pair : StructureCountPerBiome)
    {
        Pair.Value = 0;
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Cleared all structures"));
}

int32 AArchitecturalManager::GetStructureCount() const
{
    return StructureDatabase.Num();
}

TArray<FArch_StructureData> AArchitecturalManager::GetStructuresInRadius(FVector Center, float Radius) const
{
    TArray<FArch_StructureData> NearbyStructures;
    
    for (const FArch_StructureData& Structure : StructureDatabase)
    {
        float Distance = FVector::Dist(Center, Structure.Location);
        if (Distance <= Radius)
        {
            NearbyStructures.Add(Structure);
        }
    }
    
    return NearbyStructures;
}

void AArchitecturalManager::ApplyWeatheringToStructures(float WeatheringAmount)
{
    for (FArch_StructureData& Structure : StructureDatabase)
    {
        Structure.DegradationLevel = FMath::Clamp(
            Structure.DegradationLevel + WeatheringAmount, 0.0f, 1.0f);
        
        // Increase chance of vegetation overgrowth with degradation
        if (Structure.DegradationLevel > 0.5f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            Structure.bHasVegetationOvergrowth = true;
        }
    }
}

void AArchitecturalManager::UpdateStructureDegradation(float DeltaTime)
{
    // Gradual degradation over time (very slow)
    float DegradationRate = 0.00001f; // Extremely slow for gameplay purposes
    
    for (FArch_StructureData& Structure : StructureDatabase)
    {
        Structure.DegradationLevel = FMath::Clamp(
            Structure.DegradationLevel + (DegradationRate * DeltaTime), 0.0f, 1.0f);
    }
}

void AArchitecturalManager::GenerateArchitecturalLayout()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Generating architectural layout"));
    
    // Clear existing structures
    ClearAllStructures();
    
    // Generate structures for each biome
    SpawnStructuresInBiome(EBiomeType::Savanna, 8);
    SpawnStructuresInBiome(EBiomeType::Swamp, 6);
    SpawnStructuresInBiome(EBiomeType::Forest, 10);
    SpawnStructuresInBiome(EBiomeType::Desert, 7);
    SpawnStructuresInBiome(EBiomeType::Mountain, 9);
}

void AArchitecturalManager::ValidateStructurePlacement()
{
    // Remove structures that are too close to each other
    for (int32 i = StructureDatabase.Num() - 1; i >= 0; i--)
    {
        for (int32 j = i - 1; j >= 0; j--)
        {
            float Distance = FVector::Dist(StructureDatabase[i].Location, StructureDatabase[j].Location);
            if (Distance < MinDistanceBetweenStructures)
            {
                StructureDatabase.RemoveAt(i);
                break;
            }
        }
    }
}

FVector AArchitecturalManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    FVector BiomeCenter;
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            BiomeCenter = FVector(0, 0, 0);
            break;
        case EBiomeType::Swamp:
            BiomeCenter = FVector(-50000, -45000, 0);
            break;
        case EBiomeType::Forest:
            BiomeCenter = FVector(-45000, 40000, 0);
            break;
        case EBiomeType::Desert:
            BiomeCenter = FVector(55000, 0, 0);
            break;
        case EBiomeType::Mountain:
            BiomeCenter = FVector(40000, 50000, 0);
            break;
        default:
            BiomeCenter = FVector::ZeroVector;
    }
    
    // Add random offset within biome bounds
    FVector RandomOffset = FVector(
        FMath::RandRange(-15000.0f, 15000.0f),
        FMath::RandRange(-15000.0f, 15000.0f),
        FMath::RandRange(50.0f, 200.0f)
    );
    
    return BiomeCenter + RandomOffset;
}

bool AArchitecturalManager::IsLocationValid(FVector Location) const
{
    // Check if location is too close to existing structures
    for (const FArch_StructureData& Structure : StructureDatabase)
    {
        float Distance = FVector::Dist(Location, Structure.Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    // Check if location is within reasonable bounds
    if (FMath::Abs(Location.X) > 100000.0f || FMath::Abs(Location.Y) > 100000.0f)
    {
        return false;
    }
    
    return true;
}

AActor* AArchitecturalManager::SpawnStructureMesh(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // For now, spawn a basic StaticMeshActor as placeholder
    // In production, this would load specific meshes based on StructureType
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParams);
    
    if (SpawnedActor)
    {
        // Set appropriate scale based on structure type
        FVector Scale = FVector::OneVector;
        switch (StructureType)
        {
            case EArch_StructureType::Pillar:
                Scale = FVector(0.5f, 0.5f, 2.0f);
                break;
            case EArch_StructureType::Wall:
                Scale = FVector(3.0f, 0.3f, 1.5f);
                break;
            case EArch_StructureType::Watchtower:
                Scale = FVector(1.0f, 1.0f, 3.0f);
                break;
            default:
                Scale = FVector::OneVector;
        }
        
        SpawnedActor->SetActorScale3D(Scale);
    }
    
    return SpawnedActor;
}