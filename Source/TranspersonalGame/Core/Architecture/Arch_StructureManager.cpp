#include "Arch_StructureManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MinDistanceBetweenStructures = 500.0f;
    MaxStructuresPerBiome = 10;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
}

void AArch_StructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    if (!IsValidSpawnLocation(Location))
    {
        return;
    }

    AActor* NewStructure = nullptr;

    switch (StructureType)
    {
        case EArch_StructureType::StoneCircle:
            NewStructure = CreateStoneCircle(Location, Rotation);
            break;
        case EArch_StructureType::CaveEntrance:
            NewStructure = CreateCaveEntrance(Location, Rotation);
            break;
        case EArch_StructureType::RockFormation:
            NewStructure = CreateRockFormation(Location, Rotation);
            break;
        case EArch_StructureType::AncientRuin:
            NewStructure = CreateAncientRuin(Location, Rotation);
            break;
        case EArch_StructureType::NaturalArch:
            NewStructure = CreateNaturalArch(Location, Rotation);
            break;
    }

    if (NewStructure)
    {
        SpawnedStructures.Add(NewStructure);
    }
}

void AArch_StructureManager::GenerateStructuresInRadius(FVector CenterLocation, float Radius, int32 StructureCount)
{
    for (int32 i = 0; i < StructureCount; i++)
    {
        FVector RandomLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );

        EArch_StructureType RandomType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

        SpawnStructureAtLocation(RandomType, RandomLocation, RandomRotation);
    }
}

void AArch_StructureManager::ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringLevel)
{
    if (!StructureActor)
    {
        return;
    }

    UMaterialInterface* MaterialToApply = StoneMaterial;

    if (WeatheringLevel > 0.7f && MossyStoneMaterial)
    {
        MaterialToApply = MossyStoneMaterial;
    }
    else if (WeatheringLevel > 0.3f && WeatheredStoneMaterial)
    {
        MaterialToApply = WeatheredStoneMaterial;
    }

    ApplyMaterialToActor(StructureActor, MaterialToApply);
}

TArray<AActor*> AArch_StructureManager::GetAllStructuresInRadius(FVector CenterLocation, float Radius)
{
    TArray<AActor*> StructuresInRadius;

    for (AActor* Structure : SpawnedStructures)
    {
        if (Structure && FVector::Dist(Structure->GetActorLocation(), CenterLocation) <= Radius)
        {
            StructuresInRadius.Add(Structure);
        }
    }

    return StructuresInRadius;
}

void AArch_StructureManager::GeneratePrehistoricStructures()
{
    ClearAllStructures();

    // Generate stone circles
    for (int32 i = 0; i < 3; i++)
    {
        FVector Location = FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            100.0f
        );
        SpawnStructureAtLocation(EArch_StructureType::StoneCircle, Location);
    }

    // Generate cave entrances
    for (int32 i = 0; i < 2; i++)
    {
        FVector Location = FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            200.0f
        );
        SpawnStructureAtLocation(EArch_StructureType::CaveEntrance, Location);
    }

    // Generate rock formations
    GenerateStructuresInRadius(FVector::ZeroVector, 8000.0f, 5);
}

void AArch_StructureManager::ClearAllStructures()
{
    for (AActor* Structure : SpawnedStructures)
    {
        if (Structure)
        {
            Structure->Destroy();
        }
    }
    SpawnedStructures.Empty();
}

AActor* AArch_StructureManager::CreateStoneCircle(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Create parent actor for the stone circle
    AActor* CircleParent = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (!CircleParent)
    {
        return nullptr;
    }

    CircleParent->SetActorLabel(TEXT("Stone_Circle"));

    // Create individual stones in a circle
    const int32 StoneCount = 8;
    const float CircleRadius = 1000.0f;

    for (int32 i = 0; i < StoneCount; i++)
    {
        float Angle = (2.0f * PI * i) / StoneCount;
        FVector StoneOffset = FVector(
            CircleRadius * FMath::Cos(Angle),
            CircleRadius * FMath::Sin(Angle),
            0.0f
        );

        AStaticMeshActor* StoneActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location + StoneOffset,
            Rotation
        );

        if (StoneActor)
        {
            StoneActor->AttachToActor(CircleParent, FAttachmentTransformRules::KeepWorldTransform);
            StoneActor->SetActorScale3D(FVector(0.5f, 0.5f, 3.0f));
        }
    }

    return CircleParent;
}

AActor* AArch_StructureManager::CreateCaveEntrance(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AStaticMeshActor* CaveActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        Rotation
    );

    if (CaveActor)
    {
        CaveActor->SetActorLabel(TEXT("Cave_Entrance"));
        CaveActor->SetActorScale3D(FVector(4.0f, 2.0f, 3.0f));
    }

    return CaveActor;
}

AActor* AArch_StructureManager::CreateRockFormation(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AActor* FormationParent = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (!FormationParent)
    {
        return nullptr;
    }

    FormationParent->SetActorLabel(TEXT("Rock_Formation"));

    // Create cluster of rocks
    const int32 RockCount = FMath::RandRange(3, 7);
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockOffset = FVector(
            FMath::RandRange(-300.0f, 300.0f),
            FMath::RandRange(-300.0f, 300.0f),
            0.0f
        );

        AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location + RockOffset,
            FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)
        );

        if (RockActor)
        {
            RockActor->AttachToActor(FormationParent, FAttachmentTransformRules::KeepWorldTransform);
            float RandomScale = FMath::RandRange(0.8f, 2.5f);
            RockActor->SetActorScale3D(FVector(RandomScale));
        }
    }

    return FormationParent;
}

AActor* AArch_StructureManager::CreateAncientRuin(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AActor* RuinParent = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (!RuinParent)
    {
        return nullptr;
    }

    RuinParent->SetActorLabel(TEXT("Ancient_Ruin"));

    // Create broken pillars and wall segments
    const int32 PillarCount = 4;
    for (int32 i = 0; i < PillarCount; i++)
    {
        FVector PillarOffset = FVector(
            (i % 2) * 400.0f - 200.0f,
            (i / 2) * 600.0f - 300.0f,
            0.0f
        );

        AStaticMeshActor* PillarActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location + PillarOffset,
            Rotation
        );

        if (PillarActor)
        {
            PillarActor->AttachToActor(RuinParent, FAttachmentTransformRules::KeepWorldTransform);
            // Broken pillar - random height
            float BrokenHeight = FMath::RandRange(1.0f, 4.0f);
            PillarActor->SetActorScale3D(FVector(0.6f, 0.6f, BrokenHeight));
        }
    }

    return RuinParent;
}

AActor* AArch_StructureManager::CreateNaturalArch(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AStaticMeshActor* ArchActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        Rotation
    );

    if (ArchActor)
    {
        ArchActor->SetActorLabel(TEXT("Natural_Arch"));
        ArchActor->SetActorScale3D(FVector(6.0f, 2.0f, 4.0f));
    }

    return ArchActor;
}

void AArch_StructureManager::ApplyMaterialToActor(AActor* Actor, UMaterialInterface* Material)
{
    if (!Actor || !Material)
    {
        return;
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetMaterial(0, Material);
        }
    }
}

bool AArch_StructureManager::IsValidSpawnLocation(FVector Location)
{
    // Check minimum distance from other structures
    for (AActor* ExistingStructure : SpawnedStructures)
    {
        if (ExistingStructure)
        {
            float Distance = FVector::Dist(ExistingStructure->GetActorLocation(), Location);
            if (Distance < MinDistanceBetweenStructures)
            {
                return false;
            }
        }
    }

    return true;
}