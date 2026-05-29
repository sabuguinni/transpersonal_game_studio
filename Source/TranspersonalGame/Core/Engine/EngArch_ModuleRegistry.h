#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Containers/Map.h"
#include "SharedTypes.h"
#include "EngArch_ModuleRegistry.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 DependencyCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString AgentResponsible;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        Status = EEng_ModuleStatus::Uninitialized;
        InitializationTime = 0.0f;
        DependencyCount = 0;
        AgentResponsible = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        ActiveActors = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_ModuleRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_ModuleRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName, const FString& AgentResponsible, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetModuleStatus(const FString& ModuleName, EEng_ModuleStatus NewStatus);

    // Module Queries
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ModuleInfo GetModuleInfo(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetAllModules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetModulesByStatus(EEng_ModuleStatus Status) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetModulesByAgent(const FString& AgentName) const;

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetUnmetDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetInitializationOrder() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceWithinLimits() const;

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void PrintModuleStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ForceReloadModule(const FString& ModuleName);

protected:
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    float LastMetricsUpdate;

    // Performance Limits
    UPROPERTY(EditAnywhere, Category = "Performance Limits")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, Category = "Performance Limits")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, Category = "Performance Limits")
    float MaxMemoryUsageMB;

private:
    void InitializeCoreModules();
    bool CheckCircularDependencies(const FString& ModuleName, const TArray<FString>& Dependencies) const;
    void RecalculateInitializationOrder();
    
    TArray<FString> ModuleInitOrder;
    bool bInitializationOrderDirty;
};