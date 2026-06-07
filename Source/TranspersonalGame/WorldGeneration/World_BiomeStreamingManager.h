#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeStreamingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive;

    FWorld_BiomeZone()
    {
        BiomeName = TEXT("DefaultBiome");
        Center = FVector::ZeroVector;
        StreamingRadius = 1000.0f;
        VegetationDensity = 0.5f;
        BiomeType = EBiomeType::Temperate;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceOptimized;

    FWorld_StreamingPerformanceData()
    {
        ActiveActorCount = 0;
        FrameTime = 0.0f;
        StreamingDistance = 2000.0f;
        bPerformanceOptimized = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core biome streaming data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_StreamingPerformanceData PerformanceData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* StreamingTrigger;

    // Streaming configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    float MaxStreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Config")
    bool bEnablePerformanceMonitoring;

public:
    // Biome streaming interface
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void InitializeBiomeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateBiomeStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void ActivateBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void DeactivateBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    FWorld_BiomeZone GetBiomeAtLocation(const FVector& Location);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldOptimizeStreaming() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeStreamingDistance();

    // Vegetation streaming integration
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void StreamVegetationForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UnstreamVegetationForBiome(const FString& BiomeName);

    // Water body streaming
    UFUNCTION(BlueprintCallable, Category = "Water")
    void StreamWaterBodies(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Water")
    void UpdateWaterBodyLOD(float Distance);

    // Physics integration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UpdatePhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsForStreamedActors(bool bEnable);

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateStreamingSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetStreamingSystemStatus() const;

private:
    // Internal streaming logic
    void ProcessBiomeActivation();
    void ProcessBiomeDeactivation();
    void UpdateStreamingLOD();
    void MonitorPerformanceMetrics();
    
    // Performance optimization
    void OptimizeActorCount();
    void AdjustStreamingDistances();
    void ManageMemoryUsage();

    // Internal state
    TArray<FString> ActiveBiomes;
    TArray<FString> PendingActivation;
    TArray<FString> PendingDeactivation;
    float LastPerformanceCheck;
    bool bStreamingSystemInitialized;
};

#include "World_BiomeStreamingManager.generated.h"