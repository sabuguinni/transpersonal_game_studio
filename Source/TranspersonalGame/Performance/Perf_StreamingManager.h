#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Perf_StreamingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_StreamingState : uint8
{
    Unloaded UMETA(DisplayName = "Unloaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Unloading UMETA(DisplayName = "Unloading"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FPerf_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ZoneRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TSoftObjectPtr<UWorld> LevelToStream;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    EPerf_StreamingState CurrentState = EPerf_StreamingState::Unloaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEssential = false;
};

USTRUCT(BlueprintType)
struct FPerf_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxConcurrentLoads = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float MemoryBudgetMB = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnablePreloading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float PreloadDistance = 15000.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UPerf_StreamingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    virtual void Tick(float DeltaTime);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    FPerf_StreamingSettings StreamingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    TArray<FPerf_StreamingZone> StreamingZones;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 LoadedZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 LoadingZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float LastUpdateTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreaming();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void AddStreamingZone(const FPerf_StreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void RemoveStreamingZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadZoneImmediate(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadZoneImmediate(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    EPerf_StreamingState GetZoneState(const FString& ZoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    TArray<FString> GetLoadedZones() const;

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    TArray<FString> GetLoadingZones() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsagePercent() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinMemoryBudget() const;

private:
    FTimerHandle UpdateTimerHandle;
    TMap<FString, ULevelStreaming*> StreamingLevels;
    TArray<FString> LoadQueue;
    TArray<FString> UnloadQueue;
    
    FVector GetPlayerLocation() const;
    float GetDistanceToPlayer(const FVector& Location) const;
    bool ShouldLoadZone(const FPerf_StreamingZone& Zone) const;
    bool ShouldUnloadZone(const FPerf_StreamingZone& Zone) const;
    void ProcessLoadQueue();
    void ProcessUnloadQueue();
    void UpdateMemoryUsage();
    FPerf_StreamingZone* FindZone(const FString& ZoneName);
    const FPerf_StreamingZone* FindZone(const FString& ZoneName) const;
};