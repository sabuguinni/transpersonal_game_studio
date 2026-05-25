#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

UArch_StructureManager::UArch_StructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;

    MaxStructureDistance = 100000.0f;
    MaxStructuresPerBiome = 50;
    bAutoSpawnStructures = false;
}

void UArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStructureDatabase();
    
    if (bAutoSpawnStructures)
    {
        PopulateBiomeWithStructures();
    }
}

void UArch_StructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structural integrity over time
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.bIsRuin)
        {
            Structure.StructuralIntegrity = FMath::Max(0.0f, Structure.StructuralIntegrity - (DeltaTime * 0.1f));
        }
    }
}

void UArch_StructureManager::SpawnStructureAtLocation(const FVector& Location, EArch_StructureType StructureType, EBiomeType BiomeType)
{
    if (!IsLocationSuitableForStructure(Location, StructureType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Location not suitable for structure"));
        return;
    }

    FArch_StructureData NewStructure;
    NewStructure.StructureName = FString::Printf(TEXT("%s_%d"), 
        *UEnum::GetValueAsString(StructureType), ManagedStructures.Num());
    NewStructure.Location = Location;
    NewStructure.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    NewStructure.BiomeType = BiomeType;
    NewStructure.StructuralIntegrity = FMath::RandRange(60.0f, 100.0f);
    NewStructure.bIsRuin = (NewStructure.StructuralIntegrity < 80.0f);

    ManagedStructures.Add(NewStructure);
    SpawnStructureActor(NewStructure, StructureType);

    UE_LOG(LogTemp, Log, TEXT("Spawned structure: %s at %s"), 
        *NewStructure.StructureName, *Location.ToString());
}

void UArch_StructureManager::RemoveStructureAtLocation(const FVector& Location, float SearchRadius)
{
    for (int32 i = ManagedStructures.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(ManagedStructures[i].Location, Location) <= SearchRadius)
        {
            UE_LOG(LogTemp, Log, TEXT("Removed structure: %s"), *ManagedStructures[i].StructureName);
            ManagedStructures.RemoveAt(i);
            break;
        }
    }
}

TArray<FArch_StructureData> UArch_StructureManager::GetStructuresInBiome(EBiomeType BiomeType) const
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void UArch_StructureManager::UpdateStructuralIntegrity(const FVector& Location, float IntegrityChange)
{
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        if (FVector::Dist(Structure.Location, Location) <= 1000.0f)
        {
            Structure.StructuralIntegrity = FMath::Clamp(
                Structure.StructuralIntegrity + IntegrityChange, 0.0f, 100.0f);
            
            Structure.bIsRuin = (Structure.StructuralIntegrity < 50.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Updated structure integrity: %s -> %.1f"), 
                *Structure.StructureName, Structure.StructuralIntegrity);
        }
    }
}

void UArch_StructureManager::PopulateBiomeWithStructures()
{
    TArray<FVector> BiomeLocations = {
        FVector(0.0f, 0.0f, 100.0f),           // Savanna
        FVector(-45000.0f, 40000.0f, 100.0f),  // Forest
        FVector(55000.0f, 0.0f, 100.0f),       // Desert
        FVector(-50000.0f, -45000.0f, 100.0f), // Swamp
        FVector(40000.0f, 50000.0f, 200.0f)    // Mountain
    };

    TArray<EBiomeType> BiomeTypes = {
        EBiomeType::Savanna,
        EBiomeType::Forest,
        EBiomeType::Desert,
        EBiomeType::Swamp,
        EBiomeType::Mountain
    };

    for (int32 BiomeIndex = 0; BiomeIndex < BiomeLocations.Num(); BiomeIndex++)
    {
        FVector BaseLocation = BiomeLocations[BiomeIndex];
        EBiomeType BiomeType = BiomeTypes[BiomeIndex];

        // Spawn 5-10 structures per biome
        int32 StructureCount = FMath::RandRange(5, 10);
        
        for (int32 i = 0; i < StructureCount; i++)
        {
            FVector Offset = FVector(
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(-5000.0f, 5000.0f),
                0.0f
            );
            
            FVector SpawnLocation = BaseLocation + Offset;
            EArch_StructureType StructureType = static_cast<EArch_StructureType>(
                FMath::RandRange(0, static_cast<int32>(EArch_StructureType::Bridge)));
            
            SpawnStructureAtLocation(SpawnLocation, StructureType, BiomeType);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Populated all biomes with structures. Total: %d"), ManagedStructures.Num());
}

bool UArch_StructureManager::IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const
{
    // Check if too close to existing structures
    for (const FArch_StructureData& ExistingStructure : ManagedStructures)
    {
        if (FVector::Dist(ExistingStructure.Location, Location) < 1000.0f)
        {
            return false;
        }
    }

    // Check height constraints
    if (Location.Z < -1000.0f || Location.Z > 5000.0f)
    {
        return false;
    }

    return true;
}

void UArch_StructureManager::InitializeStructureDatabase()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Architecture Structure Database"));
    
    // Clear existing structures
    ManagedStructures.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture database initialized"));
}

void UArch_StructureManager::SpawnStructureActor(const FArch_StructureData& StructureData, EArch_StructureType StructureType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FString MeshPath = GetStructureMeshPath(StructureType, StructureData.BiomeType);
    
    // Try to spawn a basic structure actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*StructureData.StructureName);
    
    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        StructureData.Location,
        StructureData.Rotation,
        SpawnParams
    );

    if (StructureActor)
    {
        StructureActor->SetActorLabel(StructureData.StructureName);
        
        // Set tags for identification
        StructureActor->Tags.Add(TEXT("Architecture"));
        StructureActor->Tags.Add(FName(*UEnum::GetValueAsString(StructureType)));
        StructureActor->Tags.Add(FName(*UEnum::GetValueAsString(StructureData.BiomeType)));
        
        UE_LOG(LogTemp, Log, TEXT("Spawned structure actor: %s"), *StructureData.StructureName);
    }
}

FString UArch_StructureManager::GetStructureMeshPath(EArch_StructureType StructureType, EBiomeType BiomeType) const
{
    // Return appropriate mesh paths based on structure type and biome
    switch (StructureType)
    {
        case EArch_StructureType::Shelter:
            return TEXT("/Game/Architecture/Shelters/");
        case EArch_StructureType::Ruin:
            return TEXT("/Game/Architecture/Ruins/");
        case EArch_StructureType::Wall:
            return TEXT("/Game/Architecture/Walls/");
        case EArch_StructureType::Platform:
            return TEXT("/Game/Architecture/Platforms/");
        case EArch_StructureType::Bridge:
            return TEXT("/Game/Architecture/Bridges/");
        default:
            return TEXT("/Game/Architecture/Default/");
    }
}