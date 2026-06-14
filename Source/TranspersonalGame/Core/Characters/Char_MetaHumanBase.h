#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_MetaHumanBase.generated.h"

USTRUCT(BlueprintType)
struct FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 TribalMarkingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor MarkingColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ClothingStyle;

    FChar_AppearanceData()
    {
        SkinTone = 0.5f;
        BodyBuild = 0.5f;
        TribalMarkingType = 0;
        MarkingColor = FLinearColor::White;
        ClothingStyle = 0;
    }
};

USTRUCT(BlueprintType)
struct FChar_EquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMesh* EquippedMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FTransform RelativeTransform;

    FChar_EquipmentSlot()
    {
        EquippedMesh = nullptr;
        SocketName = NAME_None;
        RelativeTransform = FTransform::Identity;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanBase : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_MetaHumanBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Appearance System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FChar_AppearanceData AppearanceData;

    // Equipment System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Equipment")
    TMap<FString, FChar_EquipmentSlot> EquipmentSlots;

    // Equipment Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Equipment")
    UStaticMeshComponent* MainWeaponComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Equipment")
    UStaticMeshComponent* SecondaryWeaponComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Equipment")
    UStaticMeshComponent* ShieldComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Equipment")
    UStaticMeshComponent* AccessoryComponent;

    // Appearance Functions
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void SetAppearanceData(const FChar_AppearanceData& NewAppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ApplyTribalMarkings(int32 MarkingType, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void SetSkinTone(float ToneValue);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void SetBodyBuild(float BuildValue);

    // Equipment Functions
    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipItem(const FString& SlotName, UStaticMesh* ItemMesh, const FTransform& RelativeTransform);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void UnequipItem(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipMainWeapon(UStaticMesh* WeaponMesh);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipShield(UStaticMesh* ShieldMesh);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipAccessory(UStaticMesh* AccessoryMesh);

    // Lighting Integration
    UFUNCTION(BlueprintCallable, Category = "Character|Lighting")
    void UpdateCharacterLighting(float FireIntensity, FLinearColor AmbientColor);

private:
    void InitializeEquipmentComponents();
    void UpdateMaterialParameters();
    void ApplyLightingToMaterials(float FireIntensity, FLinearColor AmbientColor);

    // Material Dynamic Instances
    UPROPERTY()
    UMaterialInstanceDynamic* BodyMaterialInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* ClothingMaterialInstance;
};