#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "DinosaurCrowdFragments.generated.h"

UENUM(BlueprintType)
enum class EAI_DinosaurBehaviorState_7CD : uint8
{
    Grazing,
    Moving,
    Resting,
    Fleeing,
    Hunting,
    Drinking,
    Socializing,
    Migrating,
    Alert
};

UENUM(BlueprintType)
enum class EAI_DinosaurType_7CD : uint8
{
    SmallHerbivore,     // Compsognathus, small plant-eaters
    LargeHerbivore,     // Triceratops, Brontosaurus
    SmallCarnivore,     // Velociraptor, Dilophosaurus  
    LargeCarnivore,     // T-Rex, Allosaurus
    Flying,             // Pteranodon
    Aquatic,            // Plesiosaur
    Scavenger           // Specialized behavior
};

USTRUCT()
struct TRANSPERSONALGAME_API FAI_DinosaurTypeFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    EAI_DinosaurType_7CD DinosaurType = EAI_DinosaurType_7CD::SmallHerbivore;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    float Size = 1.0f; // Relative size multiplier

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    float AggressionLevel = 0.1f; // 0.0 = Peaceful, 1.0 = Highly Aggressive

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    float FearThreshold = 0.5f; // How easily spooked this dinosaur is
};

USTRUCT()
struct TRANSPERSONALGAME_API FAI_DinosaurBehaviorFragment_7CD : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAI_DinosaurBehaviorState_7CD CurrentState = EAI_DinosaurBehaviorState_7CD::Grazing;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAI_DinosaurBehaviorState_7CD PreviousState = EAI_DinosaurBehaviorState_7CD::Grazing;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateDuration = 10.0f; // How long to stay in current state

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertLevel = 0.0f; // 0.0 = Calm, 1.0 = Maximum Alert

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector ThreatLocation = FVector::ZeroVector;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ThreatDistance = 0.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FHerdMemberFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3] // [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FMassEntityHandle HerdLeader;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 HerdID = -1;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float CohesionWeight = 1.0f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float SeparationWeight = 1.5f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float AlignmentWeight = 1.0f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float PreferredDistance = 500.0f; // Preferred distance from other herd members

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsHerdLeader = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FPackMemberFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3] // [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FMassEntityHandle PackLeader;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackID = -1;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackRank = 0; // 0 = Alpha, higher numbers = lower rank

// [UHT-FIX3] // [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FMassEntityHandle HuntTarget;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector HuntPosition = FVector::ZeroVector;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float CoordinationRadius = 1500.0f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FAI_DinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hunger = 0.3f; // 0.0 = Full, 1.0 = Starving

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thirst = 0.2f; // 0.0 = Hydrated, 1.0 = Dehydrated

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Energy = 0.8f; // 0.0 = Exhausted, 1.0 = Full Energy

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Social = 0.5f; // 0.0 = Isolated, 1.0 = Socially Satisfied

    // Need change rates per second
// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need Rates")
    float HungerRate = 0.01f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need Rates")
    float ThirstRate = 0.015f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need Rates")
    float EnergyDecayRate = 0.005f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need Rates")
    float SocialDecayRate = 0.002f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurMemoryFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownFoodSources;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownWaterSources;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownDangerZones;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeRestingSpots;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerSighting = FVector::ZeroVector;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightingTime = 0.0f;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime = 300.0f; // 5 minutes in seconds
};

USTRUCT()
struct TRANSPERSONALGAME_API FAI_DinosaurVariationFragment_7CD : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float SizeVariation = 1.0f; // 0.8 to 1.2 typical range

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    FLinearColor PrimaryColor = FLinearColor::White;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    FLinearColor SecondaryColor = FLinearColor::Gray;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float SpeedVariation = 1.0f; // Movement speed multiplier

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    int32 UniqueID = 0; // For player recognition and tracking

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    FString VariationSeed; // Seed for consistent appearance generation
};

USTRUCT()
struct TRANSPERSONALGAME_API FMigrationFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsMigrating = false;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector MigrationDestination = FVector::ZeroVector;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationProgress = 0.0f; // 0.0 to 1.0

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 CurrentWaypointIndex = 0;

// [UHT-FIX3]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalTrigger = 0.0f; // Seasonal migration trigger value
};