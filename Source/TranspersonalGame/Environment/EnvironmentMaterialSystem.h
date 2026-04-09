#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape.h"
#include "EnvironmentMaterialSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvironmentMaterialType : uint8
{
    Landscape_Grass        UMETA(DisplayName = "Grass Terrain"),
    Landscape_Dirt         UMETA(DisplayName = "Dirt Terrain"),
    Landscape_Rock         UMETA(DisplayName = "Rocky Terrain"),
    Landscape_Mud          UMETA(DisplayName = "Muddy Terrain"),
    Landscape_Sand         UMETA(DisplayName = "Sandy Terrain"),
    Vegetation_Bark        UMETA(DisplayName = "Tree Bark"),
    Vegetation_Leaves      UMETA(DisplayName = "Foliage Leaves"),
    Rock_Weathered         UMETA(DisplayName = "Weathered Rock"),
    Rock_Moss_Covered      UMETA(DisplayName = "Moss-Covered Rock"),
    Crystal_Glowing        UMETA(DisplayName = "Glowing Crystal"),
    Bone_Aged              UMETA(DisplayName = "Aged Bone"),
    Water_Stream           UMETA(DisplayName = "Stream Water"),
    Water_Muddy            UMETA(DisplayName = "Muddy Water")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentMaterialData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EEnvironmentMaterialType MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> DiffuseTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> NormalTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> RoughnessTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float Roughness = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float Metallic = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    FLinearColor BaseColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    float TextureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    bool bSupportsNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Properties")
    bool bUsesVertexColors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float WetnessAmount = 0.0f; // For rain/moisture effects

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float WindStrength = 0.0f; // For vegetation movement

    FEnvironmentMaterialData()
    {
        MaterialType = EEnvironmentMaterialType::Landscape_Grass;
        Roughness = 0.8f;
        Metallic = 0.0f;
        BaseColor = FLinearColor::White;
        TextureScale = 1.0f;
        bSupportsNanite = true;
        bUsesVertexColors = false;
        WetnessAmount = 0.0f;
        WindStrength = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeMaterialSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<EEnvironmentMaterialType> PrimaryMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<EEnvironmentMaterialType> SecondaryMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palette")
    FLinearColor DominantColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palette")
    FLinearColor AccentColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    float AverageWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    float AverageWindStrength = 0.5f;

    FBiomeMaterialSet()
    {
        BiomeName = TEXT("Default");
        DominantColor = FLinearColor::Green;
        AccentColor = FLinearColor::Brown;
        AverageWetness = 0.3f;
        AverageWindStrength = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentMaterialSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentMaterialSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Materials")
    TArray<FEnvironmentMaterialData> EnvironmentMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Materials")
    TArray<FBiomeMaterialSet> BiomeMaterialSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalWetness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalWindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    FLinearColor GlobalColorTint = FLinearColor::White;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void ApplyBiomeMaterials(const FString& BiomeName, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    UMaterialInstanceDynamic* CreateDynamicMaterial(EEnvironmentMaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void UpdateGlobalEnvironmentalEffects(float Wetness, float WindStrength, FLinearColor ColorTint);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void ApplyMaterialToLandscape(ALandscape* Landscape, const FString& LayerName, EEnvironmentMaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void ApplyMaterialToStaticMesh(UStaticMeshComponent* MeshComponent, EEnvironmentMaterialType MaterialType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Material Events")
    void OnBiomeMaterialsApplied(const FString& BiomeName, int32 MaterialsApplied);

private:
    UPROPERTY()
    TMap<EEnvironmentMaterialType, TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

    void InitializeEnvironmentMaterials();
    void InitializeBiomeMaterialSets();
    FEnvironmentMaterialData* GetMaterialData(EEnvironmentMaterialType MaterialType);
    FBiomeMaterialSet* GetBiomeMaterialSet(const FString& BiomeName);
    void UpdateDynamicMaterialParameters(UMaterialInstanceDynamic* DynamicMaterial, const FEnvironmentMaterialData& MaterialData);
};