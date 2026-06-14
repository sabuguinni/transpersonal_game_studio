#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_SystemRegistry.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core = 0,
    Physics = 1,
    Rendering = 2,
    Audio = 3,
    AI = 4,
    Networking = 5,
    UI = 6,
    Gameplay = 7
};

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Running = 2,
    Paused = 3,
    Error = 4,
    Shutdown = 5
};

USTRUCT(BlueprintType)
struct FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 Priority;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        SystemType = EEng_SystemType::Core;
        Status = EEng_SystemStatus::Uninitialized;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        Priority = 0;
    }
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

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FEng_SystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FEng_SystemInfo> GetSystemsByType(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool IsSystemRunning(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    int32 GetSystemCount() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    float GetTotalInitializationTime() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    float TotalInitTime;

private:
    void UpdateSystemTime(const FString& SystemName);
    void SortSystemsByPriority();
};