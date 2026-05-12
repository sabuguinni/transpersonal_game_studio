#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

/**
 * ENGINE ARCHITECT - CYCLE 009 ARCHITECTURAL FRAMEWORK
 * 
 * CRITICAL SYSTEM: Central architectural enforcement and coordination framework
 * Ensures all 18 agents follow mandatory architectural patterns and performance rules
 * 
 * RESPONSIBILITIES:
 * - Enforce architectural compliance across all agent outputs
 * - Validate system integration points and dependencies
 * - Monitor performance baselines (60fps PC / 30fps Console)
 * - Coordinate module initialization and shutdown sequences
 * - Provide architectural guidance to all technical agents
 */

// Architectural Compliance Levels
UENUM(BlueprintType)
enum class EEng_ComplianceLevel : uint8
{
    Critical        UMETA(DisplayName = "Critical - Must Fix"),
    Warning         UMETA(DisplayName = "Warning - Should Fix"),
    Advisory        UMETA(DisplayName = "Advisory - Consider Fix"),
    Compliant       UMETA(DisplayName = "Compliant - No Issues")
};

// Performance Profile Types
UENUM(BlueprintType)
enum class EEng_PerformanceProfile : uint8
{
    Development     UMETA(DisplayName = "Development - Debug Mode"),
    Console         UMETA(DisplayName = "Console - 30fps Target"),
    PC_High         UMETA(DisplayName = "PC High - 60fps Target"),
    PC_Ultra        UMETA(DisplayName = "PC Ultra - 120fps Target")
};

// System Integration Status
UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

// Architectural Rule Violation Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalViolation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString ViolationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_ComplianceLevel Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString SourceModule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString RecommendedFix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float DetectionTimestamp;

    FEng_ArchitecturalViolation()
    {
        ViolationType = "Unknown";
        Description = "No description provided";
        Severity = EEng_ComplianceLevel::Advisory;
        SourceModule = "Unknown";
        RecommendedFix = "Review architectural guidelines";
        DetectionTimestamp = 0.0f;
    }
};

// Performance Metrics Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PhysicsObjectCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceTargetMet;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        TargetFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        GPUUsagePercent = 0.0f;
        ActiveActorCount = 0;
        PhysicsObjectCount = 0;
        bPerformanceTargetMet = false;
    }
};

// System Module Registration Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemModule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString AgentOwner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bCriticalSystem;

    FEng_SystemModule()
    {
        ModuleName = "Unknown";
        AgentOwner = "Unknown";
        Status = EEng_SystemStatus::Uninitialized;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        bCriticalSystem = false;
    }
};

/**
 * ARCHITECTURAL FRAMEWORK SUBSYSTEM
 * 
 * Game Instance Subsystem that provides architectural oversight for the entire game
 * Validates all systems follow Engine Architect rules and performance requirements
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ARCHITECTURAL COMPLIANCE SYSTEM
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterArchitecturalViolation(const FEng_ArchitecturalViolation& Violation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ArchitecturalViolation> GetActiveViolations() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemCompliant(const FString& SystemName) const;

    // PERFORMANCE MONITORING SYSTEM
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceProfile(EEng_PerformanceProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // SYSTEM MODULE MANAGEMENT
    UFUNCTION(BlueprintCallable, Category = "System Management")
    void RegisterSystemModule(const FEng_SystemModule& Module);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void UpdateSystemStatus(const FString& ModuleName, EEng_SystemStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    TArray<FEng_SystemModule> GetRegisteredModules() const;

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool ValidateSystemDependencies(const FString& ModuleName) const;

    // ARCHITECTURAL GUIDANCE SYSTEM
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FString GetArchitecturalGuidance(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleIntegration(const FString& ModuleA, const FString& ModuleB) const;

    // EMERGENCY SYSTEMS
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyShutdown(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void InitializeRecoveryMode();

    // DEBUG AND MONITORING
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogArchitecturalStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateComplianceReport();

protected:
    // ARCHITECTURAL DATA
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ArchitecturalViolation> ActiveViolations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_SystemModule> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceProfile ActiveProfile;

    // CONFIGURATION
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableRealTimeValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bStrictComplianceMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEmergencyModeActive;

private:
    // Internal validation methods
    void ValidatePerformanceCompliance();
    void ValidateMemoryUsage();
    void ValidateSystemIntegrity();
    void UpdateSystemMetrics();
    
    // Timer handles
    FTimerHandle ValidationTimerHandle;
    FTimerHandle MetricsTimerHandle;
};

/**
 * WORLD ARCHITECTURAL COORDINATOR
 * 
 * World Subsystem that coordinates architectural compliance within specific world contexts
 * Handles level-specific architectural requirements and validations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitecturalCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitecturalCoordinator();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // WORLD-SPECIFIC VALIDATION
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldCompliance();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterWorldSystem(const FString& SystemName, AActor* SystemActor);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldArchitecturallySound() const;

    // LEVEL INTEGRATION
    UFUNCTION(BlueprintCallable, Category = "Level Integration")
    void OnLevelLoaded();

    UFUNCTION(BlueprintCallable, Category = "Level Integration")
    void OnLevelUnloaded();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Systems")
    TMap<FString, AActor*> WorldSystems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Validation")
    bool bWorldValidationComplete;

private:
    void ValidateActorCompliance();
    void ValidateSystemIntegration();
};