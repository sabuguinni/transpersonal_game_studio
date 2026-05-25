#include "ArchitecturalStructureManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArchitecturalStructureManager::AArchitecturalStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxStructuresPerBiome = 50;
    MinDistanceBetweenStructures = 1000.0f;

    // Initialize structure count map
    StructureCountPerBiome.Add(EBiomeType::Savana, 0);
    StructureCountPerBiome.Add(EBiomeType::Floresta, 0);
    StructureCountPerBiome.Add(EBiomeType::Pantano, 0);
    StructureCountPerBiome.Add(EBiomeType::Deserto, 0);
    StructureCountPerBiome.Add(EBiomeType::Montanha, 0);
}

void AArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: BeginPlay - Initializing architectural systems"));
}

void AArchitecturalStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArchitecturalStructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitecturalStructureManager: No valid world found"));
        return;
    }

    if (StructureType == EArch_StructureType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Cannot spawn structure of type None"));
        return;
    }

    // Check if location is suitable
    if (!IsLocationSuitableForStructure(Location, StructureType))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Location not suitable for structure"));
        return;
    }

    // Create structure actor
    AActor* StructureActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitecturalStructureManager: Failed to spawn structure actor"));
        return;
    }

    // Add static mesh component
    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(StructureActor);
    StructureActor->SetRootComponent(MeshComponent);

    // Get appropriate mesh for structure type
    UStaticMesh* StructureMesh = GetMeshForStructureType(StructureType);
    if (StructureMesh)
    {
        MeshComponent->SetStaticMesh(StructureMesh);
    }

    // Create structure data
    FArch_StructureData StructureData;
    StructureData.StructureType = StructureType;
    StructureData.Location = Location;
    StructureData.Rotation = Rotation;
    StructureData.WearLevel = FMath::RandRange(0.3f, 0.8f); // Random weathering
    StructureData.bHasMoss = FMath::RandBool(); // Random moss coverage

    // Configure material based on structure data
    ConfigureStructureMaterial(MeshComponent, StructureData);

    // Set actor label
    FString StructureTypeName = UEnum::GetValueAsString(StructureType);
    StructureTypeName = StructureTypeName.Replace(TEXT("EArch_StructureType::"), TEXT(""));
    StructureActor->SetActorLabel(FString::Printf(TEXT("Structure_%s_%d"), *StructureTypeName, StructureDatabase.Num()));

    // Add to database
    StructureDatabase.Add(StructureData);

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Spawned %s at %s"), *StructureTypeName, *Location.ToString());
}

void AArchitecturalStructureManager::PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount)
{
    if (StructureCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Invalid structure count"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Populating biome %s with %d structures"), 
           *UEnum::GetValueAsString(BiomeType), StructureCount);

    for (int32 i = 0; i < StructureCount; i++)
    {
        // Get random location in biome
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        
        // Get random structure type
        EArch_StructureType RandomStructureType = static_cast<EArch_StructureType>(FMath::RandRange(1, 5));
        
        // Random rotation
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Spawn structure
        SpawnStructureAtLocation(RandomStructureType, SpawnLocation, SpawnRotation);
    }

    // Update structure count for biome
    if (StructureCountPerBiome.Contains(BiomeType))
    {
        StructureCountPerBiome[BiomeType] += StructureCount;
    }
}

TArray<AActor*> AArchitecturalStructureManager::GetStructuresInRadius(const FVector& Center, float Radius)
{
    TArray<AActor*> FoundStructures;
    
    if (!GetWorld())
    {
        return FoundStructures;
    }

    // Get all actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    // Filter structures within radius
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Structure_")))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                FoundStructures.Add(Actor);
            }
        }
    }

    return FoundStructures;
}

void AArchitecturalStructureManager::ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringLevel)
{
    if (!StructureActor)
    {
        return;
    }

    UStaticMeshComponent* MeshComponent = StructureActor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComponent)
    {
        return;
    }

    // Apply weathering effects (placeholder implementation)
    // In a full implementation, this would modify material parameters
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Applied weathering level %.2f to %s"), 
           WeatheringLevel, *StructureActor->GetActorLabel());
}

void AArchitecturalStructureManager::GenerateArchitecturalLayout()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Generating architectural layout for all biomes"));

    // Clear existing structures
    ClearAllStructures();

    // Populate each biome with structures
    PopulateBiomeWithStructures(EBiomeType::Savana, 15);
    PopulateBiomeWithStructures(EBiomeType::Floresta, 20);
    PopulateBiomeWithStructures(EBiomeType::Pantano, 10);
    PopulateBiomeWithStructures(EBiomeType::Deserto, 12);
    PopulateBiomeWithStructures(EBiomeType::Montanha, 18);

    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Generated %d total structures"), StructureDatabase.Num());
}

void AArchitecturalStructureManager::ClearAllStructures()
{
    if (!GetWorld())
    {
        return;
    }

    // Find and destroy all structure actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    int32 ClearedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Structure_")))
        {
            Actor->Destroy();
            ClearedCount++;
        }
    }

    // Clear database
    StructureDatabase.Empty();
    
    // Reset counts
    for (auto& Pair : StructureCountPerBiome)
    {
        Pair.Value = 0;
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager: Cleared %d structures"), ClearedCount);
}

FVector AArchitecturalStructureManager::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BaseLocation;
    float SpreadRadius = 10000.0f; // 10km spread within biome

    // Get biome center coordinates from SharedTypes
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BaseLocation = FVector(0.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Floresta:
            BaseLocation = FVector(-45000.0f, 40000.0f, 100.0f);
            break;
        case EBiomeType::Pantano:
            BaseLocation = FVector(-50000.0f, -45000.0f, 100.0f);
            break;
        case EBiomeType::Deserto:
            BaseLocation = FVector(55000.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Montanha:
            BaseLocation = FVector(40000.0f, 50000.0f, 200.0f); // Higher altitude for mountains
            break;
        default:
            BaseLocation = FVector::ZeroVector;
            break;
    }

    // Add random offset within biome
    float RandomX = FMath::RandRange(-SpreadRadius, SpreadRadius);
    float RandomY = FMath::RandRange(-SpreadRadius, SpreadRadius);
    float RandomZ = FMath::RandRange(-100.0f, 300.0f); // Vertical variation

    return BaseLocation + FVector(RandomX, RandomY, RandomZ);
}

bool AArchitecturalStructureManager::IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType)
{
    // Check minimum distance from other structures
    TArray<AActor*> NearbyStructures = GetStructuresInRadius(Location, MinDistanceBetweenStructures);
    if (NearbyStructures.Num() > 0)
    {
        return false;
    }

    // Additional checks could include terrain slope, water proximity, etc.
    return true;
}

UStaticMesh* AArchitecturalStructureManager::GetMeshForStructureType(EArch_StructureType StructureType)
{
    // In a full implementation, this would load appropriate meshes for each structure type
    // For now, return nullptr and let the system use default cube
    return nullptr;
}

void AArchitecturalStructureManager::ConfigureStructureMaterial(UStaticMeshComponent* MeshComponent, const FArch_StructureData& StructureData)
{
    if (!MeshComponent)
    {
        return;
    }

    // In a full implementation, this would apply materials based on:
    // - StructureData.WearLevel (weathering)
    // - StructureData.bHasMoss (moss coverage)
    // - Structure type (stone, wood, etc.)
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager: Configured material for structure with wear level %.2f"), 
           StructureData.WearLevel);
}