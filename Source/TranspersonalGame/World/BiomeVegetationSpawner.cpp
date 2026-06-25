// BiomeVegetationSpawner.cpp
// Procedural biome vegetation spawner — Agent #5 Procedural World Generator
// Cycle: PROD_CYCLE_AUTO_20260625_009

#include "BiomeVegetationSpawner.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ABiomeVegetationSpawner::ABiomeVegetationSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bSpawnOnBeginPlay = true;
    RandomSeed = 42;
}

void ABiomeVegetationSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (bSpawnOnBeginPlay)
    {
        SpawnVegetation();
    }
}

void ABiomeVegetationSpawner::SpawnVegetation()
{
    if (BiomeConfig.VegetationEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeVegetationSpawner: No vegetation entries configured for biome %d"),
            (int32)BiomeConfig.BiomeType);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FRandomStream Stream(RandomSeed);
    int32 SpawnCount = 0;

    for (int32 i = 0; i < BiomeConfig.TreeDensity; ++i)
    {
        // Pick a random vegetation entry weighted by SpawnWeight
        float TotalWeight = 0.0f;
        for (const FWorld_VegetationEntry& Entry : BiomeConfig.VegetationEntries)
        {
            TotalWeight += Entry.SpawnWeight;
        }

        float Roll = Stream.FRandRange(0.0f, TotalWeight);
        float Accumulated = 0.0f;
        const FWorld_VegetationEntry* ChosenEntry = nullptr;

        for (const FWorld_VegetationEntry& Entry : BiomeConfig.VegetationEntries)
        {
            Accumulated += Entry.SpawnWeight;
            if (Roll <= Accumulated)
            {
                ChosenEntry = &Entry;
                break;
            }
        }

        if (!ChosenEntry) continue;

        UStaticMesh* Mesh = ChosenEntry->Mesh.LoadSynchronous();
        if (!Mesh) continue;

        FVector SpawnPos = GetRandomPositionInBiome(BiomeConfig.SpawnRadius, Stream);
        SpawnPos.Z = GetGroundHeight(SpawnPos);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AStaticMeshActor* TreeActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnPos,
            FRotator(0.0f, Stream.FRandRange(0.0f, 360.0f), 0.0f),
            SpawnParams
        );

        if (TreeActor)
        {
            UStaticMeshComponent* MeshComp = TreeActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(Mesh);
                float Scale = Stream.FRandRange(ChosenEntry->MinScale, ChosenEntry->MaxScale);
                TreeActor->SetActorScale3D(FVector(Scale));
            }

            FString Label = FString::Printf(TEXT("BiomeVeg_%s_%03d"),
                *UEnum::GetValueAsString(BiomeConfig.BiomeType), i);
            TreeActor->SetActorLabel(Label);

            SpawnedActors.Add(TreeActor);
            SpawnCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeVegetationSpawner: Spawned %d vegetation actors for biome %s"),
        SpawnCount, *UEnum::GetValueAsString(BiomeConfig.BiomeType));
}

void ABiomeVegetationSpawner::ClearSpawnedVegetation()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();
    UE_LOG(LogTemp, Log, TEXT("BiomeVegetationSpawner: Cleared all spawned vegetation."));
}

int32 ABiomeVegetationSpawner::GetSpawnedCount() const
{
    return SpawnedActors.Num();
}

FVector ABiomeVegetationSpawner::GetRandomPositionInBiome(float Radius, FRandomStream& Stream) const
{
    float Angle = Stream.FRandRange(0.0f, 2.0f * PI);
    float Distance = Stream.FRandRange(0.0f, Radius);
    FVector Center = BiomeConfig.BiomeCenter;

    return FVector(
        Center.X + Distance * FMath::Cos(Angle),
        Center.Y + Distance * FMath::Sin(Angle),
        Center.Z
    );
}

float ABiomeVegetationSpawner::GetGroundHeight(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World) return 0.0f;

    FHitResult HitResult;
    FVector Start = FVector(Location.X, Location.Y, 10000.0f);
    FVector End = FVector(Location.X, Location.Y, -10000.0f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params))
    {
        return HitResult.ImpactPoint.Z;
    }

    return Location.Z;
}
