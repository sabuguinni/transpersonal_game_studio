#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitectureValidationLevel : uint8
{
    None = 0,
    Basic = 1,
    Standard = 2,
    Strict = 3,
    Enterprise = 4
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Optional = 4
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Console = 0,      // 30fps target
    PC_Low = 1,       // 45fps target
    PC_Medium = 2,    // 60fps target
    PC_High = 3,      // 90fps target
    PC_Ultra = 4      // 120fps target
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float LoadOrder;

    FEng_SystemDependency()
    {
        SystemName = TEXT("");
        Priority = EEng_SystemPriority::Medium;
        bIsRequired = false;
        LoadOrder = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles;

    FEng_PerformanceBudget()
    {
        CPUBudgetMS = 16.67f;  // 60fps
        GPUBudgetMS = 16.67f;
        MaxMemoryMB = 8192;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> ValidationWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationScore;

    FEng_ModuleValidationResult()
    {
        ModuleName = TEXT("");
        bIsValid = false;
        ValidationScore = 0.0f;
    }
};

/**
 * Engine Architecture Manager - Central authority for technical architecture
 * Enforces coding standards, performance budgets, and system dependencies
 * Created by Engine Architect Agent #02
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_ModuleValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystemDependency(const FEng_SystemDependency& Dependency);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTier(EEng_PerformanceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceBudget GetCurrentPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceBudget(const FString& SystemName, float CPUTime, float GPUTime);

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetRegisteredSystems() const;

    // Validation Rules
    UFUNCTION(BlueprintCallable, Category = "Validation")
    void SetValidationLevel(EEng_ArchitectureValidationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool EnforceArchitectureRules(const FString& SystemName);

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void RunArchitectureValidation();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogSystemStatus();

protected:
    // Core Architecture Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_ArchitectureValidationLevel ValidationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_PerformanceTier CurrentPerformanceTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_PerformanceBudget PerformanceBudget;

    // System Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    TArray<FEng_SystemDependency> SystemDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    TMap<FString, EEng_SystemPriority> RegisteredSystems;

    // Validation Results
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FEng_ModuleValidationResult> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bStrictModeEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidationEnabled;

private:
    // Internal validation methods
    bool ValidatePerformanceCompliance(const FString& SystemName);
    bool ValidateDependencyChain(const FString& SystemName);
    bool ValidateMemoryUsage(const FString& SystemName);
    void UpdatePerformanceBudgetForTier();
    void LogValidationResult(const FEng_ModuleValidationResult& Result);
};