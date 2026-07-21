#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Eng_ArchitecturalFramework.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Background = 4
};

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Paused = 3,
    Error = 4,
    Shutdown = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemState State;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 DependencyCount;

    FEng_SystemInfo()
    {
        SystemName = TEXT("");
        Priority = EEng_SystemPriority::Medium;
        State = EEng_SystemState::Uninitialized;
        InitializationTime = 0.0f;
        DependencyCount = 0;
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
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System State Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetSystemState(const FString& SystemName, EEng_SystemState NewState);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddSystemDependency(const FString& SystemName, const FString& DependsOn);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetSystemDependencies(const FString& SystemName) const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitecturalErrors() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TArray<FString>> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> ArchitecturalErrors;

private:
    void InitializeCoreSystems();
    void ValidateDependencies();
    bool CheckCircularDependencies(const FString& SystemName, TSet<FString>& Visited, TSet<FString>& RecursionStack) const;
};