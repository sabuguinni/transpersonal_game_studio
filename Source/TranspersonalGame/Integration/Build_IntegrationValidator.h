#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Validating,
    Success,
    Warning,
    Failed
};

USTRUCT(BlueprintType)
struct FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        ErrorMessage = TEXT("");
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealth> SystemHealthChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CustomClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLevelPlayable;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ReportTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        CustomClassCount = 0;
        OverallHealthScore = 0.0f;
        bIsLevelPlayable = false;
        ReportTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_IntegrationReport ValidateCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateSystemHealth(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckCrossSystemCompatibility();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> SystemsToValidate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnPlay;

private:
    float LastValidationTime;
    bool bValidationInProgress;

    FBuild_SystemHealth ValidateCharacterSystem();
    FBuild_SystemHealth ValidateWorldGenSystem();
    FBuild_SystemHealth ValidateFoliageSystem();
    FBuild_SystemHealth ValidateCrowdSimSystem();
    FBuild_SystemHealth ValidateQAFramework();

    void LogIntegrationResults(const FBuild_IntegrationReport& Report);
    bool CheckActorCountLimits();
    bool CheckPerformanceMetrics();
};