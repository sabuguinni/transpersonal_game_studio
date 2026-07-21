#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Char_PrimitiveHumanAsset.h"
#include "Char_MetaHumanIntegration.generated.h"

class USkeletalMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EChar_MetaHumanPreset : uint8
{
    CretaceousHunter    UMETA(DisplayName = "Cretaceous Hunter"),
    TribalWarrior       UMETA(DisplayName = "Tribal Warrior"),
    PrimitiveGatherer   UMETA(DisplayName = "Primitive Gatherer"),
    CaveShaman          UMETA(DisplayName = "Cave Shaman"),
    NomadScout          UMETA(DisplayName = "Nomad Scout")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> FaceMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> BodyMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_PrimitiveHumanSpec PrimitiveSpec;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SubsurfaceScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkinRoughness = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkinBaseColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
};

/**
 * Component that integrates MetaHuman characters with primitive human customization
 * Handles material blending for Cretaceous atmospheric lighting
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanIntegration();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Setup")
    EChar_MetaHumanPreset CurrentPreset = EChar_MetaHumanPreset::CretaceousHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Setup")
    FChar_MetaHumanConfig MetaHumanConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Reference")
    TSoftObjectPtr<UChar_PrimitiveHumanAsset> PrimitiveHumanAsset;

    // Apply MetaHuman configuration to character
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyMetaHumanConfiguration(USkeletalMeshComponent* MeshComponent);

    // Configure materials for Cretaceous lighting conditions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureForCretaceousLighting(USkeletalMeshComponent* MeshComponent);

    // Switch between MetaHuman presets
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetMetaHumanPreset(EChar_MetaHumanPreset NewPreset);

    // Generate random MetaHuman configuration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void GenerateRandomConfiguration();

    // Blend primitive human traits with MetaHuman base
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void BlendPrimitiveTraits(const FChar_PrimitiveHumanSpec& PrimitiveSpec);

protected:
    // Initialize preset configurations
    void InitializePresets();

    // Create dynamic materials for customization
    void CreateDynamicMaterials(USkeletalMeshComponent* MeshComponent);

    // Apply weathering and primitive effects
    void ApplyPrimitiveEffects(UMaterialInstanceDynamic* DynamicMaterial, const FChar_PrimitiveHumanSpec& Spec);

private:
    UPROPERTY()
    TMap<EChar_MetaHumanPreset, FChar_MetaHumanConfig> PresetConfigurations;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;
};