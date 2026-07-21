#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "Char_VisualCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterVisualPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Preset")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Preset")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Preset")
    FVector MeshScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Preset")
    ECharacterArchetype ArchetypeCategory;

    FChar_CharacterVisualPreset()
    {
        PresetName = TEXT("Default");
        MeshScale = FVector(1.0f, 1.0f, 1.0f);
        ArchetypeCategory = ECharacterArchetype::Survivor;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> TorseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> LegsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> FeetMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor;

    FChar_TribalClothingSet()
    {
        ClothingSetName = TEXT("Basic Leather");
        PrimaryColor = FLinearColor::Brown;
        SecondaryColor = FLinearColor::Black;
    }
};

/**
 * Visual Character Manager - Handles character appearance, clothing, and visual diversity
 * Integrates with MetaHuman Creator workflow and tribal character customization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_VisualCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_VisualCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Presets")
    TArray<FChar_CharacterVisualPreset> TribalCharacterPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing System")
    TArray<FChar_TribalClothingSet> AvailableClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    int32 MaxCharacterVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    float GenderRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    float AgeVariationRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    bool ApplyVisualPresetToCharacter(class ACharacter* TargetCharacter, const FChar_CharacterVisualPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterVisualPreset GenerateRandomTribalPreset();

    UFUNCTION(BlueprintCallable, Category = "Clothing System")
    bool ApplyClothingSetToCharacter(class ACharacter* TargetCharacter, const FChar_TribalClothingSet& ClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    TArray<FChar_CharacterVisualPreset> GenerateCharacterVariations(int32 Count, ECharacterArchetype ArchetypeFilter);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman Integration")
    bool SetupMetaHumanCharacter(class ACharacter* TargetCharacter, const FString& MetaHumanAssetPath);

    UFUNCTION(BlueprintCallable, Category = "Visual Management")
    void InitializeCharacterVisualSystem();

    UFUNCTION(BlueprintCallable, Category = "Visual Management")
    bool ValidateCharacterMeshCompatibility(USkeletalMesh* TestMesh);

private:
    void LoadDefaultTribalPresets();
    void LoadDefaultClothingSets();
    FChar_TribalClothingSet CreateClothingVariation(const FChar_TribalClothingSet& BaseSet, float VariationAmount);
    bool ApplyMaterialToMeshComponent(USkeletalMeshComponent* MeshComp, UMaterialInterface* Material, int32 MaterialIndex);
};