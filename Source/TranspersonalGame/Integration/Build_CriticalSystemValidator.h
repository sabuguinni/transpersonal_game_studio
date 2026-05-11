#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CriticalSystemValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CriticalSystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Critical System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Critical System")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Critical System")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Critical System")
    float LastCheckTime;

    UPROPERTY(BlueprintReadOnly, Category = "Critical System")
    int32 FailureCount;

    FBuild_CriticalSystemStatus()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        LastError = TEXT("");
        LastCheckTime = 0.0f;
        FailureCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health Report")
    TArray<FBuild_CriticalSystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Health Report")
    bool bAllSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Health Report")
    int32 TotalFailures;

    UPROPERTY(BlueprintReadOnly, Category = "Health Report")
    FString GeneratedAt;

    UPROPERTY(BlueprintReadOnly, Category = "Health Report")
    float ValidationDuration;

    FBuild_SystemHealthReport()
    {
        bAllSystemsOperational = false;
        TotalFailures = 0;
        GeneratedAt = TEXT("");
        ValidationDuration = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UBuild_CriticalSystemValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    FBuild_SystemHealthReport ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    bool ValidateUE5Bridge();

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    bool ValidateCompilationSystem();

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    bool ValidateMemorySystem();

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    bool ValidateFileSystem();

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    void LogCriticalFailure(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    void ResetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Critical System Validator")
    TArray<FString> GetFailedSystems();

protected:
    UPROPERTY()
    TMap<FString, FBuild_CriticalSystemStatus> SystemStatusMap;

    UPROPERTY()
    float LastFullValidation;

    UPROPERTY()
    int32 ValidationCounter;

    void InitializeCriticalSystems();
    void UpdateSystemStatus(const FString& SystemName, bool bOperational, const FString& ErrorMessage = TEXT(""));
    bool CheckUE5BridgeConnectivity();
    bool CheckCompilationEnvironment();
    bool CheckMemoryAvailability();
    bool CheckFileSystemAccess();
    FString GetCurrentTimestamp();
};