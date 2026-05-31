#include "Arch_StructuralManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"

AArch_StructuralManager::AArch_StructuralManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default values
    StructureConfig = FArch_StructureConfig();
    LastWeatheringUpdate = 0.0f;
    bIsInitialized = false;

    // Set default scale
    SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
}

void AArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structure appearance
    UpdateStructureAppearance();
    ApplyMaterialVariations();
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Initialized structure of type %d"), 
           static_cast<int32>(StructureConfig.StructureType));
}

void AArch_StructuralManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsInitialized)
        return;

    // Update weathering over time (very slow process)
    LastWeatheringUpdate += DeltaTime;
    if (LastWeatheringUpdate > 60.0f) // Update every minute
    {
        if (StructureConfig.WeatheringLevel < 1.0f)
        {
            StructureConfig.WeatheringLevel += 0.001f; // Very gradual weathering
            ApplyMaterialVariations();
        }
        LastWeatheringUpdate = 0.0f;
    }
}

void AArch_StructuralManager::SetStructureType(EArch_StructureType NewType)
{
    StructureConfig.StructureType = NewType;
    UpdateStructureAppearance();

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Changed structure type to %d"), 
           static_cast<int32>(NewType));
}

void AArch_StructuralManager::ApplyWeathering(float WeatheringAmount)
{
    StructureConfig.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    ApplyMaterialVariations();

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Applied weathering level %.2f"), 
           StructureConfig.WeatheringLevel);
}

void AArch_StructuralManager::ToggleMossOvergrowth(bool bEnable)
{
    StructureConfig.bHasMossOvergrowth = bEnable;
    ApplyMaterialVariations();

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Moss overgrowth %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AArch_StructuralManager::AddCarvedSymbols(bool bEnable)
{
    StructureConfig.bHasCarvedSymbols = bEnable;
    ApplyMaterialVariations();

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Carved symbols %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AArch_StructuralManager::GenerateRandomStructure()
{
    // Randomly select structure type
    int32 RandomType = FMath::RandRange(0, 5);
    StructureConfig.StructureType = static_cast<EArch_StructureType>(RandomType);

    // Random scale variations
    float ScaleVariation = FMath::RandRange(0.7f, 1.5f);
    StructureConfig.Scale = FVector(ScaleVariation, ScaleVariation, ScaleVariation);

    // Random weathering
    StructureConfig.WeatheringLevel = FMath::RandRange(0.2f, 0.9f);

    // Random features
    StructureConfig.bHasMossOvergrowth = FMath::RandBool();
    StructureConfig.bHasCarvedSymbols = FMath::RandRange(0.0f, 1.0f) > 0.7f; // 30% chance

    // Apply changes
    UpdateStructureAppearance();
    ApplyMaterialVariations();

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Generated random structure"));
}

FVector AArch_StructuralManager::GetStructureBounds() const
{
    if (StructureMesh && StructureMesh->GetStaticMesh())
    {
        FBoxSphereBounds Bounds = StructureMesh->GetStaticMesh()->GetBounds();
        return Bounds.BoxExtent * 2.0f * StructureConfig.Scale;
    }
    return FVector(100.0f, 100.0f, 100.0f); // Default bounds
}

bool AArch_StructuralManager::IsStructureStable() const
{
    // Structure becomes less stable as weathering increases
    float StabilityThreshold = 0.8f;
    return StructureConfig.WeatheringLevel < StabilityThreshold;
}

void AArch_StructuralManager::UpdateStructureAppearance()
{
    if (!StructureMesh)
        return;

    // Apply scale based on structure config
    SetActorScale3D(StructureConfig.Scale);

    // Different mesh setups based on structure type
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StonePillar:
            // Tall and narrow
            StructureMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 3.0f));
            break;
        case EArch_StructureType::StoneWall:
            // Wide and medium height
            StructureMesh->SetRelativeScale3D(FVector(3.0f, 0.3f, 1.5f));
            break;
        case EArch_StructureType::StoneArch:
            // Arch-like proportions
            StructureMesh->SetRelativeScale3D(FVector(2.0f, 0.5f, 2.5f));
            break;
        case EArch_StructureType::Ruins:
            // Irregular, partially collapsed
            StructureMesh->SetRelativeScale3D(FVector(1.5f, 1.2f, 0.8f));
            break;
        case EArch_StructureType::ShelterEntrance:
            // Low and wide
            StructureMesh->SetRelativeScale3D(FVector(2.5f, 1.0f, 1.2f));
            break;
        case EArch_StructureType::CaveOpening:
            // Natural opening proportions
            StructureMesh->SetRelativeScale3D(FVector(1.8f, 0.8f, 2.0f));
            break;
    }
}

void AArch_StructuralManager::ApplyMaterialVariations()
{
    if (!StructureMesh)
        return;

    // Apply weathered materials based on weathering level
    if (WeatheredMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::FloorToInt(StructureConfig.WeatheringLevel * WeatheredMaterials.Num());
        MaterialIndex = FMath::Clamp(MaterialIndex, 0, WeatheredMaterials.Num() - 1);
        
        if (WeatheredMaterials[MaterialIndex])
        {
            StructureMesh->SetMaterial(0, WeatheredMaterials[MaterialIndex]);
        }
    }

    // Apply mossy materials if enabled
    if (StructureConfig.bHasMossOvergrowth && MossyMaterials.Num() > 0)
    {
        int32 MossIndex = FMath::RandRange(0, MossyMaterials.Num() - 1);
        if (MossyMaterials[MossIndex])
        {
            StructureMesh->SetMaterial(1, MossyMaterials[MossIndex]);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralManager: Applied material variations"));
}