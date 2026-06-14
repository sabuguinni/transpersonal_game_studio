#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_QualityAssurance.generated.h"

UENUM(BlueprintType)
enum class EBuild_QualityLevel : uint8
{
    Critical    UMETA(DisplayName = "Critical - Build Broken"),
    Poor        UMETA(DisplayName = "Poor - Major Issues"),
    Fair        UMETA(DisplayName = "Fair - Some Issues"),
    Good        UMETA(DisplayName = "Good - Minor Issues"),
    Excellent   UMETA(DisplayName = "Excellent - Production Ready")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    bool bIsFunctional;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    FString ErrorMessage;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        bIsFunctional = false;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QualityReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    EBuild_QualityLevel QualityLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    float QualityScore;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    int32 SystemsReady;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    int32 SystemsTotal;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    FString BuildTimestamp;

    FBuild_QualityReport()
    {
        QualityLevel = EBuild_QualityLevel::Critical;
        QualityScore = 0.0f;
        TotalActors = 0;
        SystemsReady = 0;
        SystemsTotal = 0;
        BuildTimestamp = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_QualityAssurance : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_QualityAssurance();

    UFUNCTION(BlueprintCallable, Category = "Build Quality")
    FBuild_QualityReport GenerateQualityReport();

    UFUNCTION(BlueprintCallable, Category = "Build Quality")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Quality")
    bool CheckPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build Quality")
    TArray<FBuild_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Quality")
    EBuild_QualityLevel CalculateQualityLevel(float QualityScore);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    TArray<FString> RequiredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    FBuild_QualityReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    float MinimumQualityThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Build Quality")
    int32 MaxActorCount;

private:
    bool ValidateSystem(const FString& SystemName);
    float CalculateSystemScore();
    void LogQualityReport(const FBuild_QualityReport& Report);
};