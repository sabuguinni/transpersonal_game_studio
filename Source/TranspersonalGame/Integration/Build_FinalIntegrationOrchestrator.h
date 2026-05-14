#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

// Forward declarations
class UVFXParticleSystemManager;
class UQAVFXIntegrationValidator;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    Initialization      UMETA(DisplayName = "Initialization"),
    SystemValidation    UMETA(DisplayName = "System Validation"),
    CrossSystemTesting  UMETA(DisplayName = "Cross System Testing"),
    PerformanceCheck    UMETA(DisplayName = "Performance Check"),
    FinalValidation     UMETA(DisplayName = "Final Validation"),
    BuildComplete       UMETA(DisplayName = "Build Complete"),
    BuildFailed         UMETA(DisplayName = "Build Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bSystemLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilationSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bRuntimeStable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime = 0.0f;

    FBuild_SystemValidationResult()
    {
        bSystemLoaded = false;
        bCompilationSuccess = false;
        bRuntimeStable = false;
        SystemName = TEXT("Unknown");
        ValidationMessage = TEXT("Not Validated");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveVFXCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceAcceptable = false;

    FBuild_PerformanceMetrics()
    {
        TotalActorCount = 0;
        ActiveVFXCount = 0;
        CharacterCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        bPerformanceAcceptable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationPhase CurrentPhase = EBuild_IntegrationPhase::Initialization;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_PerformanceMetrics PerformanceData;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalIntegrationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Warnings;

    FBuild_IntegrationReport()
    {
        CurrentPhase = EBuild_IntegrationPhase::Initialization;
        bBuildSuccessful = false;
        BuildVersion = TEXT("1.0.0");
        BuildTimestamp = FDateTime::Now();
        TotalIntegrationTime = 0.0f;
    }
};

/**
 * Final Integration Orchestrator - Coordinates all systems for final build validation
 * Manages the complete integration pipeline from system validation to final deployment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AGameStateBase
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunCrossSystemTests();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsBuildReady() const;

    // System Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemValidationResult ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemValidationResult ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemValidationResult ValidateGameStateSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemValidationResult ValidateQASystem();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FBuild_PerformanceMetrics GatherPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable(const FBuild_PerformanceMetrics& Metrics) const;

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestVFXCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestSystemCommunication();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestMemoryManagement();

    // Reporting Functions
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void ExportBuildReport();

    // Cleanup and Finalization
    UFUNCTION(BlueprintCallable, Category = "Cleanup")
    void CleanupTestActors();

    UFUNCTION(BlueprintCallable, Category = "Cleanup")
    void FinalizeIntegration();

protected:
    // Integration State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBuild_IntegrationPhase CurrentIntegrationPhase;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FBuild_IntegrationReport IntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIntegrationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float IntegrationStartTime;

    // System References
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TArray<AActor*> TestActors;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TArray<UActorComponent*> SystemComponents;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxIntegrationTime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxAcceptableActorCount = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MinAcceptableFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxAcceptableMemoryMB = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoStartIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bVerboseLogging = true;

private:
    // Internal Helper Functions
    void InitializeIntegration();
    void AdvanceIntegrationPhase();
    void HandleIntegrationFailure(const FString& ErrorMessage);
    void HandleIntegrationSuccess();
    
    FBuild_SystemValidationResult CreateValidationResult(
        const FString& SystemName, 
        bool bLoaded, 
        bool bCompiled, 
        bool bStable, 
        const FString& Message = TEXT("")
    );

    // Timers and Delegates
    FTimerHandle IntegrationTimerHandle;
    FTimerHandle PerformanceMonitorHandle;
};