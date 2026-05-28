#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

AArchitecturalStructureManager::AArchitecturalStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);

    // Initialize default values
    SpawnRadius = 1000.0f;
    MaxStructuresPerBiome = 20;
    AssociatedBiome = EBiomeType::Savana;

    // Initialize structure data with defaults
    StructureInfo.StructureType = EArch_StructureType::Shelter;
    StructureInfo.Material = EArch_ConstructionMaterial::Stone;
    StructureInfo.StructuralIntegrity = 100.0f;
    StructureInfo.WeatheringLevel = 0.0f;
    StructureInfo.EstimatedAge = 0;
    StructureInfo.bIsHabitable = true;
    StructureInfo.bHasInterior = true;
}

void AArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    ValidateStructureData();
    UpdateMaterialBasedOnWeathering();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager initialized: %s"), 
           *GetStructureDescription());
}

void AArchitecturalStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradually apply weathering over time
    if (StructureInfo.WeatheringLevel < 100.0f)
    {
        float WeatheringRate = 0.001f; // Very slow weathering
        StructureInfo.WeatheringLevel = FMath::Clamp(
            StructureInfo.WeatheringLevel + (WeatheringRate * DeltaTime), 
            0.0f, 100.0f
        );

        // Update material every 10 seconds
        static float MaterialUpdateTimer = 0.0f;
        MaterialUpdateTimer += DeltaTime;
        if (MaterialUpdateTimer >= 10.0f)
        {
            UpdateMaterialBasedOnWeathering();
            MaterialUpdateTimer = 0.0f;
        }
    }
}

void AArchitecturalStructureManager::InitializeStructure(EArch_StructureType Type, EBiomeType Biome)
{
    StructureInfo.StructureType = Type;
    AssociatedBiome = Biome;

    // Set structure properties based on type
    switch (Type)
    {
        case EArch_StructureType::Shelter:
            StructureInfo.bIsHabitable = true;
            StructureInfo.bHasInterior = true;
            break;
        case EArch_StructureType::Watchtower:
            StructureInfo.bIsHabitable = false;
            StructureInfo.bHasInterior = true;
            break;
        case EArch_StructureType::StoneCircle:
            StructureInfo.bIsHabitable = false;
            StructureInfo.bHasInterior = false;
            break;
        case EArch_StructureType::Ruins:
            StructureInfo.bIsHabitable = false;
            StructureInfo.bHasInterior = false;
            StructureInfo.WeatheringLevel = 75.0f;
            StructureInfo.StructuralIntegrity = 25.0f;
            break;
        case EArch_StructureType::Bridge:
            StructureInfo.bIsHabitable = false;
            StructureInfo.bHasInterior = false;
            break;
        case EArch_StructureType::Wall:
            StructureInfo.bIsHabitable = false;
            StructureInfo.bHasInterior = false;
            break;
    }

    // Select appropriate mesh for biome
    UStaticMesh* BiomeMesh = SelectMeshForBiome(Biome);
    if (BiomeMesh)
    {
        SetStructureMesh(BiomeMesh);
    }

    ValidateStructureData();
    UpdateMaterialBasedOnWeathering();

    UE_LOG(LogTemp, Log, TEXT("Structure initialized: %s in %s biome"), 
           *GetStructureDescription(), 
           *UEnum::GetValueAsString(AssociatedBiome));
}

void AArchitecturalStructureManager::SetStructureMesh(UStaticMesh* NewMesh)
{
    if (NewMesh && MainStructureMesh)
    {
        MainStructureMesh->SetStaticMesh(NewMesh);
        UE_LOG(LogTemp, Log, TEXT("Structure mesh updated: %s"), *NewMesh->GetName());
    }
}

void AArchitecturalStructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureInfo.WeatheringLevel = FMath::Clamp(
        StructureInfo.WeatheringLevel + WeatheringAmount, 
        0.0f, 100.0f
    );

    // Weathering affects structural integrity
    float IntegrityLoss = WeatheringAmount * 0.5f;
    UpdateStructuralIntegrity(-IntegrityLoss);

    UpdateMaterialBasedOnWeathering();

    UE_LOG(LogTemp, Log, TEXT("Weathering applied: %.1f%%, Integrity: %.1f%%"), 
           StructureInfo.WeatheringLevel, StructureInfo.StructuralIntegrity);
}

void AArchitecturalStructureManager::UpdateStructuralIntegrity(float DamageAmount)
{
    StructureInfo.StructuralIntegrity = FMath::Clamp(
        StructureInfo.StructuralIntegrity - DamageAmount, 
        0.0f, 100.0f
    );

    // Update habitability based on integrity
    if (StructureInfo.StructuralIntegrity < 30.0f)
    {
        StructureInfo.bIsHabitable = false;
    }

    UE_LOG(LogTemp, Log, TEXT("Structural integrity updated: %.1f%%"), 
           StructureInfo.StructuralIntegrity);
}

bool AArchitecturalStructureManager::CanPlayerEnter() const
{
    return StructureInfo.bHasInterior && StructureInfo.StructuralIntegrity > 20.0f;
}

FVector AArchitecturalStructureManager::GetInteriorSpawnPoint() const
{
    if (CanPlayerEnter())
    {
        FVector BaseLocation = GetActorLocation();
        return BaseLocation + FVector(0.0f, 0.0f, 100.0f); // Slightly elevated
    }
    return FVector::ZeroVector;
}

