#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Char_TribalWarrior.generated.h"

// TODO_ASSET_GENERATION_FAILED: Meshy and DALL-E failed - manual asset creation needed
// Character specifications for manual implementation:
// - Primitive Cretaceous tribal warrior
// - Weathered leather clothing from dinosaur hide
// - Bone jewelry and ornaments
// - Stone tools and weapons
// - Realistic human proportions
// - Facial features showing survival hardships
// - Tribal tattoos or scars
// - Target polycount: 25,000 triangles

UENUM(BlueprintType)
enum class EChar_WarriorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Wounded         UMETA(DisplayName = "Wounded"),
    Resting         UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class EChar_WeaponType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneSpear      UMETA(DisplayName = "Stone Spear"),
    BoneClub        UMETA(DisplayName = "Bone Club"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Sling           UMETA(DisplayName = "Sling"),
    BoneKnife       UMETA(DisplayName = "Bone Knife")
};

USTRUCT(BlueprintType)
struct FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatExperience = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingSkill = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalInstinct = 50.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalWarrior : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalWarrior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    EChar_WarriorState CurrentState = EChar_WarriorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    EChar_WeaponType EquippedWeapon = EChar_WeaponType::None;

    // Survival Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FChar_TribalStats TribalStats;

    // Equipment Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* ShieldMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* BackpackMesh;

    // Character Appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    class UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    class UMaterialInterface* ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<class UMaterialInterface*> TattooMaterials;

    // Character Functions
    UFUNCTION(BlueprintCallable, Category = "Character Actions")
    void EquipWeapon(EChar_WeaponType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "Character Actions")
    void UnequipWeapon();

    UFUNCTION(BlueprintCallable, Category = "Character State")
    void SetWarriorState(EChar_WarriorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float WaterValue);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void GainCombatExperience(float ExperienceAmount);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ImproveCraftingSkill(float SkillAmount);

    // Visual Customization
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplySkinMaterial(class UMaterialInterface* NewSkinMaterial);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyClothingMaterial(class UMaterialInterface* NewClothingMaterial);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void AddTattoo(class UMaterialInterface* TattooMaterial);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void RemoveAllTattoos();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsWounded() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsStarving() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsDehydrated() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsExhausted() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCombatEffectiveness() const;

private:
    // Internal state management
    void UpdateCharacterAppearance();
    void HandleStateTransitions();
    void ApplyWeaponMesh(EChar_WeaponType WeaponType);
    void UpdateAnimationState();
};