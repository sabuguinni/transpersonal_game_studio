#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_SystemRegistry.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    WorldGeneration,
    BiomeManagement,
    PerformanceMonitoring,
    ResourceStreaming,
    PhysicsCore,
    AudioCore,
    VFXCore
};

USTRUCT(BlueprintType)
struct FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EEng_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly)
    float InitializationTime;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        SystemType = EEng_SystemType::WorldGeneration;
        bIsActive = false;
        InitializationTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UEng_SystemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterSystem(const FString& SystemName, EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FEng_SystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void ValidateSystemDependencies();

private:
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    void LogSystemStatus() const;
};