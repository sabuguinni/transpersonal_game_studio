#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "CharacterTypes.generated.h"

/**
 * Character archetype definitions for the prehistoric world
 */
UENUM(BlueprintType)
enum class EChar_CharacterArchetype : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Chief           UMETA(DisplayName = "Chief"),
    Child           UMETA(DisplayName = "Child"),
    Elder           UMETA(DisplayName = "Elder"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Healer          UMETA(DisplayName = "Healer"),
    Storyteller     UMETA(DisplayName = "Storyteller")
};

/**
 * Character customization data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_CharacterArchetype Archetype = EChar_CharacterArchetype::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Build = 0.5f; // 0.0 = lean, 1.0 = muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> ClothingItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> TribalMarkings;

    FChar_CharacterCustomization()
    {
        CharacterName = TEXT("Unnamed");
        ClothingItems.Empty();
        Accessories.Empty();
        TribalMarkings.Empty();
    }
};

/**
 * Character physical traits for gameplay mechanics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Endurance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Wisdom = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Charisma = 50.0f;

    FChar_PhysicalTraits()
    {
        // Default values already set above
    }
};

/**
 * Character equipment slot definitions
 */
UENUM(BlueprintType)
enum class EChar_EquipmentSlot : uint8
{
    Head            UMETA(DisplayName = "Head"),
    Torso           UMETA(DisplayName = "Torso"),
    Legs            UMETA(DisplayName = "Legs"),
    Feet            UMETA(DisplayName = "Feet"),
    MainHand        UMETA(DisplayName = "Main Hand"),
    OffHand         UMETA(DisplayName = "Off Hand"),
    Neck            UMETA(DisplayName = "Neck"),
    Ring            UMETA(DisplayName = "Ring"),
    Back            UMETA(DisplayName = "Back")
};

/**
 * Equipment item data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EquipmentItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EChar_EquipmentSlot Slot = EChar_EquipmentSlot::MainHand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> ItemMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FVector AttachmentOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FRotator AttachmentRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString SocketName;

    FChar_EquipmentItem()
    {
        ItemName = TEXT("Unknown Item");
        SocketName = TEXT("hand_r");
    }
};