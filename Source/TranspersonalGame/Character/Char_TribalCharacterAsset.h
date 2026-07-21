#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalCharacterAsset.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Shaman      UMETA(DisplayName = "Shaman")
};

USTRUCT(BlueprintType)
struct FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> AccessoryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    FChar_TribalEquipment()
    {
        WeaponMesh = nullptr;
        ClothingMesh = nullptr;
        AccessoryMesh = nullptr;
        SkinMaterial = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Endurance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SurvivalSkill = 50.0f;
};

/**
 * Data asset for configuring tribal character appearances and equipment
 * Used to create diverse NPCs and player character variations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalCharacterAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterAsset();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FChar_TribalEquipment Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FChar_TribalStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FLinearColor> HairColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FLinearColor> EyeColors;

    // Asset reference for failed meshy_generate - placeholder for manual import
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Generation", meta = (AllowPrivateAccess = "true"))
    FString AssetGenerationNotes;

    UFUNCTION(BlueprintCallable, Category = "Character")
    FChar_TribalStats GetModifiedStats() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool HasValidEquipment() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    TArray<TSoftObjectPtr<UMaterialInterface>> GetAllMaterials() const;
};

#include "Char_TribalCharacterAsset.generated.h"