#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Critical    UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StaticMeshCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SkeletalMeshCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    FBuild_PerformanceMetrics()
    {
        MemoryUsagePercent = 0.0f;
        CPUUsagePercent = 0.0f;
        TotalActorCount = 0;
        StaticMeshCount = 0;
        SkeletalMeshCount = 0;
        FrameRate = 0.0f;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Manages final build integration, system validation, and performance monitoring
 * Ensures all agent outputs are properly integrated into a cohesive game build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsIntegrationComplete();

    // System-specific validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateVFXSystems();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimal();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus CurrentIntegrationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FBuild_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceMonitoringActive;

    // Internal validation helpers
    FBuild_SystemValidationResult ValidateSystemInternal(const FString& SystemName, TFunction<bool()> ValidationFunction);
    void UpdatePerformanceMetrics();
    void LogIntegrationStatus(const FString& Message, EBuild_IntegrationStatus Status);

private:
    FTimerHandle PerformanceMonitoringTimer;
    float IntegrationStartTime;
};

/**
 * Integration Component - Attachable to key actors for validation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_IntegrationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationComponent();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterForIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetIntegrationStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bRegisteredForIntegration;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus ComponentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationError;
};

#include "Build_FinalIntegrationOrchestrator.generated.h"