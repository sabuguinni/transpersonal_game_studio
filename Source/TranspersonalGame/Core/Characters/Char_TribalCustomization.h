#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SharedTypes.h"
#include "Char_TribalCustomization.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FVector AttachmentOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FRotator AttachmentRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WeatheringLevel;

    FChar_TribalClothing()
    {
        ClothingName = TEXT("Basic Hide");
        AttachmentOffset = FVector::ZeroVector;
        AttachmentRotation = FRotator::ZeroRotator;
        SocketName = NAME_None;
        WeatheringLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FString WeaponName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSoftObjectPtr<UStaticMesh> WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSoftObjectPtr<UMaterialInterface> WeaponMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName AttachSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EChar_WeaponType WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float CraftingQuality;

    FChar_TribalWeapon()
    {
        WeaponName = TEXT("Stone Spear");
        AttachSocket = TEXT("hand_r");
        WeaponType = EChar_WeaponType::Spear;
        CraftingQuality = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAccessory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
    FString AccessoryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
    TSoftObjectPtr<UStaticMesh> AccessoryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
    TSoftObjectPtr<UMaterialInterface> AccessoryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
    FName AttachSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
    EChar_AccessoryType AccessoryType;

    FChar_TribalAccessory()
    {
        AccessoryName = TEXT("Bone Necklace");
        AttachSocket = TEXT("neck_01");
        AccessoryType = EChar_AccessoryType::Necklace;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCustomization();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TArray<FChar_TribalClothing> AvailableClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Weapons")
    TArray<FChar_TribalWeapon> AvailableWeapons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TArray<FChar_TribalAccessory> AvailableAccessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Equipment")
    FChar_TribalClothing CurrentClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Equipment")
    FChar_TribalWeapon CurrentWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Equipment")
    TArray<FChar_TribalAccessory> CurrentAccessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Components")
    TArray<UStaticMeshComponent*> AttachedComponents;

public:
    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void EquipClothing(const FChar_TribalClothing& Clothing);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void EquipWeapon(const FChar_TribalWeapon& Weapon);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void EquipAccessory(const FChar_TribalAccessory& Accessory);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void RemoveAccessory(EChar_AccessoryType AccessoryType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void RandomizeTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void SetCraftingQuality(float QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalClothing GetRandomClothing();

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalWeapon GetRandomWeapon();

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalAccessory GetRandomAccessory();

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void InitializeDefaultItems();

private:
    void CreateClothingItems();
    void CreateWeaponItems();
    void CreateAccessoryItems();
    
    UStaticMeshComponent* CreateAttachedComponent();
    void AttachComponentToSocket(UStaticMeshComponent* Component, FName SocketName, const FVector& Offset, const FRotator& Rotation);
    void UpdateMaterialWeathering(UMaterialInstanceDynamic* Material, float WeatheringLevel);
};