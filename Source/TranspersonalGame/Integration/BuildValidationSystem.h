#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    InProgress  UMETA(DisplayName = "In Progress")
};

UENUM(BlueprintType)
enum class EBuild_SystemType : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    World       UMETA(DisplayName = "World Generation"),
    Character   UMETA(DisplayName = "Character Systems"),
    AI          UMETA(DisplayName = "AI Systems"),
    Audio       UMETA(DisplayName = "Audio Systems"),
    VFX         UMETA(DisplayName = "VFX Systems"),
    UI          UMETA(DisplayName = "UI Systems"),
    Physics     UMETA(DisplayName = "Physics Systems"),
    Lighting    UMETA(DisplayName = "Lighting Systems"),
    Integration UMETA(DisplayName = "Integration Systems")
};

USTRUCT(BlueprintType)
struct FBuild_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EBuild_SystemType SystemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bCritical;

    FBuild_ValidationTest()
    {
        TestName = TEXT("Unknown Test");
        SystemType = EBuild_SystemType::Core;
        Result = EBuild_ValidationResult::Unknown;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bCritical = false;
    }
};

USTRUCT(BlueprintType)
struct FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    EBuild_SystemType SystemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    EBuild_ValidationResult OverallStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    TArray<FString> CriticalErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    float LastValidationTime;

    FBuild_SystemStatus()
    {
        SystemType = EBuild_SystemType::Core;
        OverallStatus = EBuild_ValidationResult::Unknown;
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        LastValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildValidationSystem : public AActor
{
    GENERATED_BODY()

public:
    ABuildValidationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bAutoValidationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bStopOnCriticalError;

    // Validation results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    TArray<FBuild_ValidationTest> ValidationTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    EBuild_ValidationResult OverallBuildStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    int32 TotalTestsRun;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    int32 TotalTestsPassed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    int32 TotalTestsFailed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    float LastFullValidationTime;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunSystemValidation(EBuild_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateSystemInteractions();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationResults();

    // Result query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    EBuild_ValidationResult GetSystemStatus(EBuild_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    TArray<FString> GetCriticalErrors() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    bool HasCriticalErrors() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Validation")
    float GetValidationProgress() const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ExportValidationResults(const FString& FilePath);

protected:
    // Internal validation methods
    void ValidateCoreClasses();
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateAudioSystems();
    void ValidateVFXSystems();
    void ValidatePhysicsSystems();
    void ValidateLightingSystems();

    // Helper methods
    void AddValidationTest(const FString& TestName, EBuild_SystemType SystemType, 
                          EBuild_ValidationResult Result, const FString& ErrorMessage = TEXT(""), 
                          bool bCritical = false);
    
    void UpdateSystemStatus(EBuild_SystemType SystemType);
    void CalculateOverallStatus();
    void LogValidationResult(const FBuild_ValidationTest& Test);

private:
    // Internal state
    float TimeSinceLastValidation;
    bool bValidationInProgress;
    int32 CurrentValidationStep;
    int32 TotalValidationSteps;

    // Root component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;
};