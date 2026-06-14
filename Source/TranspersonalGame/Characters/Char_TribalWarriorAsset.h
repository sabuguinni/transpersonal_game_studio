#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_TribalWarriorAsset.generated.h"

USTRUCT(BlueprintType)
struct FChar_EquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMesh* EquipmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UMaterialInterface* EquipmentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName AttachSocketName;

    FChar_EquipmentSlot()
    {
        SlotName = TEXT("Default");
        EquipmentMesh = nullptr;
        EquipmentMaterial = nullptr;
        AttachSocketName = NAME_None;
    }
};

USTRUCT(BlueprintType)
struct FChar_MaterialVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    FString VariantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HairMaterial;

    FChar_MaterialVariant()
    {
        VariantName = TEXT("Default");
        SkinMaterial = nullptr;
        ClothingMaterial = nullptr;
        HairMaterial = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    // Base character mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Mesh")
    USkeletalMesh* CharacterMesh;

    // Material variants for diversity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<FChar_MaterialVariant> MaterialVariants;

    // Equipment slots for weapons and accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FChar_EquipmentSlot> EquipmentSlots;

    // Character stats that affect appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BattleWearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TribalRank;

    // Functions for character customization
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyMaterialVariant(USkeletalMeshComponent* MeshComponent, int32 VariantIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void AttachEquipment(USkeletalMeshComponent* MeshComponent, const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetBattleWear(USkeletalMeshComponent* MeshComponent, float WearLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateCharacterAsset() const;

private:
    // Internal helper functions
    void InitializeDefaultMaterials();
    void SetupDefaultEquipment();
};