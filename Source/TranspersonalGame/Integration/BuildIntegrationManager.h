#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready          UMETA(DisplayName = "Ready"),
    Testing        UMETA(DisplayName = "Testing"),
    Failed         UMETA(DisplayName = "Failed"),
    Success        UMETA(DisplayName = "Success")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastTestTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TestsFailed;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        LastError = TEXT("");
        LastTestTime = 0.0f;
        TestsPassed = 0;
        TestsFailed = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalTestTime;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        BuildVersion = TEXT("1.0.0");
        LastBuildTime = FDateTime::Now();
        TotalTestTime = 0.0f;
    }
};

/**
 * Build Integration Manager - Coordinates integration testing across all game modules
 * Ensures all 18 agent outputs work together cohesively
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

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunFullIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunModuleTest(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunCrossModuleTest();

    // Status and Reporting
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetOverallStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetFailedModules() const;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateAllSystems();

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationTestComplete, const FBuild_IntegrationReport&, Report);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleTestComplete, const FString&, ModuleName, EBuild_IntegrationStatus, Status);

    UPROPERTY(BlueprintAssignable, Category = "Integration")
    FOnIntegrationTestComplete OnIntegrationTestComplete;

    UPROPERTY(BlueprintAssignable, Category = "Integration")
    FOnModuleTestComplete OnModuleTestComplete;

protected:
    // Core module testing functions
    void TestCorePhysicsModule();
    void TestWorldGenerationModule();
    void TestEnvironmentModule();
    void TestCharacterModule();
    void TestAIModule();
    void TestQuestModule();
    void TestAudioModule();
    void TestVFXModule();

    // Cross-module integration tests
    void TestPhysicsWorldIntegration();
    void TestCharacterEnvironmentIntegration();
    void TestAIQuestIntegration();
    void TestAudioVFXIntegration();

    // Utility functions
    void UpdateModuleStatus(const FString& ModuleName, EBuild_IntegrationStatus Status, const FString& Error = TEXT(""));
    bool IsModuleLoaded(const FString& ModuleName) const;
    void LogIntegrationResult(const FString& TestName, bool bSuccess, const FString& Details = TEXT(""));

private:
    UPROPERTY()
    TMap<FString, FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY()
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY()
    bool bIsRunningTests;

    // Module names for testing
    static const TArray<FString> CoreModules;
    static const TArray<FString> GameplayModules;
    static const TArray<FString> ContentModules;
};