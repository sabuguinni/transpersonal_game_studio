#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BuildManager.generated.h"

UENUM(BlueprintType)
enum class EBuildStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Stable          UMETA(DisplayName = "Stable"),
    Unstable        UMETA(DisplayName = "Unstable"),
    Broken          UMETA(DisplayName = "Broken"),
    Testing         UMETA(DisplayName = "Testing")
};

UENUM(BlueprintType)
enum class EModuleStatus : uint8
{
    NotLoaded       UMETA(DisplayName = "Not Loaded"),
    Loading         UMETA(DisplayName = "Loading"),
    Loaded          UMETA(DisplayName = "Loaded"),
    Failed          UMETA(DisplayName = "Failed"),
    Integrated      UMETA(DisplayName = "Integrated")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FDateTime LastUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 IssueCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bDependenciesMet;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    FModuleInfo()
    {
        ModuleName = TEXT("");
        Status = EModuleStatus::NotLoaded;
        LastUpdate = FDateTime::Now();
        IssueCount = 0;
        bDependenciesMet = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildId;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EBuildStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TestsFailed;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float IntegrationHealthPercentage;

    FBuildReport()
    {
        BuildId = TEXT("");
        Status = EBuildStatus::Unknown;
        Timestamp = FDateTime::Now();
        TestsPassed = 0;
        TestsFailed = 0;
        IntegrationHealthPercentage = 0.0f;
    }
};

/**
 * Build Manager - Handles integration, compilation, and build verification
 * Responsible for coordinating all 18 agent outputs into a coherent build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void StartBuildVerification(const FString& BuildId);

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void RunCompilationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void CreateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    EBuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    FBuildReport GetLatestBuildReport() const { return LatestBuildReport; }

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void UpdateModuleStatus(const FString& ModuleName, EModuleStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    FModuleInfo GetModuleInfo(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FModuleInfo> GetAllModules() const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool AreAllDependenciesMet(const FString& ModuleName) const;

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Integration Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration Testing")
    void TestModuleCompatibility(const FString& ModuleA, const FString& ModuleB);

    UFUNCTION(BlueprintCallable, Category = "Integration Testing")
    void ValidateSystemIntegration();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateMemoryUsage(int32 MaxMemoryMB);

    // Build History
    UFUNCTION(BlueprintCallable, Category = "Build History")
    void SaveBuildSnapshot(const FString& BuildId);

    UFUNCTION(BlueprintCallable, Category = "Build History")
    void RollbackToBuild(const FString& BuildId);

    UFUNCTION(BlueprintCallable, Category = "Build History")
    TArray<FString> GetBuildHistory() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStatusChanged, EBuildStatus, NewStatus);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBuildStatusChanged OnBuildStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleStatusChanged, const FModuleInfo&, ModuleInfo);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnModuleStatusChanged OnModuleStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildReportGenerated, const FBuildReport&, Report);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBuildReportGenerated OnBuildReportGenerated;

protected:
    // Internal state
    UPROPERTY()
    EBuildStatus CurrentBuildStatus;

    UPROPERTY()
    FBuildReport LatestBuildReport;

    UPROPERTY()
    TMap<FString, FModuleInfo> RegisteredModules;

    UPROPERTY()
    TArray<FString> BuildHistory;

    UPROPERTY()
    FString CurrentBuildId;

    // Internal methods
    void InitializeDefaultModules();
    void CheckModuleDependencies();
    void UpdateBuildStatus(EBuildStatus NewStatus);
    void LogBuildEvent(const FString& Event);
    bool TestBasicFunctionality();
    bool TestAssetSystem();
    bool TestLevelSystem();
    float CalculateIntegrationHealth();

    // Module definitions for all 18 agents
    static const TArray<FString> CoreModules;
    static const TArray<FString> GameplayModules;
    static const TArray<FString> ContentModules;
    static const TArray<FString> QualityModules;
};