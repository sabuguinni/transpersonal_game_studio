#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassAvoidanceFragments.h"
#include "DinosaurHerdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Fragment that defines a dinosaur's herd behavior
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // Herd identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1;

    // Herd role
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bIsHerdLeader : 1 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bIsScout : 1 = false;

    // Herd behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 800.0f;

    // Panic behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentPanicTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bInPanic : 1 = false;
};

/**
 * Fragment that defines dinosaur species characteristics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpeciesName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bIsHerbivore : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bIsCarnivore : 1 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bCanBeHunted : 1 = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bCanHunt : 1 = false;

    // Grazing behavior for herbivores
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrazingRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrazingDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeBetweenGrazing = 120.0f;
};

/**
 * Fragment for dinosaur daily routines
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastGrazingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDrinkingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastRestTime = 0.0f;

    // Current activity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentActivity = 0; // 0=Wandering, 1=Grazing, 2=Drinking, 3=Resting, 4=Fleeing

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityStartTime = 0.0f;
};

/**
 * Processor for dinosaur herd behavior using Mass AI
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HerdQuery;

    // Herd behavior calculations
    FVector CalculateCohesion(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData);
    FVector CalculateSeparation(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData);
    FVector CalculateAlignment(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData);
    
    // Panic behavior
    void ProcessPanicBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FMassVelocityFragment& Velocity);
    
    // Environmental awareness
    bool DetectPredatorNearby(const FMassExecutionContext& Context, const FTransformFragment& Transform, float DetectionRadius);
};

/**
 * Processor for dinosaur daily routines
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurRoutineProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurRoutineProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery RoutineQuery;

    // Activity management
    void UpdateCurrentActivity(FDinosaurRoutineFragment& Routine, const FDinosaurSpeciesFragment& Species, float CurrentTime);
    FVector FindGrazingLocation(const FTransformFragment& Transform, const FDinosaurSpeciesFragment& Species);
    FVector FindWaterSource(const FTransformFragment& Transform);
    FVector FindRestingSpot(const FTransformFragment& Transform);
};

/**
 * Subsystem for managing dinosaur herds and populations
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Herd management
    UFUNCTION(BlueprintCallable)
    int32 CreateHerd(FName SpeciesName, const FVector& Location, int32 HerdSize);

    UFUNCTION(BlueprintCallable)
    void SpawnHerdAtLocation(int32 HerdID, const FVector& Location);

    UFUNCTION(BlueprintCallable)
    void TriggerHerdPanic(int32 HerdID, const FVector& ThreatLocation);

    // Population management
    UFUNCTION(BlueprintCallable)
    void SetSpeciesPopulationTarget(FName SpeciesName, int32 TargetPopulation);

    UFUNCTION(BlueprintCallable)
    int32 GetCurrentSpeciesPopulation(FName SpeciesName) const;

protected:
    UPROPERTY()
    TMap<int32, TArray<FMassEntityHandle>> HerdEntities;

    UPROPERTY()
    TMap<FName, int32> SpeciesPopulationTargets;

    UPROPERTY()
    TMap<FName, int32> CurrentSpeciesPopulations;

private:
    int32 NextHerdID = 1;
    
    void SpawnDinosaurEntity(FName SpeciesName, const FVector& Location, int32 HerdID, bool bIsLeader = false);
    void UpdatePopulationCounts();
};