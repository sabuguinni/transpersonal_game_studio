#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "IntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIntegration, Log, All);

USTRUCT(BlueprintType)
struct FModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly)
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly)
    FString LastError;

    UPROPERTY(BlueprintReadOnly)
    float LoadTime;

    FModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        LastError = TEXT("");
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuildStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 BuildNumber;

    UPROPERTY(BlueprintReadOnly)
    FDateTime BuildTime;

    UPROPERTY(BlueprintReadOnly)
    bool bIsStable;

    UPROPERTY(BlueprintReadOnly)
    TArray<FModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly)
    FString BuildNotes;

    FBuildStatus()
    {
        BuildNumber = 0;
        BuildTime = FDateTime::Now();
        bIsStable = false;
        BuildNotes = TEXT("");
    }
};

/**
 * Integration Manager - Coordinates all game systems and ensures compatibility
 * Responsible for module loading, dependency resolution, and build verification
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool LoadGameModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool UnloadGameModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FModuleStatus GetModuleStatus(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FModuleStatus> GetAllModuleStatuses();

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuildStatus GetCurrentBuildStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CreateBuildSnapshot(const FString& BuildNotes);

    // System Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RefreshSystemDependencies();

    // Debugging and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunIntegrationDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateSystemReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogSystemStatus();

protected:
    // Internal module tracking
    UPROPERTY()
    TMap<FString, FModuleStatus> ModuleRegistry;

    UPROPERTY()
    FBuildStatus CurrentBuild;

    UPROPERTY()
    TArray<FBuildStatus> BuildHistory;

    // System initialization order
    UPROPERTY()
    TArray<FString> SystemInitOrder;

    // Internal methods
    void RegisterCoreModules();
    void ValidateModuleDependencies();
    bool CheckModuleCompatibility(const FString& ModuleName);
    void UpdateModuleStatus(const FString& ModuleName, bool bLoaded, const FString& Error = TEXT(""));

private:
    // Build management
    int32 NextBuildNumber;
    bool bSystemsInitialized;

    // Performance tracking
    double LastIntegrationCheck;
    float IntegrationCheckInterval;
};