#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DataTable.h"
#include "Char_PrimitiveWarrior.generated.h"

// TODO_ASSET_GENERATION_FAILED: Tribal warrior 3D model and concept art
// Specifications for manual asset creation:
// - Full body primitive human character in T-pose
// - Muscular build, weathered skin with tribal markings
// - Animal hide clothing (leather wraps, fur pelts)
// - Bone jewelry and necklaces
// - Stone tools and weapons (spear, axe, knife)
// - Realistic human proportions for Cretaceous period
// - Game-ready topology ~25k polygons
// - PBR materials for skin, clothing, bone, stone

UENUM(BlueprintType)
enum class EChar_TribalRank : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Chieftain   UMETA(DisplayName = "Chieftain"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EChar_WeaponType : uint8
{
    Spear       UMETA(DisplayName = "Stone Spear"),
    Club        UMETA(DisplayName = "Bone Club"),
    Axe         UMETA(DisplayName = "Stone Axe"),
    Bow         UMETA(DisplayName = "Primitive Bow"),
    Knife       UMETA(DisplayName = "Flint Knife")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* WarPaintMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WarPaintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BattleScars;

    FChar_TribalAppearance()
    {
        SkinMaterial = nullptr;
        ClothingMaterial = nullptr;
        WarPaintMaterial = nullptr;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        WarPaintColor = FLinearColor(0.8f, 0.2f, 0.1f, 1.0f);
        MuscleMass = 1.0f;
        BattleScars = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMesh* WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    EChar_WeaponType WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMesh* JewelryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMesh* ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName WeaponSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FName JewelrySocket;

    FChar_TribalEquipment()
    {
        WeaponMesh = nullptr;
        WeaponType = EChar_WeaponType::Spear;
        JewelryMesh = nullptr;
        ClothingMesh = nullptr;
        WeaponSocket = TEXT("WeaponSocket");
        JewelrySocket = TEXT("NeckSocket");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveWarrior : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveWarrior();

protected:
    virtual void BeginPlay() override;

    // Character appearance system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FChar_TribalAppearance TribalAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Equipment")
    FChar_TribalEquipment TribalEquipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
    EChar_TribalRank TribalRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
    FString TribalName;

    // Equipment attachment components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WeaponComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* JewelryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ClothingComponent;

    // Material instances for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseWarPaintMaterial;

public:
    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void SetTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipWeapon(UStaticMesh* WeaponMesh, EChar_WeaponType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipJewelry(UStaticMesh* JewelryMesh);

    UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
    void EquipClothing(UStaticMesh* ClothingMesh);

    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ApplyWarPaint(const FLinearColor& PaintColor, UMaterialInterface* PaintMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character|Identity")
    void SetTribalRank(EChar_TribalRank NewRank);

    UFUNCTION(BlueprintPure, Category = "Character|Identity")
    EChar_TribalRank GetTribalRank() const { return TribalRank; }

    UFUNCTION(BlueprintPure, Category = "Character|Identity")
    FString GetTribalName() const { return TribalName; }

    // Material customization
    UFUNCTION(BlueprintCallable, Category = "Character|Materials")
    void UpdateSkinMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character|Materials")
    void UpdateClothingMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character|Materials")
    void UpdateWarPaintMaterial();

protected:
    // Internal setup functions
    void SetupEquipmentComponents();
    void SetupMaterialInstances();
    void AttachEquipmentToSockets();
    void ApplyTribalCustomization();

    // Material instance references
    UPROPERTY()
    UMaterialInstanceDynamic* SkinMaterialInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* ClothingMaterialInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* WarPaintMaterialInstance;
};