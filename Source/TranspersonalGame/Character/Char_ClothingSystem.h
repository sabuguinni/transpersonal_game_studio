#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_ClothingSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_ClothingType : uint8
{
    None            UMETA(DisplayName = "None"),
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    WovenFiber      UMETA(DisplayName = "Woven Fiber"),
    BoneArmor       UMETA(DisplayName = "Bone Armor"),
    LeatherGear     UMETA(DisplayName = "Leather Gear"),
    TribalRobes     UMETA(DisplayName = "Tribal Robes")
};

UENUM(BlueprintType)
enum class EChar_EquipmentSlot : uint8
{
    Head            UMETA(DisplayName = "Head"),
    Torso           UMETA(DisplayName = "Torso"),
    Legs            UMETA(DisplayName = "Legs"),
    Feet            UMETA(DisplayName = "Feet"),
    Hands           UMETA(DisplayName = "Hands"),
    Back            UMETA(DisplayName = "Back"),
    Belt            UMETA(DisplayName = "Belt")
};

USTRUCT(BlueprintType)
struct FChar_ClothingItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingType ClothingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_EquipmentSlot EquipmentSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WarmthValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ProtectionValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DurabilityValue;

    FChar_ClothingItem()
    {
        ItemName = TEXT("Basic Clothing");
        ClothingType = EChar_ClothingType::AnimalHide;
        EquipmentSlot = EChar_EquipmentSlot::Torso;
        WarmthValue = 10.0f;
        ProtectionValue = 5.0f;
        DurabilityValue = 100.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_ClothingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_ClothingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing System")
    TMap<EChar_EquipmentSlot, FChar_ClothingItem> EquippedClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing System")
    TArray<FChar_ClothingItem> AvailableClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TotalWarmth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TotalProtection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TMap<EChar_EquipmentSlot, class UStaticMeshComponent*> ClothingMeshComponents;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool EquipClothing(const FChar_ClothingItem& ClothingItem);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool UnequipClothing(EChar_EquipmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    FChar_ClothingItem GetEquippedClothing(EChar_EquipmentSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void UpdateClothingVisuals();

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetTotalWarmth() const { return TotalWarmth; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetTotalProtection() const { return TotalProtection; }

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void InitializeDefaultClothing();

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void CreateClothingMeshComponents();

private:
    void CalculateClothingStats();
    void UpdateMeshComponent(EChar_EquipmentSlot Slot, const FChar_ClothingItem& Item);
};