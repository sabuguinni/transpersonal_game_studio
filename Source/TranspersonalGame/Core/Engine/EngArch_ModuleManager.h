#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "EngArch_ModuleManager.generated.h"

UENUM(BlueprintType)
enum class EEng_ModuleState : uint8
{
    Unloaded,
    Loading,
    Loaded,
    Failed,
    Disabled
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleState State;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LoadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ErrorMessage;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        State = EEng_ModuleState::Unloaded;
        LoadTime = 0.0f;
        ErrorMessage = TEXT("");
    }
};

/**
 * Engine Architect Module Manager
 * Manages loading, dependencies, and validation of all game modules
 * Ensures compilation order and prevents circular dependencies
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_ModuleManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_ModuleManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool LoadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnloadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ModuleState GetModuleState(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleInfo> GetAllModules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetLoadOrder() const;

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ForceRecompile();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetAgentBlocked(int32 AgentID, const FString& Reason);

private:
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    UPROPERTY()
    TMap<int32, bool> AgentStates;

    UPROPERTY()
    TMap<int32, FString> BlockedReasons;

    // Internal helpers
    bool CheckCircularDependencies(const FString& ModuleName, TSet<FString>& Visited) const;
    TArray<FString> TopologicalSort() const;
    void UpdateModuleState(const FString& ModuleName, EEng_ModuleState NewState, const FString& Error = TEXT(""));
};