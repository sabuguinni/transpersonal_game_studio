#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_SystemRegistry.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly)
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority;

    FEng_SystemInfo()
        : bIsInitialized(false)
        , InitializationTime(0.0f)
        , Priority(0)
    {}
};

UCLASS()
class TRANSPERSONALGAME_API UEng_SystemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterSystem(const FString& SystemName, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void MarkSystemInitialized(const FString& SystemName, float InitTime);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FEng_SystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void ValidateSystemDependencies();

private:
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    void LogSystemStatus();
};

#include "Eng_SystemRegistry.generated.h"