#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny (0-50cm)"),
    Small       UMETA(DisplayName = "Small (50cm-1.5m)"),
    Medium      UMETA(DisplayName = "Medium (1.5m-3m)"),
    Large       UMETA(DisplayName = "Large (3m-6m)"),
    Massive     UMETA(DisplayName = "Massive (6m+)")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Insectivore UMETA(DisplayName = "Insectivore"),
    Piscivore   UMETA(DisplayName = "Piscivore")
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive - Never attacks"),
    Defensive   UMETA(DisplayName = "Defensive - Only when threatened"),
    Territorial UMETA(DisplayName = "Territorial - Defends area"),
    Aggressive  UMETA(DisplayName = "Aggressive - Actively hunts"),
    Apex        UMETA(DisplayName = "Apex - Dominates all others")
};

UENUM(BlueprintType)
enum class EDinosaurSocialType : uint8
{
    Solitary    UMETA(DisplayName = "Solitary"),
    Pair        UMETA(DisplayName = "Pair Bonded"),
    SmallPack   UMETA(DisplayName = "Small Pack (3-6)"),
    LargePack   UMETA(DisplayName = "Large Pack (7-15)"),
    Herd        UMETA(DisplayName = "Herd (16+)")
};

UENUM(BlueprintType)
enum class EDinosaurActivity : uint8
{
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Resting     UMETA(DisplayName = "Resting"),
    Foraging    UMETA(DisplayName = "Foraging/Hunting"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Socializing UMETA(DisplayName = "Socializing"),
    Patrolling  UMETA(DisplayName = "Patrolling Territory"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stalking    UMETA(DisplayName = "Stalking Prey"),
    Mating      UMETA(DisplayName = "Mating Behavior"),
    Nesting     UMETA(DisplayName = "Nesting/Caring for Young")
};

UENUM(BlueprintType)
enum class EDinosaurMood : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Nervous     UMETA(DisplayName = "Nervous"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Curious     UMETA(DisplayName = "Curious"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Tired       UMETA(DisplayName = "Tired")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Unique identifier for this individual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IndividualName;

    // Physical variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float SizeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorVariation = FLinearColor::White;

    FDinosaurPersonality()
    {
        // Generate random personality on creation
        Aggression = FMath::RandRange(0.0f, 1.0f);
        Curiosity = FMath::RandRange(0.0f, 1.0f);
        Fearfulness = FMath::RandRange(0.0f, 1.0f);
        Sociability = FMath::RandRange(0.0f, 1.0f);
        Intelligence = FMath::RandRange(0.0f, 1.0f);
        
        SizeVariation = FMath::RandRange(0.8f, 1.2f);
        ColorVariation = FLinearColor(
            FMath::RandRange(0.8f, 1.2f),
            FMath::RandRange(0.8f, 1.2f),
            FMath::RandRange(0.8f, 1.2f),
            1.0f
        );
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RememberedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalValue = 0.0f; // -1.0 (enemy) to 1.0 (friend)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EncounterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag RelationshipType;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize Size = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurDiet Diet = EDinosaurDiet::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSocialType SocialType = EDinosaurSocialType::Solitary;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float TerritoryRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DomesticationPotential = 0.0f; // 0 = cannot be domesticated, 1 = easily domesticated

    // Daily routine timings (0.0 to 24.0 hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> ActiveHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> RestingHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FeedingHours;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurDailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<float, EDinosaurActivity> ScheduledActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> FeedingSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterSources;
};