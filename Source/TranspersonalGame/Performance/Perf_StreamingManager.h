#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Pawn.h"
#include "Perf_StreamingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_StreamingState : uint8
{
    Unloaded UMETA(DisplayName = "Unloaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Unloading UMETA(DisplayName = "Unloading")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadRadius = 7500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    EPerf_StreamingState CurrentState = EPerf_StreamingState::Unloaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<FString> AssociatedLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 Priority = 1;

    FPerf_StreamingZone()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        StreamingRadius = 5000.0f;
        UnloadRadius = 7500.0f;
        CurrentState = EPerf_StreamingState::Unloaded;
        Priority = 1;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UPerf_StreamingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    TArray<FPerf_StreamingZone> StreamingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxConcurrentLoads = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    bool bEnableDistanceBasedStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    bool bEnableMemoryBasedUnloading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "1024", ClampMax = "8192"))
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming Stats")
    int32 LoadedZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming Stats")
    int32 LoadingZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming Stats")
    float CurrentMemoryUsageMB = 0.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void AddStreamingZone(const FPerf_StreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void RemoveStreamingZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void ForceLoadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void ForceUnloadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetPlayerLocation(const FVector& Location);

    UFUNCTION(BlueprintPure, Category = "Streaming")
    bool IsZoneLoaded(const FString& ZoneName) const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    TArray<FString> GetLoadedZones() const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    float GetMemoryUsage() const { return CurrentMemoryUsageMB; }

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void OptimizeMemoryUsage();

private:
    void UpdateStreaming();
    void ProcessZoneStreaming(FPerf_StreamingZone& Zone, const FVector& PlayerLocation);
    void LoadZone(FPerf_StreamingZone& Zone);
    void UnloadZone(FPerf_StreamingZone& Zone);
    void UpdateMemoryStats();
    FVector GetPlayerLocation() const;
    bool ShouldLoadZone(const FPerf_StreamingZone& Zone, const FVector& PlayerLocation) const;
    bool ShouldUnloadZone(const FPerf_StreamingZone& Zone, const FVector& PlayerLocation) const;

    float LastUpdateTime = 0.0f;
    FVector CachedPlayerLocation = FVector::ZeroVector;
    int32 CurrentConcurrentLoads = 0;
};