#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_AppearanceManager.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class EChar_Gender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_AgeGroup : uint8
{
    Young       UMETA(DisplayName = "Young (18-30)"),
    Adult       UMETA(DisplayName = "Adult (30-50)"),
    Elder       UMETA(DisplayName = "Elder (50+)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_Gender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_AgeGroup AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole TribalRole;

    FChar_AppearanceData()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyScale = 1.0f;
        Gender = EChar_Gender::Male;
        AgeGroup = EChar_AgeGroup::Adult;
        TribalRole = EChar_TribalRole::Hunter;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_TribalRole SuitableForRole;

    FChar_TribalClothing()
    {
        ClothingName = TEXT("Basic Hide");
        SuitableForRole = EChar_TribalRole::Hunter;
    }
};

/**
 * Character Appearance Manager - Handles visual customization of tribal characters
 * Manages skin tones, clothing, accessories, and role-based appearance variations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_AppearanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_AppearanceManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TArray<FChar_AppearanceData> PresetAppearances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TArray<FChar_TribalClothing> AvailableClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    float SkinToneVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    float BodyScaleVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    bool bRandomizeOnStart;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearanceData(const FChar_AppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetGender(EChar_Gender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetAgeGroup(EChar_AgeGroup NewAgeGroup);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GenerateRandomAppearance(EChar_TribalRole Role = EChar_TribalRole::Hunter);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyTribalClothing(const FChar_TribalClothing& Clothing);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    TArray<FChar_TribalClothing> GetClothingForRole(EChar_TribalRole Role) const;

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateMeshMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    USkeletalMeshComponent* GetCharacterMesh() const;

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    FChar_AppearanceData GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    EChar_TribalRole GetTribalRole() const { return CurrentAppearance.TribalRole; }

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    EChar_Gender GetGender() const { return CurrentAppearance.Gender; }

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    EChar_AgeGroup GetAgeGroup() const { return CurrentAppearance.AgeGroup; }

private:
    void InitializePresetAppearances();
    void InitializeTribalClothing();
    FLinearColor GenerateRandomSkinTone() const;
    FLinearColor GenerateRandomHairColor() const;
    float GenerateRandomBodyScale() const;
};