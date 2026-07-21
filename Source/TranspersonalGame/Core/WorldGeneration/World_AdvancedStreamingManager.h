#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "Engine/LevelStreamingVolume.h"
#include "SharedTypes.h"
#include "World_AdvancedStreamingManager.generated.h"

// Biome zone data for streaming optimization
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    bool bIsStreamingActive;

    FWorld_BiomeZoneData()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        BiomeType = EBiomeType::Temperate;
        VegetationDensity = 50;
        bIsStreamingActive = false;
    }
};

// Water body configuration for dynamic water system
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FString WaterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float WaterWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    bool bIsDynamic;

    FWorld_WaterBodyData()
    {
        WaterName = TEXT("DefaultWater");
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector(1000, 0, 0);
        WaterWidth = 200.0f;
        FlowSpeed = 100.0f;
        bIsDynamic = true;
    }
};

// Streaming volume configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingVolumeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString VolumeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector VolumeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector VolumeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bAutoActivate;

    FWorld_StreamingVolumeData()
    {
        VolumeName = TEXT("DefaultVolume");
        VolumeCenter = FVector::ZeroVector;
        VolumeSize = FVector(2000, 2000, 400);
        StreamingDistance = 5000.0f;
        bAutoActivate = true;
    }
};

/**
 * Advanced streaming manager for procedural world generation
 * Handles biome zone streaming, water system management, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_AdvancedStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_AdvancedStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Biome zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    bool bEnableBiomeStreaming;

    // Water system management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    bool bEnableDynamicWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterUpdateFrequency;

    // Streaming optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<FWorld_StreamingVolumeData> StreamingVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float PlayerStreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnablePerformanceOptimization;

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoLOD;

public:
    // Biome zone functions
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UpdateBiomeStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ActivateBiomeZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void DeactivateBiomeZone(const FString& ZoneName);

    // Water system functions
    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void InitializeWaterSystem();

    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void UpdateWaterFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void CreateWaterBody(const FWorld_WaterBodyData& WaterData);

    UFUNCTION(BlueprintCallable, Category = "Water Management")
    void RemoveWaterBody(const FString& WaterName);

    // Streaming functions
    UFUNCTION(BlueprintCallable, Category = "Streaming Management")
    void InitializeStreamingVolumes();

    UFUNCTION(BlueprintCallable, Category = "Streaming Management")
    void UpdateStreamingState(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming Management")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Streaming Management")
    void SetStreamingDistance(float NewDistance);

    // Performance functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantActors(const FVector& PlayerLocation);

private:
    // Internal tracking
    FVector LastPlayerLocation;
    float LastUpdateTime;
    int32 CurrentActiveActors;
    
    // Helper functions
    void UpdateBiomeTransitions();
    void ManageActorLOD();
    void OptimizeVegetationDensity();
    void UpdateWaterMaterials();
    bool IsLocationInBiomeZone(const FVector& Location, const FWorld_BiomeZoneData& Zone) const;
    float CalculateDistanceToZone(const FVector& Location, const FWorld_BiomeZoneData& Zone) const;
};