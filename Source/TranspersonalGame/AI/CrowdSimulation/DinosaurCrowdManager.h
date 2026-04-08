#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassAgentComponent.h"
#include "DinosaurCrowdManager.generated.h"

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    Herbivore_Small     UMETA(DisplayName = "Small Herbivores (Compsognathus, Dryosaurus)"),
    Herbivore_Medium    UMETA(DisplayName = "Medium Herbivores (Parasaurolophus, Iguanodon)"),
    Herbivore_Large     UMETA(DisplayName = "Large Herbivores (Triceratops, Brachiosaurus)"),
    Carnivore_Pack      UMETA(DisplayName = "Pack Hunters (Velociraptor, Deinonychus)"),
    Carnivore_Solo      UMETA(DisplayName = "Solitary Predators (T-Rex, Allosaurus)"),
    Scavenger_Flock     UMETA(DisplayName = "Scavenger Flocks (Pteranodon, Compsognathus)")
};

USTRUCT(BlueprintType)
struct FDinosaurHerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType = EDinosaurHerdType::Herbivore_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "200"))
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "200"))
    int32 MaxHerdSize = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float CohesionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float SeparationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "200.0", ClampMax = "15000.0"))
    float AlertRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MovementSpeed = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float FleeSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMigratesSeasonally = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "24.0"))
    float ActiveHoursStart = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "24.0"))
    float ActiveHoursEnd = 18.0f;
};

USTRUCT(BlueprintType)
struct FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WayPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTrigger = 0.25f; // 0.0 = Winter, 0.25 = Spring, 0.5 = Summer, 0.75 = Autumn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "50000.0"))
    float RouteRadius = 5000.0f;
};

/**
 * Manages large-scale dinosaur crowd simulation using Mass Entity framework
 * Handles herd behaviors, migrations, predator-prey dynamics, and seasonal patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === HERD CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<FDinosaurHerdConfig> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxActiveHerds = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration", meta = (ClampMin = "1000.0", ClampMax = "100000.0"))
    float SpawnRadius = 25000.0f;

    // === MIGRATION SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SeasonalCycleSpeed = 1.0f;

    // === PREDATOR-PREY DYNAMICS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator-Prey")
    float PredatorDetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator-Prey")
    float FleeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator-Prey")
    float HuntingSuccessRate = 0.15f;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000", ClampMax = "50000"))
    int32 MaxSimulatedEntities = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "20000.0"))
    float LODDistance1 = 2000.0f; // Full simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float LODDistance2 = 8000.0f; // Reduced simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "2000.0", ClampMax = "100000.0"))
    float LODDistance3 = 25000.0f; // Minimal simulation

    // === ENVIRONMENTAL FACTORS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FVector> WaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FVector> FeedingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FVector> DangerZones; // Areas to avoid (volcanic activity, etc.)

public:
    // === PUBLIC INTERFACE ===
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(EDinosaurHerdType HerdType, FVector Location, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMigration(int32 RouteIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSeasonalCycle(float SeasonProgress); // 0.0 to 1.0

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void AddDangerZone(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveDangerZone(FVector Location);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    TArray<FVector> GetNearbyHerdLocations(FVector PlayerLocation, float SearchRadius) const;

private:
    // === INTERNAL SYSTEMS ===
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Current seasonal progress (0.0 to 1.0)
    float CurrentSeasonalProgress;

    // Active herd tracking
    TArray<FMassEntityHandle> ActiveHerds;

    // Migration state tracking
    TMap<int32, bool> ActiveMigrations;

    // Performance monitoring
    float LastPerformanceCheck;
    int32 CurrentEntityCount;

    // === INTERNAL METHODS ===
    void InitializeMassEntity();
    void UpdateSeasonalBehaviors(float DeltaTime);
    void UpdateMigrations(float DeltaTime);
    void UpdatePredatorPreyDynamics(float DeltaTime);
    void ManagePerformanceLOD();
    void ProcessHerdBehaviors(float DeltaTime);
    
    FVector GetOptimalSpawnLocation(EDinosaurHerdType HerdType) const;
    bool IsLocationSafe(FVector Location, EDinosaurHerdType HerdType) const;
    float CalculateSeasonalInfluence() const;
};