#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ModuleRegistry.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 006 - MODULE REGISTRY SYSTEM
 * 
 * Central registry for all game modules and their interdependencies.
 * This system ensures proper module loading order and validates
 * that all required dependencies are met before system initialization.
 * 
 * ARCHITECTURAL RULES:
 * 1. All modules must register here before initialization
 * 2. Dependencies must be declared explicitly
 * 3. Circular dependencies are detected and prevented
 * 4. Module load order is automatically determined
 * 5. Failed modules are isolated to prevent cascade failures
 */

// Module Load Priority
UENUM(BlueprintType)
enum class EEng_ModuleLoadPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),        // Core, Physics
    High           UMETA(DisplayName = "High"),            // World, Character
    Normal         UMETA(DisplayName = "Normal"),          // AI, Combat, Quest
    Low            UMETA(DisplayName = "Low"),             // Audio, VFX, UI
    Optional       UMETA(DisplayName = "Optional")         // Debug, Analytics
};

// Module Load State
UENUM(BlueprintType)
enum class EEng_ModuleLoadState : uint8
{
    Unloaded       UMETA(DisplayName = "Unloaded"),
    Loading        UMETA(DisplayName = "Loading"),
    Loaded         UMETA(DisplayName = "Loaded"),
    Initializing   UMETA(DisplayName = "Initializing"),
    Ready          UMETA(DisplayName = "Ready"),
    Error          UMETA(DisplayName = "Error"),
    Disabled       UMETA(DisplayName = "Disabled")
};

// Module Information
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleLoadPriority LoadPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleLoadState LoadState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> RequiredDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> OptionalDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float LoadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FDateTime LoadTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString LoadError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    int32 RetryCount;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("Unknown");
        ModuleVersion = TEXT("1.0.0");
        LoadPriority = EEng_ModuleLoadPriority::Normal;
        LoadState = EEng_ModuleLoadState::Unloaded;
        LoadTime = 0.0f;
        LoadTimestamp = FDateTime::Now();
        LoadError = TEXT("");
        RetryCount = 0;
    }
};

// Module Load Result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleLoadResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    bool bSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    float LoadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    TArray<FString> MissingDependencies;

    FEng_ModuleLoadResult()
    {
        bSuccess = false;
        ModuleName = TEXT("");
        LoadTime = 0.0f;
        ErrorMessage = TEXT("");
    }
};

/**
 * MODULE REGISTRY SUBSYSTEM
 * 
 * Manages the loading, initialization, and dependency resolution
 * of all game modules. Ensures proper load order and handles
 * module failures gracefully.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ModuleRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Registration
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool IsModuleRegistered(const FString& ModuleName);

    // Module Loading
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    FEng_ModuleLoadResult LoadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool LoadAllModules();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool LoadModulesByPriority(EEng_ModuleLoadPriority Priority);

    // Module Information
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    FEng_ModuleInfo GetModuleInfo(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FEng_ModuleInfo> GetAllModules();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FString> GetModulesByState(EEng_ModuleLoadState State);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FString> GetModulesByPriority(EEng_ModuleLoadPriority Priority);

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FString> GetMissingDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FString> GetLoadOrder();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool HasCircularDependencies();

    // Module Control
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool EnableModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool DisableModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    bool ReloadModule(const FString& ModuleName);

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    int32 GetLoadedModuleCount();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    int32 GetFailedModuleCount();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    float GetTotalLoadTime();

    UFUNCTION(BlueprintCallable, Category = "Module Registry")
    TArray<FString> GetLoadErrors();

protected:
    // Module storage
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    // Load tracking
    UPROPERTY()
    TArray<FString> LoadOrder;

    UPROPERTY()
    TArray<FString> LoadErrors;

    UPROPERTY()
    float TotalLoadTime;

    // Settings
    UPROPERTY(EditAnywhere, Category = "Module Registry")
    int32 MaxRetryAttempts;

    UPROPERTY(EditAnywhere, Category = "Module Registry")
    float RetryDelay;

    UPROPERTY(EditAnywhere, Category = "Module Registry")
    bool bAutoLoadOnStart;

    UPROPERTY(EditAnywhere, Category = "Module Registry")
    bool bFailOnMissingDependencies;

    // Internal methods
    void RegisterCoreModules();
    bool CalculateLoadOrder();
    bool CheckCircularDependencies(const FString& ModuleName, TArray<FString>& VisitedModules);
    void UpdateModuleState(const FString& ModuleName, EEng_ModuleLoadState NewState);
    void LogModuleEvent(const FString& ModuleName, const FString& Event, const FString& Details);

private:
    // Load state tracking
    TMap<FString, FDateTime> ModuleLoadStartTimes;
    bool bIsLoadingModules;
};