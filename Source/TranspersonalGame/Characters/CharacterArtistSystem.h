#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataAsset.h"
#include "CharacterArtistSystem.generated.h"

/**
 * Character Archetype System for Transpersonal Game Studio
 * Defines visual archetypes for human characters in the prehistoric world
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonist - Paleontologist"),
    TribalHunter       UMETA(DisplayName = "Tribal Hunter"),
    TribalShaman       UMETA(DisplayName = "Tribal Shaman"),
    TribalElder        UMETA(DisplayName = "Tribal Elder"),
    TribalChild        UMETA(DisplayName = "Tribal Child"),
    TimeStranded       UMETA(DisplayName = "Time Stranded Survivor"),
    WildHermit         UMETA(DisplayName = "Wild Hermit"),
    MAX                UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male,
    Female,
    NonBinary
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child,      // 8-15 anos
    Young,      // 16-25 anos
    Adult,      // 26-45 anos
    Mature,     // 46-60 anos
    Elder       // 60+ anos
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    // MetaHuman Base Preset
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MetaHumanPresetID;

    // Physical Variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BodyMassVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeightVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FacialFeatureVariation = 0.5f;

    // Skin and Hair
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EyeColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    // Clothing and Accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ClothingPieces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Accessories;

    // Weathering and Scars
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ScarsAndMarks;
};

USTRUCT(BlueprintType)
struct FCharacterArchetypeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype ArchetypeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    // Visual Guidelines
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> TypicalAccessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PostureDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FacialExpressionGuideline;

    // Age and Gender Preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ECharacterAge> PreferredAges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ECharacterGender> PreferredGenders;

    // Variation Ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterVariationData BaseVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterVariationData VariationRange;
};

/**
 * Data Asset that defines all character archetypes for the game
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCharacterArchetypeDefinition> Archetypes;

    UFUNCTION(BlueprintCallable)
    FCharacterArchetypeDefinition GetArchetypeDefinition(ECharacterArchetype ArchetypeType) const;

    UFUNCTION(BlueprintCallable)
    TArray<ECharacterArchetype> GetAvailableArchetypes() const;
};

/**
 * Component responsible for managing character visual variations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterVariationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterVariationComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Current character data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype CurrentArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterAge Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterVariationData VariationData;

    // Unique identifier for this character instance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterInstanceID;

    // Reference to archetype database
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UCharacterArchetypeDatabase> ArchetypeDatabase;

    // MetaHuman skeletal mesh reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMeshComponent* MetaHumanMesh;

    UFUNCTION(BlueprintCallable)
    void GenerateRandomVariation(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable)
    void ApplyVariationToMesh();

    UFUNCTION(BlueprintCallable)
    void SetArchetype(ECharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable)
    FString GenerateUniqueCharacterID() const;

private:
    void ApplyPhysicalVariations();
    void ApplyClothingAndAccessories();
    void ApplyWeatheringAndScars();
    void ValidateVariationData();
};