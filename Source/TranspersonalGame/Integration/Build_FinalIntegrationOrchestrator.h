#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed"),
    Validated       UMETA(DisplayName = "Validated")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsOperational;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float PerformanceScore;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsOperational = false;
        ValidationMessage = TEXT("");
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalSystemsCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 LoadedSystemsCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 OperationalSystemsCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 WorldActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallPerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float IntegrationCompleteness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastValidationTime;

    FBuild_IntegrationMetrics()
    {
        TotalSystemsCount = 0;
        LoadedSystemsCount = 0;
        OperationalSystemsCount = 0;
        WorldActorCount = 0;
        OverallPerformanceScore = 0.0f;
        IntegrationCompleteness = 0.0f;
        LastValidationTime = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Manages complete build integration and system validation
 * Responsible for coordinating all game systems and ensuring build readiness
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* IntegrationVisualizerMesh;

    // Integration Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus CurrentIntegrationStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FBuild_IntegrationMetrics IntegrationMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemValidationResults;

    // Critical System Classes to Validate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalSystemClasses;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxWorldActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinPerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinIntegrationCompleteness;

    // Validation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bContinuousValidation;

private:
    // Internal state
    float LastValidationTime;
    bool bValidationInProgress;

public:
    // Main Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartIntegrationOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemByClass(const FString& SystemClassName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // System Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateGameModeSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateWorldGenerationSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateFoliageSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateQASystem();

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateSystemPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateIntegrationCompleteness();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinThresholds();

    // Build Readiness
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool IsBuildReady();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void PrepareBuildPackage();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void FinalizeIntegration();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ResetIntegrationState();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LogIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetIntegrationStatusString();

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationStatusChanged, EBuild_IntegrationStatus, NewStatus);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnIntegrationStatusChanged OnIntegrationStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemValidationComplete, const FBuild_SystemValidationResult&, ValidationResult);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSystemValidationComplete OnSystemValidationComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntegrationComplete);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnIntegrationComplete OnIntegrationComplete;

protected:
    // Internal helper functions
    void InitializeCriticalSystemsList();
    void SetupIntegrationVisualizer();
    void UpdateVisualizerAppearance();
    FBuild_SystemValidationResult CreateValidationResult(const FString& SystemName, bool bLoaded, bool bOperational, const FString& Message, float Score);
};