#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown,
    Passed,
    Failed,
    Warning,
    Critical
};

USTRUCT(BlueprintType)
struct FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status = EBuild_ValidationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime = 0.0f;

    FBuild_ValidationResult()
    {
        Status = EBuild_ValidationStatus::Unknown;
        TestName = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_ActorInventory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 EnvironmentActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LightingActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CustomActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCapExceeded = false;

    FBuild_ActorInventory()
    {
        TotalActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        LightingActors = 0;
        CustomActors = 0;
        bCapExceeded = false;
    }
};

/**
 * Integration Validator - Ensures all agent outputs integrate correctly
 * Validates compilation, actor limits, contamination, and cross-system compatibility
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationResult> RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateActorCap();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateContamination();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateCrossSystemIntegration();

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ActorInventory GetActorInventory();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool EnforceActorCap(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 CleanupContaminatedActors();

    // Build status
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetBuildStatusReport();

    // Editor-only validation
    UFUNCTION(CallInEditor, Category = "Integration")
    void RunEditorValidation();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationResult> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_ActorInventory CurrentInventory;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationInProgress = false;

private:
    // Internal validation helpers
    bool ValidateSourceFileStructure();
    bool ValidateBinaryFiles();
    bool ValidateModuleDependencies();
    TArray<AActor*> FindContaminatedActors();
    TArray<UClass*> GetCoreGameClasses();
    bool TestCrossSystemCompatibility();

    // Contamination keywords
    TArray<FString> ContaminationKeywords;
    
    // Essential actor types to preserve during cleanup
    TArray<FString> EssentialActorTypes;
    TArray<FString> DinosaurActorTypes;
};