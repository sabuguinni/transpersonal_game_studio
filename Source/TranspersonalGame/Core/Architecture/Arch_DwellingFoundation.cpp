#include "Arch_DwellingFoundation.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AArch_DwellingFoundation::AArch_DwellingFoundation()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create foundation mesh component
    FoundationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoundationMesh"));
    FoundationMesh->SetupAttachment(RootComponent);

    // Create interaction bounds
    InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
    InteractionBounds->SetupAttachment(RootComponent);
    InteractionBounds->SetBoxExtent(FVector(300.0f, 400.0f, 100.0f));

    // Set default foundation data
    FoundationData = FArch_FoundationData();

    // Try to load default static mesh (cube as fallback)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        FoundationMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Set default scale for foundation appearance
    FoundationMesh->SetWorldScale3D(FVector(4.0f, 6.0f, 0.5f));
}

void AArch_DwellingFoundation::BeginPlay()
{
    Super::BeginPlay();
    
    GenerateFoundation();
    UpdateMaterialBasedOnWeathering();
    SetupCollision();
}

void AArch_DwellingFoundation::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradual weathering over time (very slow)
    if (FoundationData.WeatheringLevel < 1.0f)
    {
        FoundationData.WeatheringLevel += DeltaTime * 0.00001f; // Very slow weathering
        FoundationData.WeatheringLevel = FMath::Clamp(FoundationData.WeatheringLevel, 0.0f, 1.0f);
    }

    // Gradual moss growth in humid conditions
    if (FoundationData.MossGrowth < 0.8f)
    {
        FoundationData.MossGrowth += DeltaTime * 0.00005f; // Slightly faster moss growth
        FoundationData.MossGrowth = FMath::Clamp(FoundationData.MossGrowth, 0.0f, 1.0f);
    }
}

void AArch_DwellingFoundation::GenerateFoundation()
{
    if (!FoundationMesh)
    {
        return;
    }

    // Scale foundation mesh based on data
    FVector NewScale = FVector(
        FoundationData.Width / 100.0f,
        FoundationData.Length / 100.0f,
        FoundationData.Height / 100.0f
    );
    FoundationMesh->SetWorldScale3D(NewScale);

    // Update interaction bounds
    if (InteractionBounds)
    {
        InteractionBounds->SetBoxExtent(FVector(
            FoundationData.Width * 0.6f,
            FoundationData.Length * 0.6f,
            FoundationData.Height * 2.0f
        ));
    }

    CreateFoundationGeometry();
}

void AArch_DwellingFoundation::UpdateWeathering(float NewWeatheringLevel)
{
    FoundationData.WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
    UpdateMaterialBasedOnWeathering();
}

void AArch_DwellingFoundation::UpdateMossGrowth(float NewMossLevel)
{
    FoundationData.MossGrowth = FMath::Clamp(NewMossLevel, 0.0f, 1.0f);
    UpdateMaterialBasedOnWeathering();
}

void AArch_DwellingFoundation::SetFoundationDimensions(float NewWidth, float NewLength, float NewHeight)
{
    FoundationData.Width = FMath::Max(NewWidth, 100.0f);
    FoundationData.Length = FMath::Max(NewLength, 100.0f);
    FoundationData.Height = FMath::Max(NewHeight, 20.0f);
    
    GenerateFoundation();
}

FVector AArch_DwellingFoundation::GetFoundationCenter() const
{
    return GetActorLocation();
}

bool AArch_DwellingFoundation::IsPointOnFoundation(const FVector& WorldPosition) const
{
    FVector LocalPosition = GetActorTransform().InverseTransformPosition(WorldPosition);
    
    return (FMath::Abs(LocalPosition.X) <= FoundationData.Width * 0.5f &&
            FMath::Abs(LocalPosition.Y) <= FoundationData.Length * 0.5f &&
            LocalPosition.Z >= -FoundationData.Height * 0.5f &&
            LocalPosition.Z <= FoundationData.Height * 1.5f);
}

TArray<FVector> AArch_DwellingFoundation::GetCornerPositions() const
{
    TArray<FVector> Corners;
    FVector Center = GetActorLocation();
    float HalfWidth = FoundationData.Width * 0.5f;
    float HalfLength = FoundationData.Length * 0.5f;

    Corners.Add(Center + FVector(HalfWidth, HalfLength, 0.0f));
    Corners.Add(Center + FVector(-HalfWidth, HalfLength, 0.0f));
    Corners.Add(Center + FVector(-HalfWidth, -HalfLength, 0.0f));
    Corners.Add(Center + FVector(HalfWidth, -HalfLength, 0.0f));

    return Corners;
}

void AArch_DwellingFoundation::UpdateMaterialBasedOnWeathering()
{
    if (!FoundationMesh)
    {
        return;
    }

    // Choose material based on weathering and moss levels
    UMaterialInterface* TargetMaterial = StoneMaterial;

    if (FoundationData.MossGrowth > 0.6f)
    {
        TargetMaterial = MossyStoneMaterial;
    }
    else if (FoundationData.WeatheringLevel > 0.7f)
    {
        TargetMaterial = WeatheredStoneMaterial;
    }

    if (TargetMaterial)
    {
        FoundationMesh->SetMaterial(0, TargetMaterial);
    }
}

void AArch_DwellingFoundation::SetupCollision()
{
    if (FoundationMesh)
    {
        FoundationMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        FoundationMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        FoundationMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    }

    if (InteractionBounds)
    {
        InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionBounds->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        InteractionBounds->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        InteractionBounds->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }
}

void AArch_DwellingFoundation::CreateFoundationGeometry()
{
    // This method would create procedural foundation geometry
    // For now, we use the scaled static mesh
    // Future implementation could generate individual stone blocks
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Foundation created: %dx%d stones, weathering %.1f%%, moss %.1f%%"),
                static_cast<int32>(FoundationData.Width/50), 
                static_cast<int32>(FoundationData.Length/50),
                FoundationData.WeatheringLevel * 100.0f,
                FoundationData.MossGrowth * 100.0f));
    }
}