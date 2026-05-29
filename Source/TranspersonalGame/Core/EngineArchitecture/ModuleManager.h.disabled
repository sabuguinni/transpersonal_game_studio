#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Containers/Map.h"
#include "../../SharedTypes.h"
#include "ModuleManager.generated.h"

/**
 * Module dependency information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Info")
    FDateTime LoadTime;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        Priority = EEng_SystemPriority::Medium;
        bIsLoaded = false;
        bIsInitialized = false;
        LoadTime = FDateTime::Now();
    }

    FEng_ModuleInfo(const FString& InModuleName, EEng_SystemPriority InPriority)
        : ModuleName(InModuleName)
        , Priority(InPriority)
        , bIsLoaded(false)
        , bIsInitialized(false)
        , LoadTime(FDateTime::Now())
    {
    }
};

/**
 * Module load order configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleLoadOrder
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Order")
    TArray<FString> CriticalModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Order")
    TArray<FString> HighPriorityModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Order")
    TArray<FString> MediumPriorityModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load Order")
    TArray<FString> LowPriorityModules;

    FEng_ModuleLoadOrder()
    {
        // Default critical modules
        CriticalModules = {
            TEXT("Core"),
            TEXT("Physics"),
            TEXT("Rendering")
        };

        // Default high priority modules
        HighPriorityModules = {
            TEXT("WorldGeneration"),
            TEXT("CharacterSystem"),
            TEXT("AI")
        };

        // Default medium priority modules
        MediumPriorityModules = {
            TEXT("Audio"),
            TEXT("VFX"),
            TEXT("Quest")
        };

        // Default low priority modules
        LowPriorityModules = {
            TEXT("UI"),
            TEXT("Analytics"),
            TEXT("Debug")
        };
    }
};

/**
 * Advanced Module Manager
 * Handles complex module dependencies, load ordering, and lifecycle management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineModuleManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineModuleManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module registration and management
    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool RegisterModule(const FString& ModuleName, EEng_SystemPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool IsModuleRegistered(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool IsModuleInitialized(const FString& ModuleName) const;

    // Dependency management
    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool ValidateModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FString> GetDependentModules(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FString> GetLoadOrder() const;

    // Module lifecycle
    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool LoadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool InitializeModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool ShutdownModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void LoadAllModules();

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void InitializeAllModules();

    // Module information
    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    FEng_ModuleInfo GetModuleInfo(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FEng_ModuleInfo> GetAllModuleInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    int32 GetModuleCount() const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    int32 GetLoadedModuleCount() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void SetModuleLoadOrder(const FEng_ModuleLoadOrder& LoadOrder);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    FEng_ModuleLoadOrder GetModuleLoadOrder() const;

    // Debug and diagnostics
    UFUNCTION(BlueprintCallable, Category = "Module Manager", CallInEditor)
    void PrintModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Module Manager", CallInEditor)
    void ValidateAndReportDependencies();

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool HasCircularDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FString> FindCircularDependencies() const;

protected:
    // Module registry
    UPROPERTY(BlueprintReadOnly, Category = "Module Manager")
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    // Load order configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_ModuleLoadOrder LoadOrderConfig;

    // Module state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Module Manager")
    TArray<FString> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Module Manager")
    TArray<FString> InitializedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Module Manager")
    TArray<FString> FailedModules;

    // Configuration flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoLoadDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bStrictDependencyOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bFailOnCircularDependencies;

private:
    // Internal helper methods
    bool CanLoadModule(const FString& ModuleName) const;
    bool AreDependenciesSatisfied(const FString& ModuleName) const;
    TArray<FString> TopologicalSort() const;
    void BuildDependencyGraph(TMap<FString, TArray<FString>>& OutGraph) const;
    bool HasCircularDependencyRecursive(const FString& ModuleName, TSet<FString>& Visited, TSet<FString>& RecStack) const;
    void LogModuleOperation(const FString& Operation, const FString& ModuleName, bool bSuccess) const;
};