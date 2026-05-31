#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_SystemArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Paused = 3,
    Error = 4
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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemActive(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetAllSystemsInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetTotalInitializationTime() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalInitTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveSystemCount;

private:
    void InitializeSystemByPriority(EEng_SystemPriority Priority);
    bool ValidateSystemDependency(const FString& SystemName);
    void LogSystemStatus(const FString& SystemName, const FString& Message);
};