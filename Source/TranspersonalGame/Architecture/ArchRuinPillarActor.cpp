#include "ArchRuinPillarActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AArchRuinPillarActor::AArchRuinPillarActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root — base stone slab
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    SetRootComponent(BaseMesh);
    BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));
    BaseMesh->SetGenerateOverlapEvents(false);

    // Pillar column on top of base
    PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
    PillarMesh->SetupAttachment(BaseMesh);
    PillarMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    PillarMesh->SetCollisionProfileName(TEXT("BlockAll"));
    PillarMesh->SetGenerateOverlapEvents(false);

    // Default ruin data — half-buried, heavily weathered
    RuinData.RuinState = EArch_RuinState::HalfBuried;
    RuinData.WeatheringLevel = 0.75f;
    RuinData.MossCoverage = 0.6f;
    RuinData.bHasVegetationGrowth = true;

    BiomeX = 50000.0f;
    BiomeY = 50000.0f;
}

void AArchRuinPillarActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyWeathering(RuinData.WeatheringLevel);
}

void AArchRuinPillarActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArchRuinPillarActor::ApplyWeathering(float WeatherAmount)
{
    if (!PillarMesh || !BaseMesh)
    {
        return;
    }

    // Clamp weathering to valid range
    RuinData.WeatheringLevel = FMath::Clamp(WeatherAmount, 0.0f, 1.0f);

    // Scale pillar based on ruin state
    switch (RuinData.RuinState)
    {
        case EArch_RuinState::Intact:
            PillarMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
            break;
        case EArch_RuinState::Cracked:
            PillarMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.85f));
            break;
        case EArch_RuinState::HalfBuried:
            // Sink base into ground to simulate burial
            BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));
            PillarMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.6f));
            break;
        case EArch_RuinState::Collapsed:
            PillarMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 85.0f));
            PillarMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.4f));
            break;
        default:
            break;
    }
}

EArch_RuinState AArchRuinPillarActor::GetRuinState() const
{
    return RuinData.RuinState;
}

void AArchRuinPillarActor::LogRuinStatus()
{
    UE_LOG(LogTemp, Log, TEXT("[ArchRuinPillar] Actor: %s | State: %d | Weathering: %.2f | Moss: %.2f | Biome: (%.0f, %.0f)"),
        *GetActorLabel(),
        (int32)RuinData.RuinState,
        RuinData.WeatheringLevel,
        RuinData.MossCoverage,
        BiomeX,
        BiomeY);
}
