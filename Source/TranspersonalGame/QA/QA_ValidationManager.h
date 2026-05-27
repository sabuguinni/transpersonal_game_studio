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
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_ValidationResult::Pass;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 VFXCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PopulatedBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString OverallGrade;

    FQA_SystemReport()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        VFXCount = 0;
        PopulatedBiomes = 0;
        OverallGrade = TEXT("Not Tested");
    }
};

/**
 * QA Validation Manager - Comprehensive system testing and validation
 * Validates all game systems, performance metrics, and integration points
 */
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
    FQA_SystemReport RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateIntegrationPoints();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 CountActorsInBiome(FVector BiomeCenter, float Radius);

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<AActor*> GetActorsByKeyword(const FString& Keyword);

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MinBiomeActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationInterval;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    FQA_SystemReport LastReport;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    float LastValidationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bValidationInProgress;

private:
    // Internal validation helpers
    FQA_ValidationTest CreateTest(const FString& Name, const FString& Description);
    void LogTestResult(const FQA_ValidationTest& Test);
    EQA_ValidationResult DetermineOverallResult(const TArray<FQA_ValidationTest>& Tests);
};