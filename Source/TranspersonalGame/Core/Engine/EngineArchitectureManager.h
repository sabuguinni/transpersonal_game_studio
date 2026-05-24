#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Performance monitoring data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
        : FrameTime(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , DrawCalls(0)
        , TriangleCount(0)
        , MemoryUsageMB(0.0f)
    {}
};

/**
 * System validation result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FEng_SystemValidation()
        : bIsValid(false)
        , ValidationTime(0.0f)
    {}
};

/**
 * Engine Architecture Manager - Core system that validates and monitors all game systems
 * This is the technical foundation that ensures architectural integrity across all modules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validate all core systems - called during startup and periodically
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAllSystems();

    /**
     * Get current performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetPerformanceMetrics() const;

    /**
     * Check if a specific system is valid and operational
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemValid(const FString& SystemName) const;

    /**
     * Get validation results for all systems
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemValidation> GetSystemValidations() const;

    /**
     * Force revalidation of a specific system
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RevalidateSystem(const FString& SystemName);

    /**
     * Register a new system for validation
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    /**
     * Unregister a system from validation
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    /**
     * Get architectural compliance score (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetArchitecturalComplianceScore() const;

    /**
     * Enable/disable performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceMonitoringEnabled(bool bEnabled);

    /**
     * Check if performance monitoring is active
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceMonitoringEnabled() const { return bPerformanceMonitoringEnabled; }

protected:
    /**
     * Validate core engine systems
     */
    bool ValidateCoreEngineSystems();

    /**
     * Validate game-specific systems
     */
    bool ValidateGameSystems();

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics();

    /**
     * Validate a single system
     */
    FEng_SystemValidation ValidateSystem(const FString& SystemName, UObject* SystemObject);

private:
    // Registered systems for validation
    UPROPERTY()
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    // Cached validation results
    UPROPERTY()
    TArray<FEng_SystemValidation> SystemValidations;

    // Current performance metrics
    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    // Performance monitoring state
    UPROPERTY()
    bool bPerformanceMonitoringEnabled;

    // Last validation time
    double LastValidationTime;

    // Validation interval (seconds)
    float ValidationInterval;

    // Timer handle for periodic validation
    FTimerHandle ValidationTimerHandle;
};