#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "CharacterSystem.generated.h"

// Enum for character archetypes
UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist - Paleontologist"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    TribalShaman    UMETA(DisplayName = "Tribal Shaman"),
    TribalCrafter   UMETA(DisplayName = "Tribal Crafter"),
    TribalChild     UMETA(DisplayName = "Tribal Child"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Survivor        UMETA(DisplayName = "Other Time Traveler"),
    Hermit          UMETA(DisplayName = "Isolated Survivor")
};

// Enum for age groups
UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child       UMETA(DisplayName = "Child (8-16)"),
    YoungAdult  UMETA(DisplayName = "Young Adult (17-30)"),
    Adult       UMETA(DisplayName = "Adult (31-50)"),
    MiddleAged  UMETA(DisplayName = "Middle Aged (51-65)"),
    Elder       UMETA(DisplayName = "Elder (65+)")
};

// Struct for character physical traits
USTRUCT(BlueprintType)
struct FCharacterPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 1.0f; // Multiplier for base height

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 1.0f; // Multiplier for base weight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleDefinition = 0.5f; // 0 = lean, 1 = very muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor EyeColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
};

// Struct for character survival marks
USTRUCT(BlueprintType)
struct FCharacterSurvivalMarks
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Marks")
    float ScarIntensity = 0.0f; // 0 = no scars, 1 = heavily scarred

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Marks")
    float WeatheringLevel = 0.0f; // Sun damage, exposure

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Marks")
    float DirtLevel = 0.0f; // How dirty/unkempt

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Marks")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Marks")
    bool bHasBattleScars = false;
};

// Main character data structure
USTRUCT(BlueprintType)
struct FCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    ECharacterAge AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FCharacterPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FCharacterSurvivalMarks SurvivalMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> ClothingAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText PersonalityTraits;
};

// Character generation system
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterGenerationSystem : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterData GenerateRandomCharacter(ECharacterArchetype Archetype, bool bIsMale = true);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterData GenerateProtagonist();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static TArray<FCharacterData> GenerateTribalGroup(int32 GroupSize = 5);

private:
    static FCharacterPhysicalTraits GeneratePhysicalTraits(ECharacterArchetype Archetype, ECharacterAge Age, bool bIsMale);
    static FCharacterSurvivalMarks GenerateSurvivalMarks(ECharacterArchetype Archetype, ECharacterAge Age);
    static FString GenerateName(ECharacterArchetype Archetype, bool bIsMale);
};