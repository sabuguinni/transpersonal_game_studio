#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Char_MetaHumanManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FaceVariant;

    FChar_CharacterAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyScale = 1.0f;
        FaceVariant = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FString VariantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    ETribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FChar_CharacterAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    TArray<TSoftObjectPtr<UStaticMesh>> EquipmentMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float AgeModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    bool bHasTribalMarkings;

    FChar_TribalVariant()
    {
        VariantName = TEXT("Default");
        TribalRole = ETribalRole::Gatherer;
        AgeModifier = 0.0f;
        bHasTribalMarkings = false;
    }
};

/**
 * MetaHuman Manager - Handles character creation, appearance customization, and tribal variants
 * Manages the visual diversity of characters in the Cretaceous world
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    // === CHARACTER APPEARANCE SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<FChar_TribalVariant> TribalVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<TSoftObjectPtr<USkeletalMesh>> BaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<TSoftObjectPtr<UMaterialInterface>> HairMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    // === METAHUMAN INTEGRATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    bool bUseMetaHumans;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<USkeletalMesh>> MetaHumanMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    int32 MaxCharacterVariants;

public:
    // === CHARACTER CREATION ===
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterAppearance CreateRandomAppearance(ETribalRole Role = ETribalRole::Gatherer);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_TribalVariant CreateTribalVariant(const FString& Name, ETribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    bool ApplyAppearanceToCharacter(AActor* Character, const FChar_CharacterAppearance& Appearance);

    // === APPEARANCE CUSTOMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetCharacterSkinTone(AActor* Character, const FLinearColor& SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetCharacterHairColor(AActor* Character, const FLinearColor& HairColor);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetCharacterBodyScale(AActor* Character, float Scale);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyTribalMarkings(AActor* Character, bool bApply);

    // === TRIBAL SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FChar_TribalVariant GetTribalVariantByRole(ETribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    TArray<FChar_TribalVariant> GetAllTribalVariants() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void RegisterTribalVariant(const FChar_TribalVariant& Variant);

    // === METAHUMAN INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool LoadMetaHumanAssets();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    USkeletalMesh* GetRandomMetaHumanMesh();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool IsMetaHumanAvailable() const;

    // === UTILITY ===
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeDefaultVariants();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetCharacterVariantCount() const;

private:
    void LoadDefaultAssets();
    void CreateDefaultTribalVariants();
    FLinearColor GenerateRandomSkinTone();
    FLinearColor GenerateRandomHairColor();
};