#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Char_CharacterCustomizationManager.generated.h"

// Character appearance data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FaceVariation = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyle = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMassIndex = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBattleScars = false;

    FChar_CharacterAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        FaceVariation = 0;
        HairStyle = 0;
        BodyMassIndex = 1.0f;
        Height = 1.0f;
        bHasTribalMarkings = false;
        bHasBattleScars = false;
    }
};

// Character archetype definitions
UENUM(BlueprintType)
enum class EChar_CharacterArchetype : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Scout           UMETA(DisplayName = "Scout"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child")
};

// Equipment slot types
UENUM(BlueprintType)
enum class EChar_EquipmentSlot : uint8
{
    Head            UMETA(DisplayName = "Head"),
    Chest           UMETA(DisplayName = "Chest"),
    Legs            UMETA(DisplayName = "Legs"),
    Feet            UMETA(DisplayName = "Feet"),
    Hands           UMETA(DisplayName = "Hands"),
    Back            UMETA(DisplayName = "Back"),
    MainHand        UMETA(DisplayName = "Main Hand"),
    OffHand         UMETA(DisplayName = "Off Hand"),
    Accessory       UMETA(DisplayName = "Accessory")
};

// Equipment data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EquipmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EChar_EquipmentSlot Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FVector RelativeScale = FVector::OneVector;

    FChar_EquipmentData()
    {
        ItemName = TEXT("");
        Slot = EChar_EquipmentSlot::Chest;
        MeshAsset = nullptr;
        MaterialOverride = nullptr;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        RelativeScale = FVector::OneVector;
    }
};

/**
 * Character Customization Manager
 * Handles character appearance, equipment, and visual customization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_CharacterCustomizationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_CharacterCustomizationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CharacterAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    EChar_CharacterArchetype CharacterArchetype = EChar_CharacterArchetype::Hunter;

    // Equipment system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TMap<EChar_EquipmentSlot, FChar_EquipmentData> EquippedItems;

    // Character mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> BaseFemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    bool bIsFemale = false;

    // Material instances for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseHairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseClothingMaterial;

    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterAppearance(const FChar_CharacterAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterArchetype(EChar_CharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipItem(EChar_EquipmentSlot Slot, const FChar_EquipmentData& ItemData);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void UnequipItem(EChar_EquipmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyArchetypePreset(EChar_CharacterArchetype Archetype);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_CharacterAppearance GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    EChar_CharacterArchetype GetCharacterArchetype() const { return CharacterArchetype; }

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    bool IsItemEquipped(EChar_EquipmentSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_EquipmentData GetEquippedItem(EChar_EquipmentSlot Slot) const;

protected:
    // Internal functions
    void UpdateCharacterMesh();
    void UpdateMaterialParameters();
    void UpdateEquipmentMeshes();
    void CreateMaterialInstances();

private:
    // Dynamic material instances
    UPROPERTY()
    class UMaterialInstanceDynamic* SkinMaterialInstance;

    UPROPERTY()
    class UMaterialInstanceDynamic* HairMaterialInstance;

    UPROPERTY()
    class UMaterialInstanceDynamic* ClothingMaterialInstance;

    // Component references
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    // Equipment mesh components
    UPROPERTY()
    TMap<EChar_EquipmentSlot, class USkeletalMeshComponent*> EquipmentMeshComponents;
};