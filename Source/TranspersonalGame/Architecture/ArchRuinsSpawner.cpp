#include "ArchRuinsSpawner.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

AArchRuinsSpawner::AArchRuinsSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPillarCount = 5;
    ScatterRadius = 1000.0f;

    // Default pillar mesh — cylinder as stand-in until GLB asset is imported
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
        TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        PillarMesh = CylinderMesh.Object;
    }
}

void AArchRuinsSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (PillarConfigs.Num() == 0)
    {
        GenerateDefaultPillarConfigs();
    }
    SpawnRuins();
}

void AArchRuinsSpawner::GenerateDefaultPillarConfigs()
{
    // Generate a ring of 5 pillars around the spawner's location
    const float AngleStep = 360.0f / AutoPillarCount;
    for (int32 i = 0; i < AutoPillarCount; ++i)
    {
        float Angle = FMath::DegreesToRadians(AngleStep * i);
        FArch_RuinPillarConfig Config;
        Config.Location = GetActorLocation() + FVector(
            FMath::Cos(Angle) * ScatterRadius,
            FMath::Sin(Angle) * ScatterRadius,
            0.0f
        );
        Config.Scale = FVector(0.5f, 0.5f, 3.0f + FMath::FRandRange(0.0f, 2.0f));
        Config.TiltAngleDegrees = FMath::FRandRange(-5.0f, 5.0f);
        Config.bIsBroken = (i % 3 == 0); // Every 3rd pillar is broken/shorter
        if (Config.bIsBroken)
        {
            Config.Scale.Z *= 0.5f;
        }
        PillarConfigs.Add(Config);
    }
}

void AArchRuinsSpawner::SpawnRuins()
{
    if (!PillarMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AArchRuinsSpawner: No PillarMesh assigned — ruins not spawned."));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    for (const FArch_RuinPillarConfig& Config : PillarConfigs)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FRotator Rotation(Config.TiltAngleDegrees, FMath::FRandRange(0.0f, 360.0f), 0.0f);
        AStaticMeshActor* Pillar = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Config.Location,
            Rotation,
            SpawnParams
        );

        if (Pillar)
        {
            UStaticMeshComponent* MeshComp = Pillar->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(PillarMesh);
                Pillar->SetActorScale3D(Config.Scale);
            }
            SpawnedRuinActors.Add(Pillar);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AArchRuinsSpawner: Spawned %d ruin pillars."), SpawnedRuinActors.Num());
}

void AArchRuinsSpawner::ClearRuins()
{
    for (AActor* RuinActor : SpawnedRuinActors)
    {
        if (IsValid(RuinActor))
        {
            RuinActor->Destroy();
        }
    }
    SpawnedRuinActors.Empty();
    PillarConfigs.Empty();
    UE_LOG(LogTemp, Log, TEXT("AArchRuinsSpawner: All ruins cleared."));
}
