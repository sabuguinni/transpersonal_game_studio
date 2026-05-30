#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateEnvironmentSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport();

    // Biome distribution validation
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateBiomeDistribution();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetMemoryUsage();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCountWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCountCritical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MinFPSWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MinFPSCritical;

private:
    float LastValidationTime;
    
    // Helper functions
    FQA_ValidationReport CreateReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime);
    void LogValidationResult(const FQA_ValidationReport& Report);
    bool ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
    bool ValidateAssetLoading(const FString& AssetPath);
    int32 CountActorsOfType(const FString& ActorType);
    void CheckBiomeActorDistribution();
};

#include "QA_ValidationManager.generated.h"