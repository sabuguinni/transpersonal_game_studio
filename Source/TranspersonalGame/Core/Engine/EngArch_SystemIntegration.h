#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngArch_SystemIntegration.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadWrite, Category = "System")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadWrite, Category = "System")
    float InitializationTime;

    FEng_SystemRegistration()
    {
        SystemName = TEXT("");
        Priority = EEng_SystemPriority::Medium;
        bIsInitialized = false;
        InitializationTime = 0.0f;
    }
};

/**
 * Engine Architect System Integration Manager
 * Manages initialization order, dependencies, and health monitoring for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_SystemIntegration : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_SystemIntegration();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System Initialization Control
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetFailedSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetSystemCount() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemInitTime(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetTotalInitTime() const;

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void PrintSystemStatus();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TArray<FEng_SystemRegistration> RegisteredSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TArray<FString> FailedSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float TotalInitializationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bAllSystemsInitialized;

private:
    void SortSystemsByPriority();
    bool ValidateSystemDependencies(const FString& SystemName);
    void LogSystemError(const FString& SystemName, const FString& Error);
};