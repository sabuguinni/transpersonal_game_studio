#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

// Forward declarations
class UEng_ModuleManager;
class UEng_PerformanceMonitor;
class UEng_SystemCoordinator;

/**
 * Core Architecture Subsystem
 * Central nervous system of the Transpersonal Game engine architecture.
 * Manages all major systems, coordinates module communication, and enforces architectural rules.
 * This is the foundation that all other systems build upon.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureReady() const;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName, const FString& ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetRegisteredModules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsModuleRegistered(const FString& ModuleName) const;

    // System Coordination
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void NotifySystemEvent(const FString& SystemName, const FString& EventType, const FString& EventData);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RequestSystemShutdown(const FString& SystemName, const FString& Reason);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName, const FString& MetricName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceThreshold(const FString& SystemName, const FString& MetricName, float Threshold);

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitectureWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitectureErrors() const;

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void DumpArchitectureState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void RunArchitectureDiagnostics();

protected:
    // Core subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Architecture Components")
    TObjectPtr<UEng_ModuleManager> ModuleManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Components")
    TObjectPtr<UEng_PerformanceMonitor> PerformanceMonitor;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Components")
    TObjectPtr<UEng_SystemCoordinator> SystemCoordinator;

    // Architecture state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    bool bArchitectureInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    bool bArchitectureReady;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    float InitializationTime;

    // Module registry
    UPROPERTY()
    TMap<FString, FString> RegisteredModules;

    // System event tracking
    UPROPERTY()
    TArray<FString> SystemEvents;

    UPROPERTY()
    TArray<FString> ArchitectureWarnings;

    UPROPERTY()
    TArray<FString> ArchitectureErrors;

    // Performance thresholds
    UPROPERTY()
    TMap<FString, float> PerformanceThresholds;

private:
    // Internal initialization methods
    void InitializeSubsystems();
    void SetupModuleManager();
    void SetupPerformanceMonitor();
    void SetupSystemCoordinator();

    // Internal validation methods
    bool ValidateSubsystems() const;
    bool ValidateModuleIntegrity() const;
    bool ValidatePerformanceMetrics() const;

    // Event handling
    void HandleSystemEvent(const FString& SystemName, const FString& EventType, const FString& EventData);
    void HandlePerformanceAlert(const FString& SystemName, const FString& MetricName, float Value);

    // Logging and diagnostics
    void LogArchitectureEvent(const FString& EventMessage, bool bIsWarning = false, bool bIsError = false);
    void UpdateArchitectureMetrics();

    // Timing and performance tracking
    double LastUpdateTime;
    double LastDiagnosticsTime;
    int32 UpdateFrameCounter;
};

/**
 * World-specific Architecture Coordinator
 * Manages architecture concerns specific to individual world instances.
 * Handles world loading/unloading, level streaming, and world-specific system coordination.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World Architecture Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ShutdownWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldReady() const;

    // Level Streaming Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterStreamingLevel(const FString& LevelName, const FString& LevelType);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void UnregisterStreamingLevel(const FString& LevelName);

    // World-specific System Coordination
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void NotifyWorldEvent(const FString& EventType, const FString& EventData);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World State")
    bool bWorldSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    bool bWorldReady;

    UPROPERTY()
    TMap<FString, FString> StreamingLevels;

    UPROPERTY()
    TArray<FString> WorldEvents;

private:
    void SetupWorldSystems();
    void ValidateWorldIntegrity();
    void HandleWorldEvent(const FString& EventType, const FString& EventData);
};