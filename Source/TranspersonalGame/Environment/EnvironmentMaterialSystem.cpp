#include "EnvironmentMaterialSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "LandscapeComponent.h"

AEnvironmentMaterialSystem::AEnvironmentMaterialSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    GlobalWetness = 0.0f;
    GlobalWindStrength = 1.0f;
    GlobalColorTint = FLinearColor::White;

    // Initialize material systems
    InitializeEnvironmentMaterials();
    InitializeBiomeMaterialSets();
}

void AEnvironmentMaterialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Initialized with %d materials and %d biome sets"), 
           EnvironmentMaterials.Num(), BiomeMaterialSets.Num());
}

void AEnvironmentMaterialSystem::InitializeEnvironmentMaterials()
{
    EnvironmentMaterials.Empty();

    // Landscape Materials
    FEnvironmentMaterialData GrassTerrain;
    GrassTerrain.MaterialType = EEnvironmentMaterialType::Landscape_Grass;
    GrassTerrain.BaseColor = FLinearColor(0.2f, 0.4f, 0.1f, 1.0f); // Dark green
    GrassTerrain.Roughness = 0.9f;
    GrassTerrain.Metallic = 0.0f;
    GrassTerrain.TextureScale = 4.0f;
    GrassTerrain.WindStrength = 0.8f;
    EnvironmentMaterials.Add(GrassTerrain);

    FEnvironmentMaterialData DirtTerrain;
    DirtTerrain.MaterialType = EEnvironmentMaterialType::Landscape_Dirt;
    DirtTerrain.BaseColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Brown
    DirtTerrain.Roughness = 0.8f;
    DirtTerrain.Metallic = 0.0f;
    DirtTerrain.TextureScale = 2.0f;
    EnvironmentMaterials.Add(DirtTerrain);

    FEnvironmentMaterialData RockTerrain;
    RockTerrain.MaterialType = EEnvironmentMaterialType::Landscape_Rock;
    RockTerrain.BaseColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f); // Gray
    RockTerrain.Roughness = 0.7f;
    RockTerrain.Metallic = 0.0f;
    RockTerrain.TextureScale = 1.0f;
    EnvironmentMaterials.Add(RockTerrain);

    FEnvironmentMaterialData MudTerrain;
    MudTerrain.MaterialType = EEnvironmentMaterialType::Landscape_Mud;
    MudTerrain.BaseColor = FLinearColor(0.3f, 0.25f, 0.15f, 1.0f); // Dark brown
    MudTerrain.Roughness = 0.6f;
    MudTerrain.Metallic = 0.0f;
    MudTerrain.TextureScale = 3.0f;
    MudTerrain.WetnessAmount = 0.8f;
    EnvironmentMaterials.Add(MudTerrain);

    FEnvironmentMaterialData SandTerrain;
    SandTerrain.MaterialType = EEnvironmentMaterialType::Landscape_Sand;
    SandTerrain.BaseColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f); // Sandy yellow
    SandTerrain.Roughness = 0.9f;
    SandTerrain.Metallic = 0.0f;
    SandTerrain.TextureScale = 2.0f;
    EnvironmentMaterials.Add(SandTerrain);

    // Vegetation Materials
    FEnvironmentMaterialData TreeBark;
    TreeBark.MaterialType = EEnvironmentMaterialType::Vegetation_Bark;
    TreeBark.BaseColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Dark brown
    TreeBark.Roughness = 0.9f;
    TreeBark.Metallic = 0.0f;
    TreeBark.TextureScale = 1.0f;
    EnvironmentMaterials.Add(TreeBark);

    FEnvironmentMaterialData Leaves;
    Leaves.MaterialType = EEnvironmentMaterialType::Vegetation_Leaves;
    Leaves.BaseColor = FLinearColor(0.3f, 0.6f, 0.2f, 1.0f); // Green
    Leaves.Roughness = 0.8f;
    Leaves.Metallic = 0.0f;
    Leaves.TextureScale = 0.5f;
    Leaves.WindStrength = 1.2f;
    EnvironmentMaterials.Add(Leaves);

    // Rock Materials
    FEnvironmentMaterialData WeatheredRock;
    WeatheredRock.MaterialType = EEnvironmentMaterialType::Rock_Weathered;
    WeatheredRock.BaseColor = FLinearColor(0.4f, 0.4f, 0.35f, 1.0f); // Weathered gray
    WeatheredRock.Roughness = 0.8f;
    WeatheredRock.Metallic = 0.0f;
    WeatheredRock.TextureScale = 1.0f;
    EnvironmentMaterials.Add(WeatheredRock);

    FEnvironmentMaterialData MossCoveredRock;
    MossCoveredRock.MaterialType = EEnvironmentMaterialType::Rock_Moss_Covered;
    MossCoveredRock.BaseColor = FLinearColor(0.25f, 0.4f, 0.2f, 1.0f); // Mossy green-gray
    MossCoveredRock.Roughness = 0.9f;
    MossCoveredRock.Metallic = 0.0f;
    MossCoveredRock.TextureScale = 0.8f;
    MossCoveredRock.WetnessAmount = 0.6f;
    EnvironmentMaterials.Add(MossCoveredRock);

    // Special Materials
    FEnvironmentMaterialData GlowingCrystal;
    GlowingCrystal.MaterialType = EEnvironmentMaterialType::Crystal_Glowing;
    GlowingCrystal.BaseColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f); // Bright blue-white
    GlowingCrystal.Roughness = 0.1f;
    GlowingCrystal.Metallic = 0.0f;
    GlowingCrystal.TextureScale = 0.5f;
    EnvironmentMaterials.Add(GlowingCrystal);

    FEnvironmentMaterialData AgedBone;
    AgedBone.MaterialType = EEnvironmentMaterialType::Bone_Aged;
    AgedBone.BaseColor = FLinearColor(0.8f, 0.75f, 0.6f, 1.0f); // Aged bone white
    AgedBone.Roughness = 0.7f;
    AgedBone.Metallic = 0.0f;
    AgedBone.TextureScale = 1.0f;
    EnvironmentMaterials.Add(AgedBone);

    // Water Materials
    FEnvironmentMaterialData StreamWater;
    StreamWater.MaterialType = EEnvironmentMaterialType::Water_Stream;
    StreamWater.BaseColor = FLinearColor(0.1f, 0.3f, 0.4f, 0.8f); // Clear blue water
    StreamWater.Roughness = 0.0f;
    StreamWater.Metallic = 0.0f;
    StreamWater.TextureScale = 2.0f;
    EnvironmentMaterials.Add(StreamWater);

    FEnvironmentMaterialData MuddyWater;
    MuddyWater.MaterialType = EEnvironmentMaterialType::Water_Muddy;
    MuddyWater.BaseColor = FLinearColor(0.3f, 0.25f, 0.15f, 0.9f); // Muddy brown water
    MuddyWater.Roughness = 0.2f;
    MuddyWater.Metallic = 0.0f;
    MuddyWater.TextureScale = 3.0f;
    EnvironmentMaterials.Add(MuddyWater);

    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Initialized %d environment materials"), EnvironmentMaterials.Num());
}

