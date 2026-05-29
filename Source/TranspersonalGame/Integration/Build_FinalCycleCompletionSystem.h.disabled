#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Build_FinalCycleCompletionSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_CycleCompletionStatus : uint8
{
    Pending UMETA(DisplayName = "Pending"),
    InProgress UMETA(DisplayName = "In Progress"),
    ValidationPhase UMETA(DisplayName = "Validation Phase"),
    Complete UMETA(DisplayName = "Complete"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 CoreSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 VFXSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 DinosaurSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 CharacterSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 QASystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 IntegrationSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 EnvironmentSystemActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FString BuildStatus = TEXT("Unknown");

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FDateTime CompletionTimestamp;

    FBuild_CycleMetrics()
    {
        CompletionTimestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float HealthScore = 0.0f;
};

/**
 * Final Cycle Completion System - Orchestrates the completion of production cycles
 * and validates all integrated systems for build health and readiness.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalCycleCompletionSystem : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalCycleCompletionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    EBuild_CycleCompletionStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    FBuild_CycleMetrics CurrentCycleMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    TArray<FBuild_SystemValidationResult> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    bool bAllSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Completion")
    float OverallBuildHealth;

public:
    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void InitializeCycleCompletion(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void GenerateCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    FBuild_CycleMetrics GetCurrentCycleMetrics() const { return CurrentCycleMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    EBuild_CycleCompletionStatus GetCompletionStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    bool AreAllSystemsOperational() const { return bAllSystemsOperational; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    float GetOverallBuildHealth() const { return OverallBuildHealth; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Completion")
    TArray<FBuild_SystemValidationResult> GetSystemValidationResults() const { return SystemValidationResults; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Cycle Completion")
    void OnCycleInitialized(const FString& CycleID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cycle Completion")
    void OnSystemValidationComplete(const TArray<FBuild_SystemValidationResult>& Results);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cycle Completion")
    void OnCycleComplete(const FBuild_CycleMetrics& Metrics);

private:
    void ValidateSystemActors(const FString& SystemName, const TArray<FString>& Keywords, FBuild_SystemValidationResult& Result);
    void CalculateOverallHealth();
    void UpdateCompletionStatus();
    void LogCycleCompletion();
};