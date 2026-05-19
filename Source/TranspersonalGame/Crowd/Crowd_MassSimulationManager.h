#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing         UMETA(DisplayName = "Grazing"),
    Migration       UMETA(DisplayName = "Migration"), 
    Flocking        UMETA(DisplayName = "Flocking"),
    DefensiveCircle UMETA(DisplayName = "Defensive Circle"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Hunting         UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ECrowd_DinosaurSpecies : uint8
{
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalo")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    ECrowd_DinosaurSpecies Species = ECrowd_DinosaurSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    ECrowd_HerdBehavior Behavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    int32 HerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    float FormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Config")
    bool bHasSocialHierarchy = true;

    FCrowd_HerdConfiguration()
    {
        Species = ECrowd_DinosaurSpecies::Triceratops;
        Behavior = ECrowd_HerdBehavior::Grazing;
        HerdSize = 20;
        FormationRadius = 500.0f;
        SpawnCenter = FVector::ZeroVector;
        MovementSpeed = 100.0f;
        bHasSocialHierarchy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomePopulation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FCrowd_HerdConfiguration> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TotalPopulation = 0;

    FCrowd_BiomePopulation()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        TotalPopulation = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Mass Simulation Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    TArray<FCrowd_BiomePopulation> BiomePopulations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float SimulationRadius = 100000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float LODDistance2 = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float LODDistance3 = 50000.0f;

    // Crowd Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeBiomePopulations();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerdInBiome(const FCrowd_HerdConfiguration& HerdConfig, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CreateMigrationPattern(ECrowd_DinosaurSpecies Species, const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerDefensiveBehavior(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateFlockingBehavior(const TArray<AActor*>& FlockMembers, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetHerdBehavior(const FString& HerdID, ECrowd_HerdBehavior NewBehavior);

    // LOD and Performance Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSimulationPerformance();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterBiome(const FString& BiomeName, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FCrowd_BiomePopulation* GetBiomePopulation(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void PopulateBiomeWithSpecies(const FString& BiomeName, ECrowd_DinosaurSpecies Species, int32 Count);

private:
    // Internal tracking
    UPROPERTY()
    TMap<FString, int32> ActiveHerds;

    UPROPERTY()
    TArray<AActor*> ManagedActors;

    UPROPERTY()
    float LastLODUpdateTime;

    UPROPERTY()
    FVector LastPlayerLocation;

    // Helper functions
    FString GetSpeciesMeshPath(ECrowd_DinosaurSpecies Species) const;
    FVector CalculateFlockingForce(AActor* Agent, const TArray<AActor*>& Neighbors) const;
    void ApplyHierarchyScaling(AActor* Actor, int32 HierarchyLevel) const;
    bool IsWithinBiome(const FVector& Location, const FCrowd_BiomePopulation& Biome) const;
};