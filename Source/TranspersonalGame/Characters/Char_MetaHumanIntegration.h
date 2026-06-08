#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SharedTypes.h"
#include "Char_MetaHumanIntegration.generated.h"

// Enums for character customization
UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Agile       UMETA(DisplayName = "Agile Scout")
};

UENUM(BlueprintType)
enum class EChar_WeatheringLevel : uint8
{
    Fresh       UMETA(DisplayName = "Fresh"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Hardened    UMETA(DisplayName = "Hardened"),
    Scarred     UMETA(DisplayName = "Battle Scarred")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Short       UMETA(DisplayName = "Short"),
    Medium      UMETA(DisplayName = "Medium"),
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Shaved      UMETA(DisplayName = "Shaved")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Gear"),
    Warrior     UMETA(DisplayName = "Warrior Armor"),
    Shaman      UMETA(DisplayName = "Shaman Robes"),
    Crafter     UMETA(DisplayName = "Crafter Apron")
};

// Struct for character appearance configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_WeatheringLevel WeatheringLevel = EChar_WeatheringLevel::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float DirtLevel = 0.7f;

    FChar_AppearanceConfig()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        WeatheringLevel = EChar_WeatheringLevel::Weathered;
        HairStyle = EChar_HairStyle::Medium;
        ClothingStyle = EChar_ClothingStyle::Hunter;
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        ScarIntensity = 0.5f;
        DirtLevel = 0.7f;
    }
};

/**
 * MetaHuman Integration Component for Cretaceous Period Characters
 * Handles character appearance customization, material application, and primitive human aesthetics
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceConfig AppearanceConfig;

    // MetaHuman mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    class USkeletalMesh* MaleMetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    class USkeletalMesh* FemaleMetaHumanMesh;

    // Material references for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* SkinMasterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* HairMasterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* ClothingMasterMaterial;

    // Dynamic material instances
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Materials")
    class UMaterialInstanceDynamic* SkinMaterialInstance;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Materials")
    class UMaterialInstanceDynamic* HairMaterialInstance;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Materials")
    class UMaterialInstanceDynamic* ClothingMaterialInstance;

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearanceConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWeatheringLevel(EChar_WeatheringLevel NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairStyle(EChar_HairStyle NewHairStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetEyeColor(FLinearColor NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScarIntensity(float NewScarIntensity);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetDirtLevel(float NewDirtLevel);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character Utilities")
    void CreateDynamicMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Utilities")
    void ApplyMaterialsToMesh();

    UFUNCTION(BlueprintCallable, Category = "Character Utilities")
    FLinearColor GetSkinColorForTone(EChar_SkinTone SkinTone) const;

    UFUNCTION(BlueprintCallable, Category = "Character Utilities")
    float GetBodyScaleForBuild(EChar_BodyBuild BodyBuild) const;

    UFUNCTION(BlueprintCallable, Category = "Character Utilities")
    float GetWeatheringIntensity(EChar_WeatheringLevel WeatheringLevel) const;

private:
    // Internal helper functions
    void InitializeMaterials();
    void UpdateSkinMaterial();
    void UpdateHairMaterial();
    void UpdateClothingMaterial();
    void ApplyBodyBuildModifications();

    // Cached component references
    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    // Runtime state
    bool bMaterialsInitialized;
    bool bAppearanceApplied;
};