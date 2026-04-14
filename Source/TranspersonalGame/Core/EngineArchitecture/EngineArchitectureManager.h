#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Engine Architecture Performance Metrics
 * Tracks system performance and validates architecture rules
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

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsWithinTargetFrameRate;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        ActiveActorCount = 0;
        bIsWithinTargetFrameRate = true;
    }
};

/**
 * Architecture Rule Validation Status
 * Tracks compliance with engine architecture rules
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureRuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsCompliant;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EEng_ValidationSeverity Severity;

    FEng_ArchitectureRuleStatus()
    {
        RuleName = TEXT("");
        bIsCompliant = true;
        ValidationMessage = TEXT("");
        Severity = EEng_ValidationSeverity::Info;
    }
};

/**
 * Module Dependency Information
 * Tracks dependencies between game modules
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LoadTime;

    FEng_ModuleDependency()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        LoadTime = 0.0f;
    }
};

/**
 * Engine Architecture Manager
 * Central system that enforces architecture rules and validates system performance
 * This is the technical foundation that all other agents must follow
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
     * Performance Monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    bool ValidateTargetFrameRate(float TargetFPS = 60.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Performance")
    void SetPerformanceTarget(float TargetFPS, float MaxMemoryMB);

    /**
     * Architecture Rule Validation
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    TArray<FEng_ArchitectureRuleStatus> ValidateArchitectureRules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    bool ValidateWorldPartitionRequirements() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    bool ValidateLODChainCompliance() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Validation")
    bool ValidateMemoryManagement() const;

    /**
     * Module Management
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Modules")
    TArray<FEng_ModuleDependency> GetModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Modules")
    bool ValidateModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Modules")
    void RegisterModuleValidation(const FString& ModuleName, const TArray<FString>& Dependencies);

    /**
     * System Integration
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture|Integration")
    bool CheckCrossSystemCompatibility(const FString& SystemA, const FString& SystemB) const;

    /**
     * Development Tools
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture|Tools")
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture|Tools")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture|Tools")
    void EnablePerformanceMonitoring(bool bEnable);

protected:
    /**
     * Performance Tracking
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bPerformanceMonitoringEnabled;

    /**
     * Architecture Rules
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_ArchitectureRuleStatus> ArchitectureRules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bEnforceArchitectureRules;

    /**
     * Module Dependencies
     */
    UPROPERTY(BlueprintReadOnly, Category = "Modules", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_ModuleDependency> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Modules", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> ModuleDependencyMap;

    /**
     * System Validation
     */
    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidatedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

private:
    /**
     * Internal validation methods
     */
    void InitializeArchitectureRules();
    void InitializeModuleDependencies();
    FEng_ArchitectureRuleStatus ValidateRule(const FString& RuleName, bool bCondition, const FString& Message, EEng_ValidationSeverity Severity) const;
    void UpdateFrameTimeMetrics();
    void UpdateMemoryMetrics();
    void UpdateRenderMetrics();

    /**
     * Timer handles
     */
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle ValidationTimer;
};