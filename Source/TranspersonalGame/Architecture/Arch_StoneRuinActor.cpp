#include "Arch_StoneRuinActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AArch_StoneRuinActor::AArch_StoneRuinActor()
{
    PrimaryActorTick.bCanEverTick = false;

    RuinMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RuinMesh"));
    RootComponent = RuinMeshComponent;

    RuinMeshComponent->SetCastShadow(true);
    RuinMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RuinMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // Default weathering config
    RuinConfig.RuinType = EArch_RuinType::Pillar;
    RuinConfig.WeatheringLevel = 0.5f;
    RuinConfig.bHasMossGrowth = true;
    RuinConfig.HeightScale = 1.0f;
}

void AArch_StoneRuinActor::BeginPlay()
{
    Super::BeginPlay();
    SetupMeshForRuinType();
}

void AArch_StoneRuinActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    SetupMeshForRuinType();
}

void AArch_StoneRuinActor::SetupMeshForRuinType()
{
    if (!RuinMeshComponent)
    {
        return;
    }

    // Apply scale based on ruin type
    FVector Scale = FVector(1.0f, 1.0f, RuinConfig.HeightScale);

    switch (RuinConfig.RuinType)
    {
        case EArch_RuinType::Pillar:
            Scale = FVector(0.5f, 0.5f, 3.0f * RuinConfig.HeightScale);
            break;
        case EArch_RuinType::Wall:
            Scale = FVector(4.0f, 0.5f, 2.0f * RuinConfig.HeightScale);
            break;
        case EArch_RuinType::Archway:
            Scale = FVector(3.0f, 0.8f, 2.5f * RuinConfig.HeightScale);
            break;
        case EArch_RuinType::Foundation:
            Scale = FVector(5.0f, 5.0f, 0.3f);
            break;
        case EArch_RuinType::Rubble:
            Scale = FVector(1.5f, 1.5f, 0.8f);
            break;
        default:
            break;
    }

    SetActorScale3D(Scale);
    RuinMeshComponent->SetCastShadow(bCastsDynamicShadow);
}

void AArch_StoneRuinActor::ApplyWeathering(float Level)
{
    RuinConfig.WeatheringLevel = FMath::Clamp(Level, 0.0f, 1.0f);
    // Weathering level drives material parameter — material instance handles visual
    if (RuinMeshComponent)
    {
        UMaterialInstanceDynamic* DynMat = RuinMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMat)
        {
            DynMat->SetScalarParameterValue(FName("WeatheringLevel"), RuinConfig.WeatheringLevel);
            DynMat->SetScalarParameterValue(FName("MossAmount"), RuinConfig.bHasMossGrowth ? RuinConfig.WeatheringLevel : 0.0f);
        }
    }
}

EArch_RuinType AArch_StoneRuinActor::GetRuinType() const
{
    return RuinConfig.RuinType;
}

FVector AArch_StoneRuinActor::GetRuinExtents() const
{
    if (RuinMeshComponent)
    {
        FBoxSphereBounds Bounds = RuinMeshComponent->CalcBounds(RuinMeshComponent->GetComponentTransform());
        return Bounds.BoxExtent;
    }
    return FVector::ZeroVector;
}
