#include "ArchRuinSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

AArchRuinCluster::AArchRuinCluster()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root component
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RuinClusterRoot"));
    SetRootComponent(Root);

    // Default cluster: 3 standing pillars + 1 broken wall + 1 fallen column
    FArch_RuinElementData Pillar1;
    Pillar1.ElementType = EArch_RuinElementType::StandingPillar;
    Pillar1.RelativeOffset = FVector(0.0f, 0.0f, 0.0f);
    Pillar1.Scale = FVector(1.0f, 1.0f, 4.0f);
    Pillar1.WeatheringLevel = 0.7f;
    RuinElements.Add(Pillar1);

    FArch_RuinElementData Pillar2;
    Pillar2.ElementType = EArch_RuinElementType::StandingPillar;
    Pillar2.RelativeOffset = FVector(300.0f, 0.0f, 0.0f);
    Pillar2.Scale = FVector(1.0f, 1.0f, 3.0f);
    Pillar2.WeatheringLevel = 0.8f;
    RuinElements.Add(Pillar2);

    FArch_RuinElementData Wall;
    Wall.ElementType = EArch_RuinElementType::BrokenWall;
    Wall.RelativeOffset = FVector(150.0f, 200.0f, 0.0f);
    Wall.Scale = FVector(5.0f, 0.5f, 2.5f);
    Wall.WeatheringLevel = 0.6f;
    RuinElements.Add(Wall);

    FArch_RuinElementData FallenCol;
    FallenCol.ElementType = EArch_RuinElementType::FallenColumn;
    FallenCol.RelativeOffset = FVector(-100.0f, 150.0f, 30.0f);
    FallenCol.RelativeRotation = FRotator(90.0f, 45.0f, 0.0f);
    FallenCol.Scale = FVector(0.8f, 0.8f, 3.5f);
    FallenCol.WeatheringLevel = 0.9f;
    RuinElements.Add(FallenCol);

    FArch_RuinElementData Rubble;
    Rubble.ElementType = EArch_RuinElementType::RubblePile;
    Rubble.RelativeOffset = FVector(50.0f, -150.0f, 0.0f);
    Rubble.Scale = FVector(2.0f, 2.0f, 0.8f);
    Rubble.WeatheringLevel = 1.0f;
    RuinElements.Add(Rubble);
}

void AArchRuinCluster::BeginPlay()
{
    Super::BeginPlay();
}

void AArchRuinCluster::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RebuildCluster();
}

void AArchRuinCluster::RebuildCluster()
{
    ClearMeshComponents();

    for (int32 i = 0; i < RuinElements.Num(); ++i)
    {
        SpawnElementMesh(RuinElements[i], i);
    }
}

void AArchRuinCluster::RandomizeLayout()
{
    for (FArch_RuinElementData& Element : RuinElements)
    {
        float RandX = FMath::RandRange(-ScatterRadius, ScatterRadius);
        float RandY = FMath::RandRange(-ScatterRadius, ScatterRadius);
        Element.RelativeOffset = FVector(RandX, RandY, 0.0f);

        float RandYaw = FMath::RandRange(0.0f, 360.0f);
        Element.RelativeRotation = FRotator(0.0f, RandYaw, 0.0f);

        Element.WeatheringLevel = FMath::RandRange(0.4f, 1.0f);
    }

    RebuildCluster();
}

void AArchRuinCluster::ClearMeshComponents()
{
    for (UStaticMeshComponent* Comp : RuinMeshComponents)
    {
        if (Comp)
        {
            Comp->DestroyComponent();
        }
    }
    RuinMeshComponents.Empty();
}

void AArchRuinCluster::SpawnElementMesh(const FArch_RuinElementData& ElementData, int32 Index)
{
    FString CompName = FString::Printf(TEXT("RuinElement_%d"), Index);
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this, *CompName);

    if (!MeshComp)
    {
        return;
    }

    MeshComp->RegisterComponent();
    MeshComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    // Set relative transform from element data
    MeshComp->SetRelativeLocation(ElementData.RelativeOffset);
    MeshComp->SetRelativeRotation(ElementData.RelativeRotation);
    MeshComp->SetRelativeScale3D(ElementData.Scale);

    // Use engine cube as placeholder mesh — replaced by GLB import from Meshy pipeline
    // Mesh path: /Engine/BasicShapes/Cube (always available in UE5)
    UStaticMesh* PlaceholderMesh = Cast<UStaticMesh>(
        StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/Cube"))
    );

    if (PlaceholderMesh)
    {
        MeshComp->SetStaticMesh(PlaceholderMesh);
    }

    // Collision: block all for physical ruins
    MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

    // Cast shadows for visual quality
    MeshComp->CastShadow = true;
    MeshComp->bCastDynamicShadow = true;

    RuinMeshComponents.Add(MeshComp);
}
