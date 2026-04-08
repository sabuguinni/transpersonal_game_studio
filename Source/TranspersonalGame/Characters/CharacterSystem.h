#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonist - Paleontologist"),
    Survivor_Veteran    UMETA(DisplayName = "Veteran Survivor"),
    Survivor_Newcomer   UMETA(DisplayName = "Newcomer Survivor"),
    Tribal_Elder        UMETA(DisplayName = "Tribal Elder"),
    Tribal_Hunter       UMETA(DisplayName = "Tribal Hunter"),
    Tribal_Gatherer     UMETA(DisplayName = "Tribal Gatherer"),
    Tribal_Shaman       UMETA(DisplayName = "Tribal Shaman"),
    Mysterious_Wanderer UMETA(DisplayName = "Mysterious Wanderer"),
    Child_Survivor      UMETA(DisplayName = "Child Survivor"),
    Injured_Refugee     UMETA(DisplayName = "Injured Refugee")
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    European        UMETA(DisplayName = "European"),
    African         UMETA(DisplayName = "African"),
    Asian           UMETA(DisplayName = "Asian"),
    LatinAmerican   UMETA(DisplayName = "Latin American"),
    MiddleEastern   UMETA(DisplayName = "Middle Eastern"),
    NativeAmerican  UMETA(DisplayName = "Native American"),
    Oceanic         UMETA(DisplayName = "Oceanic"),
    Mixed           UMETA(DisplayName = "Mixed Heritage")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child       UMETA(DisplayName = "Child (8-12)"),
    Teen        UMETA(DisplayName = "Teen (13-17)"),
    YoungAdult  UMETA(DisplayName = "Young Adult (18-30)"),
    Adult       UMETA(DisplayName = "Adult (31-50)"),
    MiddleAge   UMETA(DisplayName = "Middle Age (51-65)"),
    Elder       UMETA(DisplayName = "Elder (66+)")
};

UENUM(BlueprintType)
enum class ECharacterBodyType : uint8
{
    Slim        UMETA(DisplayName = "Slim"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Average     UMETA(DisplayName = "Average"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Muscular    UMETA(DisplayName = "Muscular")
};

USTRUCT(BlueprintType)
struct FCharacterVisualTraits
{
    GENERATED_BODY()

    // MetaHuman Base Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanPresetID;

    // Physical Characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceLength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MouthSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CheekboneHeight = 0.5f;

    // Survival Marks and Weathering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinWeathering = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SunDamage = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fatigue = 0.3f;

    // Hair and Grooming
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grooming")
    FString HairStyleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grooming")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grooming", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HairLength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grooming", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HairMessiness = 0.6f;

    // Eyes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeBagIntensity = 0.3f;

    // Clothing System Reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingSetID;
};

USTRUCT(BlueprintType)
struct FCharacterPersonality
{
    GENERATED_BODY()

    // Core Personality Traits (affect animations and idle behaviors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Alertness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Friendliness = 0.5f;

    // Posture and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PostureConfidence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MovementSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeadMovementFrequency = 0.6f; // How often they look around (survival instinct)
};

USTRUCT(BlueprintType)
struct FCharacterBackstory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FText SurvivalSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    int32 DaysInJurassic = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FText> NotableExperiences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FText CurrentGoal;
};

USTRUCT(BlueprintType)
struct FCharacterDefinition : public FTableRowBase
{
    GENERATED_BODY()

    // Core Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterEthnicity Ethnicity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterAge AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterBodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    bool bIsMale = true;

    // Visual Definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FCharacterVisualTraits VisualTraits;

    // Personality and Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FCharacterPersonality Personality;

    // Narrative Background
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FCharacterBackstory Backstory;

    // Technical References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    TSoftObjectPtr<class USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    TSoftObjectPtr<class UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    TArray<TSoftObjectPtr<class UMaterialInterface>> ClothingMaterials;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterArchetype ArchetypeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FText ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FText ArchetypeDescription;

    // Default trait ranges for this archetype
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
    FCharacterPersonality DefaultPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
    float DefaultSurvivalExperience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> PreferredClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FText> TypicalBehaviors;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterGenerationSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    // MetaHuman Presets Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<FString> AvailableMetaHumanPresets;

    // Diversity Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TMap<ECharacterEthnicity, float> EthnicityDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TMap<ECharacterAge, float> AgeDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float GenderBalance = 0.5f; // 0.0 = all female, 1.0 = all male

    // Visual Variation Ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D SurvivalWearRange = FVector2D(0.3f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D ScarIntensityRange = FVector2D(0.0f, 0.6f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    FVector2D FatigueRange = FVector2D(0.2f, 0.7f);
};