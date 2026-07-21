#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalBuildReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    FString LastValidation;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        ActorCount = 0;
        LastValidation = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    FString BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    FString CycleID;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        bBuildSuccessful = false;
        BuildTimestamp = TEXT("Unknown");
        CycleID = TEXT("Unknown");
    }
};

/**
 * Final Build Report Component - Integration Agent #19
 * Provides comprehensive build status and system integration validation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_FinalBuildReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalBuildReport();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    TArray<FBuild_IntegrationReport> BuildHistory;

    UFUNCTION(BlueprintCallable, Category = "Build Report")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Report")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Report")
    FBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Report")
    void LogBuildStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Report")
    bool IsGamePlayable();

private:
    void ValidateCharacterSystem(FBuild_SystemStatus& Status);
    void ValidateDinosaurSystem(FBuild_SystemStatus& Status);
    void ValidateEnvironmentSystem(FBuild_SystemStatus& Status);
    void ValidateQAFramework(FBuild_SystemStatus& Status);
    void ValidateWorldGeneration(FBuild_SystemStatus& Status);
    void ValidatePhysicsSystem(FBuild_SystemStatus& Status);
    void ValidateAudioSystem(FBuild_SystemStatus& Status);

    float LastValidationTime;
    bool bValidationInProgress;
};