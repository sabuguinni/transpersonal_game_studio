#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CoreSystemsArchitect.generated.h"

/**
 * Engine Architect Core Systems Manager
 * Defines and enforces architectural standards across all game systems
 * Manages system initialization order, dependencies, and performance constraints
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreSystemsArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreSystemsArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration and Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterCoreSystem(const FString& SystemName, int32 Priority, bool bCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceThreshold(const FString& SystemName, float MaxFrameTime);

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool AddSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemDependencies(const FString& SystemName) const;

protected:
    // Core system registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> SystemPriorities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, bool> CriticalSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> SystemPerformanceMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> PerformanceThresholds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TArray<FString>> SystemDependencies;

    // Architectural constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxConcurrentSystems = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnforceStrictDependencies = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnablePerformanceMonitoring = true;

private:
    // Internal system management
    void InitializeSystemInOrder(const FString& SystemName);
    bool ValidateSystemDependencies(const FString& SystemName) const;
    void UpdatePerformanceMetrics();
    
    // System state tracking
    TSet<FString> InitializedSystems;
    TSet<FString> FailedSystems;
    
    // Performance tracking
    TMap<FString, double> SystemInitTimes;
    TMap<FString, int32> SystemFrameCount;
    
    // Validation cache
    mutable TArray<FString> CachedViolations;
    mutable bool bValidationCacheDirty = true;
};

/**
 * Architectural Standards Enforcer
 * Validates that all systems follow the established architectural patterns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalStandardsEnforcer : public UObject
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalStandardsEnforcer();

    // Standards validation
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidateClassArchitecture(UClass* ClassToValidate) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidateModuleStructure(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    TArray<FString> GetStandardsViolations(UClass* ClassToValidate) const;

    // Naming convention enforcement
    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    bool ValidateNamingConventions(const FString& ClassName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Standards")
    FString GetRecommendedClassName(const FString& SystemType, const FString& Purpose) const;

protected:
    // Standards definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    TMap<FString, FString> NamingConventions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    TArray<FString> RequiredBaseClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    TArray<FString> ForbiddenPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    bool bEnforceUPropertyMacros = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    bool bRequireGeneratedBody = true;

private:
    void InitializeStandards();
    bool CheckBaseClassCompliance(UClass* ClassToCheck) const;
    bool CheckPropertyMacroUsage(UClass* ClassToCheck) const;
    bool CheckForbiddenPatterns(const FString& ClassName) const;
};

/**
 * System Performance Monitor
 * Tracks and reports performance metrics for all core systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameTime = 999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 SampleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bExceedsThreshold = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ThresholdValue = 16.67f;

    FEng_SystemPerformanceData()
    {
        SystemName = TEXT("Unknown");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemPerformanceMonitor : public UObject
{
    GENERATED_BODY()

public:
    UEng_SystemPerformanceMonitor();

    // Performance tracking
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartSystemProfiling(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void EndSystemProfiling(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FEng_SystemPerformanceData GetSystemPerformanceData(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    TArray<FEng_SystemPerformanceData> GetAllPerformanceData() const;

    // Threshold management
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetPerformanceThreshold(const FString& SystemName, float ThresholdMs);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    TArray<FString> GetSystemsExceedingThreshold() const;

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FString GeneratePerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ResetPerformanceData();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, FEng_SystemPerformanceData> PerformanceData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, double> ProfilingStartTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DefaultThreshold = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSamples = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoResetOnOverflow = true;

private:
    void UpdatePerformanceData(const FString& SystemName, float FrameTime);
    double GetCurrentTimeMs() const;
};