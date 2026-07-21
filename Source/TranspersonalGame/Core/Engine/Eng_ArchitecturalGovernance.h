#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalGovernance.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Foundation      UMETA(DisplayName = "Foundation Layer"),
    Core           UMETA(DisplayName = "Core Systems Layer"),
    Gameplay       UMETA(DisplayName = "Gameplay Layer"),
    Interface      UMETA(DisplayName = "Interface Layer"),
    Performance    UMETA(DisplayName = "Performance Layer")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical Priority"),
    High          UMETA(DisplayName = "High Priority"),
    Medium        UMETA(DisplayName = "Medium Priority"),
    Low           UMETA(DisplayName = "Low Priority"),
    Background    UMETA(DisplayName = "Background Priority")
};

UENUM(BlueprintType)
enum class EEng_ArchitecturalRule : uint8
{
    MandatoryCompliance    UMETA(DisplayName = "Mandatory Compliance"),
    StrongRecommendation   UMETA(DisplayName = "Strong Recommendation"),
    BestPractice          UMETA(DisplayName = "Best Practice"),
    Guideline             UMETA(DisplayName = "Guideline"),
    Suggestion            UMETA(DisplayName = "Suggestion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    FString RuleDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    EEng_ArchitecturalRule RuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    EEng_ArchitecturalLayer ApplicableLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    bool bIsEnforced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Rule")
    float ComplianceScore;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("");
        RuleDescription = TEXT("");
        RuleType = EEng_ArchitecturalRule::Guideline;
        ApplicableLayer = EEng_ArchitecturalLayer::Core;
        bIsEnforced = false;
        ComplianceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    float PerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    float CPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    int32 ActiveComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Metrics")
    bool bIsHealthy;

    FEng_SystemMetrics()
    {
        SystemName = TEXT("");
        PerformanceScore = 0.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        ActiveComponents = 0;
        bIsHealthy = true;
    }
};

/**
 * Eng_ArchitecturalGovernance - Supreme architectural authority system
 * Enforces architectural standards, monitors system compliance, and governs technical decisions
 * across all game systems in the prehistoric survival game.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalGovernance : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalGovernance();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architectural Rules Management
    UFUNCTION(BlueprintCallable, Category = "Architectural Governance")
    void RegisterArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Architectural Governance")
    bool ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Governance")
    float GetSystemComplianceScore(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Governance")
    TArray<FEng_ArchitecturalRule> GetRulesForLayer(EEng_ArchitecturalLayer Layer);

    // System Registration and Monitoring
    UFUNCTION(BlueprintCallable, Category = "System Management")
    void RegisterSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool IsSystemRegistered(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    TArray<FString> GetSystemsInLayer(EEng_ArchitecturalLayer Layer);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdateSystemMetrics(const FString& SystemName, const FEng_SystemMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FEng_SystemMetrics GetSystemMetrics(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    TArray<FString> GetUnhealthySystems();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetOverallArchitecturalHealth();

    // Architectural Enforcement
    UFUNCTION(BlueprintCallable, Category = "Architectural Enforcement")
    void EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Architectural Enforcement")
    bool CanSystemActivate(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Enforcement")
    void BlockSystemActivation(const FString& SystemName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Architectural Enforcement")
    void UnblockSystemActivation(const FString& SystemName);

    // Governance Reporting
    UFUNCTION(BlueprintCallable, Category = "Governance Reporting")
    FString GenerateArchitecturalReport();

    UFUNCTION(BlueprintCallable, Category = "Governance Reporting")
    TArray<FString> GetComplianceViolations();

    UFUNCTION(BlueprintCallable, Category = "Governance Reporting")
    void LogArchitecturalDecision(const FString& Decision, const FString& Justification);

protected:
    // Architectural Rules Storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architectural Rules", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    // System Registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Registry", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EEng_ArchitecturalLayer> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Registry", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EEng_SystemPriority> SystemPriorities;

    // Performance Metrics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Metrics", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemMetrics> SystemMetrics;

    // Enforcement Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement", meta = (AllowPrivateAccess = "true"))
    TArray<FString> BlockedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FString> BlockReasons;

    // Governance Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Governance Config")
    bool bStrictComplianceMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Governance Config")
    float MinimumComplianceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Governance Config")
    bool bAutoEnforcement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Governance Config")
    float PerformanceThreshold;

private:
    void InitializeDefaultRules();
    void ValidateSystemArchitecture();
    float CalculateComplianceScore(const FString& SystemName);
    bool CheckRuleCompliance(const FString& SystemName, const FEng_ArchitecturalRule& Rule);
    void LogGovernanceAction(const FString& Action, const FString& Details);
};