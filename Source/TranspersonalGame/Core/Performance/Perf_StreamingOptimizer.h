#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "SharedTypes.h"
#include "Perf_StreamingOptimizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerfStreaming, Log, All);

/**
 * Streaming state for a level
 */
UENUM(BlueprintType)
enum class EPerf_StreamingState : uint8
{
    Unloaded    UMETA(DisplayName = "Unloaded"),
    Loading     UMETA(DisplayName = "Loading"),
    Loaded      UMETA(DisplayName = "Loaded"),
    Unloading   UMETA(DisplayName = "Unloading"),
    Failed      UMETA(DisplayName = "Failed")
};

/**
 * Streaming priority levels
 */
UENUM(BlueprintType)
enum class EPerf_StreamingPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Background  UMETA(DisplayName = "Background")
};

/**
 * Information about a streaming level
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingLevelInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    FString LevelName;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    EPerf_StreamingState State;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    EPerf_StreamingPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    float LoadProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    bool bShouldBeLoaded;

    FPerf_StreamingLevelInfo()
    {
        LevelName = TEXT("");
        State = EPerf_StreamingState::Unloaded;
        Priority = EPerf_StreamingPriority::Medium;
        DistanceToPlayer = 0.0f;
        LoadProgress = 0.0f;
        MemoryUsageMB = 0.0f;
        bShouldBeLoaded = false;
    }
};

/**
 * Streaming configuration settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "500.0", ClampMax = "25000.0"))
    float UnloadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxConcurrentLoads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "100.0", ClampMax = "8192.0"))
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnablePreloading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bUnloadDistantLevels;

    FPerf_StreamingConfig()
    {
        StreamingDistance = 10000.0f;
        UnloadDistance = 15000.0f;
        MaxConcurrentLoads = 3;
        MaxMemoryUsageMB = 2048.0f;
        bEnablePreloading = true;
        bUnloadDistantLevels = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelStreamingStateChanged, const FString&, LevelName, EPerf_StreamingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamingMemoryWarning, float, CurrentMemoryMB);

/**
 * Streaming Optimizer Component
 * Manages level streaming based on player position and performance requirements
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_StreamingOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_StreamingOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    FPerf_StreamingConfig StreamingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    bool bAutoManageStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    float UpdateInterval;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Streaming Events")
    FOnLevelStreamingStateChanged OnLevelStreamingStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Streaming Events")
    FOnStreamingMemoryWarning OnStreamingMemoryWarning;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingLevels();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadLevel(const FString& LevelName, EPerf_StreamingPriority Priority = EPerf_StreamingPriority::Medium);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void PreloadNearbyLevels();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadDistantLevels();

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "Streaming")
    TArray<FPerf_StreamingLevelInfo> GetStreamingLevelInfo() const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    EPerf_StreamingState GetLevelStreamingState(const FString& LevelName) const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    float GetTotalStreamingMemoryUsage() const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    int32 GetActiveStreamingLevels() const;

    UFUNCTION(BlueprintPure, Category = "Streaming")
    bool IsLevelLoaded(const FString& LevelName) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Streaming Config")
    void SetStreamingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Streaming Config")
    void SetMaxConcurrentLoads(int32 MaxLoads);

    UFUNCTION(BlueprintCallable, Category = "Streaming Config")
    void SetAutoManageStreaming(bool bEnabled);

    // Performance
    UFUNCTION(BlueprintCallable, Category = "Streaming Performance")
    void OptimizeStreamingForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Streaming Performance")
    void FlushStreamingRequests();

    UFUNCTION(BlueprintCallable, Category = "Streaming Performance")
    void EmergencyUnloadLevels();

private:
    // Internal state
    UPROPERTY()
    TMap<FString, FPerf_StreamingLevelInfo> StreamingLevels;

    UPROPERTY()
    TArray<FString> LoadQueue;

    UPROPERTY()
    TArray<FString> UnloadQueue;

    float LastUpdateTime;
    int32 CurrentConcurrentLoads;
    float CurrentMemoryUsage;

    // Internal functions
    void InitializeStreamingLevels();
    void UpdateLevelStates();
    void ProcessLoadQueue();
    void ProcessUnloadQueue();
    void UpdatePlayerDistance();
    void CheckMemoryUsage();
    bool ShouldLoadLevel(const FString& LevelName, float Distance) const;
    bool ShouldUnloadLevel(const FString& LevelName, float Distance) const;
    float CalculateMemoryUsage(const FString& LevelName) const;
    EPerf_StreamingPriority CalculatePriority(float Distance) const;
    FVector GetPlayerLocation() const;
    ULevelStreaming* FindStreamingLevel(const FString& LevelName) const;
    void OnLevelStreamingFinished(const FString& LevelName, bool bSuccess);
    void BroadcastStateChange(const FString& LevelName, EPerf_StreamingState NewState);
};