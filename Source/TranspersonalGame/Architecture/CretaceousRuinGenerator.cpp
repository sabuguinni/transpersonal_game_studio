#include "CretaceousRuinGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

ACretaceousRuinGenerator::ACretaceousRuinGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root component
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RuinRoot"));
    SetRootComponent(Root);
}

void ACretaceousRuinGenerator::BeginPlay()
{
    Super::BeginPlay();
    if (!bRuinGenerated)
    {
        GenerateRuin();
    }
}

void ACretaceousRuinGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
}

void ACretaceousRuinGenerator::GenerateRuin()
{
    ClearRuin();
    SpawnedElements.Empty();

    if (bSpawnArchGateway)
    {
        SpawnArchGateway();
    }

    SpawnColumns();
    SpawnWalls();
    SpawnRubble();

    if (bSpawnAltarStone)
    {
        SpawnAltarStone();
    }

    bRuinGenerated = true;

    UE_LOG(LogTemp, Log, TEXT("ACretaceousRuinGenerator: Generated ruin with %d elements at %s"),
        SpawnedElements.Num(), *GetActorLocation().ToString());
}

void ACretaceousRuinGenerator::ClearRuin()
{
    for (UStaticMeshComponent* Comp : RuinComponents)
    {
        if (Comp)
        {
            Comp->DestroyComponent();
        }
    }
    RuinComponents.Empty();
    SpawnedElements.Empty();
    bRuinGenerated = false;
}

void ACretaceousRuinGenerator::RegenerateInEditor()
{
    GenerateRuin();
    UE_LOG(LogTemp, Log, TEXT("ACretaceousRuinGenerator: Regenerated in editor — %d elements"), SpawnedElements.Num());
}

int32 ACretaceousRuinGenerator::GetElementCount() const
{
    return SpawnedElements.Num();
}

FVector ACretaceousRuinGenerator::GetRuinCenter() const
{
    return GetActorLocation();
}

void ACretaceousRuinGenerator::SpawnColumns()
{
    if (!ColumnMesh)
    {
        return;
    }

    const float AngleStep = 360.0f / FMath::Max(NumColumns, 1);
    for (int32 i = 0; i < NumColumns; ++i)
    {
        const float Angle = FMath::DegreesToRadians(AngleStep * i);
        const FVector Offset(
            FMath::Cos(Angle) * RuinRadius,
            FMath::Sin(Angle) * RuinRadius,
            0.0f
        );

        // Vary column height based on wear level
        const float HeightScale = FMath::Lerp(4.0f, 1.5f, GlobalWearLevel * FMath::FRand());
        const FVector Scale(0.5f, 0.5f, HeightScale);

        AddRuinMeshComponent(ColumnMesh, Offset, FRotator::ZeroRotator, Scale);

        FArch_RuinElement Element;
        Element.RuinType = EArch_RuinType::StoneColumn;
        Element.RelativeOffset = Offset;
        Element.Scale = Scale;
        Element.WearLevel = GlobalWearLevel;
        SpawnedElements.Add(Element);
    }
}

void ACretaceousRuinGenerator::SpawnWalls()
{
    if (!WallMesh)
    {
        return;
    }

    // 4 wall segments connecting the column ring
    const TArray<TPair<FVector, float>> Walls = {
        { FVector(0.0f, -RuinRadius * 0.8f, 0.0f), 0.0f },
        { FVector(0.0f,  RuinRadius * 0.8f, 0.0f), 0.0f },
        { FVector(-RuinRadius * 0.8f, 0.0f, 0.0f), 90.0f },
        { FVector( RuinRadius * 0.8f, 0.0f, 0.0f), 90.0f },
    };

    for (const auto& Wall : Walls)
    {
        const float WallHeight = FMath::Lerp(3.0f, 1.0f, GlobalWearLevel);
        const FVector Scale(RuinRadius * 0.01f, 0.4f, WallHeight);
        const FRotator Rotation(0.0f, Wall.Value, 0.0f);

        AddRuinMeshComponent(WallMesh, Wall.Key, Rotation, Scale);

        FArch_RuinElement Element;
        Element.RuinType = EArch_RuinType::WallSegment;
        Element.RelativeOffset = Wall.Key;
        Element.Scale = Scale;
        Element.YawRotation = Wall.Value;
        Element.WearLevel = GlobalWearLevel;
        SpawnedElements.Add(Element);
    }
}

