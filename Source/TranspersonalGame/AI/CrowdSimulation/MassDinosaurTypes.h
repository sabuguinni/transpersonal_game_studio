#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/DataTable.h"
#include "MassDinosaurTypes.generated.h"

// Forward Declarations
class UMassEntitySubsystem;

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    None = 0,
    SmallHerbivore,     // Compsognathus, Dryosaurus (5-15 indivíduos)
    MediumHerbivore,    // Parasaurolophus, Triceratops (10-30 indivíduos)
    LargeHerbivore,     // Brontosaurus, Diplodocus (5-12 indivíduos)
    PackPredator,       // Velociraptor, Deinonychus (3-8 indivíduos)
    SolitaryPredator,   // T-Rex, Allosaurus (1-2 indivíduos)
    FlyingFlock,        // Pteranodon, Quetzalcoatlus (8-25 indivíduos)
    AquaticGroup        // Mosasaurus, Plesiosaurs (2-6 indivíduos)
};

UENUM(BlueprintType)
enum class EAI_DinosaurBehaviorState : uint8
{
    Grazing = 0,        // Herbívoros alimentando-se
    Hunting,            // Predadores caçando
    Migrating,          // Movimento de longa distância
    Resting,            // Descanso/sono
    Drinking,           // Bebendo água
    Socializing,        // Interações sociais
    Fleeing,            // Fugindo de ameaça
    Territorial,        // Comportamento territorial
    Nesting,            // Comportamento reprodutivo
    Investigating       // Investigando distúrbio
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassDinosaurFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType = EDinosaurHerdType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_DinosaurBehaviorState BehaviorState = EAI_DinosaurBehaviorState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastStateChange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredGrazingArea = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassHerdLeaderFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMigrating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecisionCooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LeadershipStrength = 1.0f; // Quão bem outros seguem este líder
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassFlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LeaderFollowWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FlockCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AverageVelocity = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialAggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDefendingTerritory = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_DinosaurSpeciesData_B43 : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType DefaultHerdType = EDinosaurHerdType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSwim = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerIncreaseRate = 0.05f;
};

// Tags para identificação rápida
USTRUCT()
struct TRANSPERSONALGAME_API FMassDinosaurTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassHerbivorTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassCarnivoreTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassHerdLeaderTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassFlockMemberTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassMigratingTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FMassFleeingTag : public FMassTag
{
    GENERATED_BODY()
};