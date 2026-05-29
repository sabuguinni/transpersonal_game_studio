#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Char_TribalPlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FChar_TribalAppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UStaticMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UStaticMesh> WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    FChar_TribalAppearanceData()
        : CharacterName(TEXT("Tribal Hunter"))
        , SkinTone(FLinearColor(0.8f, 0.6f, 0.4f, 1.0f))
        , HairColor(FLinearColor(0.2f, 0.1f, 0.05f, 1.0f))
        , MuscleMass(1.0f)
        , Height(1.0f)
    {}
};

UENUM(BlueprintType)
enum class EChar_TribalGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Crafter     UMETA(DisplayName = "Crafter")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalPlayerCharacter();

protected:
    virtual void BeginPlay() override;

    // Character appearance components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    class UStaticMeshComponent* ClothingMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    class UStaticMeshComponent* WeaponMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    class UStaticMeshComponent* AccessoryMeshComponent;

    // Character data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    FChar_TribalAppearanceData AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    EChar_TribalGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    class UDataTable* AppearanceDataTable;

public:
    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetCharacterAppearance(const FChar_TribalAppearanceData& NewAppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetGender(EChar_TribalGender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplyRandomAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetSkinTone(const FLinearColor& NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetHairColor(const FLinearColor& NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetBodyScale(float NewHeight, float NewMuscleMass);

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Character Equipment")
    void EquipWeapon(UStaticMesh* WeaponMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Equipment")
    void EquipClothing(UStaticMesh* ClothingMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Equipment")
    void EquipAccessory(UStaticMesh* AccessoryMesh);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character Info")
    FChar_TribalAppearanceData GetAppearanceData() const { return AppearanceData; }

    UFUNCTION(BlueprintPure, Category = "Character Info")
    EChar_TribalGender GetGender() const { return Gender; }

    UFUNCTION(BlueprintPure, Category = "Character Info")
    EChar_TribalRole GetTribalRole() const { return TribalRole; }

private:
    void UpdateCharacterMesh();
    void UpdateMaterials();
    void LoadAppearanceFromDataTable();
    FChar_TribalAppearanceData GenerateRandomAppearance();
};