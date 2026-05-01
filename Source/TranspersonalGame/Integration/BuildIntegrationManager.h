#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EInteg_BuildStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Compiling   UMETA(DisplayName = "Compiling"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class EInteg_ModuleType : uint8
{
    Core        UMETA(DisplayName = "Core"),
    WorldGen    UMETA(DisplayName = "World Generation"),
    Character   UMETA(DisplayName = "Character"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "VFX"),
    QA          UMETA(DisplayName = "QA"),
    AI          UMETA(DisplayName = "AI"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FInteg_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EInteg_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EInteg_BuildStatus BuildStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float LastBuildTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ClassCount;

    FInteg_ModuleStatus()
    {
        ModuleType = EInteg_ModuleType::Core;
        BuildStatus = EInteg_BuildStatus::Unknown;
        ModuleName = TEXT("");
        LastError = TEXT("");
        LastBuildTime = 0.0f;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FInteg_CrossModuleTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<EInteg_ModuleType> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bTestPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString TestResult;

    FInteg_CrossModuleTest()
    {
        TestName = TEXT("");
        bTestPassed = false;
        TestResult = TEXT("");
    }
};

/**
 * Build Integration Manager - Orchestrates compilation and cross-module testing
 * Ensures all agent outputs integrate correctly into a playable build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Module Status Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FInteg_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FInteg_CrossModuleTest> CrossModuleTests;

    // Build Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnTick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnableCrossModuleTesting;

    // Integration Status
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    EInteg_BuildStatus OverallBuildStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    int32 TotalModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    int32 SuccessfulModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    int32 FailedModules;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModule(EInteg_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunCrossModuleTests();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsModuleReady(EInteg_ModuleType ModuleType) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_ModuleStatus GetModuleStatus(EInteg_ModuleType ModuleType) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ResetIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Integration")
    void EditorValidateAllModules();

    UFUNCTION(CallInEditor, Category = "Integration")
    void EditorRunCrossModuleTests();

    UFUNCTION(CallInEditor, Category = "Integration")
    void EditorGenerateReport();

protected:
    // Internal Validation
    void ValidateCharacterModule();
    void ValidateWorldGenModule();
    void ValidateAudioModule();
    void ValidateVFXModule();
    void ValidateQAModule();
    void ValidateAIModule();
    void ValidateCombatModule();

    // Cross-Module Testing
    void TestCharacterWorldGenIntegration();
    void TestAudioVFXIntegration();
    void TestQASystemIntegration();
    void TestAICombatIntegration();

    // Utility Functions
    void UpdateModuleStatus(EInteg_ModuleType ModuleType, EInteg_BuildStatus Status, const FString& Error = TEXT(""));
    void LogIntegrationStatus();

private:
    UPROPERTY()
    USceneComponent* RootSceneComponent;

    float LastValidationTime;
    bool bValidationInProgress;
};