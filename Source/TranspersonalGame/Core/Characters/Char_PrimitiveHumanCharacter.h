#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

// Character customization data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_HumanCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone = 0.5f; // 0.0 = pale, 1.0 = dark

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild = 0.5f; // 0.0 = lean, 1.0 = muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.3f; // 0.0 = clean, 1.0 = heavily weathered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ScarPattern = 0; // Different scar configurations

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    int32 ClothingVariant = 0; // Different fur/leather combinations

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<int32> EquippedTools; // Bone tools, stone weapons
};

// Survival stats for primitive humans
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Warmth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f; // 0.0 = calm, 100.0 = terrified

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Exhaustion = 0.0f;
};

/**
 * Primitive human character for Cretaceous survival gameplay
 * Represents early humans adapted to prehistoric environments
 * Features dynamic customization, survival stats, and tool usage
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_HumanCustomization CustomizationData;

    // Survival system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FChar_SurvivalStats SurvivalStats;

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SpearComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StoneToolComponent;

    // Dynamic materials for customization
    UPROPERTY()
    UMaterialInstanceDynamic* BodyMaterialInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* ClothingMaterialInstance;

    // Customization assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assets")
    TArray<USkeletalMesh*> BodyMeshVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assets")
    TArray<UStaticMesh*> SpearVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assets")
    TArray<UStaticMesh*> ToolVariants;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assets")
    UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assets")
    UMaterialInterface* BaseClothingMaterial;

public:
    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCustomization(const FChar_HumanCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(float NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyBuild(float NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWeatheringLevel(float NewWeathering);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingVariant(int32 VariantIndex);

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipSpear(int32 SpearIndex);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipTool(int32 ToolIndex);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipWeapons();

    // Survival functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStarving() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsDehydrated() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsTerrified() const;

    // Animation and movement
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalAnimation(const FString& AnimationName);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementStyle(bool bIsSneaking);

    // Interaction with environment
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractWithObject(AActor* TargetObject);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void GatherResource(AActor* ResourceObject);

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ThrowSpear(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void MeleeAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DefensiveStance(bool bActivate);

protected:
    // Internal customization helpers
    void UpdateBodyMaterial();
    void UpdateClothingMaterial();
    void UpdateEquipmentVisibility();
    void ApplySurvivalEffects();

    // Survival timers
    float HungerDecayRate = 1.0f; // Per minute
    float ThirstDecayRate = 1.5f; // Per minute
    float StaminaRegenRate = 10.0f; // Per minute
    float FearDecayRate = 5.0f; // Per minute

    // Animation state
    bool bIsSneaking = false;
    bool bIsInCombat = false;
    bool bIsGathering = false;

    // Equipment state
    bool bHasSpearEquipped = false;
    bool bHasToolEquipped = false;
    int32 CurrentSpearIndex = -1;
    int32 CurrentToolIndex = -1;
};