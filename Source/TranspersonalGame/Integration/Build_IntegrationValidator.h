#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CustomActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LightActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS;

    FBuild_SystemMetrics()
    {
        TotalActors = 0;
        CustomActors = 0;
        DinosaurActors = 0;
        StaticMeshActors = 0;
        LightActors = 0;
        MemoryUsageMB = 0.0f;
        FrameTimeMS = 0.0f;
    }
};

/**
 * Integration validation component for build quality assurance
 * Validates cross-system compatibility and performance metrics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Validation Methods
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateCrossSystemCompatibility();

    // Reporting Methods
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemMetrics GetSystemMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateHealthReport() const;

protected:
    // Internal validation methods
    FBuild_ValidationReport ValidateClassLoading();
    FBuild_ValidationReport ValidateActorSpawning();
    FBuild_ValidationReport ValidateComponentIntegration();
    FBuild_ValidationReport ValidateMemoryUsage();
    FBuild_ValidationReport ValidateFrameRate();

    // Utility methods
    void CollectSystemMetrics();
    void LogValidationResult(const FBuild_ValidationReport& Report);
    bool CheckClassExists(const FString& ClassName);

private:
    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics", meta = (AllowPrivateAccess = "true"))
    FBuild_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCountWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeMS;

    // Internal state
    float LastValidationTime;
    bool bValidationInProgress;
    int32 ValidationRunCount;
};