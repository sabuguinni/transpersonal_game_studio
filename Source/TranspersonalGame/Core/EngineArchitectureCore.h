#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitectureCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Central Engine Architecture System - Manages all core architectural patterns
 * and ensures compliance across all game systems
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

    /** Get the singleton instance */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    static UEngineArchitectureCore* Get(const UObject* WorldContext);

    /** Initialize all core systems */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeCoreArchitecture();

    /** Validate system architecture compliance */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateArchitecturalCompliance();

    /** Register a new system with the architecture */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemComponent(const FString& SystemName, UObject* SystemInstance);

    /** Get system performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEnginePerformanceMetrics GetPerformanceMetrics() const;

    /** Force garbage collection and memory cleanup */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void ForceMemoryCleanup();

protected:
    /** Map of registered systems */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    /** Current performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEnginePerformanceMetrics CurrentMetrics;

    /** Architecture validation enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableArchitectureValidation = true;

    /** Maximum allowed systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxRegisteredSystems = 100;

private:
    /** Internal validation timer */
    FTimerHandle ValidationTimerHandle;

    /** Perform periodic validation */
    void PerformPeriodicValidation();

    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Cleanup orphaned systems */
    void CleanupOrphanedSystems();
};