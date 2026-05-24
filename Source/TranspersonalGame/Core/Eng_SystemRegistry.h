#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemRegistry.generated.h"

/**
 * System Registry - Central repository for all game systems
 * Manages system lifecycle, dependencies, and communication
 * Ensures proper initialization order and prevents conflicts
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemClass;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    FEng_SystemInfo()
    {
        SystemName = TEXT("");
        SystemClass = TEXT("");
        Priority = 0;
        bIsInitialized = false;
        InitializationTime = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized,
    Initializing,
    Running,
    Paused,
    Error,
    Shutdown
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool RegisterSystem(const FString& SystemName, const FString& SystemClass, int32 Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System Lifecycle
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool ShutdownSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void ShutdownAllSystems();

    // System State Management
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void SetSystemState(const FString& SystemName, EEng_SystemState NewState);

    // System Information
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    FEng_SystemInfo GetSystemInfo(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FString> GetSystemsByPriority() const;

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool ValidateDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FString> GetInitializationOrder() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool HasCircularDependency() const;

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // System States
    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TMap<FString, EEng_SystemState> SystemStates;

    // Initialization Order Cache
    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TArray<FString> InitializationOrderCache;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TMap<FString, float> SystemPerformanceMetrics;

private:
    // Internal methods
    void RegisterCoreSystemsInternal();
    bool ResolveDependencies();
    TArray<FString> TopologicalSort() const;
    bool ValidateSystemClass(const FString& SystemClass) const;
    void UpdatePerformanceMetrics();
};