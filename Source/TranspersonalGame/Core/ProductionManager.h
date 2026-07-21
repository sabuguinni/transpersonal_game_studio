#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "ProductionManager.generated.h"

/**
 * Studio Director Production Manager
 * Coordinates agent outputs and manages production pipeline
 * Ensures compilation readiness and tracks deliverable progress
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Agent Task Distribution System
     */
    UFUNCTION(BlueprintCallable, Category = "Production")
    void DistributeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateCompilationReadiness();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMinPlayableMapReady() const;

    /**
     * Critical Production Metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetPhantomHeaderCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetDuplicateActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool HasPlayableCharacter() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool HasFunctionalTerrain() const;

    /**
     * Agent Coordination
     */
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentPriority(const FString& AgentName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPath() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportAgentProgress(const FString& AgentName, const FString& Deliverable);

protected:
    /**
     * Production State Tracking
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bCompilationReady;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bMinPlayableMapValid;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 PhantomHeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 DuplicateActorCount;

    /**
     * Agent Priority System
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> AgentPriorities;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalPathAgents;

    /**
     * Biome Coordination Data
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, FVector> BiomeCenters;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> BiomeActorCounts;

private:
    void InitializeBiomeData();
    void ScanForPhantomHeaders();
    void CountDuplicateActors();
    void ValidateAgentOutputs();
};