#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalRules.generated.h"

UENUM(BlueprintType)
enum class EEng_RuleViolationType : uint8
{
    None = 0,
    CircularDependency,
    MissingDependency,
    PerformanceViolation,
    LayerViolation,
    CompilationError,
    IntegrationFailure
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FString Description;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    bool bIsEnforced;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    int32 Severity; // 1-10, 10 being critical

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    TArray<FString> AffectedModules;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bIsEnforced = true;
        Severity = 5;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_RuleViolation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEng_RuleViolationType ViolationType;

    UPROPERTY(BlueprintReadOnly)
    FString RuleName;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    int32 Severity;

    UPROPERTY(BlueprintReadOnly)
    FDateTime DetectionTime;

    UPROPERTY(BlueprintReadOnly)
    bool bIsResolved;

    FEng_RuleViolation()
    {
        ViolationType = EEng_RuleViolationType::None;
        RuleName = TEXT("");
        Description = TEXT("");
        ModuleName = TEXT("");
        Severity = 0;
        DetectionTime = FDateTime::Now();
        bIsResolved = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalRules : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalRules();

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    void InitializeDefaultRules();

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    bool ValidateRule(const FString& RuleName, const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    TArray<FEng_RuleViolation> CheckAllRules(const TArray<FString>& ModuleNames);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    void AddRule(const FEng_ArchitecturalRule& NewRule);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    void RemoveRule(const FString& RuleName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    FEng_ArchitecturalRule GetRule(const FString& RuleName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    TArray<FEng_ArchitecturalRule> GetRulesByModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    void ReportViolation(const FEng_RuleViolation& Violation);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    TArray<FEng_RuleViolation> GetActiveViolations();

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    void ResolveViolation(const FString& RuleName, const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Rules")
    int32 GetViolationCount() const;

protected:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rules")
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    UPROPERTY(BlueprintReadOnly, Category = "Violations")
    TArray<FEng_RuleViolation> ActiveViolations;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
    bool bAutoEnforcement;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
    float ValidationInterval;

private:
    void CreateCoreRules();
    void CreatePerformanceRules();
    void CreateIntegrationRules();
    bool ValidateModuleDependencies(const FString& ModuleName);
    bool ValidatePerformanceTargets(const FString& ModuleName);
    bool ValidateLayerCompliance(const FString& ModuleName);
};