#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_UnifiedArchitectureManager.generated.h"

// Forward declarations
class UEng_CompilationHealthMonitor;
class UBiomeManager;
class UEng_ModuleDependencyManager;

/**
 * ENGINE ARCHITECT CYCLE 006 - UNIFIED ARCHITECTURE MANAGEMENT SYSTEM
 * 
 * Consolidates all Engine Architect responsibilities into a single, coherent system:
 * - Compilation health monitoring and fixing
 * - Module dependency management
 * - Architecture validation and enforcement
 * - Performance optimization coordination
 * - Cross-system integration oversight
 * 
 * Replaces fragmented systems: Eng_ArchitectureManager, Eng_CompilationOrchestrator (v1-v3),
 * Eng_CompilationFixer, and provides unified interface for all architectural concerns.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_UnifiedArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_UnifiedArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === COMPILATION MANAGEMENT ===
    
    /** Check overall compilation health and identify critical issues */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Compilation")
    bool ValidateCompilationHealth();
    
    /** Fix identified compilation issues automatically */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Compilation")
    bool FixCompilationIssues();
    
    /** Get current compilation status report */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Compilation")
    FString GetCompilationStatusReport();

    // === MODULE DEPENDENCY MANAGEMENT ===
    
    /** Validate module dependencies and detect circular references */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Dependencies")
    bool ValidateModuleDependencies();
    
    /** Register a new module with dependency tracking */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Dependencies")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies);
    
    /** Get dependency graph for debugging */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Dependencies")
    TMap<FString, TArray<FString>> GetDependencyGraph();

    // === ARCHITECTURE VALIDATION ===
    
    /** Validate architecture standards compliance */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    bool ValidateArchitectureStandards();
    
    /** Enforce coding standards across all modules */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    void EnforceCodingStandards();
    
    /** Get architecture compliance report */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    FString GetArchitectureComplianceReport();

    // === PERFORMANCE COORDINATION ===
    
    /** Monitor system performance and identify bottlenecks */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    bool MonitorPerformanceMetrics();
    
    /** Coordinate performance optimization across systems */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    void OptimizeSystemPerformance();

    // === INTEGRATION OVERSIGHT ===
    
    /** Validate cross-system integration points */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Integration")
    bool ValidateSystemIntegration();
    
    /** Coordinate system initialization order */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Integration")
    void CoordinateSystemInitialization();

protected:
    // === INTERNAL SYSTEMS ===
    
    /** Compilation health monitoring component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal Systems")
    TObjectPtr<UEng_CompilationHealthMonitor> CompilationMonitor;
    
    /** Module dependency tracking */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal Systems")
    TObjectPtr<UEng_ModuleDependencyManager> DependencyManager;
    
    /** Biome system integration */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal Systems")
    TObjectPtr<UBiomeManager> BiomeSystemRef;

    // === CONFIGURATION ===
    
    /** Architecture validation enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bArchitectureValidationEnabled = true;
    
    /** Performance monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bPerformanceMonitoringEnabled = true;
    
    /** Auto-fix compilation issues */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoFixCompilationIssues = true;
    
    /** Maximum compilation fix attempts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxCompilationFixAttempts = 3;

    // === STATUS TRACKING ===
    
    /** Last compilation validation timestamp */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    FDateTime LastCompilationValidation;
    
    /** Last architecture validation timestamp */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    FDateTime LastArchitectureValidation;
    
    /** Current system health status */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    EEng_SystemHealthStatus SystemHealthStatus = EEng_SystemHealthStatus::Unknown;
    
    /** Active compilation issues count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    int32 ActiveCompilationIssues = 0;
    
    /** Active architecture violations count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    int32 ActiveArchitectureViolations = 0;

private:
    // === INTERNAL METHODS ===
    
    /** Initialize compilation monitoring */
    void InitializeCompilationMonitoring();
    
    /** Initialize dependency management */
    void InitializeDependencyManagement();
    
    /** Initialize architecture validation */
    void InitializeArchitectureValidation();
    
    /** Cleanup duplicate systems */
    void CleanupDuplicateSystems();
    
    /** Consolidate versioned systems */
    void ConsolidateVersionedSystems();
    
    /** Update system health status */
    void UpdateSystemHealthStatus();
    
    /** Log architecture event */
    void LogArchitectureEvent(const FString& Event, const FString& Details);
};

/**
 * Module Dependency Manager - Tracks and validates module dependencies
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ModuleDependencyManager : public UObject
{
    GENERATED_BODY()

public:
    /** Register module dependency */
    UFUNCTION(BlueprintCallable, Category = "Module Dependencies")
    void RegisterDependency(const FString& Module, const FString& Dependency);
    
    /** Check for circular dependencies */
    UFUNCTION(BlueprintCallable, Category = "Module Dependencies")
    bool HasCircularDependencies();
    
    /** Get dependency chain for module */
    UFUNCTION(BlueprintCallable, Category = "Module Dependencies")
    TArray<FString> GetDependencyChain(const FString& Module);

private:
    /** Module dependency map */
    UPROPERTY()
    TMap<FString, TArray<FString>> ModuleDependencies;
    
    /** Check circular dependency recursively */
    bool CheckCircularDependency(const FString& Module, const FString& Target, TSet<FString>& Visited);
};

#include "Eng_UnifiedArchitectureManager.generated.h"