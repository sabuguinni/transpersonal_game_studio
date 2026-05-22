#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectCore.generated.h"

/**
 * Engine Architect Core System
 * Manages technical architecture, compilation validation, and module integrity
 * Ensures all systems follow established architectural patterns
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Engine Architect Core"))
class TRANSPERSONALGAME_API UEngineArchitectCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceArchitecturalRules();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetSystemPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void OptimizeMemoryUsage();

    // Module dependency management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

protected:
    // Core architecture properties
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bModuleIntegrityValid;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bCompilationStatusClean;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float MemoryUsageThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidationErrors;

private:
    // Internal validation methods
    bool ValidateClassDefinitions();
    bool ValidateHeaderIncludes();
    bool CheckForCircularDependencies();
    void LogArchitecturalViolation(const FString& Violation);
    void CleanupOrphanedReferences();

    // Performance monitoring
    float LastMemoryCheck;
    int32 LastActorCount;
    FDateTime LastValidationTime;
};

/**
 * Architecture Rule Enforcement Component
 * Attached to critical game objects to enforce architectural compliance
 */
UCLASS(BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "Architecture Enforcer"))
class TRANSPERSONALGAME_API UArchitectureEnforcerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureEnforcerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Rule enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateComponentHierarchy();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckPerformanceCompliance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnforceNamingRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bValidateHierarchy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float PerformanceCheckInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> RuleViolations;

private:
    float LastPerformanceCheck;
    void CheckOwnerCompliance();
};

/**
 * Technical Debt Tracker
 * Monitors and reports technical debt accumulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TechnicalDebtEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Debt")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Debt")
    FString DebtDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Debt")
    EEng_DebtSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Debt")
    FDateTime CreatedDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Debt")
    float EstimatedFixTime;

    FEng_TechnicalDebtEntry()
    {
        ModuleName = TEXT("");
        DebtDescription = TEXT("");
        Severity = EEng_DebtSeverity::Low;
        CreatedDate = FDateTime::Now();
        EstimatedFixTime = 1.0f;
    }
};

/**
 * Architecture Compliance Manager
 * Singleton that enforces architectural standards across all modules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureComplianceManager : public UObject
{
    GENERATED_BODY()

public:
    UArchitectureComplianceManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance", CallInEditor)
    static UArchitectureComplianceManager* GetInstance();

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    void RegisterTechnicalDebt(const FEng_TechnicalDebtEntry& DebtEntry);

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    TArray<FEng_TechnicalDebtEntry> GetTechnicalDebtReport();

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    void ClearResolvedDebt(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    float CalculateTotalDebtScore();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compliance", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_TechnicalDebtEntry> TechnicalDebtEntries;

    UPROPERTY(BlueprintReadOnly, Category = "Compliance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, int32> ModuleViolationCounts;

private:
    static UArchitectureComplianceManager* Instance;
    void InitializeComplianceRules();
};