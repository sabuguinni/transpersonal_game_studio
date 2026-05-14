#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending UMETA(DisplayName = "Pending"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed"),
    Validated UMETA(DisplayName = "Validated")
};

USTRUCT(BlueprintType)
struct FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float PerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 ActorCount;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ValidationMessage = TEXT("");
        PerformanceScore = 0.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float OverallScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float MemoryUsageMB;

    FBuild_IntegrationReport()
    {
        Status = EBuild_IntegrationStatus::Pending;
        OverallScore = 0.0f;
        BuildVersion = TEXT("1.0.0");
        CompletionTime = FDateTime::Now();
        TotalActorCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationOrchestratorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationOrchestratorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float MinimumPerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxAllowedActors;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsIntegrationComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallHealthScore() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupRedundantActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CheckSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformStressTest();

private:
    float LastValidationTime;
    bool bValidationInProgress;
    int32 ValidationStepIndex;

    void ValidateCharacterSystem();
    void ValidateVFXSystem();
    void ValidateQASystem();
    void ValidateWorldGeneration();
    void ValidateLightingSystem();
    void ValidateAudioSystem();
    void ValidatePhysicsSystem();
    void CalculatePerformanceMetrics();
    void UpdateIntegrationStatus();
    void LogValidationResults();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBuild_IntegrationOrchestratorComponent* IntegrationComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
    bool bEnableContinuousValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
    bool bAutoOptimizeOnCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
    FString BuildTargetVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
    bool bGenerateDetailedLogs;

    UFUNCTION(BlueprintCallable, Category = "Build")
    void InitializeBuildProcess();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void FinalizeBuild();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ExportBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ResetIntegrationState();

    UFUNCTION(BlueprintImplementableEvent, Category = "Build")
    void OnIntegrationComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Build")
    void OnValidationFailed(const FString& ErrorMessage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Build")
    void OnPerformanceOptimized();
};