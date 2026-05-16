#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

AArchitecturalStructure::AArchitecturalStructure()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(200.0f, 200.0f, 300.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

    // Initialize default values
    StructureType = EArch_StructureType::Pillar;
    StructureData.StructureName = TEXT("Ancient Pillar");
    StructureData.BiomeType = EBiomeType::Savanna;
    StructureData.AgeYears = 1000.0f;
    StructureData.WeatheringLevel = 0.5f;
}

void AArchitecturalStructure::BeginPlay()
{
    Super::BeginPlay();

    // Update structure data with current transform
    StructureData.Location = GetActorLocation();
    StructureData.Rotation = GetActorRotation();

    // Apply initial weathering based on age
    ApplyWeathering(StructureData.WeatheringLevel);
}

void AArchitecturalStructure::SetStructureType(EArch_StructureType NewType)
{
    StructureType = NewType;

    // Update structure name based on type
    switch (StructureType)
    {
    case EArch_StructureType::Pillar:
        StructureData.StructureName = TEXT("Stone Pillar");
        break;
    case EArch_StructureType::Archway:
        StructureData.StructureName = TEXT("Ancient Archway");
        break;
    case EArch_StructureType::Platform:
        StructureData.StructureName = TEXT("Stone Platform");
        break;
    case EArch_StructureType::Ruins:
        StructureData.StructureName = TEXT("Ancient Ruins");
        break;
    case EArch_StructureType::Shelter:
        StructureData.StructureName = TEXT("Cave Shelter");
        break;
    case EArch_StructureType::Monument:
        StructureData.StructureName = TEXT("Stone Monument");
        break;
    }
}

void AArchitecturalStructure::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);

    // Apply weathered materials based on weathering level
    if (StructureMesh && WeatheredStoneMaterial)
    {
        if (StructureData.WeatheringLevel > 0.7f && MossyMaterial)
        {
            StructureMesh->SetMaterial(0, MossyMaterial);
        }
        else
        {
            StructureMesh->SetMaterial(0, WeatheredStoneMaterial);
        }
    }
}

void AArchitecturalStructure::SetBiomeAdaptation(EBiomeType BiomeType)
{
    StructureData.BiomeType = BiomeType;

    // Adjust weathering based on biome conditions
    switch (BiomeType)
    {
    case EBiomeType::Swamp:
        StructureData.WeatheringLevel += 0.2f; // High humidity increases weathering
        break;
    case EBiomeType::Forest:
        StructureData.WeatheringLevel += 0.15f; // Moisture and organic growth
        break;
    case EBiomeType::Desert:
        StructureData.WeatheringLevel += 0.1f; // Sand erosion
        break;
    case EBiomeType::Mountain:
        StructureData.WeatheringLevel += 0.05f; // Cold and wind erosion
        break;
    case EBiomeType::Savanna:
    default:
        // Moderate weathering in savanna
        break;
    }

    ApplyWeathering(StructureData.WeatheringLevel);
}

UArchitecturalStructureManager::UArchitecturalStructureManager()
{
    // Initialize default structure class
    StructureClass = AArchitecturalStructure::StaticClass();
}

void UArchitecturalStructureManager::InitializeStructureManager()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager initialized"));
    ManagedStructures.Empty();
}

AArchitecturalStructure* UArchitecturalStructureManager::SpawnStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType)
{
    if (!StructureClass)
    {
        UE_LOG(LogTemp, Error, TEXT("StructureClass not set in ArchitecturalStructureManager"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for structure spawning"));
        return nullptr;
    }

    // Spawn the structure
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AArchitecturalStructure* NewStructure = World->SpawnActor<AArchitecturalStructure>(StructureClass, Location, Rotation, SpawnParams);
    
    if (NewStructure)
    {
        // Configure the structure
        NewStructure->SetStructureType(StructureType);
        NewStructure->SetBiomeAdaptation(BiomeType);
        
        // Set up biome-specific materials
        SetupBiomeSpecificMaterials(NewStructure, BiomeType);
        
        // Add to managed structures
        ManagedStructures.Add(NewStructure);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned architectural structure at location: %s"), *Location.ToString());
    }

    return NewStructure;
}

void UArchitecturalStructureManager::PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount)
{
    for (int32 i = 0; i < StructureCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Randomly select structure type
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 5));
        
        SpawnStructure(RandomType, SpawnLocation, SpawnRotation, BiomeType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Populated biome with %d architectural structures"), StructureCount);
}

TArray<AArchitecturalStructure*> UArchitecturalStructureManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<AArchitecturalStructure*> BiomeStructures;
    
    for (AArchitecturalStructure* Structure : ManagedStructures)
    {
        if (Structure && Structure->GetStructureData().BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void UArchitecturalStructureManager::ApplyGlobalWeathering(float WeatheringFactor)
{
    for (AArchitecturalStructure* Structure : ManagedStructures)
    {
        if (Structure)
        {
            float CurrentWeathering = Structure->GetStructureData().WeatheringLevel;
            Structure->ApplyWeathering(CurrentWeathering + WeatheringFactor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied global weathering factor: %f to %d structures"), WeatheringFactor, ManagedStructures.Num());
}

void UArchitecturalStructureManager::SetupBiomeSpecificMaterials(AArchitecturalStructure* Structure, EBiomeType BiomeType)
{
    if (!Structure)
    {
        return;
    }

    // Apply biome-specific aging and materials
    switch (BiomeType)
    {
    case EBiomeType::Swamp:
        Structure->ApplyWeathering(0.8f); // Heavy moss and algae growth
        break;
    case EBiomeType::Forest:
        Structure->ApplyWeathering(0.6f); // Moderate moss and lichen
        break;
    case EBiomeType::Desert:
        Structure->ApplyWeathering(0.3f); // Sand-blasted but preserved
        break;
    case EBiomeType::Mountain:
        Structure->ApplyWeathering(0.4f); // Wind and frost weathering
        break;
    case EBiomeType::Savanna:
    default:
        Structure->ApplyWeathering(0.5f); // Balanced weathering
        break;
    }
}

FVector UArchitecturalStructureManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BaseLocation;
    float SpreadRadius = 10000.0f; // 10km spread within biome
    
    // Get biome center coordinates
    switch (BiomeType)
    {
    case EBiomeType::Savanna:
        BaseLocation = FVector(0.0f, 0.0f, 100.0f);
        break;
    case EBiomeType::Swamp:
        BaseLocation = FVector(-50000.0f, -45000.0f, 100.0f);
        break;
    case EBiomeType::Forest:
        BaseLocation = FVector(-45000.0f, 40000.0f, 100.0f);
        break;
    case EBiomeType::Desert:
        BaseLocation = FVector(55000.0f, 0.0f, 100.0f);
        break;
    case EBiomeType::Mountain:
        BaseLocation = FVector(40000.0f, 50000.0f, 100.0f);
        break;
    default:
        BaseLocation = FVector::ZeroVector;
        break;
    }
    
    // Add random offset within biome
    float RandomX = FMath::RandRange(-SpreadRadius, SpreadRadius);
    float RandomY = FMath::RandRange(-SpreadRadius, SpreadRadius);
    float RandomZ = FMath::RandRange(50.0f, 200.0f); // Vary height slightly
    
    return BaseLocation + FVector(RandomX, RandomY, RandomZ);
}