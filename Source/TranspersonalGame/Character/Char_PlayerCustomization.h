#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "../SharedTypes.h"
#include "Char_PlayerCustomization.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WarPaintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild; // 0.0 = lean, 1.0 = muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel; // 0.0 = clean, 1.0 = heavily weathered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> EquippedAccessories;

    FChar_TribalAppearance()
    {
        TribalRole = EChar_TribalRole::Hunter;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        WarPaintColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
        BodyBuild = 0.5f;
        WeatheringLevel = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> FeetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_TribalRole RequiredRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float DurabilityLevel; // 0.0 = tattered, 1.0 = pristine

    FChar_ClothingSet()
    {
        RequiredRole = EChar_TribalRole::Hunter;
        DurabilityLevel = 0.7f;
    }
};

/**
 * Player Character Customization System
 * Handles visual appearance, clothing, and tribal identity for the player character
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerCustomization();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_TribalAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_ClothingSet CurrentClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TArray<FChar_ClothingSet> AvailableClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    USkeletalMeshComponent* TargetMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    UMaterialInstanceDynamic* DynamicSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    UMaterialInstanceDynamic* DynamicHairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TArray<TSoftObjectPtr<USkeletalMesh>> TribalMeshVariants;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ChangeClothingSet(const FChar_ClothingSet& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddTribalMarkings(const FLinearColor& MarkingColor, const FString& MarkingPattern);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_TribalAppearance GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTargetMeshComponent(USkeletalMeshComponent* MeshComp);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void LoadClothingPresets();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    bool ValidateClothingForRole(const FChar_ClothingSet& Clothing, EChar_TribalRole Role);

private:
    void UpdateMaterialParameters();
    void ApplyBodyBuildModifications();
    void LoadTribalMeshVariants();
    void CreateDynamicMaterials();
    FLinearColor GenerateRandomSkinTone();
    FLinearColor GenerateRandomHairColor();
    void ApplyRoleBasedModifications(EChar_TribalRole Role);
};