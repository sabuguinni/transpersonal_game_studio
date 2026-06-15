#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Info        UMETA(DisplayName = "Info")
};

USTRUCT(BlueprintType)
struct FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Info;
        Description = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bIsFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 ComponentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FString> Issues;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsFunctional = false;
        ComponentCount = 0;
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
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateAudioSystems();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationTest> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_SystemStatus> GetSystemStatuses() const { return SystemStatuses; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetOverallScore() const { return OverallScore; }

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void AddValidationTest(const FString& TestName, EQA_ValidationResult Result, const FString& Description, float ExecutionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationResults();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy(const FString& SystemName) const;

protected:
    // Internal validation helpers
    void ValidateClassLoading();
    void ValidateActorCounts();
    void ValidateEssentialActors();
    void ValidateComponentIntegrity();
    void CalculateOverallScore();

    // Test specific systems
    bool TestTranspersonalCharacter();
    bool TestWorldGeneration();
    bool TestVFXIntegration();
    bool TestAudioIntegration();
    bool TestPerformanceThresholds();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_ValidationTest> ValidationResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_SystemStatus> SystemStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float OverallScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxLightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance", meta = (AllowPrivateAccess = "true"))
    float MinFrameRate;

    // System names to validate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Systems", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CoreSystemNames;
};