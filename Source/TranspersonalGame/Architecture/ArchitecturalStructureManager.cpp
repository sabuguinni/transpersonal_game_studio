#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

UArchitecturalStructureManager::UArchitecturalStructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    MaxSpawnDistance = 100000.0f;
    MaxStructuresPerBiome = 20;
    bAutoSpawnStructures = true;
}

void UArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoSpawnStructures)
    {
        InitializeBiomeStructures();
    }
}

void UArchitecturalStructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structural integrity and manage structure lifecycle
    for (int32 i = StructureRegistry.Num() - 1; i >= 0; --i)
    {
        FArch_StructureData& Structure = StructureRegistry[i];
        
        // Simulate weathering over time
        Structure.StructuralIntegrity -= DeltaTime * 0.01f; // Very slow decay
        
        if (Structure.StructuralIntegrity <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Structure collapsed due to weathering"));
            StructureRegistry.RemoveAt(i);
        }
    }
}

bool UArchitecturalStructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation, FString BiomeName)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitecturalStructureManager: No valid world"));
        return false;
    }
    
    // Check if we're within spawn distance limits
    if (FVector::Dist(Location, FVector::ZeroVector) > MaxSpawnDistance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Structure spawn location too far from origin"));
        return false;
    }
    
    // Check biome structure limit
    TArray<FArch_StructureData> BiomeStructures = GetStructuresInBiome(BiomeName);
    if (BiomeStructures.Num() >= MaxStructuresPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum structures reached for biome: %s"), *BiomeName);
        return false;
    }
    
    // Create the structure actor
    AActor* StructureActor = CreateStructureActor(StructureType, Location, Rotation);
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create structure actor"));
        return false;
    }
    
    // Register the structure
    FArch_StructureData NewStructure;
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.Rotation = Rotation;
    NewStructure.BiomeName = BiomeName;
    NewStructure.bIsInteractable = true;
    NewStructure.StructuralIntegrity = 100.0f;
    
    RegisterStructure(NewStructure);
    SpawnedStructureActors.Add(StructureActor);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned structure %s at %s in biome %s"), 
           *UEnum::GetValueAsString(StructureType), *Location.ToString(), *BiomeName);
    
    return true;
}

void UArchitecturalStructureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    StructureRegistry.Add(StructureData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered structure in biome: %s. Total structures: %d"), 
           *StructureData.BiomeName, StructureRegistry.Num());
}

TArray<FArch_StructureData> UArchitecturalStructureManager::GetStructuresInBiome(const FString& BiomeName) const
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : StructureRegistry)
    {
        if (Structure.BiomeName == BiomeName)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void UArchitecturalStructureManager::ClearStructuresInBiome(const FString& BiomeName)
{
    for (int32 i = StructureRegistry.Num() - 1; i >= 0; --i)
    {
        if (StructureRegistry[i].BiomeName == BiomeName)
        {
            StructureRegistry.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleared structures in biome: %s"), *BiomeName);
}

void UArchitecturalStructureManager::SpawnStructuresForAllBiomes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world for structure spawning"));
        return;
    }
    
    // Define biome locations based on world coordinates
    TArray<TPair<FString, FVector>> BiomeLocations = {
        {TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f)},
        {TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 100.0f)},
        {TEXT("Deserto"), FVector(55000.0f, 0.0f, 100.0f)},
        {TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 100.0f)},
        {TEXT("Montanha"), FVector(40000.0f, 50000.0f, 200.0f)}
    };
    
    for (const auto& BiomePair : BiomeLocations)
    {
        FString BiomeName = BiomePair.Key;
        FVector BaseLocation = BiomePair.Value;
        
        // Spawn 3-5 structures per biome
        int32 StructuresToSpawn = FMath::RandRange(3, 5);
        
        for (int32 i = 0; i < StructuresToSpawn; ++i)
        {
            // Random offset within biome area
            FVector RandomOffset = FVector(
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(-50.0f, 100.0f)
            );
            
            FVector SpawnLocation = BaseLocation + RandomOffset;
            FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            
            // Choose random structure type
            EArch_StructureType StructureType = static_cast<EArch_StructureType>(
                FMath::RandRange(0, static_cast<int32>(EArch_StructureType::NaturalArch))
            );
            
            SpawnStructureAtLocation(StructureType, SpawnLocation, RandomRotation, BiomeName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned structures for all biomes. Total: %d"), GetTotalStructureCount());
}

int32 UArchitecturalStructureManager::GetTotalStructureCount() const
{
    return StructureRegistry.Num();
}

void UArchitecturalStructureManager::InitializeBiomeStructures()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing biome structures..."));
    
    // Spawn initial structures for each biome
    SpawnStructuresForAllBiomes();
}

AActor* UArchitecturalStructureManager::CreateStructureActor(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Create a static mesh actor
    AStaticMeshActor* StructureActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!StructureActor)
    {
        return nullptr;
    }
    
    // Get the mesh path for this structure type
    FString MeshPath = GetStructureMeshPath(StructureType);
    
    // Try to load the mesh
    UStaticMesh* StructureMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    if (StructureMesh && StructureActor->GetStaticMeshComponent())
    {
        StructureActor->GetStaticMeshComponent()->SetStaticMesh(StructureMesh);
    }
    else
    {
        // Fallback to basic cube if specific mesh not found
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh && StructureActor->GetStaticMeshComponent())
        {
            StructureActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            // Scale it appropriately for the structure type
            FVector StructureScale = FVector(2.0f, 2.0f, 4.0f); // Pillar-like proportions
            StructureActor->SetActorScale3D(StructureScale);
        }
    }
    
    // Set actor label
    FString StructureTypeName = UEnum::GetValueAsString(StructureType);
    StructureActor->SetActorLabel(FString::Printf(TEXT("Structure_%s_%d"), *StructureTypeName, FMath::Rand()));
    
    return StructureActor;
}

FString UArchitecturalStructureManager::GetStructureMeshPath(EArch_StructureType StructureType) const
{
    switch (StructureType)
    {
        case EArch_StructureType::StonePillar:
            return TEXT("/Game/LandscapePackOne/Meshes/SM_Rock_01");
        case EArch_StructureType::RockFormation:
            return TEXT("/Game/LandscapePackTwo/Meshes/SM_Rock_Formation");
        case EArch_StructureType::CaveEntrance:
            return TEXT("/Game/ANGRY_MESH/Meshes/SM_Cave_Entrance");
        case EArch_StructureType::AncientRuin:
            return TEXT("/Game/LandscapePackOne/Meshes/SM_Stone_Wall");
        case EArch_StructureType::NaturalArch:
            return TEXT("/Game/LandscapePackTwo/Meshes/SM_Natural_Arch");
        default:
            return TEXT("/Engine/BasicShapes/Cube");
    }
}