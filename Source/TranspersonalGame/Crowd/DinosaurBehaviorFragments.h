#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "DinosaurBehaviorFragments.generated.h"

UENUM(BlueprintType)
enum class ECrowd_DinosaurBehaviorState_3FD : uint8
{
    Idle,
    Grazing,
    Moving,
    Drinking,
    Sleeping,
    Alert,
    Fleeing,
    Hunting,
    Socializing
};

UENUM(BlueprintType)
enum class EDinosaurSpeciesType : uint8
{
    SmallHerbivore,     // Compsognathus, Gallimimus
    MediumHerbivore,    // Parasaurolophus, Triceratops
    LargeHerbivore,     // Brachiosaurus, Diplodocus
    SmallCarnivore,     // Velociraptor, Deinonychus
    MediumCarnivore,    // Allosaurus, Carnotaurus
    LargeCarnivore      // T-Rex, Spinosaurus
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurSpeciesFragment_3FD : public FMassFragment
{
    GENERATED_BODY()

    EDinosaurSpeciesType SpeciesType = EDinosaurSpeciesType::SmallHerbivore;
    FString SpeciesName;
    float BaseSpeed = 300.0f;
    float Size = 1.0f;
    float AggressionLevel = 0.1f;
    bool bIsNocturnal = false;
    bool bCanSwim = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurBehaviorFragment_3FD : public FMassFragment
{
    GENERATED_BODY()

    ECrowd_DinosaurBehaviorState_3FD CurrentState = ECrowd_DinosaurBehaviorState_3FD::Idle;
    ECrowd_DinosaurBehaviorState_3FD PreviousState = ECrowd_DinosaurBehaviorState_3FD::Idle;
    float StateTimer = 0.0f;
    float StateDuration = 5.0f;
    FVector TargetLocation = FVector::ZeroVector;
    bool bHasTarget = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurBehaviorNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    float Hunger = 0.5f;           // 0.0 = starving, 1.0 = full
    float Thirst = 0.5f;           // 0.0 = dehydrated, 1.0 = hydrated
    float Energy = 1.0f;           // 0.0 = exhausted, 1.0 = energized
    float Social = 0.5f;           // 0.0 = lonely, 1.0 = socially satisfied
    float Fear = 0.0f;             // 0.0 = calm, 1.0 = terrified
    
    float HungerDecayRate = 0.1f;
    float ThirstDecayRate = 0.15f;
    float EnergyDecayRate = 0.05f;
    float SocialDecayRate = 0.02f;
    float FearDecayRate = 0.3f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurBehaviorFlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    TArray<FMassEntityHandle> NearbyEntities;
    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    FVector CohesionForce = FVector::ZeroVector;
    
    float SeparationRadius = 200.0f;
    float AlignmentRadius = 500.0f;
    float CohesionRadius = 800.0f;
    
    float SeparationWeight = 2.0f;
    float AlignmentWeight = 1.0f;
    float CohesionWeight = 1.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector TerritoryCenter = FVector::ZeroVector;
    float TerritoryRadius = 1000.0f;
    bool bHasTerritory = false;
    bool bDefendsTerritory = false;
    float TerritorialAggression = 0.1f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurBehaviorMemoryFragment : public FMassFragment
{
    GENERATED_BODY()

    TArray<FVector> KnownWaterSources;
    TArray<FVector> KnownFoodSources;
    TArray<FVector> KnownDangerAreas;
    FVector LastPlayerSighting = FVector::ZeroVector;
    float TimeSincePlayerSeen = 999.0f;
    
    static constexpr int32 MaxMemoryEntries = 10;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_DinosaurVariationFragment : public FMassFragment
{
    GENERATED_BODY()

    float SizeVariation = 1.0f;        // 0.8 - 1.2 multiplier
    FLinearColor ColorTint = FLinearColor::White;
    float SpeedVariation = 1.0f;       // 0.9 - 1.1 multiplier
    float AggressionVariation = 1.0f;  // 0.5 - 1.5 multiplier
    int32 UniqueID = 0;                // For player recognition
    
    // Physical variations for unique appearance
    float HornSize = 1.0f;
    float TailLength = 1.0f;
    float NeckLength = 1.0f;
    bool bHasScar = false;
    bool bHasLimp = false;
};