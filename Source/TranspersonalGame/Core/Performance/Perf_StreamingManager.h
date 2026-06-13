#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Perf_StreamingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_StreamingState : uint8
{
    Unloaded        UMETA(DisplayName = "Unloaded"),
    Loading         UMETA(DisplayName = "Loading"),
    Loaded          UMETA(DisplayName = "Loaded"),
    Unloading       UMETA(DisplayName = "Unloading"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ZoneRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 Priority = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    EPerf_StreamingState CurrentState = EPerf_StreamingState::Unloaded;

    UPROPERTY()
    ULevelStreaming* StreamingLevel = nullptr;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingUpdateFrequency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxConcurrentLoads = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxLoadedZones = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float MemoryBudgetMB = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnablePreloading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableAsyncLoading = true;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_StreamingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_StreamingSettings StreamingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_StreamingZone> StreamingZones;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentLoadedZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentLoadingZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentMemoryUsageMB = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AddStreamingZone(const FPerf_StreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RemoveStreamingZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LoadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnloadZone(const FString& ZoneName);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_StreamingState GetZoneState(const FString& ZoneName) const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetStreamingEfficiency() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void OptimizeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPlayerLocation(const FVector& NewLocation);

protected:
    UPROPERTY()
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY()
    TArray<FString> LoadingQueue;

    UPROPERTY()
    TArray<FString> UnloadingQueue;

    FTimerHandle StreamingUpdateTimer;
    float LastUpdateTime = 0.0f;

private:
    void ProcessLoadingQueue();
    void ProcessUnloadingQueue();
    void UpdateZoneStates();
    FPerf_StreamingZone* FindZone(const FString& ZoneName);
    float GetDistanceToZone(const FPerf_StreamingZone& Zone) const;
    bool ShouldLoadZone(const FPerf_StreamingZone& Zone) const;
    bool ShouldUnloadZone(const FPerf_StreamingZone& Zone) const;
    void EstimateMemoryUsage();
};