#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "BuildIntegrationReport.generated.h"

/**
 * Build Integration Report System
 * Tracks compilation status, system health, and cross-module dependencies
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ErrorMessage;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SkeletalMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float EstimatedFrameTime;

    FBuild_PerformanceMetrics()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        LightActors = 0;
        EstimatedFrameTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UBuildIntegrationReport();

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FBuild_PerformanceMetrics PerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime LastBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildVersion;

    UFUNCTION(BlueprintCallable, Category = "Build")
    void GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void CheckPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build")
    FString GetBuildSummary() const;

private:
    void CheckSystemStatus(const FString& SystemName, UClass* SystemClass);
    void ValidateCrossSystemDependencies();
};