#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "Engine/DataTable.h"
#include "MassDinosaurFragments_CrowdSimulation.generated.h"

/**
 * Fragment definitions for dinosaur crowd simulation
 * Each fragment represents a specific aspect of dinosaur behavior
 */

UENUM(BlueprintType)
enum class ECrowdSim_DinosaurSpecies_802 : uint8
{
    Triceratops,
    Brachiosaurus,
    Parasaurolophus,
    Stegosaurus,
    TRex,
    Velociraptor,
    Allosaurus,
    Compsognathus,
    Pteranodon,
    Dimorphodon
};

UENUM(BlueprintType)
enum class ECrowdSim_DinosaurBehaviorState_802 : uint8
{
    Grazing,
    Moving,
    Resting,
    Drinking,
    Hunting,
    Fleeing,
    Socializing,
    Nesting,
    Patrolling,
    Sleeping
};

UENUM(BlueprintType)
enum class EDinosaurRole : uint8
{
    HerdLeader,
    HerdMember,
    PackAlpha,
    PackMember,
    Solitary,
    Juvenile,
    Elder,
    Sentinel
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSim_MassDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdSim_DinosaurSpecies_802 Species = ECrowdSim_DinosaurSpecies_802::Triceratops;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f; // Scale multiplier for procedural variation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 500.0f; // cm/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 2000.0f; // cm
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFlying = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UniqueID = 0; // For individual recognition
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdSim_DinosaurBehaviorState_802 CurrentState = ECrowdSim_DinosaurBehaviorState_802::Grazing;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurRole Role = EDinosaurRole::HerdMember;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateDuration = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Alertness = 0.0f; // 0-1, affects reaction time
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger = 0.5f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thirst = 0.5f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Energy = 1.0f; // 0-1
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSim_MassDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle HerdLeader;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdSize = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HerdCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MigrationTarget = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMigrating = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassDinosaurSensesFragment : public FMassFragment
{
    GENERATED_BODY()

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> VisibleEntities;
    
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyPredators;
    
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyPrey;
    
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle PlayerEntity; // If player is detected
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerDetectionTime = -1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0-1
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSim_MassDinosaurVariationFragment : public FMassFragment
{
    GENERATED_BODY()

    // Procedural visual variations for individual recognition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HornSize = 1.0f; // Scale multiplier
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TailLength = 1.0f; // Scale multiplier
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NeckLength = 1.0f; // Scale multiplier
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ScarLocations; // Battle scars for recognition
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Limp = 0.0f; // 0-1, affects movement animation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Age = 5; // Years, affects size and behavior
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassDinosaurMemoryFragment : public FMassFragment
{
    GENERATED_BODY()

    // Memory system for individual dinosaurs
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownWaterSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownFoodSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerZones;
    
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> KnownIndividuals; // Other dinosaurs they recognize
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HomeTerritory = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 3000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerEncounterTime = -1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerIsKnownThreat = false;
};