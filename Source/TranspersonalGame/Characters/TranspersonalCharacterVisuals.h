#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TranspersonalCharacterVisuals.generated.h"

/**
 * Visual configuration component for primitive human characters.
 * Manages appearance, clothing, and survival wear-and-tear on the player character.
 * Agent #09 — Character Artist | PROD_CYCLE_AUTO_20260623_002
 */

UENUM(BlueprintType)
enum class EChar_ClothingType : uint8
{
    None            UMETA(DisplayName = "None"),
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    WovenGrass      UMETA(DisplayName = "Woven Grass"),
    BarkFiber       UMETA(DisplayName = "Bark Fiber"),
    FeatherCloak    UMETA(DisplayName = "Feather Cloak")
};

UENUM(BlueprintType)
enum class EChar_SkinCondition : uint8
{
    Healthy         UMETA(DisplayName = "Healthy"),
    Sunburned       UMETA(DisplayName = "Sunburned"),
    Wounded         UMETA(DisplayName = "Wounded"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Starving        UMETA(DisplayName = "Starving")
};

USTRUCT(BlueprintType)
struct FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_ClothingType ClothingType = EChar_ClothingType::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_SkinCondition SkinCondition = EChar_SkinCondition::Healthy;

    /** Dirt/blood accumulation 0-1. Affects material parameter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.0f;

    /** Wound severity 0-1. Drives blood decal intensity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WoundLevel = 0.0f;

    /** Body mass index proxy: 0=starving, 0.5=normal, 1=well-fed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NutritionLevel = 0.5f;

    /** True if character has tribal face markings (plant dye) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    bool bHasFaceMarkings = false;

    /** Scar count accumulated from dinosaur attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0", ClampMax = "10"))
    int32 ScarCount = 0;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Character Visuals Component")
class TRANSPERSONALGAME_API UTranspersonalCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalCharacterVisuals();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Current visual appearance data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FChar_AppearanceData AppearanceData;

    /** Mesh component reference (set by owner character) */
    UPROPERTY(BlueprintReadWrite, Category = "Character|Visuals")
    TObjectPtr<class USkeletalMeshComponent> CharacterMesh;

    /** Apply dirt accumulation from environment */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void AccumulateDirt(float Amount);

    /** Apply wound mark from damage event */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ApplyWound(float Severity);

    /** Update nutrition-based morph targets */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void UpdateNutritionMorphs(float HungerLevel);

    /** Add a scar after surviving a dinosaur attack */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void AddScar();

    /** Reset appearance to clean/healthy state */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ResetAppearance();

    /** Get current clothing type */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character|Appearance")
    EChar_ClothingType GetClothingType() const { return AppearanceData.ClothingType; }

    /** Set clothing type and trigger mesh update */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void SetClothingType(EChar_ClothingType NewType);

    /** Update all material parameters based on current AppearanceData */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void RefreshMaterialParameters();

private:
    /** Time since last dirt update */
    float DirtAccumulationTimer = 0.0f;

    /** Dirt accumulation rate per second in jungle biome */
    static constexpr float DirtAccumulationRate = 0.001f;
};
