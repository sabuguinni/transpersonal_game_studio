#include "PrehistoricStructureManager.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

UPrehistoricStructureManager::UPrehistoricStructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Tick every 5 seconds for weathering

    StructureSpawnRadius = 10000.0f;
    MaxStructuresPerBiome = 15;
    bAutoGenerateStructures = true;
    WeatheringRate = 0.001f;
    MinWeatheringLevel = 0.0f;
    MaxWeatheringLevel = 1.0f;
}

void UPrehistoricStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStructureTypes();
    
    if (bAutoGenerateStructures)
    {
        // Generate structures for all biomes
        TArray<EBiomeType> BiomeTypes = {
            EBiomeType::Savanna,
            EBiomeType::Forest,
            EBiomeType::Desert,
            EBiomeType::Swamp,
            EBiomeType::Mountain
        };
        
        TArray<FVector> BiomeCenters = {
            FVector(0, 0, 100),           // Savanna
            FVector(-45000, 40000, 150),  // Forest
            FVector(55000, 0, 120),       // Desert
            FVector(-50000, -45000, 80),  // Swamp
            FVector(40000, 50000, 200)    // Mountain
        };
        
        for (int32 i = 0; i < BiomeTypes.Num(); i++)
        {
            GenerateStructuresForBiome(BiomeTypes[i], BiomeCenters[i], StructureSpawnRadius);
        }
    }
}

void UPrehistoricStructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (WeatheringRate > 0.0f)
    {
        ApplyWeathering(DeltaTime);
    }
}

void UPrehistoricStructureManager::GenerateStructuresForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius)
{
    int32 StructuresToGenerate = FMath::RandRange(5, MaxStructuresPerBiome);
    
    for (int32 i = 0; i < StructuresToGenerate; i++)
    {
        FArch_StructureData NewStructure;
        NewStructure.BiomeType = BiomeType;
        NewStructure.Location = GetRandomLocationInBiome(BiomeCenter, BiomeRadius);
        NewStructure.Rotation = FRotator(0, FMath::RandRange(0, 360), 0);
        NewStructure.StructureType = GetRandomStructureTypeForBiome(BiomeType);
        NewStructure.WeatheringLevel = FMath::RandRange(0.1f, 0.8f);
        
        // Set habitability based on structure type
        switch (NewStructure.StructureType)
        {
            case EArch_StructureType::StoneDwelling:
            case EArch_StructureType::RockShelter:
                NewStructure.bIsHabitable = true;
                NewStructure.MaxOccupants = FMath::RandRange(2, 6);
                break;
            case EArch_StructureType::StoneCircle:
            case EArch_StructureType::TribalPlatform:
                NewStructure.bIsHabitable = false;
                NewStructure.MaxOccupants = 0;
                break;
            default:
                NewStructure.bIsHabitable = FMath::RandBool();
                NewStructure.MaxOccupants = NewStructure.bIsHabitable ? FMath::RandRange(1, 4) : 0;
                break;
        }
        
        if (IsLocationSuitableForStructure(NewStructure.Location, NewStructure.StructureType))
        {
            ManagedStructures.Add(NewStructure);
            SpawnStructure(NewStructure);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d structures for biome %d at center %s"), 
           StructuresToGenerate, (int32)BiomeType, *BiomeCenter.ToString());
}

AStaticMeshActor* UPrehistoricStructureManager::SpawnStructure(const FArch_StructureData& StructureData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UStaticMesh* StructureMesh = GetMeshForStructureType(StructureData.StructureType);
    if (!StructureMesh)
    {
        // Fallback to basic cube if no specific mesh found
        StructureMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    }
    
    if (StructureMesh)
    {
        AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            StructureData.Location,
            StructureData.Rotation
        );
        
        if (StructureActor)
        {
            StructureActor->GetStaticMeshComponent()->SetStaticMesh(StructureMesh);
            
            // Apply biome-specific materials and weathering
            ApplyBiomeSpecificMaterials(StructureActor, StructureData.BiomeType);
            
            // Set actor label for identification
            FString StructureLabel = FString::Printf(TEXT("Structure_%s_%s"), 
                                                   *UEnum::GetValueAsString(StructureData.StructureType),
                                                   *UEnum::GetValueAsString(StructureData.BiomeType));
            StructureActor->SetActorLabel(StructureLabel);
            
            return StructureActor;
        }
    }
    
    return nullptr;
}

void UPrehistoricStructureManager::RemoveStructure(int32 StructureIndex)
{
    if (ManagedStructures.IsValidIndex(StructureIndex))
    {
        ManagedStructures.RemoveAt(StructureIndex);
    }
}

TArray<FArch_StructureData> UPrehistoricStructureManager::GetStructuresInRadius(const FVector& Center, float Radius) const
{
    TArray<FArch_StructureData> StructuresInRadius;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        float Distance = FVector::Dist(Structure.Location, Center);
        if (Distance <= Radius)
        {
            StructuresInRadius.Add(Structure);
        }
    }
    
    return StructuresInRadius;
}

FArch_StructureData UPrehistoricStructureManager::GetNearestStructure(const FVector& Location) const
{
    FArch_StructureData NearestStructure;
    float MinDistance = MAX_FLT;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        float Distance = FVector::Dist(Structure.Location, Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestStructure = Structure;
        }
    }
    
    return NearestStructure;
}

