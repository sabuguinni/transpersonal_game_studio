#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Eng_MasterArchitect.generated.h"

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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString RuleDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_ArchitecturalLayer TargetLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> Dependencies;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("DefaultRule");
        RuleDescription = TEXT("Default architectural rule");
        TargetLayer = EEng_ArchitecturalLayer::Core;
        Priority = EEng_SystemPriority::Medium;
        bMandatory = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemSpecification
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> OptionalModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float PerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsActive;

    FEng_SystemSpecification()
    {
        SystemName = TEXT("DefaultSystem");
        SystemDescription = TEXT("Default system specification");
        Layer = EEng_ArchitecturalLayer::Core;
        Priority = EEng_SystemPriority::Medium;
        PerformanceBudget = 1.0f;
        bIsActive = true;
    }
};

/**
 * Master Architect - The supreme technical authority that defines and enforces
 * the complete architectural framework for the prehistoric survival game.
 * This class establishes the foundational rules, patterns, and standards
 * that all other systems must follow.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_MasterArchitect : public AActor
{
    GENERATED_BODY()

public:
    AEng_MasterArchitect();

protected:
    virtual void BeginPlay() override;

    // === ARCHITECTURAL FOUNDATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Architecture")
    TArray<FEng_ArchitecturalRule> CoreArchitecturalRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Architecture")
    TArray<FEng_SystemSpecification> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Architecture")
    float GlobalPerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Architecture")
    bool bEnforceStrictCompliance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Architecture")
    int32 MaxConcurrentSystems;

    // === SYSTEM MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Management")
    TMap<FString, bool> SystemActiveStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Management")
    TMap<FString, float> SystemPerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Management")
    TArray<FString> CriticalSystemFailures;

public:
    // === ARCHITECTURAL GOVERNANCE ===
    UFUNCTION(BlueprintCallable, Category = "Master Architecture")
    bool ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Master Architecture")
    bool RegisterNewSystem(const FEng_SystemSpecification& SystemSpec);

    UFUNCTION(BlueprintCallable, Category = "Master Architecture")
    bool EnforceArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Master Architecture")
    void InitializeArchitecturalFramework();

    // === SYSTEM LIFECYCLE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool ActivateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool DeactivateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    TArray<FString> GetFailedSystems();

    // === PERFORMANCE GOVERNANCE ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemPerformanceImpact(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceBudgetExceeded();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemPerformance();

    // === ARCHITECTURAL REPORTING ===
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateArchitecturalReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    TArray<FString> GetSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogArchitecturalViolation(const FString& Violation);

    // === EDITOR UTILITIES ===
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidateProjectArchitecture();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void GenerateSystemDependencyGraph();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void AuditPerformanceBudgets();

private:
    void SetupDefaultArchitecturalRules();
    void InitializeCoreSystems();
    bool CheckSystemDependencies(const FString& SystemName);
    void UpdatePerformanceMetrics();
};