void AArchitecturalStructureManager::SpawnStructuresInBiome()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn structures: World is null"));
        return;
    }

    // Define biome coordinates
    TMap<EBiomeType, FVector> BiomeCoordinates;
    BiomeCoordinates.Add(EBiomeType::Savana, FVector(0.0f, 0.0f, 100.0f));
    BiomeCoordinates.Add(EBiomeType::Pantano, FVector(-50000.0f, -45000.0f, 100.0f));
    BiomeCoordinates.Add(EBiomeType::Floresta, FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeCoordinates.Add(EBiomeType::Deserto, FVector(55000.0f, 0.0f, 100.0f));
    BiomeCoordinates.Add(EBiomeType::Montanha, FVector(40000.0f, 50000.0f, 200.0f));

    FVector* BiomeCenter = BiomeCoordinates.Find(AssociatedBiome);
    if (!BiomeCenter)
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown biome type for spawning"));
        return;
    }

    int32 SpawnedCount = 0;
    for (int32 i = 0; i < MaxStructuresPerBiome; ++i)
    {
        // Generate random position within spawn radius
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomDistance = FMath::RandRange(100.0f, SpawnRadius);
        
        FVector SpawnLocation = *BiomeCenter + FVector(
            FMath::Cos(RandomAngle) * RandomDistance,
            FMath::Sin(RandomAngle) * RandomDistance,
            0.0f
        );

        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

        // Spawn new structure
        AArchitecturalStructureManager* NewStructure = World->SpawnActor<AArchitecturalStructureManager>(
            GetClass(), SpawnLocation, SpawnRotation
        );

        if (NewStructure)
        {
            // Randomize structure type
            EArch_StructureType RandomType = static_cast<EArch_StructureType>(
                FMath::RandRange(0, static_cast<int32>(EArch_StructureType::Wall))
            );
            
            NewStructure->InitializeStructure(RandomType, AssociatedBiome);
            NewStructure->SetActorLabel(FString::Printf(TEXT("Structure_%s_%d"), 
                *UEnum::GetValueAsString(AssociatedBiome), i));
            
            SpawnedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d structures in %s biome"), 
           SpawnedCount, *UEnum::GetValueAsString(AssociatedBiome));
}

void AArchitecturalStructureManager::ClearAllStructures()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AArchitecturalStructureManager::StaticClass(), FoundActors);

    int32 DestroyedCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != this)
        {
            Actor->Destroy();
            DestroyedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Cleared %d architectural structures"), DestroyedCount);
}

FString AArchitecturalStructureManager::GetStructureDescription() const
{
    FString TypeString = UEnum::GetValueAsString(StructureInfo.StructureType);
    FString MaterialString = UEnum::GetValueAsString(StructureInfo.Material);
    
    return FString::Printf(TEXT("%s (%s) - Integrity: %.1f%%, Weathering: %.1f%%"),
        *TypeString, *MaterialString, 
        StructureInfo.StructuralIntegrity, StructureInfo.WeatheringLevel);
}

float AArchitecturalStructureManager::GetStructureAge() const
{
    return static_cast<float>(StructureInfo.EstimatedAge);
}

void AArchitecturalStructureManager::UpdateMaterialBasedOnWeathering()
{
    if (!MainStructureMesh || WeatheringMaterials.Num() == 0)
    {
        return;
    }

    // Select material based on weathering level
    UMaterialInterface* SelectedMaterial = SelectMaterialForWeathering(StructureInfo.WeatheringLevel);
    if (SelectedMaterial)
    {
        MainStructureMesh->SetMaterial(0, SelectedMaterial);
    }
}

void AArchitecturalStructureManager::ValidateStructureData()
{
    // Ensure weathering doesn't exceed 100%
    StructureInfo.WeatheringLevel = FMath::Clamp(StructureInfo.WeatheringLevel, 0.0f, 100.0f);
    
    // Ensure structural integrity doesn't exceed 100%
    StructureInfo.StructuralIntegrity = FMath::Clamp(StructureInfo.StructuralIntegrity, 0.0f, 100.0f);
    
    // Ensure age is non-negative
    StructureInfo.EstimatedAge = FMath::Max(StructureInfo.EstimatedAge, 0);
    
    // Update habitability based on integrity
    if (StructureInfo.StructuralIntegrity < 30.0f)
    {
        StructureInfo.bIsHabitable = false;
    }
}

UStaticMesh* AArchitecturalStructureManager::SelectMeshForBiome(EBiomeType Biome) const
{
    // Return first available mesh variant
    // In a full implementation, this would select based on biome
    if (StructureMeshVariants.Num() > 0)
    {
        return StructureMeshVariants[0];
    }
    return nullptr;
}

UMaterialInterface* AArchitecturalStructureManager::SelectMaterialForWeathering(float WeatheringLevel) const
{
    if (WeatheringMaterials.Num() == 0)
    {
        return nullptr;
    }

    // Select material based on weathering level
    int32 MaterialIndex = 0;
    if (WeatheringLevel > 75.0f && WeatheringMaterials.Num() > 2)
    {
        MaterialIndex = 2; // Heavily weathered
    }
    else if (WeatheringLevel > 25.0f && WeatheringMaterials.Num() > 1)
    {
        MaterialIndex = 1; // Moderately weathered
    }
    // else use index 0 (pristine)

    return WeatheringMaterials[MaterialIndex];
}