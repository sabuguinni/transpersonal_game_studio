#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EInteg_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Functional  UMETA(DisplayName = "Functional"),
    Broken      UMETA(DisplayName = "Broken"),
    Missing     UMETA(DisplayName = "Missing")
};

USTRUCT(BlueprintType)
struct FInteg_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EInteg_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastTestTime;

    FInteg_SystemReport()
    {
        SystemName = TEXT("");
        Status = EInteg_SystemStatus::Unknown;
        ErrorMessage = TEXT("");
        LastTestTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FInteg_BuildStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FunctionalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 BrokenSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FInteg_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float BuildTime;

    FInteg_BuildStatus()
    {
        bCompilationSuccessful = false;
        TotalSystems = 0;
        FunctionalSystems = 0;
        BrokenSystems = 0;
        BuildTime = 0.0f;
    }
};

/**
 * Build Integration Manager - Coordinates all game systems and validates integration
 * Ensures all agent outputs work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_BuildStatus ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystem(const FString& SystemName);

    // Cross-system testing
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestVFXEnvironmentIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestAudioCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestAICombatIntegration();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RestoreBuildSnapshot(int32 SnapshotIndex);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    FInteg_BuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FString> GetRegisteredSystems() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FInteg_BuildStatus CurrentBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FInteg_BuildStatus> BuildSnapshots;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

private:
    void ValidateSystemClass(const FString& SystemName, const FString& ClassPath, FInteg_SystemReport& OutReport);
    void TestCrossSystemDependencies();
    void LogIntegrationResults(const FInteg_BuildStatus& Status);
};