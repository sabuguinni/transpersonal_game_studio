#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ModularSystemManager.generated.h"

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

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemState State;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsEssential;

    FEng_SystemInfo()
        : SystemName(TEXT("Unknown"))
        , State(EEng_SystemState::Uninitialized)
        , Priority(EEng_SystemPriority::Medium)
        , InitializationTime(0.0f)
        , LastUpdateTime(0.0f)
        , bIsEssential(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemStateChanged, FString, SystemName, EEng_SystemState, NewState);

UCLASS()
class TRANSPERSONALGAME_API UEng_ModularSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, bool bIsEssential = false);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool ShutdownSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool PauseSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool ResumeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    EEng_SystemState GetSystemState(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    TArray<FEng_SystemInfo> GetAllSystemInfo();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void RestartSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool IsSystemHealthy(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void RunSystemDiagnostics();

    UPROPERTY(BlueprintAssignable, Category = "System Management")
    FOnSystemStateChanged OnSystemStateChanged;

private:
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY()
    TArray<FString> InitializationOrder;

    UPROPERTY()
    bool bIsInitializing;

    void SortSystemsByPriority();
    void SetSystemState(const FString& SystemName, EEng_SystemState NewState);
    void LogSystemStatus(const FString& SystemName, const FString& Message);
    bool ValidateSystemDependencies(const FString& SystemName);
};