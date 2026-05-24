#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_EmergencyValidation.generated.h"

/**
 * Emergency validation system for critical QA scenarios
 * Activated when UE5 bridge failures are detected
 */
UENUM(BlueprintType)
enum class EQA_ValidationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Passed          UMETA(DisplayName = "Passed"),
    Failed          UMETA(DisplayName = "Failed"),
    Critical        UMETA(DisplayName = "Critical Failure")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_EmergencyReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    EQA_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    int32 CycleID;

    FQA_EmergencyReport()
    {
        TestName = TEXT("");
        Status = EQA_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        Timestamp = FDateTime::Now();
        CycleID = 0;
    }
};

/**
 * Emergency validation component for critical system monitoring
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_EmergencyValidation : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_EmergencyValidation();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Emergency validation functions
    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    bool ValidateBridgeConnectivity();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    void GenerateEmergencyReport();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    void LogCriticalIncident(const FString& IncidentDescription);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    TArray<FQA_EmergencyReport> EmergencyReports;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    bool bBridgeConnected;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    bool bModulesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    bool bActorSpawningWorking;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    bool bVFXSystemsOperational;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency QA")
    bool bAutoValidateOnTick;

private:
    float LastValidationTime;
    int32 FailureCount;
    bool bCriticalFailureDetected;

    // Internal validation helpers
    bool TestBasicFunctionality();
    bool TestMemoryUsage();
    bool TestPerformanceMetrics();
    void HandleCriticalFailure(const FString& FailureReason);
};

/**
 * Emergency validation actor for standalone testing
 */
UCLASS()
class TRANSPERSONALGAME_API AQA_EmergencyValidator : public AActor
{
    GENERATED_BODY()

public:
    AQA_EmergencyValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Emergency QA")
    void ReportToQASystem();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UQA_EmergencyValidation* ValidationComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    bool bValidationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency QA")
    int32 TestsFailed;
};