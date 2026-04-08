#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Dinosaur Species Types for crowd simulation
 */
UENUM(BlueprintType)
enum class EDinosaurSpeciesType : uint8
{
    // Large Herbivores - Herd animals
    Brachiosaurus,
    Triceratops,
    Parasaurolophus,
    
    // Medium Herbivores - Pack animals
    Iguanodon,
    Stegosaurus,
    
    // Small Herbivores - Flock animals
    Gallimimus,
    Compsognathus,
    
    // Large Predators - Solo/Small groups
    TyrannosaurusRex,
    Allosaurus,
    
    // Medium Predators - Pack hunters
    Velociraptor,
    Deinonychus,
    
    // Flying - Aerial flocks
    Pteranodon,
    Quetzalcoatlus,
    
    // Aquatic - School behavior
    Plesiosaur,
    Mosasaurus
};

/**
 * Crowd Behavior Types
 */
UENUM(BlueprintType)
enum class ECrowdBehaviorType : uint8
{
    Grazing,        // Peaceful feeding behavior
    Migrating,      // Long-distance movement
    Fleeing,        // Escape from predators
    Hunting,        // Predator coordination
    Territorial,    // Defending area
    Mating,         // Breeding behavior
    Resting,        // Low activity state
    Investigating   // Curious behavior
};

/**
 * Mass Fragment for Dinosaur Species Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpeciesType SpeciesType = EDinosaurSpeciesType::Compsognathus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeSpeed = 600.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PersonalSpace = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredatorDetectionRange = 800.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdAnimal = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredator = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredHerdSize = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f;
};

/**
 * Mass Fragment for Crowd Behavior State
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCrowdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdBehaviorType CurrentBehavior = ECrowdBehaviorType::Grazing;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdBehaviorType PreviousBehavior = ECrowdBehaviorType::Grazing;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorDuration = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle HerdLeader;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyHerdMembers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle ThreatTarget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdLeader = false;
};

/**
 * Mass Fragment for Migration Patterns
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurMigrationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> MigrationRoute;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWaypoint = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaypointReachDistance = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMigrating = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationTriggerTime = 1800.0f; // 30 minutes
};

/**
 * Main Dinosaur Crowd System Actor
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCrowdSystem : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCrowdSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Crowd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxDinosaursPerSpecies = 200;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float DespawnDistance = 8000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency = 0.1f;
    
    // Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    TMap<EDinosaurSpeciesType, int32> SpeciesPopulationLimits;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    TMap<EDinosaurSpeciesType, float> SpeciesSpawnWeights;
    
    // Environmental Factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeOfDay = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherIntensity = 0.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsStormActive = false;

public:
    // Crowd Management Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnDinosaurHerd(EDinosaurSpeciesType SpeciesType, const FVector& Location, int32 HerdSize);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerMigration(EDinosaurSpeciesType SpeciesType, const TArray<FVector>& Route);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerPanicResponse(const FVector& ThreatLocation, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetEnvironmentalStress(float StressLevel);
    
    // Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Queries")
    int32 GetDinosaurCount(EDinosaurSpeciesType SpeciesType) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Queries")
    TArray<FVector> GetNearbyDinosaurLocations(const FVector& Center, float Radius) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Queries")
    bool IsAreaSafe(const FVector& Location, float Radius) const;

private:
    // Internal Systems
    UMassEntitySubsystem* MassEntitySubsystem;
    
    // Entity tracking
    TMap<EDinosaurSpeciesType, TArray<FMassEntityHandle>> SpeciesEntities;
    
    // Timers
    float LastUpdateTime = 0.0f;
    float LastSpawnCheck = 0.0f;
    
    // Internal Functions
    void UpdateCrowdBehaviors(float DeltaTime);
    void ProcessMigrations(float DeltaTime);
    void HandlePredatorPreyInteractions(float DeltaTime);
    void ManagePopulationLevels(float DeltaTime);
    void UpdateEnvironmentalEffects(float DeltaTime);
    
    FMassEntityHandle CreateDinosaurEntity(EDinosaurSpeciesType SpeciesType, const FVector& Location);
    void InitializeSpeciesData();
    void CleanupDistantEntities();
};