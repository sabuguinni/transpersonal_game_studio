#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "TechnicalArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArchitectureLayer : uint8
{
    Engine,          // UE5 Core Systems
    Framework,       // Game Framework Classes
    Gameplay,        // Game-specific Systems
    Content,         // Asset and Content Pipeline
    Platform,        // Platform-specific Code
    Tools           // Development and Debug Tools
};

UENUM(BlueprintType)
enum class ESystemPriority : uint8
{
    Critical,       // Core engine systems
    High,          // Essential gameplay systems
    Medium,        // Important but not critical
    Low,           // Nice-to-have features
    Optional       // Debug/development only
};

USTRUCT(BlueprintType)
struct FSystemDefinition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EArchitectureLayer Layer;

    UPROPERTY(BlueprintReadOnly)
    ESystemPriority Priority;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> RequiredClasses;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> RequiredModules;

    UPROPERTY(BlueprintReadOnly)
    bool bIsImplemented;

    UPROPERTY(BlueprintReadOnly)
    bool bIsTested;

    UPROPERTY(BlueprintReadOnly)
    FString ResponsibleAgent;

    FSystemDefinition()
    {
        Layer = EArchitectureLayer::Gameplay;
        Priority = ESystemPriority::Medium;
        bIsImplemented = false;
        bIsTested = false;
    }
};

USTRUCT(BlueprintType)
struct FArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString RuleName;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    ESystemPriority Severity;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> AppliesTo;

    UPROPERTY(BlueprintReadOnly)
    FString Enforcement;

    FArchitectureRule()
    {
        Severity = ESystemPriority::Medium;
    }
};

USTRUCT(BlueprintType)
struct FPerformanceTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString TargetName;

    UPROPERTY(BlueprintReadOnly)
    float TargetValue;

    UPROPERTY(BlueprintReadOnly)
    FString Unit;

    UPROPERTY(BlueprintReadOnly)
    FString Platform;

    UPROPERTY(BlueprintReadOnly)
    bool bIsHardRequirement;

    FPerformanceTarget()
    {
        TargetValue = 0.0f;
        bIsHardRequirement = false;
    }
};

/**
 * Technical Architecture Manager
 * Manages the technical architecture documentation and rules for the game
 * Enforces architectural decisions and validates system implementations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTechnicalArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UTechnicalArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void RegisterSystem(const FSystemDefinition& SystemDef);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void UpdateSystemStatus(const FString& SystemName, bool bImplemented, bool bTested);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Technical Architecture")
    TArray<FSystemDefinition> GetSystemsByLayer(EArchitectureLayer Layer) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Technical Architecture")
    TArray<FSystemDefinition> GetSystemsByPriority(ESystemPriority Priority) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Technical Architecture")
    FSystemDefinition GetSystemDefinition(const FString& SystemName) const;

    // Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void AddArchitectureRule(const FArchitectureRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateSystemAgainstRules(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetRuleViolations(const FString& SystemName) const;

    // Performance Targets
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetPerformanceTarget(const FPerformanceTarget& Target);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Technical Architecture")
    TArray<FPerformanceTarget> GetPerformanceTargets() const { return PerformanceTargets; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Technical Architecture")
    FPerformanceTarget GetPerformanceTarget(const FString& TargetName) const;

    // Documentation Generation
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void GenerateArchitectureDocument();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void ExportSystemDependencyGraph();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void GenerateImplementationReport();

    // Validation and Compliance
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateArchitectureCompliance();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetArchitectureViolations();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    float GetImplementationProgress() const;

protected:
    // Core system definitions
    UPROPERTY()
    TArray<FSystemDefinition> RegisteredSystems;

    // Architecture rules and constraints
    UPROPERTY()
    TArray<FArchitectureRule> ArchitectureRules;

    // Performance targets
    UPROPERTY()
    TArray<FPerformanceTarget> PerformanceTargets;

    // Internal methods
    void InitializeDefaultSystems();
    void InitializeDefaultRules();
    void InitializeDefaultPerformanceTargets();
    
    bool CheckSystemDependencies(const FSystemDefinition& System) const;
    void ValidateSystemImplementation(const FString& SystemName);

private:
    // Cache for quick lookups
    TMap<FString, int32> SystemNameToIndex;
    TMap<FString, int32> RuleNameToIndex;
    TMap<FString, int32> TargetNameToIndex;

    void RebuildCaches();
};