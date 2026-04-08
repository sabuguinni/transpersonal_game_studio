#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Protective      UMETA(DisplayName = "Protective"),
    Skittish        UMETA(DisplayName = "Skittish")
};

UENUM(BlueprintType)
enum class EDinosaurMoodState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Alert           UMETA(DisplayName = "Alert"),
    Agitated        UMETA(DisplayName = "Agitated"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Thirsty         UMETA(DisplayName = "Thirsty"),
    Tired           UMETA(DisplayName = "Tired"),
    Playful         UMETA(DisplayName = "Playful"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Protective      UMETA(DisplayName = "Protective")
};

UENUM(BlueprintType)
enum class EDinosaurActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Grooming        UMETA(DisplayName = "Grooming")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny            UMETA(DisplayName = "Tiny"),      // < 0.5m
    Small           UMETA(DisplayName = "Small"),     // 0.5-2m
    Medium          UMETA(DisplayName = "Medium"),    // 2-6m
    Large           UMETA(DisplayName = "Large"),     // 6-12m
    Massive         UMETA(DisplayName = "Massive")    // > 12m
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Omnivore        UMETA(DisplayName = "Omnivore"),
    Piscivore       UMETA(DisplayName = "Piscivore"),
    Insectivore     UMETA(DisplayName = "Insectivore")
};

UENUM(BlueprintType)
enum class EDomesticationStage : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Wary            UMETA(DisplayName = "Wary"),
    Curious         UMETA(DisplayName = "Curious"),
    Accepting       UMETA(DisplayName = "Accepting"),
    Trusting        UMETA(DisplayName = "Trusting"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FDinosaurMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> AssociatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate;

    FDinosaurMemoryEntry()
    {
        Location = FVector::ZeroVector;
        EventType = FGameplayTag::EmptyTag;
        Intensity = 1.0f;
        TimeStamp = 0.0f;
        AssociatedActor = nullptr;
        DecayRate = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort;

    FDinosaurNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Social = 50.0f;
        Safety = 100.0f;
        Comfort = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float Affinity; // -100 (hostile) to +100 (bonded)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Familiarity; // 0 (unknown) to 100 (well known)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> SharedExperiences;

    FDinosaurRelationship()
    {
        TargetActor = nullptr;
        Affinity = 0.0f;
        Familiarity = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority;

    FDailyRoutineEntry()
    {
        StartHour = 0.0f;
        EndHour = 1.0f;
        Activity = EDinosaurActivity::Idle;
        PreferredLocation = FVector::ZeroVector;
        LocationRadius = 500.0f;
        Priority = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize SizeCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurDiet DietType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsTerritorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CuriosityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialNeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDailyRoutineEntry> DefaultRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredHabitats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> FearTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeDistance;

    FDinosaurSpeciesData()
    {
        SpeciesName = TEXT("Unknown Species");
        SizeCategory = EDinosaurSize::Medium;
        DietType = EDinosaurDiet::Herbivore;
        bCanBeDomesticated = false;
        bIsPackAnimal = false;
        bIsTerritorial = false;
        bIsNocturnal = false;
        AggressionLevel = 0.3f;
        CuriosityLevel = 0.5f;
        SocialNeed = 0.5f;
        DetectionRange = 1000.0f;
        FleeDistance = 2000.0f;
    }
};