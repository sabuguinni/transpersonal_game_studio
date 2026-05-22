#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "SharedTypes.h"
#include "EngineArchitectCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitect, Log, All);

/**
 * Core Engine Architecture System
 * Defines the fundamental technical rules and constraints for all game systems
 * Enforces performance budgets, memory limits, and system integration patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemPerformance(const FString& SystemName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CanSpawnActor(UClass* ActorClass, const FVector& Location);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetMemoryUsagePercent() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetActiveActorCount() const;

    // World Partition Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsWorldPartitionRequired() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetWorldBounds(const FBox& NewBounds);

    // Performance Budgets
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceBudget(const FString& SystemName, float MaxFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsWithinPerformanceBudget(const FString& SystemName, float CurrentTime);

protected:
    // Core Architecture Properties
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, int32> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, float> PerformanceBudgets;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    FBox WorldBounds;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bWorldPartitionEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorsPerBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float MemoryBudgetMB;

private:
    // Internal tracking
    TMap<FString, float> SystemFrameTimes;
    float LastMemoryCheck;
    int32 LastActorCount;

    // Architecture validation
    bool ValidateModuleDependencies();
    void InitializePerformanceBudgets();
    void CheckMemoryThresholds();
};

/**
 * Engine Architecture Rules Enforcer
 * Static class that enforces architectural constraints across all systems
 */
UCLASS(BlueprintType, Abstract)
class TRANSPERSONALGAME_API UEngineRulesEnforcer : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // World Partition Rules
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool IsWorldSizeValid(float WorldSizeKm);

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool RequiresWorldPartition(float WorldSizeKm);

    // Actor Spawning Rules
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool CanSpawnActorAtLocation(UClass* ActorClass, const FVector& Location, UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static int32 GetMaxActorsForBiome(EEng_BiomeType BiomeType);

    // Performance Rules
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool IsFrameTimeAcceptable(float FrameTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static float GetTargetFrameTime();

    // Memory Rules
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool IsMemoryUsageAcceptable(float MemoryUsageMB);

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static float GetMaxMemoryBudget();

    // Module Integration Rules
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static bool CanModuleAccessOther(const FString& RequesterModule, const FString& TargetModule);

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules")
    static TArray<FString> GetModuleDependencies(const FString& ModuleName);

private:
    // Architecture constants
    static constexpr float WORLD_PARTITION_THRESHOLD_KM = 4.0f;
    static constexpr float TARGET_FRAME_TIME_MS = 16.67f; // 60 FPS
    static constexpr float MAX_MEMORY_BUDGET_MB = 8192.0f; // 8GB
    static constexpr int32 MAX_ACTORS_PER_BIOME = 1000;
};

/**
 * System Module Registration
 * Used by all agent systems to register with the core architecture
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemModule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "System Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadWrite, Category = "System Module")
    int32 Priority;

    UPROPERTY(BlueprintReadWrite, Category = "System Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadWrite, Category = "System Module")
    float FrameTimeBudget;

    UPROPERTY(BlueprintReadWrite, Category = "System Module")
    bool bRequiresWorldPartition;

    FEng_SystemModule()
    {
        ModuleName = TEXT("");
        Priority = 0;
        Dependencies = TArray<FString>();
        FrameTimeBudget = 1.0f;
        bRequiresWorldPartition = false;
    }
};

#include "EngineArchitectCore.generated.h"