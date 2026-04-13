#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_ModuleStatus : uint8
{
    NotLoaded       UMETA(DisplayName = "Not Loaded"),
    Loading         UMETA(DisplayName = "Loading"),
    Loaded          UMETA(DisplayName = "Loaded"),
    Failed          UMETA(DisplayName = "Failed"),
    Deprecated      UMETA(DisplayName = "Deprecated")
};

UENUM(BlueprintType)
enum class EBuild_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    Optional        UMETA(DisplayName = "Optional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    EBuild_ModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    EBuild_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    FString Version;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Module Info")
    float LoadTime;

    FBuild_ModuleInfo()
    {
        ModuleName = TEXT("");
        Status = EBuild_ModuleStatus::NotLoaded;
        Priority = EBuild_SystemPriority::Medium;
        Version = TEXT("1.0.0");
        LastError = TEXT("");
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FDateTime BuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    float TotalBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bBuildSuccessful;

    FBuild_IntegrationReport()
    {
        BuildTime = FDateTime::Now();
        TotalModules = 0;
        LoadedModules = 0;
        FailedModules = 0;
        TotalBuildTime = 0.0f;
        bBuildSuccessful = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuild_ModuleStatusChanged, const FBuild_ModuleInfo&, ModuleInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuild_IntegrationComplete, const FBuild_IntegrationReport&, Report);

/**
 * Build Integration Manager - Coordinates all game systems and modules
 * Ensures proper loading order, dependency resolution, and error handling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModule(const FString& ModuleName, EBuild_SystemPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ModuleInfo GetModuleInfo(const FString& ModuleName) const;

    // Integration Control
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StopIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsIntegrationRunning() const { return bIntegrationRunning; }

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void TriggerBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GetLastBuildReport() const { return LastBuildReport; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleInfo> GetAllModules() const;

    // Error Handling
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ReportModuleError(const FString& ModuleName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ClearModuleErrors(const FString& ModuleName);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuild_ModuleStatusChanged OnModuleStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuild_IntegrationComplete OnIntegrationComplete;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void LogModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void ValidateAllDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void ForceReloadAllModules();

protected:
    // Internal module tracking
    UPROPERTY()
    TMap<FString, FBuild_ModuleInfo> RegisteredModules;

    UPROPERTY()
    FBuild_IntegrationReport LastBuildReport;

    UPROPERTY()
    bool bIntegrationRunning;

    UPROPERTY()
    FTimerHandle IntegrationTimerHandle;

    // Internal Methods
    void UpdateModuleStatus();
    void ProcessModuleDependencies();
    void LoadModuleInOrder(const FString& ModuleName);
    bool ValidateModuleDependencies(const FString& ModuleName) const;
    void GenerateBuildReport();
    void BroadcastModuleStatusChange(const FBuild_ModuleInfo& ModuleInfo);

private:
    // Core system modules that must load first
    void RegisterCoreModules();
    void InitializeDefaultModules();
    
    // Build process
    void ExecuteBuildProcess();
    void OnBuildComplete(bool bSuccess);
    
    // Dependency resolution
    TArray<FString> ResolveDependencyOrder() const;
    bool HasCircularDependencies() const;
};