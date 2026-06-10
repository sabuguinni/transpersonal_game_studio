#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Char_VisualCustomization.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_SkinTone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor BaseColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Roughness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Subsurface = 0.3f;

    FChar_SkinTone()
    {
        BaseColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        Roughness = 0.7f;
        Subsurface = 0.3f;
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
    FString ClothingName = TEXT("Basic Hide");

    FChar_ClothingSet()
    {
        ClothingName = TEXT("Basic Hide");
    }
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky")
};

UENUM(BlueprintType)
enum class EChar_Gender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

/**
 * Component responsible for character visual customization and appearance management
 * Handles skin tones, clothing, body types, and visual variations for NPCs and player
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_VisualCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_VisualCustomization();

protected:
    virtual void BeginPlay() override;

    // Character appearance properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_Gender CharacterGender = EChar_Gender::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_ClothingSet CurrentClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float HeightScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 0.5f;

    // Available customization options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    TArray<FChar_SkinTone> AvailableSkinTones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    TArray<FChar_ClothingSet> AvailableClothing;

    // Character mesh references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

public:
    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplySkinTone(const FChar_SkinTone& NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyClothingSet(const FChar_ClothingSet& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetGender(EChar_Gender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyTribalVariation(int32 TribeIndex);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character Customization")
    EChar_Gender GetGender() const { return CharacterGender; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    EChar_BodyType GetBodyType() const { return BodyType; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    FChar_SkinTone GetSkinTone() const { return SkinTone; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    FChar_ClothingSet GetClothingSet() const { return CurrentClothing; }

private:
    void InitializeDefaultAppearance();
    void UpdateCharacterMesh();
    void ApplyMaterialParameters();
    void LoadAvailableAssets();

    // Material parameter names
    static const FName SkinColorParam;
    static const FName RoughnessParam;
    static const FName SubsurfaceParam;
    static const FName MuscleMassParam;
};