#include "ArchitecturalManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

// AArch_StructureActor Implementation
AArch_StructureActor::AArch_StructureActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default values
    StructureType = EArch_StructureType::Dwelling;
    bCanBeEntered = true;
    
    // Set default structure data
    StructureData.StructureName = TEXT("Prehistoric Dwelling");
    StructureData.StructuralIntegrity = 100.0f;
    StructureData.bIsRuin = false;
    StructureData.BiomeType = EBiomeType::Savana;

    // Add default interior spawn points
    InteriorSpawnPoints.Add(FVector(0, 0, 0));
    InteriorSpawnPoints.Add(FVector(100, 0, 0));
    InteriorSpawnPoints.Add(FVector(-100, 0, 0));
}

void AArch_StructureActor::BeginPlay()
{
    Super::BeginPlay();

    // Register with architectural manager
    if (UWorld* World = GetWorld())
    {
        if (UArchitecturalManager* ArchManager = World->GetSubsystem<UArchitecturalManager>())
        {
            ArchManager->RegisterStructure(this);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchStructureActor BeginPlay: %s"), *StructureData.StructureName);
}

void AArch_StructureActor::SetStructureData(const FArch_StructureData& NewData)
{
    StructureData = NewData;
    
    // Update actor location and rotation
    SetActorLocation(StructureData.Location);
    SetActorRotation(StructureData.Rotation);
    
    // Update actor label for easier identification
    SetActorLabel(StructureData.StructureName);
}

FArch_StructureData AArch_StructureActor::GetStructureData() const
{
    return StructureData;
}

void AArch_StructureActor::ApplyWeathering(float WeatheringAmount)
{
    StructureData.StructuralIntegrity = FMath::Clamp(StructureData.StructuralIntegrity - WeatheringAmount, 0.0f, 100.0f);
    
    // Mark as ruin if integrity is too low
    if (StructureData.StructuralIntegrity < 30.0f)
    {
        StructureData.bIsRuin = true;
        StructureType = EArch_StructureType::Ruin;
    }
}

bool AArch_StructureActor::CanPlayerEnter() const
{
    return bCanBeEntered && StructureData.StructuralIntegrity > 20.0f;
}

TArray<FVector> AArch_StructureActor::GetInteriorSpawnPoints() const
{
    TArray<FVector> WorldSpawnPoints;
    FVector ActorLocation = GetActorLocation();
    FRotator ActorRotation = GetActorRotation();
    
    for (const FVector& LocalPoint : InteriorSpawnPoints)
    {
        FVector WorldPoint = ActorLocation + ActorRotation.RotateVector(LocalPoint);
        WorldSpawnPoints.Add(WorldPoint);
    }
    
    return WorldSpawnPoints;
}

// UArchitecturalManager Implementation
void UArchitecturalManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ManagedStructures.Empty();
    StructureCountPerBiome.Empty();
    
    // Initialize biome counters
    StructureCountPerBiome.Add(EBiomeType::Savana, 0);
    StructureCountPerBiome.Add(EBiomeType::Floresta, 0);
    StructureCountPerBiome.Add(EBiomeType::Deserto, 0);
    StructureCountPerBiome.Add(EBiomeType::Pantano, 0);
    StructureCountPerBiome.Add(EBiomeType::Montanha, 0);
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager initialized"));
}

void UArchitecturalManager::Deinitialize()
{
    ManagedStructures.Empty();
    StructureCountPerBiome.Empty();
    
    Super::Deinitialize();
}

AArch_StructureActor* UArchitecturalManager::SpawnStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType)
{
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AArch_StructureActor* NewStructure = World->SpawnActor<AArch_StructureActor>(AArch_StructureActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (NewStructure)
        {
            // Configure structure data
            FArch_StructureData StructureData;
            StructureData.Location = Location;
            StructureData.Rotation = Rotation;
            StructureData.BiomeType = BiomeType;
            StructureData.StructuralIntegrity = FMath::RandRange(60.0f, 100.0f);
            StructureData.bIsRuin = (StructureData.StructuralIntegrity < 70.0f);
            
            // Set structure name based on type
            switch (StructureType)
            {
                case EArch_StructureType::Dwelling:
                    StructureData.StructureName = TEXT("Prehistoric Dwelling");
                    break;
                case EArch_StructureType::Monument:
                    StructureData.StructureName = TEXT("Stone Monument");
                    break;
                case EArch_StructureType::DefensiveWall:
                    StructureData.StructureName = TEXT("Defensive Wall");
                    break;
                case EArch_StructureType::StorageHut:
                    StructureData.StructureName = TEXT("Storage Hut");
                    break;
                case EArch_StructureType::Watchtower:
                    StructureData.StructureName = TEXT("Watchtower");
                    break;
                case EArch_StructureType::Bridge:
                    StructureData.StructureName = TEXT("Stone Bridge");
                    break;
                case EArch_StructureType::Ruin:
                    StructureData.StructureName = TEXT("Ancient Ruins");
                    break;
                default:
                    StructureData.StructureName = TEXT("Unknown Structure");
                    break;
            }
            
            NewStructure->SetStructureData(StructureData);
            RegisterStructure(NewStructure);
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned structure: %s at %s"), *StructureData.StructureName, *Location.ToString());
            
            return NewStructure;
        }
    }
    
    return nullptr;
}

