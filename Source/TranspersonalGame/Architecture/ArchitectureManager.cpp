#include "ArchitectureManager.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// UArch_StructureComponent
// ============================================================

UArch_StructureComponent::UArch_StructureComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    StructureType = EArch_StructureType::None;
    WeatheringLevel = 0.5f;
    bHasVegetationGrowth = true;
}

void UArch_StructureComponent::InitializeStructure(EArch_StructureType InType, float InWeathering)
{
    StructureType = InType;
    WeatheringLevel = FMath::Clamp(InWeathering, 0.0f, 1.0f);
}

EArch_StructureType UArch_StructureComponent::GetStructureType() const
{
    return StructureType;
}

float UArch_StructureComponent::GetWeatheringLevel() const
{
    return WeatheringLevel;
}

// ============================================================
// AArch_CretaceousRuin
// ============================================================

AArch_CretaceousRuin::AArch_CretaceousRuin()
{
    PrimaryActorTick.bCanEverTick = false;

    RuinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RuinMesh"));
    RootComponent = RuinMesh;

    StructureComponent = CreateDefaultSubobject<UArch_StructureComponent>(TEXT("StructureComponent"));

    CollapseRadius = 500.0f;
    bStructureIntact = true;

    StructureData.StructureType = EArch_StructureType::RuinPillar;
    StructureData.WeatheringLevel = 0.7f;
    StructureData.bHasVegetationGrowth = true;
    StructureData.Scale = FVector(0.4f, 0.4f, 4.0f);
}

void AArch_CretaceousRuin::BeginPlay()
{
    Super::BeginPlay();

    if (StructureComponent)
    {
        StructureComponent->InitializeStructure(
            StructureData.StructureType,
            StructureData.WeatheringLevel
        );
    }
}

void AArch_CretaceousRuin::ApplyWeathering(float WeatherAmount)
{
    if (!StructureComponent) return;

    float NewWeathering = FMath::Clamp(
        StructureComponent->GetWeatheringLevel() + WeatherAmount,
        0.0f, 1.0f
    );
    StructureComponent->InitializeStructure(
        StructureComponent->GetStructureType(),
        NewWeathering
    );

    // If fully weathered, mark as not intact
    if (NewWeathering >= 1.0f)
    {
        bStructureIntact = false;
    }
}

bool AArch_CretaceousRuin::IsStructureIntact() const
{
    return bStructureIntact;
}

FVector AArch_CretaceousRuin::GetStructureCenter() const
{
    return GetActorLocation();
}
