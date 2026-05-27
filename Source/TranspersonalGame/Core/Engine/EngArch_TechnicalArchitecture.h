#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "EngArch_TechnicalArchitecture.generated.h"

UENUM(BlueprintType)
enum class EEngArch_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

UENUM(BlueprintType)
enum class EEngArch_ModuleState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Error = 3,
    Disabled = 4
};

USTRUCT(BlueprintType)
struct FEngArch_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEngArch_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEngArch_ModuleState State;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 MemoryUsageMB;

    FEngArch_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EEngArch_SystemPriority::Medium;
        State = EEngArch_ModuleState::Uninitialized;
        InitializationTime = 0.0f;
        MemoryUsageMB = 0;
    }
};

USTRUCT(BlueprintType)
struct FEngArch_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageGB;

    FEngArch_PerformanceMetrics()
    {
        FrameRate = 60.0f;
        GameThreadTime = 16.67f;
        RenderThreadTime = 16.67f;
        DrawCalls = 0;
        ActiveActors = 0;
        MemoryUsageGB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_TechnicalArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, EEngArch_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetSystemState(const FString& SystemName, EEngArch_ModuleState State);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEngArch_ModuleState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEngArch_SystemInfo> GetAllSystems() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEngArch_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceWithinTargets() const;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemErrors() const;

    // Module Integration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<FString, FEngArch_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEngArch_PerformanceMetrics CachedMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> SystemErrors;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxGameThreadTime = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxMemoryUsageGB = 8.0f;

private:
    void UpdatePerformanceMetrics();
    void ValidatePerformanceTargets();
    bool CheckSystemDependency(const FString& SystemName) const;
};