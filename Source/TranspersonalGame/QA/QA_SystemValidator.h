#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QA_SystemValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_SystemStatus : uint8
{
    Unknown,
    Healthy,
    Warning,
    Critical,
    Offline
};

USTRUCT(BlueprintType)
struct FQA_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    EQA_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString StatusMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    float LastCheckTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FString> RecentErrors;

    FQA_SystemReport()
    {
        SystemName = TEXT("");
        Status = EQA_SystemStatus::Unknown;
        StatusMessage = TEXT("");
        LastCheckTime = 0.0f;
        ErrorCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQA_SystemValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_SystemValidator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FQA_SystemReport> SystemReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bAutoValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bLogValidationResults;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemReport GetSystemReport(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_SystemReport> GetCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ClearValidationHistory();

protected:
    FTimerHandle ValidationTimerHandle;

    void PerformScheduledValidation();
    void UpdateSystemReport(const FString& SystemName, EQA_SystemStatus Status, const FString& Message);
    void LogSystemError(const FString& SystemName, const FString& ErrorMessage);
    bool ValidateClassExists(const FString& ClassName);
    bool ValidateSubsystemExists(const FString& SubsystemName);
    int32 CountActorsOfType(const FString& ActorType);
};