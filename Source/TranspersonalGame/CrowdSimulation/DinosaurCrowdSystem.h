#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "Engine/World.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward Declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

// Dinosaur Species Types for Mass Simulation
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbivores - Herd Behavior
    Triceratops,
    Brachiosaurus,
    Stegosaurus,
    Parasaurolophus,
    
    // Carnivores - Pack Behavior
    Velociraptor,
    Dilophosaurus,
    Carnotaurus,
    
    // Apex Predators - Solitary
    TyrannosaurusRex,
    Spinosaurus,
    Giganotosaurus,
    
    // Flying - Flock Behavior
    Pteranodon,
    Quetzalcoatlus,
    
    // Aquatic - School Behavior
    Mosasaurus,
    Plesiosaur
};

// Behavior States for Dinosaur AI
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Grazing,        // Herbivores feeding
    Hunting,        // Carnivores seeking prey
    Patrolling,     // General movement
    Resting,        // Idle/sleeping
    Fleeing,        // Escaping from threat
    Aggressive,     // Combat/territorial
    Migrating,      // Long-distance movement
    Socializing,    // Intra-species interaction
    Drinking,       // At water sources
    Nesting         // Breeding areas
};

// Mass Fragment for Dinosaur Identity
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurIdentityFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UniqueID = 0;
    
    // Physical Variations (0.0-1.0 range for procedural generation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ColorVariation = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FeatureVariation = 0.5f; // Horn size, crest shape, etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialTendency = 0.5f; // How much they seek group behavior
};

// Mass Fragment for Dinosaur Behavior
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState CurrentState = EDinosaurBehaviorState::Grazing;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState PreviousState = EDinosaurBehaviorState::Grazing;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateDuration = 30.0f; // How long to stay in current state
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 1000.0f; // How far they can sense threats/food
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeRadius = 500.0f; // Distance at which they start fleeing
};

// Mass Fragment for Herd/Pack Behavior
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurGroupFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = -1; // -1 means no group
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGroupLeader = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector GroupCenterLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCohesionRadius = 2000.0f; // How close to stay to group
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 300.0f; // Minimum distance from other group members
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupSize = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupMovementWeight = 0.7f; // How much group movement influences individual
};

// Mass Fragment for Player Awareness
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPlayerAwarenessFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerDetected = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerDetectionTime = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerMemoryDuration = 10.0f; // How long they remember player location
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // How threatening they consider the player
};

/**
 * Main Dinosaur Crowd System
 * Manages large-scale dinosaur populations using Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurCrowdSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDinosaurCrowdSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnDinosaurHerd(EDinosaurSpecies Species, FVector Location, int32 HerdSize = 10, float SpawnRadius = 1000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnDinosaurPack(EDinosaurSpecies Species, FVector Location, int32 PackSize = 5, float SpawnRadius = 500.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnSolitaryDinosaur(EDinosaurSpecies Species, FVector Location);

    // Population Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetMaxDinosaurPopulation(int32 MaxPopulation) { MaxDinosaurCount = MaxPopulation; }
    
    UFUNCTION(BlueprintPure, Category = "Dinosaur Crowd")
    int32 GetCurrentDinosaurCount() const { return CurrentDinosaurCount; }
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void ClearAllDinosaurs();

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void TriggerMigrationEvent(FVector StartArea, FVector TargetArea, float AreaRadius = 5000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetGlobalThreatLevel(float ThreatLevel); // 0.0 = calm, 1.0 = high alert

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void NotifyPlayerPresence(FVector PlayerLocation, float NoiseLevel = 0.5f);

protected:
    // Mass Entity System
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Population Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 MaxDinosaurCount = 50000;
    
    UPROPERTY(BlueprintReadOnly, Category = "Population")
    int32 CurrentDinosaurCount = 0;

    // Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    TMap<EDinosaurSpecies, float> SpeciesSpawnWeights;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    TMap<EDinosaurSpecies, int32> SpeciesMaxPopulation;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GlobalThreatLevel = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DayNightCycleInfluence = 1.0f; // How much day/night affects behavior
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WeatherInfluence = 1.0f; // How much weather affects behavior

    // Group Management
    UPROPERTY()
    TMap<int32, TArray<FMassEntityHandle>> DinosaurGroups;
    
    UPROPERTY()
    int32 NextGroupID = 1;

    // Helper Functions
    int32 CreateNewGroup();
    void AssignToGroup(FMassEntityHandle Entity, int32 GroupID);
    void UpdateGroupCenters();
    FVector CalculateGroupCenter(int32 GroupID);
    
    // Species-specific behavior
    EDinosaurBehaviorState GetDefaultBehaviorForSpecies(EDinosaurSpecies Species);
    float GetDefaultSpeedForSpecies(EDinosaurSpecies Species);
    bool IsHerdSpecies(EDinosaurSpecies Species);
    bool IsPackSpecies(EDinosaurSpecies Species);
    bool IsSolitarySpecies(EDinosaurSpecies Species);
};

/**
 * Mass Processor for Dinosaur Behavior Updates
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Behavior update functions
    void UpdateGrazingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior, 
                              const FTransformFragment& Transform, const FDinosaurIdentityFragment& Identity);
    
    void UpdateHuntingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior, 
                              const FTransformFragment& Transform, const FDinosaurIdentityFragment& Identity);
    
    void UpdateFleeingBehavior(FMassExecutionContext& Context, FDinosaurBehaviorFragment& Behavior, 
                              const FTransformFragment& Transform, const FDinosaurPlayerAwarenessFragment& PlayerAwareness);
};

/**
 * Mass Processor for Group/Herd Behavior
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurGroupProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurGroupProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Group behavior functions
    FVector CalculateCohesionForce(const FTransformFragment& Transform, const FDinosaurGroupFragment& Group);
    FVector CalculateSeparationForce(const FTransformFragment& Transform, const FDinosaurGroupFragment& Group, 
                                   FMassExecutionContext& Context);
    FVector CalculateAlignmentForce(const FDinosaurGroupFragment& Group, FMassExecutionContext& Context);
};

/**
 * Mass Processor for Player Detection and Response
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurPlayerDetectionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurPlayerDetectionProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Player detection functions
    bool CanDetectPlayer(const FTransformFragment& Transform, const FDinosaurBehaviorFragment& Behavior, 
                        FVector PlayerLocation);
    float CalculateThreatLevel(EDinosaurSpecies Species, FVector PlayerLocation, FVector DinosaurLocation);
};