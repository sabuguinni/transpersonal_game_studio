#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "IntegrationValidationManager.generated.h"

/**
 * Integration Validation Manager
 * Agent #19 - Integration & Build Agent
 * 
 * Validates integration between all game systems and ensures
 * cross-agent compatibility. Monitors build health and system dependencies.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasDependencyErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float HealthScore;

    FInteg_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        bHasDependencyErrors = false;
        HealthScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_AgentOutput
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CycleNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ValidationNotes;

    FInteg_AgentOutput()
    {
        AgentName = TEXT("");
        CycleNumber = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bValidationPassed = false;
        ValidationNotes = TEXT("");
    }
};

UENUM(BlueprintType)
enum class EInteg_ValidationLevel : uint8
{
    Basic       UMETA(DisplayName = "Basic"),
    Standard    UMETA(DisplayName = "Standard"),
    Comprehensive UMETA(DisplayName = "Comprehensive"),
    Critical    UMETA(DisplayName = "Critical")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationValidationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_SystemStatus ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FInteg_SystemStatus> GetSystemStatusList();

    // Agent output validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAgentOutput(const FString& AgentName, int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FInteg_AgentOutput> GetAgentOutputHistory();

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallHealthScore();

    // Cross-system dependency checks
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetMissingDependencies(const FString& SystemName);

    // Level validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TMap<FString, int32> GetActorInventory();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetCurrentFrameRate();

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetCompilationErrors();

protected:
    // System status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FInteg_SystemStatus> SystemStatusList;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FInteg_AgentOutput> AgentOutputHistory;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EInteg_ValidationLevel ValidationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float MinHealthScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnLevelLoad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bLogValidationDetails;

private:
    // Internal validation methods
    bool ValidateCoreClasses();
    bool ValidateGameplayFramework();
    bool ValidateAudioSystems();
    bool ValidateVFXSystems();
    bool ValidateAISystems();
    bool ValidateWorldGeneration();
    
    // Helper methods
    void LogValidationResult(const FString& SystemName, bool bPassed, const FString& Details = TEXT(""));
    void UpdateSystemStatus(const FString& SystemName, bool bLoaded, bool bCompiled, float HealthScore);
    void RecordAgentOutput(const FString& AgentName, int32 CycleNumber, int32 FilesCreated, int32 CommandsExecuted, bool bPassed);
};

#include "IntegrationValidationManager.generated.h"