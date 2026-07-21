#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_PerformanceOptimizedBiomes.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_BiomeType BiomeType;

    FWorld_VegetationZone()
    {
        ZoneName = TEXT("DefaultZone");
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxDensity = 20;
        BiomeType = EWorld_BiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float EffectRadius;

    FWorld_WeatherZone()
    {
        ZoneName = TEXT("DefaultWeather");
        Location = FVector::ZeroVector;
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 1.0f;
        EffectRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PerformanceOptimizedBiomes : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PerformanceOptimizedBiomes();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Performance-optimized vegetation management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VegetationCullingDistance;

    // Vegetation zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FWorld_VegetationZone> VegetationZones;

    // Weather zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherZone> WeatherZones;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceLimitReached;

    // Biome management functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateVegetationZone(const FWorld_VegetationZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateWeatherZone(const FWorld_WeatherZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVegetationDensity();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantVegetation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void SpawnVegetationBatch(const FWorld_VegetationZone& Zone, int32 BatchSize);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CanSpawnMoreActors() const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

private:
    // Internal performance tracking
    float PerformanceCheckTimer;
    TArray<AActor*> SpawnedVegetationActors;
    TArray<AActor*> WeatherMarkerActors;

    // Performance optimization helpers
    void UpdatePerformanceMetrics();
    void ManageActorLOD();
    bool IsLocationInVegetationZone(const FVector& Location, const FWorld_VegetationZone& Zone) const;
    float CalculateDistanceToPlayer(const FVector& ActorLocation) const;
};