#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EngineArchitectRules.generated.h"

/**
 * Technical Architecture Rules and Constraints
 * Defines mandatory rules that all agents must follow
 */

/**
 * Module dependency rule types
 */
UENUM(BlueprintType)
enum class EEng_DependencyRule : uint8
{
    Required        UMETA(DisplayName = "Required"),
    Optional        UMETA(DisplayName = "Optional"),
    Forbidden       UMETA(DisplayName = "Forbidden"),
    Conditional     UMETA(DisplayName = "Conditional")
};

/**
 * Performance constraint types
 */
UENUM(BlueprintType)
enum class EEng_PerformanceConstraint : uint8
{
    FrameRate       UMETA(DisplayName = "Frame Rate"),
    MemoryUsage     UMETA(DisplayName = "Memory Usage"),
    ActorCount      UMETA(DisplayName = "Actor Count"),
    DrawCalls       UMETA(DisplayName = "Draw Calls"),
    TickTime        UMETA(DisplayName = "Tick Time")
};

/**
 * Architecture rule definition
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
    bool bIsMandatory = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
    FString ViolationMessage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
    int32 Priority = 1; // 1 = Critical, 5 = Low

    FEng_ArchitectureRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bIsMandatory = true;
        ViolationMessage = TEXT("Architecture rule violated");
        Priority = 1;
    }
};

/**
 * Module dependency rule
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    EEng_DependencyRule DependencyType = EEng_DependencyRule::Optional;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    TArray<FString> RequiredByModules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    TArray<FString> ConflictsWith;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    FString MinVersion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    FString MaxVersion;

    FEng_ModuleRule()
    {
        ModuleName = TEXT("");
        DependencyType = EEng_DependencyRule::Optional;
        MinVersion = TEXT("1.0.0");
        MaxVersion = TEXT("999.0.0");
    }
};

/**
 * Performance constraint rule
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceConstraint ConstraintType = EEng_PerformanceConstraint::FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MinValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxValue = 999999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float WarningThreshold = 0.8f; // 80% of max before warning

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    FString Units;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnforceStrictly = false;

    FEng_PerformanceRule()
    {
        ConstraintType = EEng_PerformanceConstraint::FrameRate;
        MinValue = 0.0f;
        MaxValue = 999999.0f;
        WarningThreshold = 0.8f;
        Units = TEXT("");
        bEnforceStrictly = false;
    }
};

/**
 * Coding standard rule
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CodingRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding")
    FString RuleCategory; // "Naming", "Structure", "Documentation", etc.

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding")
    FString Pattern; // Regex pattern or description

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding")
    FString Example;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding")
    FString CounterExample;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding")
    bool bAutoFixable = false;

    FEng_CodingRule()
    {
        RuleCategory = TEXT("General");
        Pattern = TEXT("");
        Example = TEXT("");
        CounterExample = TEXT("");
        bAutoFixable = false;
    }
};

/**
 * Engine Architecture Rules Data Asset
 * Contains all technical rules and constraints for the project
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectRules : public UDataAsset
{
    GENERATED_BODY()

public:
    // General architecture rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture Rules")
    TArray<FEng_ArchitectureRule> GeneralRules;

    // Module dependency rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module Rules")
    TArray<FEng_ModuleRule> ModuleRules;

    // Performance constraint rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Rules")
    TArray<FEng_PerformanceRule> PerformanceRules;

    // Coding standard rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coding Rules")
    TArray<FEng_CodingRule> CodingRules;

    // Rule validation
    UFUNCTION(BlueprintCallable, Category = "Rule Validation")
    bool ValidateRule(const FString& RuleName, const FString& Context = TEXT("")) const;

    UFUNCTION(BlueprintCallable, Category = "Rule Validation")
    TArray<FString> GetViolatedRules(const FString& Context = TEXT("")) const;

    UFUNCTION(BlueprintCallable, Category = "Rule Validation")
    FEng_ArchitectureRule GetRule(const FString& RuleName) const;

    // Module rule helpers
    UFUNCTION(BlueprintCallable, Category = "Module Rules")
    bool IsModuleRequired(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Rules")
    bool IsModuleForbidden(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Rules")
    TArray<FString> GetRequiredModules() const;

    // Performance rule helpers
    UFUNCTION(BlueprintCallable, Category = "Performance Rules")
    float GetPerformanceLimit(EEng_PerformanceConstraint ConstraintType) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Rules")
    bool IsPerformanceWithinLimits(EEng_PerformanceConstraint ConstraintType, float Value) const;

    // Coding rule helpers
    UFUNCTION(BlueprintCallable, Category = "Coding Rules")
    TArray<FEng_CodingRule> GetCodingRules(const FString& Category = TEXT("")) const;

    UFUNCTION(BlueprintCallable, Category = "Coding Rules")
    bool ValidateCodingPattern(const FString& Category, const FString& Code) const;

protected:
    // Initialize default rules
    UFUNCTION(CallInEditor)
    void InitializeDefaultRules();

private:
    void AddDefaultArchitectureRules();
    void AddDefaultModuleRules();
    void AddDefaultPerformanceRules();
    void AddDefaultCodingRules();
};