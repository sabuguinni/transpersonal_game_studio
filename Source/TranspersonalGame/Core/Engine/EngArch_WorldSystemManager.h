#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "EngArch_WorldSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngArch_WorldSystemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World System")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World System")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World System")
    int32 MaxSimultaneousActors = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World System")
    bool bEnableWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World System")
    float WorldPartitionCellSize = 25600.0f;

    FEngArch_WorldSystemConfig()
    {
        bEnablePhysicsOptimization = true;
        PhysicsTickRate = 60.0f;
        MaxSimultaneousActors = 50000;
        bEnableWorldPartition = true;
        WorldPartitionCellSize = 25600.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_WorldSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_WorldSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World System Management
    UFUNCTION(BlueprintCallable, Category = "World System")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World System")
    void ShutdownWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World System")
    bool ValidateWorldConfiguration();

    UFUNCTION(BlueprintCallable, Category = "World System")
    void OptimizeWorldPerformance();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "World System")
    void SetWorldSystemConfig(const FEngArch_WorldSystemConfig& NewConfig);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World System")
    FEngArch_WorldSystemConfig GetWorldSystemConfig() const { return WorldSystemConfig; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World System")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World System")
    float GetCurrentPhysicsTickRate() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World System")
    bool IsWorldPartitionEnabled() const;

    // System Status
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World System")
    bool IsWorldSystemInitialized() const { return bIsInitialized; }

    UFUNCTION(BlueprintCallable, Category = "World System")
    void ForceGarbageCollection();

private:
    UPROPERTY()
    FEngArch_WorldSystemConfig WorldSystemConfig;

    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    float LastOptimizationTime = 0.0f;

    // Internal methods
    void ConfigurePhysicsSettings();
    void ConfigureWorldPartition();
    void SetupPerformanceMonitoring();
    void CleanupWorldSystems();
};

#include "EngArch_WorldSystemManager.generated.h"