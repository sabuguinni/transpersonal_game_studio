#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemCoordinator.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Inactive UMETA(DisplayName = "Inactive"),
    Initializing UMETA(DisplayName = "Initializing"),
    Active UMETA(DisplayName = "Active"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 Priority;

    FEng_SystemInfo()
    {
        SystemName = "Unknown";
        Status = EEng_SystemStatus::Inactive;
        InitializationTime = 0.0f;
        Priority = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "System Coordination")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "System Coordination")
    TArray<FString> InitializationOrder;

public:
    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void RegisterSystem(const FString& SystemName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void SetSystemStatus(const FString& SystemName, EEng_SystemStatus Status);

    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    TArray<FString> GetSystemsInOrder();

    UFUNCTION(BlueprintCallable, Category = "System Coordination")
    int32 GetActiveSystemCount();
};