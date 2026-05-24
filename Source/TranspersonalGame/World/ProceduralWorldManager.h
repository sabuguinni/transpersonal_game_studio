#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "WorldGenerationTypes.h"
#include "ProceduralWorldManager.generated.h"

class UPCGComponent;
class ALandscape;
class UMaterialInterface;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProceduralWorldManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UProceduralWorldManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World Generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorldChunk(FVector ChunkLocation, FVector ChunkSize);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadWorldChunk(FVector ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_TerrainChunk GetChunkData(FVector WorldLocation);

    // Biome System
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeParameters GetBiomeParameters(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeParameters(EWorld_BiomeType BiomeType, const FWorld_BiomeParameters& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EWorld_BiomeType> GetBiomesInRadius(FVector CenterLocation, float Radius);

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherState GetCurrentWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(const FWorld_WeatherState& NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWorld_WeatherType NewWeather, float TransitionDuration);

    // Day/Night Cycle
    UFUNCTION(BlueprintCallable, Category = "Time")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Time")
    FWorld_DayNightCycle GetCurrentTimeState();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDayDuration(float NewDuration);

    // Streaming and LOD
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateWorldStreaming(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetStreamingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    int32 GetLoadedChunkCount();

protected:
    // Core world parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    int32 MaxLoadedChunks;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TMap<EWorld_BiomeType, FWorld_BiomeParameters> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    float BiomeBlendDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    float BiomeNoiseScale;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    TArray<EWorld_WeatherType> WeatherSequence;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    float WeatherChangeInterval;

    // Day/Night cycle
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
    FWorld_DayNightCycle DayNightState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
    bool bEnableDayNightCycle;

    // Runtime data
    UPROPERTY()
    TMap<FVector, FWorld_TerrainChunk> LoadedChunks;

    UPROPERTY()
    TArray<UPCGComponent*> ActivePCGComponents;

    UPROPERTY()
    ALandscape* MainLandscape;

private:
    // Internal generation methods
    void GenerateTerrainChunk(const FWorld_TerrainChunk& ChunkData);
    void ApplyBiomeToChunk(const FWorld_TerrainChunk& ChunkData);
    void UpdateChunkLOD(const FWorld_TerrainChunk& ChunkData, float DistanceToPlayer);
    
    // Noise generation
    float GenerateHeightNoise(float X, float Y, EWorld_BiomeType BiomeType);
    float GenerateBiomeNoise(float X, float Y);
    float GenerateWeatherNoise(float X, float Y, float Time);
    
    // Utility methods
    FVector WorldLocationToChunkCoordinate(FVector WorldLocation);
    FVector ChunkCoordinateToWorldLocation(FVector ChunkCoordinate);
    float CalculateDistanceToPlayer(FVector ChunkLocation);
    
    // Weather transition
    float WeatherTransitionTimer;
    EWorld_WeatherType TargetWeather;
    bool bIsTransitioningWeather;
};