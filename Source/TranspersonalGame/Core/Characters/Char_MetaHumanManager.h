#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Char_MetaHumanManager.generated.h"

// Character appearance variation types
UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    VeryLight   UMETA(DisplayName = "Very Light"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Average     UMETA(DisplayName = "Average"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None        UMETA(DisplayName = "None"),
    Warrior     UMETA(DisplayName = "Warrior Marks"),
    Hunter      UMETA(DisplayName = "Hunter Marks"),
    Shaman      UMETA(DisplayName = "Shaman Marks"),
    Elder       UMETA(DisplayName = "Elder Marks")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Basic       UMETA(DisplayName = "Basic Wraps"),
    Decorated   UMETA(DisplayName = "Decorated Hide"),
    Ceremonial  UMETA(DisplayName = "Ceremonial Gear")
};

// Character appearance configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Average;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarkings TribalMarkings = EChar_TribalMarkings::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor MarkingColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale = 1.0f;

    FChar_AppearanceConfig()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Average;
        TribalMarkings = EChar_TribalMarkings::None;
        ClothingStyle = EChar_ClothingStyle::Basic;
        MarkingColor = FLinearColor::Red;
        BodyScale = 1.0f;
    }
};

// Equipment slot configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName AttachmentSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> EquippedMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FTransform RelativeTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bIsEquipped = false;

    FChar_EquipmentSlot()
    {
        SlotName = NAME_None;
        AttachmentSocket = NAME_None;
        EquippedMesh = nullptr;
        RelativeTransform = FTransform::Identity;
        bIsEquipped = false;
    }
};

/**
 * MetaHuman Character Manager
 * Manages character appearance, equipment, and customization for tribal warriors
 * Integrates with UE5 MetaHuman Creator pipeline for realistic character generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    // Core character mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* CharacterMesh;

    // Equipment attachment components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    TArray<UStaticMeshComponent*> EquipmentComponents;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_AppearanceConfig AppearanceConfig;

    // Equipment slots configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FChar_EquipmentSlot> EquipmentSlots;

    // Available MetaHuman presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<USkeletalMesh>> MetaHumanPresets;

    // Material instances for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

public:
    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyAppearanceConfig(const FChar_AppearanceConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetTribalMarkings(EChar_TribalMarkings NewMarkings, FLinearColor MarkingColor);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

    // Equipment management functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(FName SlotName, UStaticMesh* ItemMesh);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool UnequipItem(FName SlotName);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipAllItems();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UStaticMeshComponent* GetEquipmentComponent(FName SlotName);

    // MetaHuman integration functions
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void LoadMetaHumanPreset(int32 PresetIndex);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void CreateTribalWarriorVariation(int32 VariationSeed);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    FChar_AppearanceConfig GetCurrentAppearanceConfig() const { return AppearanceConfig; }

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    TArray<FChar_EquipmentSlot> GetEquipmentSlots() const { return EquipmentSlots; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void InitializeEquipmentSlots();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void RefreshCharacterAppearance();

private:
    // Internal helper functions
    void UpdateMaterialParameters();
    void CreateDynamicMaterials();
    void SetupEquipmentComponents();
    UMaterialInstanceDynamic* GetOrCreateDynamicMaterial(int32 MaterialIndex);
    void ApplySkinToneToMaterial(UMaterialInstanceDynamic* Material, EChar_SkinTone SkinTone);
    void ApplyTribalMarkingsToMaterial(UMaterialInstanceDynamic* Material, EChar_TribalMarkings Markings, FLinearColor Color);
};