#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterArchetypes.h"
#include "Engine/DataTable.h"
#include "MetaHumanCharacterGenerator.generated.h"

class USkeletalMeshComponent;
class UMaterialInterface;
class UMetaHumanCreatorAsset;

/**
 * Structure for MetaHuman variation parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMetaHumanVariationParams
{
    GENERATED_BODY()

    FMetaHumanVariationParams()
        : FaceVariation(0.0f)
        , BodyVariation(0.0f)
        , SkinTone(0.5f)
        , HairColor(0.5f)
        , EyeColor(0.5f)
        , Age(0.3f)
        , Weight(0.5f)
        , Muscle(0.5f)
        , Height(0.5f)
        , WearLevel(0.0f)
        , DirtLevel(0.0f)
        , ScarIntensity(0.0f)
    {
    }

    /** Face shape variation (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceVariation;

    /** Body shape variation (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BodyVariation;

    /** Skin tone (0-1, darker to lighter) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone;

    /** Hair color variation (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HairColor;

    /** Eye color variation (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeColor;

    /** Age appearance (0-1, young to old) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Age;

    /** Weight variation (0-1, thin to heavy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Weight;

    /** Muscle definition (0-1, soft to muscular) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Muscle;

    /** Height variation (0-1, short to tall) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Height;

    /** Clothing wear level (0-1, pristine to tattered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WearLevel;

    /** Dirt and grime level (0-1, clean to filthy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel;

    /** Scar intensity (0-1, none to heavily scarred) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity;
};

/**
 * Structure for clothing and equipment configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterClothingConfig
{
    GENERATED_BODY()

    FCharacterClothingConfig()
        : ClothingStyle(TEXT(""))
        , PrimaryClothingMaterial(nullptr)
        , SecondaryClothingMaterial(nullptr)
        , AccessoryMaterial(nullptr)
        , HasBackpack(false)
        , HasHat(false)
        , HasGloves(false)
        , HasBoots(true)
        , EquipmentLevel(0)
    {
    }

    /** Clothing style identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingStyle;

    /** Primary clothing material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> PrimaryClothingMaterial;

    /** Secondary clothing material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> SecondaryClothingMaterial;

    /** Accessory material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> AccessoryMaterial;

    /** Has backpack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool HasBackpack;

    /** Has hat/headwear */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool HasHat;

    /** Has gloves */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool HasGloves;

    /** Has boots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool HasBoots;

    /** Equipment sophistication level (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (ClampMin = "0", ClampMax = "3"))
    int32 EquipmentLevel;
};

/**
 * Component responsible for generating and managing MetaHuman character variations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaHumanCharacterGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaHumanCharacterGenerator();

protected:
    virtual void BeginPlay() override;

public:
    /** Generate a character based on archetype */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    bool GenerateCharacterFromArchetype(const FCharacterArchetypeData& ArchetypeData, USkeletalMeshComponent* TargetMeshComponent);

    /** Generate random variation parameters for an archetype */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FMetaHumanVariationParams GenerateVariationParams(const FCharacterArchetypeData& ArchetypeData);

    /** Apply variation parameters to a MetaHuman */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    bool ApplyVariationToMetaHuman(USkeletalMeshComponent* MeshComponent, const FMetaHumanVariationParams& Params);

    /** Generate clothing configuration for archetype */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterClothingConfig GenerateClothingConfig(const FCharacterArchetypeData& ArchetypeData);

    /** Apply clothing configuration */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    bool ApplyClothingConfig(USkeletalMeshComponent* MeshComponent, const FCharacterClothingConfig& ClothingConfig);

    /** Get unique identifier for this character instance */
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FString GenerateUniqueCharacterID(const FCharacterArchetypeData& ArchetypeData);

protected:
    /** Base MetaHuman presets data table */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UDataTable> MetaHumanPresetsTable;

    /** Clothing materials data table */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UDataTable> ClothingMaterialsTable;

    /** Character archetype data asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UCharacterArchetypeDataAsset> ArchetypeDataAsset;

    /** Random seed for consistent generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 RandomSeed;

    /** Enable debug logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugLogging;

private:
    /** Internal random stream for consistent generation */
    FRandomStream RandomStream;

    /** Apply age-based modifications */
    void ApplyAgeModifications(FMetaHumanVariationParams& Params, ECharacterAgeGroup AgeGroup);

    /** Apply build-based modifications */
    void ApplyBuildModifications(FMetaHumanVariationParams& Params, ECharacterBuild BuildType);

    /** Apply survival condition modifications */
    void ApplySurvivalModifications(FMetaHumanVariationParams& Params, ESurvivalCondition Condition);

    /** Generate unique features based on archetype */
    void GenerateUniqueFeatures(FMetaHumanVariationParams& Params, const FString& UniqueFeatures);
};