#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Testing         UMETA(DisplayName = "Testing"),
    Stable          UMETA(DisplayName = "Stable"),
    Unstable        UMETA(DisplayName = "Unstable"),
    Critical        UMETA(DisplayName = "Critical"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    bool bHasErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    TArray<FString> ErrorMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    float LoadTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    EBuild_IntegrationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    int32 TotalTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    float SuccessRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    FDateTime BuildTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    TArray<FString> CriticalIssues;

    FBuild_IntegrationReport()
    {
        Status = EBuild_IntegrationStatus::Unknown;
        TotalTests = 0;
        PassedTests = 0;
        SuccessRate = 0.0f;
        BuildTimestamp = FDateTime::Now();
    }
};

/**
 * Build Integration Manager - Coordinates integration testing and build verification
 * Manages the integration of all game modules and systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Integration Testing Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunModuleTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void VerifySystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool RegisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ModuleStatus GetModuleStatus(const FString& ModuleName);

    // Build Verification
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool VerifyBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CreateTestActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupTestActors();

    // Status and Reporting
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    EBuild_IntegrationStatus GetCurrentStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    float GetOverallSuccessRate() const { return OverallSuccessRate; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogIntegrationStatus();

protected:
    // Internal testing functions
    bool TestCoreModules();
    bool TestGameplayModules();
    bool TestRenderingModules();
    bool TestAudioModules();
    bool TestPhysicsModules();
    
    void UpdateIntegrationStatus();
    void ReportCriticalIssue(const FString& Issue);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ModuleStatus> RegisteredModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    float OverallSuccessRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> TestActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    FBuild_IntegrationReport LastReport;

    // Integration test timers
    float TestStartTime;
    float LastTestTime;
    bool bTestingInProgress;
};