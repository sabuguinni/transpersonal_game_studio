// ClothingSystem.h
// Transpersonal Game Studio - Modular Clothing System
// Handles dynamic clothing changes and layering for characters

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "ClothingSystem.generated.h"

UENUM(BlueprintType)
enum class EClothingSlot : uint8
{
    // BASE LAYERS
    Undergarments       UMETA(DisplayName = "Undergarments"),
    
    // MAIN CLOTHING
    Torso               UMETA(DisplayName = "Torso/Shirt"),
    Legs                UMETA(DisplayName = "Legs/Pants"),
    Feet                UMETA(DisplayName = "Feet/Shoes"),
    
    // OUTER LAYERS
    Jacket              UMETA(DisplayName = "Jacket/Coat"),
    Cloak               UMETA(DisplayName = "Cloak/Cape"),
    
    // ACCESSORIES
    Hat                 UMETA(DisplayName = "Hat/Headwear"),
    Gloves              UMETA(DisplayName = "Gloves"),
    Belt                UMETA(DisplayName = "Belt"),
    Backpack            UMETA(DisplayName = "Backpack"),
    
    // ARMOR (if needed)
    ChestArmor          UMETA(DisplayName = "Chest Armor"),
    LegArmor            UMETA(DisplayName = "Leg Armor"),
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EClothingStyle : uint8
{
    // MODERN (time travelers)
    ModernCasual        UMETA(DisplayName = "Modern Casual"),
    ModernOutdoor       UMETA(DisplayName = "Modern Outdoor/Hiking"),
    ModernFormal        UMETA(DisplayName = "Modern Formal"),
    ModernWork          UMETA(DisplayName = "Modern Work Clothes"),
    
    // PRIMITIVE (tribal/survival)
    AnimalSkins         UMETA(DisplayName = "Animal Skins"),
    PlantFiber          UMETA(DisplayName = "Plant Fiber"),
    HandCrafted         UMETA(DisplayName = "Hand-Crafted"),
    
    // SURVIVAL (improvised)
    Improvised          UMETA(DisplayName = "Improvised/Scavenged"),
    Patched             UMETA(DisplayName = "Patched/Repaired"),
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EClothingCondition : uint8
{
    Perfect             UMETA(DisplayName = "Perfect/New"),
    Good                UMETA(DisplayName = "Good Condition"),
    Worn                UMETA(DisplayName = "Worn"),
    Damaged             UMETA(DisplayName = "Damaged"),
    Torn                UMETA(DisplayName = "Torn/Heavily Damaged"),
    
    MAX                 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FClothingPiece
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingSlot Slot = EClothingSlot::Undergarments;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingStyle Style = EClothingStyle::ModernCasual;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingCondition Condition = EClothingCondition::Perfect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<class USkeletalMesh> ClothingMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<class UMaterialInstance>> Materials;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    FString ClothingName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    FText Description;
    
    // ENVIRONMENTAL PROTECTION
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection")
    float ColdProtection = 0.0f;  // 0-1 scale
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection")
    float HeatProtection = 0.0f;  // 0-1 scale
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection")
    float WaterResistance = 0.0f;  // 0-1 scale
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protection")
    float PhysicalProtection = 0.0f;  // Against scratches, bites, etc.
    
    // VISUAL PROPERTIES
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor PrimaryColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SecondaryColor = FLinearColor::Gray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WearLevel = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DamageLevel = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UClothingSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing Set")
    FString SetName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing Set")
    FText Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing Set")
    EClothingStyle PrimaryStyle;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pieces")
    TArray<FClothingPiece> ClothingPieces;
    
    // CONTEXTUAL APPROPRIATENESS
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    bool bAppropriateForCold = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    bool bAppropriateForHeat = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    bool bAppropriateForRain = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    bool bAppropriateForCombat = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UClothingSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UClothingSystemComponent();

protected:
    virtual void BeginPlay() override;

public:
    // CURRENT OUTFIT
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Outfit")
    TMap<EClothingSlot, FClothingPiece> CurrentOutfit;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe")
    TArray<UClothingSet*> AvailableClothingSets;
    
    // CLOTHING MANAGEMENT
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool EquipClothingPiece(const FClothingPiece& ClothingPiece);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool UnequipClothingSlot(EClothingSlot Slot);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void EquipClothingSet(const UClothingSet* ClothingSet);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    FClothingPiece GetClothingInSlot(EClothingSlot Slot) const;
    
    // CLOTHING CONDITIONS
    UFUNCTION(BlueprintCallable, Category = "Clothing Condition")
    void DamageClothing(EClothingSlot Slot, float DamageAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing Condition")
    void MakeClothingDirty(EClothingSlot Slot, float DirtAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing Condition")
    void CleanClothing(EClothingSlot Slot, float CleanAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Clothing Condition")
    void RepairClothing(EClothingSlot Slot, float RepairAmount);
    
    // ENVIRONMENTAL ADAPTATION
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTotalColdProtection() const;
    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTotalHeatProtection() const;
    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTotalWaterResistance() const;
    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTotalPhysicalProtection() const;
    
    // VISUAL UPDATES
    UFUNCTION(BlueprintCallable, Category = "Visual")
    void UpdateClothingVisuals();
    
    UFUNCTION(BlueprintCallable, Category = "Visual")
    void ApplyWeatherEffects(float RainIntensity, float Temperature);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;
    
    void UpdateMeshComponent(EClothingSlot Slot, const FClothingPiece& ClothingPiece);
    void UpdateMaterialParameters(EClothingSlot Slot, const FClothingPiece& ClothingPiece);
};