void ACretaceousRuinGenerator::SpawnArchGateway()
{
    if (!ColumnMesh || !WallMesh)
    {
        return;
    }

    // Left pillar
    AddRuinMeshComponent(ColumnMesh, FVector(-200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, FVector(0.6f, 0.6f, 5.0f));
    // Right pillar
    AddRuinMeshComponent(ColumnMesh, FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, FVector(0.6f, 0.6f, 5.0f));
    // Lintel
    AddRuinMeshComponent(WallMesh, FVector(0.0f, 0.0f, 450.0f), FRotator::ZeroRotator, FVector(4.5f, 0.6f, 0.4f));

    FArch_RuinElement GatewayElement;
    GatewayElement.RuinType = EArch_RuinType::ArchGateway;
    GatewayElement.RelativeOffset = FVector::ZeroVector;
    GatewayElement.Scale = FVector(1.0f, 1.0f, 1.0f);
    GatewayElement.WearLevel = GlobalWearLevel;
    SpawnedElements.Add(GatewayElement);
}

void ACretaceousRuinGenerator::SpawnAltarStone()
{
    if (!AltarMesh)
    {
        return;
    }

    AddRuinMeshComponent(AltarMesh, FVector::ZeroVector, FRotator::ZeroRotator, FVector(2.0f, 1.2f, 0.6f));

    FArch_RuinElement Element;
    Element.RuinType = EArch_RuinType::AltarStone;
    Element.RelativeOffset = FVector::ZeroVector;
    Element.Scale = FVector(2.0f, 1.2f, 0.6f);
    Element.WearLevel = GlobalWearLevel * 0.5f; // Altar is better preserved
    SpawnedElements.Add(Element);
}

void ACretaceousRuinGenerator::SpawnRubble()
{
    if (!RubbleMesh)
    {
        return;
    }

    FMath::RandInit(RandomSeed);

    for (int32 i = 0; i < NumRubblePieces; ++i)
    {
        const float Angle = FMath::FRand() * 360.0f;
        const float Dist = FMath::FRandRange(RuinRadius * 0.3f, RuinRadius * 1.2f);
        const FVector Offset(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
            0.0f
        );

        const float S = FMath::FRandRange(0.3f, 1.2f);
        const FVector Scale(S, S, FMath::FRandRange(0.2f, 0.6f));
        const FRotator Rotation(
            FMath::FRandRange(-20.0f, 20.0f),
            FMath::FRandRange(0.0f, 360.0f),
            0.0f
        );

        AddRuinMeshComponent(RubbleMesh, Offset, Rotation, Scale);

        FArch_RuinElement Element;
        Element.RuinType = EArch_RuinType::RubblePile;
        Element.RelativeOffset = Offset;
        Element.Scale = Scale;
        Element.YawRotation = Rotation.Yaw;
        Element.WearLevel = 1.0f; // Rubble is maximally worn
        SpawnedElements.Add(Element);
    }
}

UStaticMeshComponent* ACretaceousRuinGenerator::AddRuinMeshComponent(
    UStaticMesh* Mesh,
    FVector Offset,
    FRotator Rotation,
    FVector Scale)
{
    if (!Mesh)
    {
        return nullptr;
    }

    UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
    if (!Comp)
    {
        return nullptr;
    }

    Comp->SetStaticMesh(Mesh);
    Comp->SetRelativeLocation(Offset);
    Comp->SetRelativeRotation(Rotation);
    Comp->SetRelativeScale3D(Scale);
    Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Comp->SetCollisionProfileName(TEXT("BlockAll"));
    Comp->RegisterComponent();
    Comp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    RuinComponents.Add(Comp);
    return Comp;
}
