#include "ArchitecturalManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AArchitecturalManager::AArchitecturalManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize structure data
    StructureData.StructureType = EArch_StructureType::Platform;
    StructureData.Dimensions = FVector(400.0f, 400.0f, 100.0f);
    StructureData.WeatheringLevel = 0.5f;
    StructureData.bHasMossGrowth = true;
    StructureData.bHasCarvings = false;
    StructureData.BiomeType = EBiomeType::Savanna;

    // Load default meshes (using engine defaults for now)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        PlatformMeshes.Add(CubeMeshAsset.Object);
        PillarMeshes.Add(CubeMeshAsset.Object);
        ArchwayMeshes.Add(CubeMeshAsset.Object);
    }

    // Set default mesh
    if (PlatformMeshes.Num() > 0)
    {
        StructureMesh->SetStaticMesh(PlatformMeshes[0]);
    }
}

void AArchitecturalManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial structure
    GenerateStructure();
    ApplyBiomeSpecificMaterials();
}

void AArchitecturalManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Add any dynamic behavior here (weathering over time, etc.)
}

void AArchitecturalManager::SetStructureType(EArch_StructureType NewType)
{
    if (StructureData.StructureType != NewType)
    {
        StructureData.StructureType = NewType;
        UpdateStructureMesh();
    }
}

void AArchitecturalManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    ApplyBiomeSpecificMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Applied weathering level: %f"), StructureData.WeatheringLevel);
}

void AArchitecturalManager::SetBiomeAdaptation(EBiomeType BiomeType)
{
    StructureData.BiomeType = BiomeType;
    ApplyBiomeSpecificMaterials();
    
    // Adjust structure properties based on biome
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
            StructureData.bHasMossGrowth = true;
            StructureData.WeatheringLevel = FMath::Max(StructureData.WeatheringLevel, 0.7f);
            break;
        case EBiomeType::Desert:
            StructureData.bHasMossGrowth = false;
            StructureData.WeatheringLevel = FMath::Max(StructureData.WeatheringLevel, 0.8f);
            break;
        case EBiomeType::Forest:
            StructureData.bHasMossGrowth = true;
            StructureData.bHasCarvings = true;
            break;
        case EBiomeType::Mountain:
            StructureData.WeatheringLevel = FMath::Min(StructureData.WeatheringLevel, 0.3f);
            break;
        default:
            break;
    }
}

void AArchitecturalManager::GenerateStructure()
{
    UpdateStructureMesh();
    ApplyBiomeSpecificMaterials();
    
    // Apply scale based on dimensions
    FVector Scale = StructureData.Dimensions / 100.0f; // Normalize to cube size
    StructureMesh->SetWorldScale3D(Scale);
    
    UE_LOG(LogTemp, Log, TEXT("Generated structure type: %d"), (int32)StructureData.StructureType);
}

void AArchitecturalManager::AddMossGrowth(bool bEnable)
{
    StructureData.bHasMossGrowth = bEnable;
    ApplyBiomeSpecificMaterials();
}

void AArchitecturalManager::AddCarvings(bool bEnable)
{
    StructureData.bHasCarvings = bEnable;
    ApplyBiomeSpecificMaterials();
}

void AArchitecturalManager::RegenerateStructure()
{
    GenerateStructure();
}

void AArchitecturalManager::UpdateStructureMesh()
{
    UStaticMesh* NewMesh = GetMeshForStructureType(StructureData.StructureType);
    if (NewMesh && StructureMesh)
    {
        StructureMesh->SetStaticMesh(NewMesh);
    }
}

void AArchitecturalManager::ApplyBiomeSpecificMaterials()
{
    if (!StructureMesh)
    {
        return;
    }

    UMaterialInterface* Material = GetMaterialForBiome(StructureData.BiomeType, StructureData.WeatheringLevel > 0.5f);
    if (Material)
    {
        StructureMesh->SetMaterial(0, Material);
    }
}

UStaticMesh* AArchitecturalManager::GetMeshForStructureType(EArch_StructureType Type)
{
    switch (Type)
    {
        case EArch_StructureType::Platform:
            return PlatformMeshes.Num() > 0 ? PlatformMeshes[0] : nullptr;
        case EArch_StructureType::Pillar:
            return PillarMeshes.Num() > 0 ? PillarMeshes[0] : nullptr;
        case EArch_StructureType::Archway:
            return ArchwayMeshes.Num() > 0 ? ArchwayMeshes[0] : nullptr;
        default:
            return PlatformMeshes.Num() > 0 ? PlatformMeshes[0] : nullptr;
    }
}

UMaterialInterface* AArchitecturalManager::GetMaterialForBiome(EBiomeType BiomeType, bool bWeathered)
{
    // Return appropriate material based on biome and weathering
    // For now, return nullptr to use default material
    // In production, this would select from StoneMaterials or WeatheredMaterials arrays
    
    if (bWeathered && WeatheredMaterials.Num() > 0)
    {
        int32 MaterialIndex = (int32)BiomeType % WeatheredMaterials.Num();
        return WeatheredMaterials[MaterialIndex];
    }
    else if (StoneMaterials.Num() > 0)
    {
        int32 MaterialIndex = (int32)BiomeType % StoneMaterials.Num();
        return StoneMaterials[MaterialIndex];
    }
    
    return nullptr;
}