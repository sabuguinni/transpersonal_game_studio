#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationState : uint8
{
    Initializing        UMETA(DisplayName = "Initializing"),
    ValidatingCore      UMETA(DisplayName = "Validating Core"),
    ValidatingIntegration UMETA(DisplayName = "Validating Integration"),
    Complete           UMETA(DisplayName = "Complete"),
    Failed             UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ValidationMessage = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        ValidationTime = FDateTime::MinValue();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationComplete, bool, bHasCriticalFailures);

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === INTEGRATION CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegrationValidation();
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CompleteIntegrationValidation();
    
    // === VALIDATION STATUS ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    EBuild_IntegrationState IntegrationState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bIsIntegrationComplete;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    bool bHasCriticalFailures;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 TotalSystemsToValidate;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 ValidatedSystems;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 CriticalErrorCount;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    int32 WarningCount;
    
    // === VALIDATION RESULTS ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FBuild_SystemValidationResult> ValidationResults;
    
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FString FinalIntegrationReport;
    
    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxCriticalErrors = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxWarnings = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationTimeoutSeconds = 300.0f;
    
    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnIntegrationComplete OnIntegrationComplete;
    
    // === QUERY FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration Query")
    FString GetIntegrationStatusString() const;
    
    UFUNCTION(BlueprintCallable, Category = "Integration Query")
    float GetValidationProgress() const;
    
    UFUNCTION(BlueprintCallable, Category = "Integration Query")
    bool IsSystemValidated(const FString& SystemName) const;
    
    UFUNCTION(BlueprintCallable, Category = "Integration Query")
    TArray<FString> GetFailedSystems() const;

protected:
    // === INTERNAL VALIDATION ===
    
    void IdentifySystemsToValidate();
    void ValidateCoreGameSystems();
    void ValidateIndividualSystem(const FString& SystemName);
    void ValidateActorSystem(UClass* ActorClass, FBuild_SystemValidationResult& Result);
    void ValidateComponentSystem(UClass* ComponentClass, FBuild_SystemValidationResult& Result);
    void ValidateSubsystemSystem(UClass* SubsystemClass, FBuild_SystemValidationResult& Result);
    
    void UpdateIntegrationProcess();
    void GenerateFinalIntegrationReport();
    void CheckValidationTimeout();
    
    // === INTERNAL DATA ===
    
    UPROPERTY()
    TArray<FString> SystemsToValidate;
    
    UPROPERTY()
    TMap<FString, bool> SystemValidationStatus;
    
    FDateTime IntegrationStartTime;
    FDateTime LastValidationTime;
};