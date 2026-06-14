#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationReport()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateActorIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupOrphanedActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorLimits();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationReport> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bSystemHealthy;

private:
    void AddValidationResult(const FString& TestName, EBuild_ValidationResult Result, const FString& Details, float ExecutionTime);
    void ValidateClassRegistration();
    void ValidateComponentIntegrity();
    void ValidateMemoryUsage();
    void ValidateFPSPerformance();
    void ValidateModuleDependencies();
    void CountActorsByType();
    void EnforceDinosaurLimit();
    void EnforceTotalActorLimit();
};