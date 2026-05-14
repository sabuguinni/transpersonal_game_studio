#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_SystemIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime Timestamp;

    FQA_ValidationReport()
    {
        SystemName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

/**
 * QA System Integration Validator
 * Validates all game systems for proper integration and functionality
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_SystemIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_SystemIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateAssetIntegrity();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ExportValidationReport(const FString& FilePath);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetMemoryUsage() const;

protected:
    // Internal validation helpers
    FQA_ValidationReport ValidateSystemInternal(const FString& SystemName, TFunction<bool()> ValidationFunction);
    
    void LogValidationResult(const FQA_ValidationReport& Report);
    
    bool CheckClassExists(const FString& ClassName);
    
    bool CheckAssetExists(const FString& AssetPath);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bLogValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bExportReportsAutomatically;

    // Performance tracking
    float LastValidationTime;
    int32 FrameCounter;
    float FPSAccumulator;
};