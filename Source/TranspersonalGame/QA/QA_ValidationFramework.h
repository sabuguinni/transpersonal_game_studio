#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Stable      UMETA(DisplayName = "Stable"),
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQA_TestType : uint8
{
    Bridge          UMETA(DisplayName = "Bridge Connectivity"),
    VFX             UMETA(DisplayName = "VFX System"),
    Character       UMETA(DisplayName = "Character System"),
    Physics         UMETA(DisplayName = "Physics System"),
    Assets          UMETA(DisplayName = "Asset Pipeline"),
    Performance     UMETA(DisplayName = "Performance"),
    BiomePopulation UMETA(DisplayName = "Biome Population"),
    Integration     UMETA(DisplayName = "System Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_TestType TestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ResultMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQA_TestResult()
    {
        TestType = EQA_TestType::Bridge;
        Status = EQA_SystemStatus::Unknown;
        TestName = TEXT("");
        ResultMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_BiomeValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 RequiredActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bMeetsRequirement;

    FQA_BiomeValidation()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        RequiredActorCount = 500;
        bMeetsRequirement = false;
    }
};

/**
 * QA Validation Framework Component
 * Provides comprehensive testing and validation for all game systems
 * Monitors system health, performance, and integration status
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateBridgeConnectivity();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_BiomeValidation> ValidateBiomePopulation();

    // Comprehensive test suite
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestResult> RunFullValidationSuite();

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    EQA_SystemStatus GetOverallSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetSystemHealthPercentage();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetCurrentActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsPerformanceWithinThreshold();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogTestResult(const FQA_TestResult& TestResult);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateValidationReport();

protected:
    // Test results storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    TArray<FQA_BiomeValidation> BiomeValidations;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 WarningActorThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogDetailedResults;

private:
    // Internal state
    float LastValidationTime;
    EQA_SystemStatus CachedSystemHealth;
    
    // Helper functions
    FQA_TestResult CreateTestResult(EQA_TestType TestType, const FString& TestName, EQA_SystemStatus Status, const FString& Message, float ExecutionTime = 0.0f);
    EQA_SystemStatus CalculateSystemHealth(const TArray<FQA_TestResult>& Results);
};