void UPrehistoricStructureManager::ApplyWeathering(float DeltaTime)
{
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        Structure.WeatheringLevel += WeatheringRate * DeltaTime;
        Structure.WeatheringLevel = FMath::Clamp(Structure.WeatheringLevel, MinWeatheringLevel, MaxWeatheringLevel);
    }
}

void UPrehistoricStructureManager::SetStructureWeathering(int32 StructureIndex, float WeatheringLevel)
{
    if (ManagedStructures.IsValidIndex(StructureIndex))
    {
        ManagedStructures[StructureIndex].WeatheringLevel = FMath::Clamp(WeatheringLevel, MinWeatheringLevel, MaxWeatheringLevel);
    }
}

bool UPrehistoricStructureManager::IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const
{
    // Check minimum distance from other structures
    const float MinDistance = 1000.0f;
    
    for (const FArch_StructureData& ExistingStructure : ManagedStructures)
    {
        if (FVector::Dist(ExistingStructure.Location, Location) < MinDistance)
        {
            return false;
        }
    }
    
    // Additional checks based on structure type
    switch (StructureType)
    {
        case EArch_StructureType::CaveEntrance:
            // Cave entrances should be near elevated terrain
            return Location.Z > 150.0f;
        case EArch_StructureType::StoneCircle:
            // Stone circles prefer flat areas
            return Location.Z < 200.0f;
        default:
            return true;
    }
}

void UPrehistoricStructureManager::ClearAllStructures()
{
    ManagedStructures.Empty();
}

void UPrehistoricStructureManager::RegenerateAllStructures()
{
    ClearAllStructures();
    
    if (bAutoGenerateStructures)
    {
        TArray<EBiomeType> BiomeTypes = {
            EBiomeType::Savanna,
            EBiomeType::Forest,
            EBiomeType::Desert,
            EBiomeType::Swamp,
            EBiomeType::Mountain
        };
        
        TArray<FVector> BiomeCenters = {
            FVector(0, 0, 100),
            FVector(-45000, 40000, 150),
            FVector(55000, 0, 120),
            FVector(-50000, -45000, 80),
            FVector(40000, 50000, 200)
        };
        
        for (int32 i = 0; i < BiomeTypes.Num(); i++)
        {
            GenerateStructuresForBiome(BiomeTypes[i], BiomeCenters[i], StructureSpawnRadius);
        }
    }
}

void UPrehistoricStructureManager::InitializeStructureTypes()
{
    // Initialize any structure type specific data
    UE_LOG(LogTemp, Warning, TEXT("PrehistoricStructureManager initialized"));
}

FVector UPrehistoricStructureManager::GetRandomLocationInBiome(const FVector& BiomeCenter, float BiomeRadius) const
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-BiomeRadius, BiomeRadius),
        FMath::RandRange(-BiomeRadius, BiomeRadius),
        FMath::RandRange(-100.0f, 100.0f)
    );
    
    return BiomeCenter + RandomOffset;
}

EArch_StructureType UPrehistoricStructureManager::GetRandomStructureTypeForBiome(EBiomeType BiomeType) const
{
    TArray<EArch_StructureType> BiomeStructures;
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            BiomeStructures = {
                EArch_StructureType::StoneDwelling,
                EArch_StructureType::StoneCircle,
                EArch_StructureType::TribalPlatform
            };
            break;
        case EBiomeType::Forest:
            BiomeStructures = {
                EArch_StructureType::RockShelter,
                EArch_StructureType::StoneDwelling,
                EArch_StructureType::AncientRuin
            };
            break;
        case EBiomeType::Mountain:
            BiomeStructures = {
                EArch_StructureType::CaveEntrance,
                EArch_StructureType::StonePillar,
                EArch_StructureType::AncientRuin
            };
            break;
        case EBiomeType::Desert:
            BiomeStructures = {
                EArch_StructureType::AncientRuin,
                EArch_StructureType::StonePillar,
                EArch_StructureType::StoneCircle
            };
            break;
        case EBiomeType::Swamp:
            BiomeStructures = {
                EArch_StructureType::RockShelter,
                EArch_StructureType::TribalPlatform,
                EArch_StructureType::StoneDwelling
            };
            break;
        default:
            BiomeStructures = {EArch_StructureType::StoneDwelling};
            break;
    }
    
    if (BiomeStructures.Num() > 0)
    {
        return BiomeStructures[FMath::RandRange(0, BiomeStructures.Num() - 1)];
    }
    
    return EArch_StructureType::StoneDwelling;
}

UStaticMesh* UPrehistoricStructureManager::GetMeshForStructureType(EArch_StructureType StructureType) const
{
    // Try to load structure-specific meshes, fallback to basic shapes
    switch (StructureType)
    {
        case EArch_StructureType::StonePillar:
            return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
        case EArch_StructureType::StoneCircle:
            return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
        case EArch_StructureType::TribalPlatform:
            return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        default:
            return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    }
}

void UPrehistoricStructureManager::ApplyBiomeSpecificMaterials(AStaticMeshActor* StructureActor, EBiomeType BiomeType) const
{
    if (!StructureActor || !StructureActor->GetStaticMeshComponent())
    {
        return;
    }
    
    // Apply basic material based on biome
    UMaterialInterface* Material = nullptr;
    
    switch (BiomeType)
    {
        case EBiomeType::Desert:
            Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EBiomeType::Forest:
            Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EBiomeType::Mountain:
            Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        default:
            Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
    }
    
    if (Material)
    {
        StructureActor->GetStaticMeshComponent()->SetMaterial(0, Material);
    }
}