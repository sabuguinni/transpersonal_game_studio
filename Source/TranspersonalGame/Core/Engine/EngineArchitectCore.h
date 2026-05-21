#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectCore.generated.h"

class UBiomeManager;
class UPhysicsSystemManager;
class UPerformanceMonitor;

/**
 * Core Engine Architecture Management System
 * Defines and enforces architectural rules across all game systems
 * Manages module dependencies, performance constraints, and integration protocols
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Registration System
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName, int32 Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsModuleRegistered(const FString& ModuleName) const;

    // Performance Constraint Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceConstraint(EEng_PerformanceLevel Level, float MaxFrameTime, int32 MaxActors);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceCompliance() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    // System Integration Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CanSystemsIntegrate(const FString& SystemA, const FString& SystemB) const;

    // Architecture Compliance
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceArchitecturalRules();

    // Module Dependency Chain
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetModuleDependencyChain(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateDependencyChain() const;

    // Critical System Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void MonitorCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsCriticalSystemHealthy(const FString& SystemName) const;

protected:
    // Module Registry
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    // Performance Constraints
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<EEng_PerformanceLevel, FEng_PerformanceConstraints> PerformanceConstraints;

    // System Integration Rules
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> SystemCompatibilityMatrix;

    // Architectural Violations
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CurrentViolations;

    // Critical System Status
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, bool> CriticalSystemStatus;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture", meta = (AllowPrivateAccess = "true"))
    FEng_PerformanceMetrics LastPerformanceCheck;

private:
    // Internal validation methods
    void ValidateModuleDependencies();
    void CheckPerformanceThresholds();
    void UpdateSystemCompatibility();
    void LogArchitecturalEvent(const FString& Event, const FString& Details);

    // Timer handles
    FTimerHandle PerformanceMonitorTimer;
    FTimerHandle SystemValidationTimer;

    // Constants
    static constexpr float PERFORMANCE_CHECK_INTERVAL = 5.0f;
    static constexpr float SYSTEM_VALIDATION_INTERVAL = 10.0f;
    static constexpr int32 MAX_ARCHITECTURAL_VIOLATIONS = 10;
};

/**
 * World-specific Engine Architecture Subsystem
 * Manages world-level architectural concerns and validation
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitectWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectWorldSubsystem();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World-level Architecture Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldArchitecture();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldArchitectureValid() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void EnforceWorldConstraints();

    // Biome Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateBiomeArchitecture(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void OptimizeBiomePerformance(EEng_BiomeType BiomeType);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    bool bWorldArchitectureValid;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> WorldArchitectureIssues;

private:
    void CheckWorldLevelConstraints();
    void ValidateActorDistribution();
    void MonitorWorldPerformance();
};

#include "EngineArchitectCore.generated.h"