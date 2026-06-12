#include "Arch_CretaceousStructures.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AArch_CretaceousStructures::AArch_CretaceousStructures()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create main structure mesh
    MainStructure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructure"));
    MainStructure->SetupAttachment(RootComponent);
    MainStructure->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructure->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create instanced detail elements
    DetailElements = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DetailElements"));
    DetailElements->SetupAttachment(RootComponent);
    DetailElements->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Initialize default settings
    StructureSettings.StructureType = EArch_StructureType::StoneColumn;
    StructureSettings.BaseScale = FVector(1.0f, 1.0f, 1.0f);
    StructureSettings.WeatheringLevel = 0.5f;
    StructureSettings.bHasVegetationGrowth = true;
    StructureSettings.StructuralIntegrity = 1.0f;
}

void AArch_CretaceousStructures::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial structure configuration
    SetStructureType(StructureSettings.StructureType);
    ApplyWeathering(StructureSettings.WeatheringLevel);
    
    if (StructureSettings.bHasVegetationGrowth)
    {
        GenerateVegetationGrowth();
    }

    // Generate climbable points for player interaction
    GenerateDetailElements();
}

void AArch_CretaceousStructures::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic weathering updates for dynamic aging
    LastWeatheringUpdate += DeltaTime;
    if (LastWeatheringUpdate >= WeatheringUpdateInterval)
    {
        LastWeatheringUpdate = 0.0f;
        
        // Gradual weathering over time
        if (StructureSettings.WeatheringLevel < 1.0f)
        {
            StructureSettings.WeatheringLevel += 0.001f; // Very slow weathering
            ApplyMaterialVariation();
        }
    }
}

void AArch_CretaceousStructures::SetStructureType(EArch_StructureType NewType)
{
    StructureSettings.StructureType = NewType;

    // Apply type-specific properties
    switch (NewType)
    {
        case EArch_StructureType::StoneColumn:
            StructureSettings.BaseScale = FVector(1.0f, 1.0f, 2.5f);
            StructureSettings.StructuralIntegrity = 0.9f;
            break;

        case EArch_StructureType::RockFormation:
            StructureSettings.BaseScale = FVector(2.0f, 2.0f, 1.5f);
            StructureSettings.StructuralIntegrity = 0.8f;
            break;

        case EArch_StructureType::NaturalArch:
            StructureSettings.BaseScale = FVector(3.0f, 1.0f, 2.0f);
            StructureSettings.StructuralIntegrity = 0.7f;
            break;

        case EArch_StructureType::CaveSystem:
            StructureSettings.BaseScale = FVector(4.0f, 4.0f, 3.0f);
            StructureSettings.StructuralIntegrity = 1.0f;
            break;

        case EArch_StructureType::CliffFace:
            StructureSettings.BaseScale = FVector(8.0f, 2.0f, 6.0f);
            StructureSettings.StructuralIntegrity = 0.95f;
            break;
    }

    // Apply scale and setup collision
    MainStructure->SetWorldScale3D(StructureSettings.BaseScale);
    SetupCollisionForStructureType();
    ApplyMaterialVariation();
}

void AArch_CretaceousStructures::ApplyWeathering(float WeatheringAmount)
{
    StructureSettings.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    ApplyMaterialVariation();
}

void AArch_CretaceousStructures::GenerateVegetationGrowth()
{
    if (!StructureSettings.bHasVegetationGrowth || VegetationMeshes.Num() == 0)
        return;

    // Clear existing vegetation instances
    DetailElements->ClearInstances();

    // Generate vegetation based on weathering level
    int32 VegetationCount = FMath::RoundToInt(StructureSettings.WeatheringLevel * 20.0f);
    
    for (int32 i = 0; i < VegetationCount; ++i)
    {
        // Random position on structure surface
        FVector LocalPosition = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 200.0f)
        );

        // Random rotation
        FRotator RandomRotation = FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );

        // Random scale
        float RandomScale = FMath::RandRange(0.5f, 1.5f);
        FVector VegetationScale = FVector(RandomScale);

        // Create transform
        FTransform VegetationTransform(RandomRotation, LocalPosition, VegetationScale);
        
        // Add instance
        DetailElements->AddInstance(VegetationTransform);
    }
}

void AArch_CretaceousStructures::UpdateStructuralIntegrity(float NewIntegrity)
{
    StructureSettings.StructuralIntegrity = FMath::Clamp(NewIntegrity, 0.0f, 1.0f);
    
    // Visual feedback for structural integrity
    if (StructureSettings.StructuralIntegrity < 0.3f)
    {
        // Structure is heavily damaged - increase weathering
        StructureSettings.WeatheringLevel = FMath::Max(StructureSettings.WeatheringLevel, 0.8f);
        ApplyMaterialVariation();
    }
}

TArray<FVector> AArch_CretaceousStructures::GetClimbablePoints() const
{
    return ClimbablePoints;
}

void AArch_CretaceousStructures::RegenerateStructure()
{
    SetStructureType(StructureSettings.StructureType);
    ApplyWeathering(StructureSettings.WeatheringLevel);
    GenerateVegetationGrowth();
    GenerateDetailElements();
}

void AArch_CretaceousStructures::ApplyMaterialVariation()
{
    if (StoneMaterials.Num() == 0 && WeatheredMaterials.Num() == 0)
        return;

    UMaterialInterface* SelectedMaterial = nullptr;

    // Choose material based on weathering level
    if (StructureSettings.WeatheringLevel > 0.6f && WeatheredMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::RandRange(0, WeatheredMaterials.Num() - 1);
        SelectedMaterial = WeatheredMaterials[MaterialIndex];
    }
    else if (StoneMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::RandRange(0, StoneMaterials.Num() - 1);
        SelectedMaterial = StoneMaterials[MaterialIndex];
    }

    if (SelectedMaterial)
    {
        MainStructure->SetMaterial(0, SelectedMaterial);
    }
}

void AArch_CretaceousStructures::SetupCollisionForStructureType()
{
    switch (StructureSettings.StructureType)
    {
        case EArch_StructureType::StoneColumn:
        case EArch_StructureType::RockFormation:
            MainStructure->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;

        case EArch_StructureType::NaturalArch:
            MainStructure->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            MainStructure->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
            break;

        case EArch_StructureType::CaveSystem:
            MainStructure->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            MainStructure->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
            break;

        case EArch_StructureType::CliffFace:
            MainStructure->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            break;
    }
}

void AArch_CretaceousStructures::GenerateDetailElements()
{
    ClimbablePoints.Empty();

    // Generate climbable points based on structure type
    switch (StructureSettings.StructureType)
    {
        case EArch_StructureType::StoneColumn:
            // Vertical climbing points
            for (int32 i = 1; i < 5; ++i)
            {
                ClimbablePoints.Add(FVector(0.0f, 0.0f, i * 50.0f));
            }
            break;

        case EArch_StructureType::CliffFace:
            // Multiple climbing routes
            for (int32 x = -2; x <= 2; ++x)
            {
                for (int32 z = 1; z < 6; ++z)
                {
                    ClimbablePoints.Add(FVector(x * 100.0f, 0.0f, z * 60.0f));
                }
            }
            break;

        case EArch_StructureType::RockFormation:
            // Scattered climbing points
            for (int32 i = 0; i < 8; ++i)
            {
                FVector Point = FVector(
                    FMath::RandRange(-150.0f, 150.0f),
                    FMath::RandRange(-150.0f, 150.0f),
                    FMath::RandRange(50.0f, 200.0f)
                );
                ClimbablePoints.Add(Point);
            }
            break;

        default:
            break;
    }
}