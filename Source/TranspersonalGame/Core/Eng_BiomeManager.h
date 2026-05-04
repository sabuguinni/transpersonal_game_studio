#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

/**
 * Engine Architect - Core Biome Management System
 * Manages the 5 biomes: Swamp, Forest, Savanna, Desert, Mountain
 * Provides biome detection, weather control, and spawn validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
	GENERATED_BODY()

public:
	AEng_BiomeManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Biome Detection
	UFUNCTION(BlueprintCallable, Category = "Biome System")
	EBiomeType GetBiomeAtLocation(const FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Biome System")
	bool IsValidSpawnLocationForBiome(const FVector& Location, EBiomeType RequiredBiome) const;

	UFUNCTION(BlueprintCallable, Category = "Biome System")
	FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

	// Weather Control
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherForBiome(EBiomeType BiomeType, EWeatherType WeatherType);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	EWeatherType GetCurrentWeather(EBiomeType BiomeType) const;

	// Biome Properties
	UFUNCTION(BlueprintCallable, Category = "Biome System")
	float GetBiomeTemperature(EBiomeType BiomeType) const;

	UFUNCTION(BlueprintCallable, Category = "Biome System")
	float GetBiomeHumidity(EBiomeType BiomeType) const;

protected:
	// Biome Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
	TMap<EBiomeType, FBiomeData> BiomeSettings;

	// Current Weather States
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
	TMap<EBiomeType, EWeatherType> CurrentWeatherStates;

	// Biome Boundaries (hardcoded for performance)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Boundaries")
	TMap<EBiomeType, FBox> BiomeBoundaries;

	// Weather Update Timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherUpdateInterval;

	UPROPERTY()
	float WeatherUpdateTimer;

private:
	// Initialize biome boundaries based on map coordinates
	void InitializeBiomeBoundaries();

	// Update weather systems
	void UpdateWeatherSystems(float DeltaTime);

	// Validate biome data
	bool ValidateBiomeConfiguration() const;
};