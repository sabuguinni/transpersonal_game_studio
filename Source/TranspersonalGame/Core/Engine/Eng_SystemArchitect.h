#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_SystemArchitect.generated.h"

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
    Deferred      UMETA(DisplayName = "Deferred Priority")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float PerformanceWeight;

    FEng_SystemDefinition()
    {
        SystemName = TEXT("");
        Layer = EEng_ArchitecturalLayer::Core;
        Priority = EEng_SystemPriority::Medium;
        bIsActive = true;
        PerformanceWeight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString RuleDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    EEng_ArchitecturalLayer ApplicableLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bIsMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float ViolationPenalty;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("");
        RuleDescription = TEXT("");
        ApplicableLayer = EEng_ArchitecturalLayer::Core;
        bIsMandatory = true;
        ViolationPenalty = 1.0f;
    }
};

/**
 * System Architect - Supreme technical authority for architectural governance
 * Defines and enforces the foundational structure that governs all game systems
 * Establishes architectural layers, system priorities, and technical standards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemArchitect();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool RegisterSystem(const FEng_SystemDefinition& SystemDef);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    FEng_SystemDefinition GetSystemDefinition(const FString& SystemName) const;

    // Architectural Rules
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    void AddArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool ValidateSystemAgainstRules(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetRuleViolations(const FString& SystemName) const;

    // Layer Management
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetSystemsByLayer(EEng_ArchitecturalLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool CanSystemAccessLayer(const FString& SystemName, EEng_ArchitecturalLayer TargetLayer) const;

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool ValidateDependencyChain(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetDependencyOrder() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool HasCircularDependencies() const;

    // Performance Governance
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    float GetTotalSystemWeight() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetHighPerformanceImpactSystems() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool IsPerformanceBudgetExceeded() const;

    // System Status
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    int32 GetRegisteredSystemCount() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    int32 GetActiveSystemCount() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetInactiveSystems() const;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "System Architect")
    bool ValidateArchitecturalIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect")
    TArray<FString> GetArchitecturalIssues() const;

    UFUNCTION(BlueprintCallable, Category = "System Architect", CallInEditor = true)
    void RunArchitecturalAudit();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "System Architect")
    void OnSystemRegistered(const FString& SystemName);

    UFUNCTION(BlueprintImplementableEvent, Category = "System Architect")
    void OnSystemUnregistered(const FString& SystemName);

    UFUNCTION(BlueprintImplementableEvent, Category = "System Architect")
    void OnArchitecturalViolation(const FString& SystemName, const FString& Violation);

protected:
    // System Registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System Architect")
    TMap<FString, FEng_SystemDefinition> RegisteredSystems;

    // Architectural Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect")
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    // Performance Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxPerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float HighImpactThreshold;

    // Validation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect")
    bool bEnableRealTimeValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect")
    bool bStrictDependencyChecking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Architect")
    bool bEnforceLayerBoundaries;

private:
    // Internal validation methods
    bool ValidateSystemDefinition(const FEng_SystemDefinition& SystemDef) const;
    bool CheckLayerAccess(EEng_ArchitecturalLayer SourceLayer, EEng_ArchitecturalLayer TargetLayer) const;
    void BuildDependencyGraph(TMap<FString, TArray<FString>>& OutGraph) const;
    bool DetectCircularDependency(const TMap<FString, TArray<FString>>& Graph, const FString& StartNode, TSet<FString>& Visited, TSet<FString>& RecStack) const;
};