#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "QAValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float HealthScore;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        LastError = TEXT("");
        HealthScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 RenderComponents;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPerformanceAcceptable;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        RenderComponents = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        bPerformanceAcceptable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQAValidationManager : public AActor
{
    GENERATED_BODY()

public:
    AQAValidationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    UStaticMeshComponent* ValidationMarkerMesh;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bContinuousValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogValidationResults;

private:
    float LastValidationTime;
    int32 ValidationCycleCount;

public:
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetOverallHealthScore() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemOperational(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA")
    void EditorValidation();

private:
    void ValidateTranspersonalCharacter();
    void ValidateGameModeAndState();
    void ValidateWorldGeneration();
    void ValidateAISystems();
    void UpdateSystemStatus(const FString& SystemName, bool bOperational, const FString& ErrorMessage = TEXT(""));
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};