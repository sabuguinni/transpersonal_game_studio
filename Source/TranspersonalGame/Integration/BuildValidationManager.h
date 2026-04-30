#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BuildValidationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown,
    Validating,
    Passed,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 CharacterActors;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bMinPlayableMapLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bCoreClassesLoaded;

    FBuild_SystemHealth()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        bMinPlayableMapLoaded = false;
        bCoreClassesLoaded = false;
    }
};

/**
 * Build Validation Manager - Validates system integrity and build health
 * Runs automated tests to ensure all systems are functional
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunQuickValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_SystemHealth GetSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationResult> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    void ValidateCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    void ValidateCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor = true)
    void GenerateHealthReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FBuild_SystemHealth CurrentSystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    // Validation functions
    void ValidateActorCounts();
    void ValidateCharacterSystem();
    void ValidateDinosaurSystem();
    void ValidateEnvironmentSystem();
    void ValidateGameModeSystem();
    void ValidatePhysicsSystem();

    // Helper functions
    void AddValidationResult(const FString& TestName, EBuild_ValidationStatus Status, const FString& Message, float ExecutionTime = 0.0f);
    void ClearValidationResults();
    void UpdateSystemHealth();

private:
    float ValidationStartTime;
};