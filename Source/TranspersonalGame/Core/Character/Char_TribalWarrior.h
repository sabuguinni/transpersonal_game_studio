#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DataTable.h"
#include "Char_TribalWarrior.generated.h"

// Character appearance variation data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    // Skin tone variations (0-1 range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone = 0.5f;

    // Body build variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild = 0.5f; // 0=lean, 1=muscular

    // Clothing style variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ClothingStyle = 0; // 0-4 different hide patterns

    // Weapon loadout
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    int32 WeaponSet = 0; // 0=spear+knife, 1=bow+arrows, 2=club+axe

    // Tribal markings/tattoos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 TribalMarkings = 0; // 0-3 different patterns

    FChar_TribalAppearance()
    {
        SkinTone = FMath::RandRange(0.2f, 0.8f);
        BodyBuild = FMath::RandRange(0.3f, 0.9f);
        ClothingStyle = FMath::RandRange(0, 4);
        WeaponSet = FMath::RandRange(0, 2);
        TribalMarkings = FMath::RandRange(0, 3);
    }
};

// Equipment attachment points
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName SocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> EquipmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FTransform RelativeTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bIsVisible = true;
};

/**
 * Tribal warrior character with customizable appearance and equipment
 * Designed for Cretaceous period survival gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalWarrior : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalWarrior();

protected:
    virtual void BeginPlay() override;

    // Character appearance system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_TribalAppearance AppearanceData;

    // Base character materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    // Equipment system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FChar_EquipmentSlot> EquipmentSlots;

    // Equipment mesh components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<UStaticMeshComponent*> EquipmentComponents;

    // Character stats for survival gameplay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Endurance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Survival = 50.0f;

public:
    // Appearance customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetAppearanceData(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearanceToMesh();

    // Equipment management
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void AttachEquipment(int32 SlotIndex, UStaticMesh* EquipmentMesh);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void DetachEquipment(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SetEquipmentVisibility(int32 SlotIndex, bool bVisible);

    // Character stats
    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    void SetCharacterStats(float NewStrength, float NewEndurance, float NewAgility, float NewSurvival);

    UFUNCTION(BlueprintPure, Category = "Character Stats")
    float GetOverallFitness() const;

    // Editor functions for quick setup
    UFUNCTION(CallInEditor, Category = "Character Setup")
    void SetupDefaultAppearance();

    UFUNCTION(CallInEditor, Category = "Character Setup")
    void SetupDefaultEquipment();

    UFUNCTION(CallInEditor, Category = "Character Setup")
    void GenerateRandomVariant();

private:
    void InitializeEquipmentComponents();
    void UpdateMaterialParameters();
    void ValidateEquipmentSlots();
};