#include "ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArchitectureManager::AArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxStructuresPerBiome = 15;
    MinDistanceBetweenStructures = 5000.0f;

    ManagedStructures.Empty();
}

void AArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: System initialized"));
}

void AArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArchitectureManager::SpawnStructureAtLocation(const FVector& Location, EArch_StructureType StructureType)
{
    if (!IsValidStructureLocation(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Invalid location for structure"));
        return;
    }

    UStaticMesh* StructureMesh = GetMeshForStructureType(StructureType);
    if (!StructureMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: No mesh found for structure type"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* StructureActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (StructureActor)
    {
        UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(StructureActor);
        MeshComponent->SetStaticMesh(StructureMesh);
        MeshComponent->SetupAttachment(StructureActor->GetRootComponent());
        StructureActor->AddInstanceComponent(MeshComponent);

        FString StructureName = FString::Printf(TEXT("Structure_%d"), ManagedStructures.Num());
        StructureActor->SetActorLabel(StructureName);

        FArch_StructureData NewStructure;
        NewStructure.StructureName = StructureName;
        NewStructure.StructureType = StructureType;
        NewStructure.Location = Location;
        NewStructure.Rotation = FRotator::ZeroRotator;
        NewStructure.StructuralIntegrity = 100.0f;
        NewStructure.bIsRuin = false;

        ManagedStructures.Add(NewStructure);

        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Spawned structure %s at %s"), *StructureName, *Location.ToString());
    }
}

void AArchitectureManager::SpawnStructuresInBiome(EBiomeType BiomeType, int32 Count)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float BiomeRadius = 15000.0f;

    for (int32 i = 0; i < Count; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            0.0f
        );

        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnLocation.Z = 100.0f;

        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
        SpawnStructureAtLocation(SpawnLocation, RandomType);
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Spawned %d structures in biome"), Count);
}

void AArchitectureManager::DestroyStructure(const FString& StructureName)
{
    for (int32 i = ManagedStructures.Num() - 1; i >= 0; i--)
    {
        if (ManagedStructures[i].StructureName == StructureName)
        {
            ManagedStructures.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Destroyed structure %s"), *StructureName);
            break;
        }
    }
}

TArray<FArch_StructureData> AArchitectureManager::GetStructuresInRadius(const FVector& Center, float Radius)
{
    TArray<FArch_StructureData> NearbyStructures;

    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        float Distance = FVector::Dist(Structure.Location, Center);
        if (Distance <= Radius)
        {
            NearbyStructures.Add(Structure);
        }
    }

    return NearbyStructures;
}

void AArchitectureManager::UpdateStructuralIntegrity(const FString& StructureName, float NewIntegrity)
{
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.StructureName == StructureName)
        {
            Structure.StructuralIntegrity = FMath::Clamp(NewIntegrity, 0.0f, 100.0f);
            
            if (Structure.StructuralIntegrity <= 0.0f)
            {
                ConvertToRuin(StructureName);
            }
            break;
        }
    }
}

void AArchitectureManager::ConvertToRuin(const FString& StructureName)
{
    for (FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.StructureName == StructureName)
        {
            Structure.bIsRuin = true;
            Structure.StructuralIntegrity = 0.0f;
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Converted %s to ruin"), *StructureName);
            break;
        }
    }
}

void AArchitectureManager::GenerateArchitecturalElements()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Generating architectural elements across all biomes"));

    // Generate structures in each biome
    SpawnStructuresInBiome(EBiomeType::Savanna, 3);
    SpawnStructuresInBiome(EBiomeType::Swamp, 2);
    SpawnStructuresInBiome(EBiomeType::Forest, 4);
    SpawnStructuresInBiome(EBiomeType::Desert, 2);
    SpawnStructuresInBiome(EBiomeType::Mountain, 3);
}

void AArchitectureManager::ClearAllStructures()
{
    ManagedStructures.Empty();
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cleared all structures"));
}

void AArchitectureManager::OnStructureDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor)
    {
        FString ActorName = DestroyedActor->GetActorLabel();
        DestroyStructure(ActorName);
    }
}

FVector AArchitectureManager::GetBiomeCenter(EBiomeType BiomeType)
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

bool AArchitectureManager::IsValidStructureLocation(const FVector& Location)
{
    for (const FArch_StructureData& ExistingStructure : ManagedStructures)
    {
        float Distance = FVector::Dist(ExistingStructure.Location, Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    return true;
}

UStaticMesh* AArchitectureManager::GetMeshForStructureType(EArch_StructureType StructureType)
{
    // Return nullptr for now - meshes will be assigned via Blueprint or asset loading
    return nullptr;
}