#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureCore.generated.h"

/**
 * Core Engine Architecture Manager
 * Responsible for overall engine architecture coordination and system integration
 * Ensures all game systems follow architectural standards and communicate properly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy() const { return bSystemHealthy; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameTime() const { return CurrentFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetActiveActorCount() const { return ActiveActorCount; }

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetMemoryUsageMB() const { return MemoryUsageMB; }

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetRegisteredSystem(const FString& SystemName) const;

protected:
    // Core system health
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bSystemHealthy;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float MemoryUsageMB;

    // System registry
    UPROPERTY()
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    // Architecture validation
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> SystemWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> SystemErrors;

private:
    // Internal validation methods
    void ValidateWorldPartition();
    void ValidateActorSystems();
    void ValidateMemoryConstraints();
    void ValidatePerformanceTargets();

    // Performance tracking
    FDateTime LastPerformanceUpdate;
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameTimeHistory = 60;

    // System health tracking
    bool bWorldPartitionHealthy;
    bool bActorSystemsHealthy;
    bool bMemoryConstraintsHealthy;
    bool bPerformanceTargetsHealthy;
};