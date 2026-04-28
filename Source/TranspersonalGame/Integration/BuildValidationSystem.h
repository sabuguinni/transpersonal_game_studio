#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "BuildValidationSystem.generated.h"

/**
 * Build Validation System - Comprehensive integration testing and validation
 * Validates module loading, actor spawning, cross-system integration
 * Used by Integration & Build Agent #19 for continuous integration
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    EBuild_ValidationResult LoadStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float LoadTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        LoadStatus = EBuild_ValidationResult::Pending;
        ErrorMessage = TEXT("");
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationTest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ExecutionTime;

    FBuild_IntegrationTest()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Pending;
        Description = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ModuleSuccessRate;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float IntegrationSuccessRate;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    EBuild_HealthStatus OverallHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> Recommendations;

    FBuild_SystemHealth()
    {
        ModuleSuccessRate = 0.0f;
        IntegrationSuccessRate = 0.0f;
        TotalActorsInLevel = 0;
        OverallHealth = EBuild_HealthStatus::Unknown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildValidationSystem : public AActor
{
    GENERATED_BODY()

public:
    ABuildValidationSystem();

protected:
    virtual void BeginPlay() override;

    // Core validation data
    UPROPERTY(BlueprintReadOnly, Category = "Build Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_IntegrationTest> IntegrationTests;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation", meta = (AllowPrivateAccess = "true"))
    FBuild_SystemHealth SystemHealth;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bAutoRunValidationOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bCreateTestActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bLogDetailedResults;

    // Timer for periodic validation
    UPROPERTY()
    FTimerHandle ValidationTimerHandle;

public:
    // Main validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void AnalyzeSystemHealth();

    // Specific module tests
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestModuleLoad(const FString& ModuleName, const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestActorSpawn(const FString& ClassName, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestCrossSystemIntegration(const FString& System1, const FString& System2);

    // Integration test scenarios
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void TestWorldGenEnvironmentIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void TestCharacterGameStateIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void TestCrowdSimulationIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void TestQuestNarrativeIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SaveValidationReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_SystemHealth GetSystemHealth() const { return SystemHealth; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_IntegrationTest> GetIntegrationTests() const { return IntegrationTests; }

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, FBuild_SystemHealth, SystemHealth);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleValidated, FString, ModuleName, EBuild_ValidationResult, Result);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntegrationTestComplete, FString, TestName, EBuild_ValidationResult, Result);

    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnModuleValidated OnModuleValidated;

    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnIntegrationTestComplete OnIntegrationTestComplete;

private:
    // Internal helper functions
    void InitializeValidationSystem();
    void SetupValidationTimer();
    void LogValidationResults();
    void UpdateSystemHealthMetrics();
    EBuild_HealthStatus CalculateOverallHealth();
    void GenerateRecommendations();
    
    // Test actor management
    UPROPERTY()
    TArray<AActor*> TestActors;
    
    void CleanupTestActors();
    void CreateIntegrationTestMarkers();
};