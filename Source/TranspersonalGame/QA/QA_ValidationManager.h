#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
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
class TRANSPERSONALGAME_API UQA_ValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_ValidationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ValidateActorDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ValidateGameplaySystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ValidateVFXSystems();

    // Report management
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ExportValidationReport(const FString& FilePath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float LastValidationTime;

private:
    void AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime);
    void ValidateBiomeDistribution();
    void ValidateAssetLoading();
    void ValidateClassCompilation();
    bool IsActorInBiome(AActor* Actor, const FVector& BiomeCenter, float Radius = 20000.0f);
};