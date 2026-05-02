#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureCore.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EModulePriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LoadTime;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        Priority = EModulePriority::Medium;
        bIsLoaded = false;
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsInitialized;

    FEng_SystemDependency()
    {
        SystemName = TEXT("");
        bIsInitialized = false;
    }
};

UENUM(BlueprintType)
enum class EEng_ArchitectureState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Loading         UMETA(DisplayName = "Loading"),
    Ready           UMETA(DisplayName = "Ready"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * Core Architecture System - Manages module loading, dependencies and system initialization
 * This is the foundation that all other systems build upon
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterModule(const FString& ModuleName, EModulePriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ModuleInfo> GetLoadedModules() const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool AddSystemDependency(const FString& SystemName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetInitializationOrder() const;

    // Architecture State
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_ArchitectureState GetArchitectureState() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetArchitectureState(EEng_ArchitectureState NewState);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetModuleLoadTime(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetTotalLoadTime() const;

    // Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void PrintArchitectureReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ModuleInfo> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_SystemDependency> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EEng_ArchitectureState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float TotalInitializationTime;

private:
    void InitializeCoreModules();
    void ValidateModuleIntegrity();
    bool ResolveDependencyOrder(TArray<FString>& OutOrder) const;
    void LogArchitectureState() const;
};