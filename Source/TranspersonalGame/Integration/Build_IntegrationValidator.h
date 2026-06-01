#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Critical    UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ActorsAffected;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        ActorsAffected = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> RequiredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    bool bHasErrors;

    FBuild_ModuleDependency()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
    }
};

/**
 * Integration Validator - Validates build integrity and system integration
 * Ensures all modules work together correctly and compilation is successful
 */
UCLASS(BlueprintType, Blueprintable)
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
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationResult> RunComprehensiveTests();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAssetReferences();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool GenerateIntegrationReport();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ValidationResult> GetLastValidationResults() const { return LastValidationResults; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ModuleDependency> GetModuleDependencies() const { return ModuleDependencies; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_ValidationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    int32 GetTotalActorCount() const { return TotalActorCount; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    int32 GetValidActorCount() const { return ValidActorCount; }

protected:
    // Validation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ValidationResult> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ModuleDependency> ModuleDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_ValidationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 ValidActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 ErrorActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidation;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

private:
    // Internal validation methods
    FBuild_ValidationResult ValidateWorldState();
    FBuild_ValidationResult ValidateGameMode();
    FBuild_ValidationResult ValidatePlayerCharacter();
    FBuild_ValidationResult ValidateDinosaurActors();
    FBuild_ValidationResult ValidateEnvironmentActors();
    FBuild_ValidationResult ValidatePhysicsSystem();
    FBuild_ValidationResult ValidateAudioSystem();
    FBuild_ValidationResult ValidateVFXSystem();

    // Helper methods
    void LogValidationResult(const FBuild_ValidationResult& Result);
    void UpdateOverallStatus();
    bool CheckModuleLoaded(const FString& ModuleName);
    void CountActorsByType();

    // Timers
    float ValidationTimer;
    bool bValidationInProgress;
};