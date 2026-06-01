#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Crowd_SimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorType : uint8
{
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"), 
    Crafting        UMETA(DisplayName = "Crafting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    ECrowd_BehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
    bool bIsActive;

    FCrowd_NPCData()
    {
        NPCName = TEXT("");
        BehaviorType = ECrowd_BehaviorType::Gathering;
        LODLevel = ECrowd_LODLevel::Medium;
        GroupID = 0;
        HomeLocation = FVector::ZeroVector;
        UpdateFrequency = 30.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    int32 MaxNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    int32 CurrentNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    TArray<ECrowd_BehaviorType> AllowedBehaviors;

    FCrowd_BiomeData()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 15000.0f;
        MaxNPCs = 30;
        CurrentNPCs = 0;
        AllowedBehaviors.Add(ECrowd_BehaviorType::Gathering);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_SimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_SimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Crowd simulation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxTotalNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SimulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableCrowdSimulation;

    // Biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FCrowd_BiomeData> BiomeDataArray;

    // NPC management
    UPROPERTY(BlueprintReadOnly, Category = "NPCs")
    TArray<FCrowd_NPCData> ActiveNPCs;

    UPROPERTY(BlueprintReadOnly, Category = "NPCs")
    TArray<APawn*> SpawnedNPCs;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveNPCCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 HighLODNPCs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MediumLODNPCs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LowLODNPCs;

public:
    // Core simulation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownCrowdSimulation();

    // NPC management functions
    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    APawn* SpawnNPCInBiome(const FString& BiomeName, ECrowd_BehaviorType BehaviorType);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void RemoveNPCFromSimulation(APawn* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void UpdateNPCBehavior(APawn* NPC, ECrowd_BehaviorType NewBehavior);

    // LOD management functions
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    ECrowd_LODLevel CalculateLODLevel(const FVector& NPCLocation, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetNPCLODLevel(APawn* NPC, ECrowd_LODLevel LODLevel);

    // Biome management functions
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FString GetClosestBiome(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    bool CanSpawnInBiome(const FString& BiomeName);

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantNPCs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSimulationPerformanceMetric();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<APawn*> GetNPCsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetNPCCountInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void DistributeNPCsAcrossBiomes();

private:
    // Internal tracking
    float LODUpdateTimer;
    FVector LastPlayerLocation;
    bool bSimulationInitialized;

    // Helper functions
    void UpdatePerformanceMetrics();
    void ValidateNPCLimits();
    APawn* GetPlayerPawn();
    void LogCrowdSimulationStats();
};

#include "Crowd_SimulationManager.generated.h"