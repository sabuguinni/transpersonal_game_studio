#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_TribalAppearanceSystem.generated.h"

// Tribal appearance enums for character customization
UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    PaleBronze      UMETA(DisplayName = "Pale Bronze"),
    MediumBronze    UMETA(DisplayName = "Medium Bronze"),
    DarkBronze      UMETA(DisplayName = "Dark Bronze"),
    DeepBronze      UMETA(DisplayName = "Deep Bronze"),
    WeatheredTan    UMETA(DisplayName = "Weathered Tan")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean            UMETA(DisplayName = "Lean Hunter"),
    Muscular        UMETA(DisplayName = "Muscular Warrior"),
    Stocky          UMETA(DisplayName = "Stocky Gatherer"),
    Tall            UMETA(DisplayName = "Tall Scout"),
    Elder           UMETA(DisplayName = "Elder Wise")
};

UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None            UMETA(DisplayName = "No Markings"),
    HunterStripes   UMETA(DisplayName = "Hunter Stripes"),
    SpiralTattoos   UMETA(DisplayName = "Spiral Tattoos"),
    WarPaint        UMETA(DisplayName = "War Paint"),
    SacredSymbols   UMETA(DisplayName = "Sacred Symbols"),
    ScarRituals     UMETA(DisplayName = "Ritual Scars")
};

UENUM(BlueprintType)
enum class EChar_TribalClothing : uint8
{
    Minimal         UMETA(DisplayName = "Minimal Hide"),
    Hunter          UMETA(DisplayName = "Hunter Leathers"),
    Gatherer        UMETA(DisplayName = "Woven Fibers"),
    Warrior         UMETA(DisplayName = "Battle Hides"),
    Shaman          UMETA(DisplayName = "Ceremonial Robes")
};

// Tribal appearance configuration struct
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::MediumBronze;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Muscular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarkings TribalMarkings = EChar_TribalMarkings::HunterStripes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalClothing Clothing = EChar_TribalClothing::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.1f, 0.05f, 0.02f, 1.0f); // Dark brown

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Brown eyes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleDefinition = 0.7f;

    FChar_TribalAppearance()
    {
        SkinTone = EChar_SkinTone::MediumBronze;
        BodyBuild = EChar_BodyBuild::Muscular;
        TribalMarkings = EChar_TribalMarkings::HunterStripes;
        Clothing = EChar_TribalClothing::Hunter;
        WeatheringIntensity = 0.5f;
        MuscleDefinition = 0.7f;
    }
};

/**
 * Tribal Appearance System Component
 * Handles visual customization for primitive human characters in Cretaceous survival setting
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalAppearanceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalAppearanceSystem();

protected:
    virtual void BeginPlay() override;

public:
    // Current appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FChar_TribalAppearance CurrentAppearance;

    // Material references for different skin tones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinToneMaterials;

    // Clothing mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    // Tribal marking textures
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> TribalMarkingMaterials;

    // Apply appearance to character mesh
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void ApplyTribalAppearance(USkeletalMeshComponent* CharacterMesh);

    // Randomize appearance for NPC generation
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void RandomizeTribalAppearance();

    // Apply specific skin tone
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    // Apply specific body build
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    // Apply tribal markings
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void SetTribalMarkings(EChar_TribalMarkings NewMarkings);

    // Apply clothing style
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void SetTribalClothing(EChar_TribalClothing NewClothing);

    // Get current appearance as string for debugging
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    FString GetAppearanceDescription() const;

private:
    // Internal material application
    void ApplySkinToneMaterial(USkeletalMeshComponent* Mesh);
    void ApplyTribalMarkingMaterial(USkeletalMeshComponent* Mesh);
    void ApplyClothingMaterial(USkeletalMeshComponent* Mesh);
    void ApplyBodyBuildScale(USkeletalMeshComponent* Mesh);

    // Cache for loaded materials
    UPROPERTY()
    TMap<EChar_SkinTone, UMaterialInterface*> LoadedSkinMaterials;

    UPROPERTY()
    TMap<EChar_TribalMarkings, UMaterialInterface*> LoadedMarkingMaterials;

    UPROPERTY()
    TMap<EChar_TribalClothing, UMaterialInterface*> LoadedClothingMaterials;
};