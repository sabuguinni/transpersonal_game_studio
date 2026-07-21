#include "PrehistoricStructure.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APrehistoricStructure::APrehistoricStructure()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Set default collision
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    StructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Initialize default configuration
    StructureConfig.StructureType = EArch_StructureType::StonePillar;
    StructureConfig.Scale = FVector(1.0f, 1.0f, 1.0f);
    StructureConfig.WeatheringLevel = 0.5f;
    StructureConfig.bHasMossGrowth = true;
    StructureConfig.BiomeType = EBiomeType::Savana;

    // Try to load default cube mesh for prototyping
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        StructureMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
}

void APrehistoricStructure::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateStructureMesh();
    UpdateMaterials();
    ApplyWeatheringEffects();
}

void APrehistoricStructure::SetStructureType(EArch_StructureType NewType)
{
    StructureConfig.StructureType = NewType;
    UpdateStructureMesh();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure: Structure type changed to %d"), (int32)NewType);
}

void APrehistoricStructure::SetWeatheringLevel(float NewLevel)
{
    StructureConfig.WeatheringLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    ApplyWeatheringEffects();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure: Weathering level set to %f"), NewLevel);
}

void APrehistoricStructure::SetMossGrowth(bool bEnabled)
{
    StructureConfig.bHasMossGrowth = bEnabled;
    UpdateMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure: Moss growth %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void APrehistoricStructure::ApplyBiomeAdaptation(EBiomeType Biome)
{
    StructureConfig.BiomeType = Biome;
    
    // Adjust weathering based on biome
    switch (Biome)
    {
        case EBiomeType::Pantano:
            StructureConfig.WeatheringLevel = FMath::Max(StructureConfig.WeatheringLevel, 0.7f);
            StructureConfig.bHasMossGrowth = true;
            break;
            
        case EBiomeType::Floresta:
            StructureConfig.WeatheringLevel = FMath::Max(StructureConfig.WeatheringLevel, 0.6f);
            StructureConfig.bHasMossGrowth = true;
            break;
            
        case EBiomeType::Deserto:
            StructureConfig.WeatheringLevel = FMath::Min(StructureConfig.WeatheringLevel, 0.3f);
            StructureConfig.bHasMossGrowth = false;
            break;
            
        case EBiomeType::Montanha:
            StructureConfig.WeatheringLevel = FMath::Max(StructureConfig.WeatheringLevel, 0.4f);
            break;
            
        default: // Savana
            break;
    }
    
    UpdateMaterials();
    ApplyWeatheringEffects();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure: Adapted to biome %d"), (int32)Biome);
}

void APrehistoricStructure::RegenerateStructure()
{
    UpdateStructureMesh();
    UpdateMaterials();
    ApplyWeatheringEffects();
    
    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure: Structure regenerated"));
}

void APrehistoricStructure::UpdateStructureMesh()
{
    if (!StructureMesh)
    {
        return;
    }

    // Apply scale based on structure type
    FVector NewScale = StructureConfig.Scale;
    
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StonePillar:
            NewScale = FVector(0.8f, 0.8f, 2.5f) * StructureConfig.Scale.X;
            break;
            
        case EArch_StructureType::RockShelter:
            NewScale = FVector(4.0f, 3.0f, 2.0f) * StructureConfig.Scale.X;
            break;
            
        case EArch_StructureType::StoneCircle:
            NewScale = FVector(1.2f, 1.2f, 1.8f) * StructureConfig.Scale.X;
            break;
            
        case EArch_StructureType::CaveEntrance:
            NewScale = FVector(3.5f, 2.0f, 3.0f) * StructureConfig.Scale.X;
            break;
            
        case EArch_StructureType::TribalHut:
            NewScale = FVector(3.0f, 3.0f, 2.5f) * StructureConfig.Scale.X;
            break;
    }
    
    StructureMesh->SetWorldScale3D(NewScale);
}

void APrehistoricStructure::UpdateMaterials()
{
    if (!StructureMesh)
    {
        return;
    }

    // Apply material based on moss growth and weathering
    UMaterialInterface* MaterialToApply = BaseStoneMaterial;
    
    if (StructureConfig.bHasMossGrowth && StructureConfig.WeatheringLevel > 0.4f)
    {
        MaterialToApply = MossyStoneMaterial;
    }
    else if (StructureConfig.WeatheringLevel > 0.5f)
    {
        MaterialToApply = WeatheredStoneMaterial;
    }
    
    if (MaterialToApply)
    {
        StructureMesh->SetMaterial(0, MaterialToApply);
    }
}

void APrehistoricStructure::ApplyWeatheringEffects()
{
    if (!StructureMesh)
    {
        return;
    }

    // Apply weathering through material parameters if available
    UMaterialInstanceDynamic* DynamicMaterial = StructureMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), StructureConfig.WeatheringLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), StructureConfig.bHasMossGrowth ? 1.0f : 0.0f);
        
        // Biome-specific color tinting
        FLinearColor BiomeColor = FLinearColor::White;
        switch (StructureConfig.BiomeType)
        {
            case EBiomeType::Pantano:
                BiomeColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f); // Greenish tint
                break;
            case EBiomeType::Floresta:
                BiomeColor = FLinearColor(0.7f, 0.9f, 0.6f, 1.0f); // Forest green
                break;
            case EBiomeType::Deserto:
                BiomeColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Sandy tint
                break;
            case EBiomeType::Montanha:
                BiomeColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f); // Cool mountain tint
                break;
            default: // Savana
                BiomeColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Warm savanna tint
                break;
        }
        
        DynamicMaterial->SetVectorParameterValue(TEXT("BiomeTint"), BiomeColor);
    }
}