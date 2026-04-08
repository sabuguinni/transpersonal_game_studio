#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Engine/DataTable.h"
#include "DinosaurMassFragments.generated.h"

// Forward declarations
class UBehaviorTree;

/**
 * Dinosaur species classification for behavior differentiation
 */
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Large Carnivores
    TRex,
    Allosaurus,
    Spinosaurus,
    
    // Pack Hunters
    Velociraptor,
    Deinonychus,
    Utahraptor,
    
    // Large Herbivores
    Triceratops,
    Brontosaurus,
    Stegosaurus,
    
    // Small Herbivores
    Parasaurolophus,
    Gallimimus,
    Compsognathus,
    
    // Flying
    Pteranodon,
    Quetzalcoatlus,
    
    // Aquatic
    Mosasaurus,
    Plesiosaur
};

/**
 * Behavioral states for dinosaur AI
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Hunting,
    Fleeing,
    Territorial,
    Migrating,
    Resting,
    Socializing,
    Nesting,
    Drinking,
    Patrolling
};

/**
 * Individual dinosaur characteristics - makes each one unique and recognizable
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurIndividualTraits
{
    GENERATED_BODY()

    // Physical variations (0.0 to 1.0 range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 0.5f; // 0.8x to 1.2x base size
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ColorHue = 0.5f; // Color wheel position
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ColorSaturation = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatternIntensity = 0.5f; // Stripes, spots intensity
    
    // Behavioral traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggressiveness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialTendency = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialNature = 0.5f;
    
    // Unique identifier for player recognition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IndividualName;
    
    // Age affects behavior and appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Age = 0.5f; // 0 = juvenile, 1 = elder
};

/**
 * Core dinosaur fragment - defines species and individual traits
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    
    UPROPERTY(EditAnywhere)
    FDinosaurIndividualTraits Traits;
    
    UPROPERTY(EditAnywhere)
    EDinosaurBehaviorState CurrentState = EDinosaurBehaviorState::Idle;
    
    UPROPERTY(EditAnywhere)
    float StateTimer = 0.0f;
    
    UPROPERTY(EditAnywhere)
    float Health = 100.0f;
    
    UPROPERTY(EditAnywhere)
    float Hunger = 0.0f; // 0 = full, 100 = starving
    
    UPROPERTY(EditAnywhere)
    float Thirst = 0.0f;
    
    UPROPERTY(EditAnywhere)
    float Fear = 0.0f; // Current fear level
    
    UPROPERTY(EditAnywhere)
    bool bIsPlayerVisible = false;
    
    UPROPERTY(EditAnywhere)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};

/**
 * Territory fragment for territorial species
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FVector TerritoryCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    float TerritoryRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> IntrudersList;
    
    UPROPERTY(EditAnywhere)
    float LastPatrolTime = 0.0f;
    
    UPROPERTY(EditAnywhere)
    TArray<FVector> PatrolPoints;
    
    UPROPERTY(EditAnywhere)
    int32 CurrentPatrolIndex = 0;
};

/**
 * Pack behavior fragment for social species
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurPackFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FMassEntityHandle PackLeader;
    
    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> PackMembers;
    
    UPROPERTY(EditAnywhere)
    FVector PackCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    FMassEntityHandle CurrentTarget; // Hunt target
    
    UPROPERTY(EditAnywhere)
    float PackCohesion = 1.0f; // How tight the pack stays together
    
    UPROPERTY(EditAnywhere)
    bool bIsHunting = false;
    
    UPROPERTY(EditAnywhere)
    float LastCommunicationTime = 0.0f;
};

/**
 * Migration fragment for species that migrate seasonally
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurMigrationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TArray<FVector> MigrationRoute;
    
    UPROPERTY(EditAnywhere)
    int32 CurrentWaypointIndex = 0;
    
    UPROPERTY(EditAnywhere)
    float MigrationSpeed = 100.0f;
    
    UPROPERTY(EditAnywhere)
    bool bIsMigrating = false;
    
    UPROPERTY(EditAnywhere)
    float SeasonalTimer = 0.0f;
    
    UPROPERTY(EditAnywhere)
    float MigrationTriggerThreshold = 3600.0f; // 1 hour game time
};

/**
 * Sensory fragment for perception and awareness
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSensoryFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float SightRange = 1500.0f;
    
    UPROPERTY(EditAnywhere)
    float SightAngle = 120.0f; // Degrees
    
    UPROPERTY(EditAnywhere)
    float HearingRange = 2000.0f;
    
    UPROPERTY(EditAnywhere)
    float SmellRange = 800.0f;
    
    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> VisibleEntities;
    
    UPROPERTY(EditAnywhere)
    TArray<FMassEntityHandle> HeardEntities;
    
    UPROPERTY(EditAnywhere)
    FVector LastHeardSound = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    float LastSenseUpdate = 0.0f;
};

/**
 * Daily routine fragment for natural behavior cycles
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TArray<EDinosaurBehaviorState> DailySchedule;
    
    UPROPERTY(EditAnywhere)
    TArray<float> ScheduleTimings; // Hours of day for each activity
    
    UPROPERTY(EditAnywhere)
    FVector PreferredRestingSpot = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    FVector PreferredFeedingArea = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    FVector PreferredWaterSource = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere)
    float LastRoutineCheck = 0.0f;
    
    UPROPERTY(EditAnywhere)
    bool bHasEstablishedRoutine = false;
};