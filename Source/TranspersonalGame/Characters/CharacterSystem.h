#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "CharacterSystem.generated.h"

/**
 * Core Character System for Transpersonal Game
 * Handles MetaHuman integration, character variation, and visual diversity
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Dr. Protagonist"),
    TribalElder        UMETA(DisplayName = "Tribal Elder"),
    TribalWarrior      UMETA(DisplayName = "Tribal Warrior"),
    TribalHealer       UMETA(DisplayName = "Tribal Healer"),
    TribalChild        UMETA(DisplayName = "Tribal Child"),
    TribalCrafter      UMETA(DisplayName = "Tribal Crafter"),
    Survivor           UMETA(DisplayName = "Lost Survivor"),
    Hermit             UMETA(DisplayName = "Wilderness Hermit")
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    African            UMETA(DisplayName = "African"),
    Asian              UMETA(DisplayName = "Asian"),
    Caucasian          UMETA(DisplayName = "Caucasian"),
    Hispanic           UMETA(DisplayName = "Hispanic"),
    Indigenous         UMETA(DisplayName = "Indigenous"),
    MiddleEastern      UMETA(DisplayName = "Middle Eastern"),
    Mixed              UMETA(DisplayName = "Mixed Heritage")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child              UMETA(DisplayName = "Child (8-12)"),
    Teenager           UMETA(DisplayName = "Teenager (13-17)"),
    YoungAdult         UMETA(DisplayName = "Young Adult (18-30)"),
    MiddleAged         UMETA(DisplayName = "Middle Aged (31-50)"),
    Elder              UMETA(DisplayName = "Elder (51+)")
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterEthnicity Ethnicity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterAge Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsMale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackstoryNotes;

    // Physical variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BodyWeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinWear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarLevel;

    FCharacterVariationData()
    {
        Archetype = ECharacterArchetype::Protagonist;
        Ethnicity = ECharacterEthnicity::Caucasian;
        Age = ECharacterAge::YoungAdult;
        bIsMale = true;
        CharacterName = TEXT("Unnamed");
        BackstoryNotes = TEXT("");
        HeightVariation = 0.5f;
        BodyWeightVariation = 0.5f;
        MuscleDefinition = 0.5f;
        SkinWear = 0.3f;
        ScarLevel = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterSystem : public UObject
{
    GENERATED_BODY()

public:
    UCharacterSystem();

    // Generate a random character variation based on archetype
    UFUNCTION(BlueprintCallable, Category = "Character System")
    static FCharacterVariationData GenerateRandomCharacter(ECharacterArchetype Archetype);

    // Validate character data for consistency
    UFUNCTION(BlueprintCallable, Category = "Character System")
    static bool ValidateCharacterData(const FCharacterVariationData& CharacterData);

    // Get appropriate clothing/equipment for archetype
    UFUNCTION(BlueprintCallable, Category = "Character System")
    static TArray<FString> GetArchetypeClothing(ECharacterArchetype Archetype);

    // Get survival wear level based on environment exposure
    UFUNCTION(BlueprintCallable, Category = "Character System")
    static float CalculateSurvivalWear(float DaysInWilderness, ECharacterArchetype Archetype);

protected:
    // Character generation tables
    UPROPERTY(EditDefaultsOnly, Category = "Generation")
    TMap<ECharacterArchetype, TArray<FString>> ArchetypeNames;

    UPROPERTY(EditDefaultsOnly, Category = "Generation")
    TMap<ECharacterEthnicity, TArray<FString>> EthnicityFeatures;
};