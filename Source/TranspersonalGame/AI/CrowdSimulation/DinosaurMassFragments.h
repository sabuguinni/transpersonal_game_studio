#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Engine/DataTable.h"
#include "DinosaurMassFragments.generated.h"

UENUM(BlueprintType)
enum class EAI_DinosaurDiet : uint8
{
    Herbivore,
    Carnivore,
    Omnivore,
    Piscivore
};

UENUM(BlueprintType)
enum class EAI_DinosaurSize_9A1 : uint8
{
    Tiny,       // < 1m (Compsognathus)
    Small,      // 1-3m (Dryosaurus)
    Medium,     // 3-8m (Parasaurolophus)
    Large,      // 8-15m (Triceratops)
    Massive     // > 15m (Brachiosaurus, T-Rex)
};

UENUM(BlueprintType)
enum class EAI_DinosaurBehaviorState_9A1 : uint8
{
    Idle,
    Foraging,
    Drinking,
    Resting,
    Socializing,
    Hunting,
    Fleeing,
    Territorial,
    Nesting,
    Migrating,
    BeingDomesticated
};

UENUM(BlueprintType)
enum class EAI_DinosaurAggressionLevel : uint8
{
    Passive,
    Defensive,
    Territorial,
    Aggressive,
    Apex
};

/**
 * Core species data for each dinosaur type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_DinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurDiet Diet = EAI_DinosaurDiet::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurSize_9A1 Size = EAI_DinosaurSize_9A1::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurAggressionLevel AggressionLevel = EAI_DinosaurAggressionLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 800.0f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 2000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f; // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredPackSize = 1;
};

/**
 * Individual dinosaur characteristics that make each one unique
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_DinosaurIndividualFragment : public FMassFragment
{
    GENERATED_BODY()

    // Unique physical variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f; // 0.8 to 1.2 multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MarkingIntensity = 0.5f;

    // Behavioral traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Boldness = 0.5f; // 0.0 = very cautious, 1.0 = very bold

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sociability = 0.5f; // 0.0 = solitary, 1.0 = highly social

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 0.5f; // 0.0 = avoids new things, 1.0 = investigates everything

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionModifier = 1.0f; // Multiplier for species base aggression

    // Unique identifier for tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid UniqueID;

    // Player relationship
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerFamiliarity = 0.0f; // 0.0 = stranger, 1.0 = fully domesticated

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrust = 0.0f; // -1.0 = hostile, 0.0 = neutral, 1.0 = trusting
};

/**
 * Current behavior state and routine management
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_DinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurBehaviorState_9A1 CurrentState = EAI_DinosaurBehaviorState_9A1::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurBehaviorState_9A1 PreviousState = EAI_DinosaurBehaviorState_9A1::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateDuration = 10.0f; // How long to stay in current state

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle TargetEntity; // For hunting, following, etc.

    // Daily routine tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DayTimePreference = 0.5f; // 0.0 = nocturnal, 1.0 = diurnal

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastFeedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDrinkTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastRestTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger = 0.0f; // 0.0 = satisfied, 1.0 = starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thirst = 0.0f; // 0.0 = hydrated, 1.0 = dehydrated

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fatigue = 0.0f; // 0.0 = rested, 1.0 = exhausted
};

/**
 * Social group and pack behavior
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_DinosaurSocialFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle PackLeader;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PackCohesion = 1.0f; // How tightly the pack stays together

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PackCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialRank = 0.5f; // 0.0 = omega, 1.0 = alpha

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSocialInteraction = 0.0f;
};

/**
 * Territory and spatial awareness
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDefendsTerritory = false;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> KnownIntruders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialAggression = 0.5f;

    // Migration patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMigratory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> MigrationRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentMigrationWaypoint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSeason = 0.0f; // 0.0-1.0 through the year
};

/**
 * Domestication progress and player interaction
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurDomesticationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DomesticationProgress = 0.0f; // 0.0 = wild, 1.0 = fully domesticated

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerInteractionTime = 0.0f; // Total time spent near player

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions = 0; // Fed, petted, protected

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions = 0; // Attacked, startled, threatened

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerInteraction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFollowsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FollowDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRespondsToCommands = false;

    // Domestication stages
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bToleratesPlayerPresence = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAcceptsFoodFromPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowsPlayerApproach = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSeeksPlayerCompany = false;
};