void UArchitecturalManager::RegisterStructure(AArch_StructureActor* Structure)
{
    if (Structure && !ManagedStructures.Contains(Structure))
    {
        ManagedStructures.Add(Structure);
        
        FArch_StructureData StructureData = Structure->GetStructureData();
        if (StructureCountPerBiome.Contains(StructureData.BiomeType))
        {
            StructureCountPerBiome[StructureData.BiomeType]++;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Registered structure: %s"), *StructureData.StructureName);
    }
}

void UArchitecturalManager::UnregisterStructure(AArch_StructureActor* Structure)
{
    if (Structure && ManagedStructures.Contains(Structure))
    {
        FArch_StructureData StructureData = Structure->GetStructureData();
        if (StructureCountPerBiome.Contains(StructureData.BiomeType))
        {
            StructureCountPerBiome[StructureData.BiomeType] = FMath::Max(0, StructureCountPerBiome[StructureData.BiomeType] - 1);
        }
        
        ManagedStructures.Remove(Structure);
        UE_LOG(LogTemp, Log, TEXT("Unregistered structure: %s"), *StructureData.StructureName);
    }
}

TArray<AArch_StructureActor*> UArchitecturalManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<AArch_StructureActor*> BiomeStructures;
    
    for (AArch_StructureActor* Structure : ManagedStructures)
    {
        if (Structure && Structure->GetStructureData().BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

int32 UArchitecturalManager::GetStructureCount() const
{
    return ManagedStructures.Num();
}

void UArchitecturalManager::PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount)
{
    // Define biome center coordinates
    FVector BiomeCenter;
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeCenter = FVector(0, 0, 100);
            break;
        case EBiomeType::Floresta:
            BiomeCenter = FVector(-45000, 40000, 100);
            break;
        case EBiomeType::Deserto:
            BiomeCenter = FVector(55000, 0, 100);
            break;
        case EBiomeType::Pantano:
            BiomeCenter = FVector(-50000, -45000, 100);
            break;
        case EBiomeType::Montanha:
            BiomeCenter = FVector(40000, 50000, 100);
            break;
        default:
            BiomeCenter = FVector::ZeroVector;
            break;
    }
    
    // Spawn structures in a radius around biome center
    float SpawnRadius = 5000.0f;
    
    for (int32 i = 0; i < StructureCount; i++)
    {
        // Random location within biome radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        // Random structure type
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 7));
        
        SpawnStructure(RandomType, SpawnLocation, SpawnRotation, BiomeType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Populated %s biome with %d structures"), 
           *UEnum::GetValueAsString(BiomeType), StructureCount);
}

void UArchitecturalManager::CreateTestStructures()
{
    // Create test structures in each biome
    PopulateBiomeWithStructures(EBiomeType::Savana, 5);
    PopulateBiomeWithStructures(EBiomeType::Floresta, 3);
    PopulateBiomeWithStructures(EBiomeType::Deserto, 4);
    PopulateBiomeWithStructures(EBiomeType::Pantano, 2);
    PopulateBiomeWithStructures(EBiomeType::Montanha, 3);
    
    UE_LOG(LogTemp, Warning, TEXT("Created test structures in all biomes. Total structures: %d"), GetStructureCount());
}