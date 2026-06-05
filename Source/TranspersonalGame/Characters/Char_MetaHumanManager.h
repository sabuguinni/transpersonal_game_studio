#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/SkeletalMesh.h"
#include "SharedTypes.h"
#include "Char_MetaHumanManager.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Sturdy      UMETA(DisplayName = "Sturdy"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Slim        UMETA(DisplayName = "Slim")
};

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    Hunter      UMETA(DisplayName = "Tribal Hunter"),
    Gatherer    UMETA(DisplayName = "Tribal Gatherer"),
    Elder       UMETA(DisplayName = "Tribal Elder"),
    Youth       UMETA(DisplayName = "Tribal Youth"),
    Warrior     UMETA(DisplayName = "Tribal Warrior")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_CharacterType CharacterType = EChar_CharacterType::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Survivor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinRoughness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacialHair = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasTribalPaint = false;

    FChar_CharacterCustomization()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        CharacterType = EChar_CharacterType::Hunter;
        CharacterName = TEXT("Survivor");
        SkinRoughness = 0.7f;
        WeatheringLevel = 0.5f;
        bHasFacialHair = false;
        bHasScars = true;
        bHasTribalPaint = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FChar_CharacterCustomization Customization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    FChar_MetaHumanPreset()
    {
        PresetName = TEXT("Default");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Presets")
    TArray<FChar_MetaHumanPreset> CharacterPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> PrimitiveSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> TribalClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    float SubsurfaceScatteringIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    float SkinTranslucency = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    FLinearColor SkinSubsurfaceColor = FLinearColor(1.0f, 0.4f, 0.3f, 1.0f);

public:
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    AActor* CreateCharacterFromCustomization(const FChar_CharacterCustomization& Customization, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyCharacterCustomization(AActor* CharacterActor, const FChar_CharacterCustomization& Customization);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterCustomization GetRandomCharacterCustomization();

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    TArray<FChar_MetaHumanPreset> GetAvailablePresets() const;

    UFUNCTION(BlueprintCallable, Category = "Material Setup")
    UMaterialInstanceDynamic* CreateVolumetricSkinMaterial(EChar_SkinTone SkinTone, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Material Setup")
    void ApplyVolumetricLightingToCharacter(AActor* CharacterActor);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void InitializeCharacterPresets();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void TestCharacterVisibilityInFog();

protected:
    void LoadCharacterAssets();
    void SetupVolumetricMaterials();
    FLinearColor GetSkinToneColor(EChar_SkinTone SkinTone);
    float GetBodyBuildScale(EChar_BodyBuild BodyBuild);
};