void AEnvironmentMaterialSystem::InitializeBiomeMaterialSets()
{
    BiomeMaterialSets.Empty();

    // Dense Forest Biome
    FBiomeMaterialSet DenseForest;
    DenseForest.BiomeName = TEXT("DenseForest");
    DenseForest.PrimaryMaterials = {
        EEnvironmentMaterialType::Landscape_Grass,
        EEnvironmentMaterialType::Vegetation_Bark,
        EEnvironmentMaterialType::Vegetation_Leaves
    };
    DenseForest.SecondaryMaterials = {
        EEnvironmentMaterialType::Rock_Moss_Covered,
        EEnvironmentMaterialType::Landscape_Dirt,
        EEnvironmentMaterialType::Bone_Aged
    };
    DenseForest.DominantColor = FLinearColor(0.2f, 0.5f, 0.2f, 1.0f); // Forest green
    DenseForest.AccentColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Brown
    DenseForest.AverageWetness = 0.6f;
    DenseForest.AverageWindStrength = 0.4f;
    BiomeMaterialSets.Add(DenseForest);

    // Wetland Biome
    FBiomeMaterialSet Wetland;
    Wetland.BiomeName = TEXT("Wetland");
    Wetland.PrimaryMaterials = {
        EEnvironmentMaterialType::Landscape_Mud,
        EEnvironmentMaterialType::Water_Muddy,
        EEnvironmentMaterialType::Vegetation_Leaves
    };
    Wetland.SecondaryMaterials = {
        EEnvironmentMaterialType::Rock_Moss_Covered,
        EEnvironmentMaterialType::Bone_Aged,
        EEnvironmentMaterialType::Landscape_Grass
    };
    Wetland.DominantColor = FLinearColor(0.3f, 0.4f, 0.3f, 1.0f); // Muddy green
    Wetland.AccentColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f); // Water blue
    Wetland.AverageWetness = 0.9f;
    Wetland.AverageWindStrength = 0.3f;
    BiomeMaterialSets.Add(Wetland);

    // Highland Biome
    FBiomeMaterialSet Highland;
    Highland.BiomeName = TEXT("Highland");
    Highland.PrimaryMaterials = {
        EEnvironmentMaterialType::Landscape_Rock,
        EEnvironmentMaterialType::Rock_Weathered,
        EEnvironmentMaterialType::Crystal_Glowing
    };
    Highland.SecondaryMaterials = {
        EEnvironmentMaterialType::Landscape_Grass,
        EEnvironmentMaterialType::Vegetation_Bark,
        EEnvironmentMaterialType::Water_Stream
    };
    Highland.DominantColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Rocky gray
    Highland.AccentColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f); // Crystal blue
    Highland.AverageWetness = 0.2f;
    Highland.AverageWindStrength = 0.8f;
    BiomeMaterialSets.Add(Highland);

    // River Valley Biome
    FBiomeMaterialSet RiverValley;
    RiverValley.BiomeName = TEXT("RiverValley");
    RiverValley.PrimaryMaterials = {
        EEnvironmentMaterialType::Water_Stream,
        EEnvironmentMaterialType::Landscape_Sand,
        EEnvironmentMaterialType::Landscape_Grass
    };
    RiverValley.SecondaryMaterials = {
        EEnvironmentMaterialType::Rock_Weathered,
        EEnvironmentMaterialType::Vegetation_Leaves,
        EEnvironmentMaterialType::Landscape_Dirt
    };
    RiverValley.DominantColor = FLinearColor(0.3f, 0.5f, 0.6f, 1.0f); // Water blue-green
    RiverValley.AccentColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f); // Sandy brown
    RiverValley.AverageWetness = 0.7f;
    RiverValley.AverageWindStrength = 0.5f;
    BiomeMaterialSets.Add(RiverValley);

    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Initialized %d biome material sets"), BiomeMaterialSets.Num());
}

