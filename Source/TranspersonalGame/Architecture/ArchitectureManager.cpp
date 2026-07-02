#include "ArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AArch_ArchitectureManager::AArch_ArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxActiveStructures = 200;
    StreamingRadius = 15000.0f;
}

void AArch_ArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initialized with %d registered structures"), RegisteredStructures.Num());
}

void AArch_ArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_ArchitectureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    if (RegisteredStructures.Num() >= MaxActiveStructures)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Max structures (%d) reached, cannot register more"), MaxActiveStructures);
        return;
    }
    RegisteredStructures.Add(StructureData);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Registered structure type %d at %s"),
        (int32)StructureData.StructureType,
        *StructureData.WorldLocation.ToString());
}

bool AArch_ArchitectureManager::FindNearestShelter(FVector FromLocation, float SearchRadius, FArch_StructureData& OutShelter)
{
    float BestDistSq = SearchRadius * SearchRadius;
    bool bFound = false;

    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        if (!Structure.bCanProvideShelte)
        {
            continue;
        }

        float DSq = DistSq(FromLocation, Structure.WorldLocation);
        if (DSq < BestDistSq)
        {
            BestDistSq = DSq;
            OutShelter = Structure;
            bFound = true;
        }
    }

    if (bFound)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Found nearest shelter at %s"), *OutShelter.WorldLocation.ToString());
    }

    return bFound;
}

TArray<FArch_StructureData> AArch_ArchitectureManager::GetStructuresOfTypeInRadius(
    EArch_StructureType Type, FVector Center, float Radius)
{
    TArray<FArch_StructureData> Result;
    float RadiusSq = Radius * Radius;

    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        if (Structure.StructureType == Type && DistSq(Center, Structure.WorldLocation) <= RadiusSq)
        {
            Result.Add(Structure);
        }
    }

    return Result;
}

AActor* AArch_ArchitectureManager::SpawnStructureActor(const FArch_StructureData& StructureData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: No world available for spawning"));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FTransform SpawnTransform(
        StructureData.WorldRotation,
        StructureData.WorldLocation,
        StructureData.Scale
    );

    AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        SpawnTransform,
        SpawnParams
    );

    if (SpawnedActor)
    {
        // Label the actor for editor identification
        FString Label = FString::Printf(TEXT("ArchStruct_%d_%s"),
            (int32)StructureData.StructureType,
            *StructureData.WorldLocation.ToCompactString());
        SpawnedActor->SetActorLabel(Label);

        SpawnedStructureActors.Add(SpawnedActor);
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Spawned structure actor '%s'"), *Label);
    }

    return SpawnedActor;
}

int32 AArch_ArchitectureManager::GetStructureCount() const
{
    return RegisteredStructures.Num();
}

void AArch_ArchitectureManager::ClearAllStructures()
{
    // Destroy all spawned actors
    for (AActor* Actor : SpawnedStructureActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedStructureActors.Empty();
    RegisteredStructures.Empty();

    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Cleared all structures"));
}

float AArch_ArchitectureManager::DistSq(const FVector& A, const FVector& B) const
{
    return FVector::DistSquared(A, B);
}
