#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Validating      UMETA(DisplayName = "Validating"),
    Passed          UMETA(DisplayName = "Passed"),
    Failed          UMETA(DisplayName = "Failed"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    float ValidationTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        ErrorCount = 0;
        LastError = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bTestPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString TestResult;

    FBuild_IntegrationTest()
    {
        TestName = TEXT("");
        bTestPassed = false;
        TestResult = TEXT("");
    }
};

/**
 * Build Validation System - Integration Agent #19
 * Validates compilation, module dependencies, and cross-system integration
 */
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

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StartFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    EBuild_ValidationStatus GetOverallStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_IntegrationTest> GetIntegrationResults() const;

    // Build system integration
    UFUNCTION(BlueprintCallable, Category = "Build System")
    void CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void ValidateBinaryFiles();

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void TestClassLoading();

    // Cross-module dependency validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAudioVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateWorldCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAICombatIntegration();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    TArray<FBuild_IntegrationTest> IntegrationTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    EBuild_ValidationStatus OverallStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    bool bAutoValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    int32 TotalErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Validation")
    int32 TotalWarnings;

private:
    float LastValidationTime;
    bool bValidationInProgress;

    // Internal validation methods
    void InitializeModuleList();
    void UpdateValidationStatus();
    FBuild_ModuleStatus ValidateModuleInternal(const FString& ModuleName);
    FBuild_IntegrationTest RunIntegrationTestInternal(const FString& TestName, const TArray<FString>& RequiredModules);
};