void AEnvironmentMaterialSystem::ApplyBiomeMaterials(const FString& BiomeName, FVector Center, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Applying biome materials for '%s' at %s (radius: %.2f)"), 
           *BiomeName, *Center.ToString(), Radius);

    FBiomeMaterialSet* BiomeSet = GetBiomeMaterialSet(BiomeName);
    if (!BiomeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentMaterialSystem: Biome '%s' not found"), *BiomeName);
        return;
    }

    // Update global environmental effects based on biome
    UpdateGlobalEnvironmentalEffects(BiomeSet->AverageWetness, BiomeSet->AverageWindStrength, BiomeSet->DominantColor);

    // Find all landscapes in the area and apply materials
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundActors);

    int32 MaterialsApplied = 0;

    for (AActor* Actor : FoundActors)
    {
        ALandscape* Landscape = Cast<ALandscape>(Actor);
        if (Landscape)
        {
            float Distance = FVector::Dist(Landscape->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                // Apply primary materials to landscape layers
                for (EEnvironmentMaterialType MaterialType : BiomeSet->PrimaryMaterials)
                {
                    if (MaterialType == EEnvironmentMaterialType::Landscape_Grass ||
                        MaterialType == EEnvironmentMaterialType::Landscape_Dirt ||
                        MaterialType == EEnvironmentMaterialType::Landscape_Rock ||
                        MaterialType == EEnvironmentMaterialType::Landscape_Mud ||
                        MaterialType == EEnvironmentMaterialType::Landscape_Sand)
                    {
                        FString LayerName = UEnum::GetValueAsString(MaterialType);
                        ApplyMaterialToLandscape(Landscape, LayerName, MaterialType);
                        MaterialsApplied++;
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Applied %d materials for biome '%s'"), MaterialsApplied, *BiomeName);
    OnBiomeMaterialsApplied(BiomeName, MaterialsApplied);
}

UMaterialInstanceDynamic* AEnvironmentMaterialSystem::CreateDynamicMaterial(EEnvironmentMaterialType MaterialType)
{
    // Check if we already have a dynamic material for this type
    if (DynamicMaterials.Contains(MaterialType))
    {
        return DynamicMaterials[MaterialType];
    }

    FEnvironmentMaterialData* MaterialData = GetMaterialData(MaterialType);
    if (!MaterialData || !MaterialData->BaseMaterial.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentMaterialSystem: No base material found for type %s"), 
               *UEnum::GetValueAsString(MaterialType));
        return nullptr;
    }

    // Create dynamic material instance
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialData->BaseMaterial.LoadSynchronous(), this);
    if (DynamicMaterial)
    {
        // Update parameters
        UpdateDynamicMaterialParameters(DynamicMaterial, *MaterialData);
        
        // Store for reuse
        DynamicMaterials.Add(MaterialType, DynamicMaterial);
        
        UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Created dynamic material for type %s"), 
               *UEnum::GetValueAsString(MaterialType));
    }

    return DynamicMaterial;
}

void AEnvironmentMaterialSystem::UpdateGlobalEnvironmentalEffects(float Wetness, float WindStrength, FLinearColor ColorTint)
{
    GlobalWetness = FMath::Clamp(Wetness, 0.0f, 1.0f);
    GlobalWindStrength = FMath::Clamp(WindStrength, 0.0f, 2.0f);
    GlobalColorTint = ColorTint;

    UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Updated global effects - Wetness: %.2f, Wind: %.2f, Tint: %s"), 
           GlobalWetness, GlobalWindStrength, *GlobalColorTint.ToString());

    // Update all existing dynamic materials
    for (auto& MaterialPair : DynamicMaterials)
    {
        if (IsValid(MaterialPair.Value))
        {
            FEnvironmentMaterialData* MaterialData = GetMaterialData(MaterialPair.Key);
            if (MaterialData)
            {
                UpdateDynamicMaterialParameters(MaterialPair.Value, *MaterialData);
            }
        }
    }
}

