#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NPCBehaviorTypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpeciesType : uint8
{
    SmallHerbivore    UMETA(DisplayName = "Small Herbivore"),
    MediumHerbivore   UMETA(DisplayName = "Medium Herbivore"),
    LargeHerbivore    UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore    UMETA(DisplayName = "Small Carnivore"),
    MediumCarnivore   UMETA(DisplayName = "Medium Carnivore"),
    LargeCarnivore    UMETA(DisplayName = "Large Carnivore"),
    Omnivore          UMETA(DisplayName = "Omnivore"),
    Flying            UMETA(DisplayName = "Flying"),
    Aquatic           UMETA(DisplayName = "Aquatic")
};

UENUM(BlueprintType)
enum class EBehaviorState : uint8
{
    Idle              UMETA(DisplayName = "Idle"),
    Foraging          UMETA(DisplayName = "Foraging"),
    Hunting           UMETA(DisplayName = "Hunting"),
    Fleeing           UMETA(DisplayName = "Fleeing"),
    Drinking          UMETA(DisplayName = "Drinking"),
    Resting           UMETA(DisplayName = "Resting"),
    Socializing       UMETA(DisplayName = "Socializing"),
    Territorial       UMETA(DisplayName = "Territorial"),
    Mating            UMETA(DisplayName = "Mating"),
    Nesting           UMETA(DisplayName = "Nesting"),
    Migrating         UMETA(DisplayName = "Migrating"),
    Investigating     UMETA(DisplayName = "Investigating"),
    Domesticated      UMETA(DisplayName = "Domesticated")
};

UENUM(BlueprintType)
enum class EPersonalityTrait : uint8
{
    Aggressive        UMETA(DisplayName = "Aggressive"),
    Timid             UMETA(DisplayName = "Timid"),
    Curious           UMETA(DisplayName = "Curious"),
    Territorial       UMETA(DisplayName = "Territorial"),
    Social            UMETA(DisplayName = "Social"),
    Solitary          UMETA(DisplayName = "Solitary"),
    Protective        UMETA(DisplayName = "Protective"),
    Opportunistic     UMETA(DisplayName = "Opportunistic"),
    Cautious          UMETA(DisplayName = "Cautious"),
    Bold              UMETA(DisplayName = "Bold")
};

UENUM(BlueprintType)
enum class ETrustLevel : uint8
{
    Hostile           UMETA(DisplayName = "Hostile"),
    Fearful           UMETA(DisplayName = "Fearful"),
    Wary              UMETA(DisplayName = "Wary"),
    Neutral           UMETA(DisplayName = "Neutral"),
    Curious           UMETA(DisplayName = "Curious"),
    Accepting         UMETA(DisplayName = "Accepting"),
    Trusting          UMETA(DisplayName = "Trusting"),
    Bonded            UMETA(DisplayName = "Bonded")
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<EPersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CuriosityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel = 0.5f;

    FDinosaurPersonality()
    {
        PrimaryTraits.Add(EPersonalityTrait::Cautious);
    }
};

USTRUCT(BlueprintType)
struct FMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FGameplayTag EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> AssociatedActor;

    FMemoryEntry()
    {
        Location = FVector::ZeroVector;
        EmotionalWeight = 0.0f;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float StartTime = 0.0f; // 0.0 = Dawn, 0.5 = Noon, 1.0 = Dusk

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float Duration = 0.25f; // Fraction of day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    EBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float LocationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    int32 Priority = 1;

    FDailyRoutine()
    {
        BehaviorState = EBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FSpeciesBehaviorData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpeciesType SpeciesType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FDailyRoutine> DefaultRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<EBehaviorState> AvailableBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationDifficulty = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    TArray<FGameplayTag> PreferredTrustActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 PreferredPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float HungerRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float ThirstRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float RestRate = 1.0f;

    FSpeciesBehaviorData()
    {
        SpeciesType = EDinosaurSpeciesType::SmallHerbivore;
        SpeciesName = TEXT("Unknown Species");
        bCanBeDomesticated = false;
        DomesticationDifficulty = 1.0f;
        bIsPackAnimal = false;
        PreferredPackSize = 1;
        TerritoryRadius = 1000.0f;
        HungerRate = 1.0f;
        ThirstRate = 1.0f;
        RestRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FRelationshipData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ETrustLevel TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Familiarity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TArray<FMemoryEntry> SharedMemories;

    FRelationshipData()
    {
        TrustLevel = ETrustLevel::Neutral;
        Familiarity = 0.0f;
        LastInteractionTime = 0.0f;
    }
};