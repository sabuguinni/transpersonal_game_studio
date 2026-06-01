#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Perf_StreamingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TSoftObjectPtr<UWorld> LevelToStream;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsVisible;

    FPerf_StreamingZone()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        StreamingRadius = 5000.0f;
        UnloadRadius = 8000.0f;
        bIsLoaded = false;
        bIsVisible = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UPerf_StreamingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    TArray<FPerf_StreamingZone> StreamingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    bool bAutoManageStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    int32 MaxConcurrentLoads;

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void UpdateStreamingForPlayer(APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void LoadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void UnloadZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void SetZoneVisibility(const FString& ZoneName, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    bool IsZoneLoaded(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void AddStreamingZone(const FPerf_StreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void RemoveStreamingZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    void PreloadZonesAroundLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Level Streaming")
    TArray<FString> GetLoadedZones();

private:
    UPROPERTY()
    TMap<FString, ULevelStreaming*> LoadedLevels;

    UPROPERTY()
    TArray<FString> PendingLoads;

    UPROPERTY()
    TArray<FString> PendingUnloads;

    FTimerHandle StreamingUpdateTimer;
    
    void UpdateStreaming();
    void ProcessPendingOperations();
    float CalculateDistanceToZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation);
    bool ShouldLoadZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation);
    bool ShouldUnloadZone(const FPerf_StreamingZone& Zone, FVector PlayerLocation);
    void StartZoneLoad(const FString& ZoneName);
    void StartZoneUnload(const FString& ZoneName);
};