void AEnvironmentMaterialSystem::ApplyMaterialToLandscape(ALandscape* Landscape, const FString& LayerName, EEnvironmentMaterialType MaterialType)
{
    if (!IsValid(Landscape))
    {
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = CreateDynamicMaterial(MaterialType);
    if (DynamicMaterial)
    {
        UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Applied material %s to landscape layer %s"), 
               *UEnum::GetValueAsString(MaterialType), *LayerName);
        
        // This would typically interface with the landscape material system
        // Implementation depends on UE5's landscape API
    }
}

void AEnvironmentMaterialSystem::ApplyMaterialToStaticMesh(UStaticMeshComponent* MeshComponent, EEnvironmentMaterialType MaterialType)
{
    if (!IsValid(MeshComponent))
    {
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = CreateDynamicMaterial(MaterialType);
    if (DynamicMaterial)
    {
        MeshComponent->SetMaterial(0, DynamicMaterial);
        UE_LOG(LogTemp, Log, TEXT("EnvironmentMaterialSystem: Applied material %s to static mesh"), 
               *UEnum::GetValueAsString(MaterialType));
    }
}

FEnvironmentMaterialData* AEnvironmentMaterialSystem::GetMaterialData(EEnvironmentMaterialType MaterialType)
{
    return EnvironmentMaterials.FindByPredicate(
        [MaterialType](const FEnvironmentMaterialData& Data) { return Data.MaterialType == MaterialType; });
}

FBiomeMaterialSet* AEnvironmentMaterialSystem::GetBiomeMaterialSet(const FString& BiomeName)
{
    return BiomeMaterialSets.FindByPredicate(
        [BiomeName](const FBiomeMaterialSet& Set) { return Set.BiomeName == BiomeName; });
}

void AEnvironmentMaterialSystem::UpdateDynamicMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, const FEnvironmentMaterialData& MaterialData)
{
    if (!IsValid(DynamicMaterial))
    {
        return;
    }

    // Set base material properties
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), MaterialData.BaseColor * GlobalColorTint);
    DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), MaterialData.Roughness);
    DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), MaterialData.Metallic);
    DynamicMaterial->SetScalarParameterValue(TEXT("TextureScale"), MaterialData.TextureScale);

    // Set environmental effects
    float FinalWetness = FMath::Clamp(MaterialData.WetnessAmount + GlobalWetness, 0.0f, 1.0f);
    float FinalWindStrength = MaterialData.WindStrength * GlobalWindStrength;
    
    DynamicMaterial->SetScalarParameterValue(TEXT("Wetness"), FinalWetness);
    DynamicMaterial->SetScalarParameterValue(TEXT("WindStrength"), FinalWindStrength);

    // Set textures if available
    if (MaterialData.DiffuseTexture.IsValid())
    {
        DynamicMaterial->SetTextureParameterValue(TEXT("DiffuseTexture"), MaterialData.DiffuseTexture.LoadSynchronous());
    }
    if (MaterialData.NormalTexture.IsValid())
    {
        DynamicMaterial->SetTextureParameterValue(TEXT("NormalTexture"), MaterialData.NormalTexture.LoadSynchronous());
    }
    if (MaterialData.RoughnessTexture.IsValid())
    {
        DynamicMaterial->SetTextureParameterValue(TEXT("RoughnessTexture"), MaterialData.RoughnessTexture.LoadSynchronous());
    }
}