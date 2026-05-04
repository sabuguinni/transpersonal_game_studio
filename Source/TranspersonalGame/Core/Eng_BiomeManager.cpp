#include "Eng_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AEng_BiomeManager::AEng_BiomeManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f; // Update every second
	
	WeatherUpdateInterval = 30.0f; // Update weather every 30 seconds
	WeatherUpdateTimer = 0.0f;
	
	// Set actor properties
	SetActorTickEnabled(true);
	bReplicates = true;
}

void AEng_BiomeManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize biome system
	InitializeBiomeBoundaries();
	
	// Set initial weather states
	CurrentWeatherStates.Add(EBiomeType::Swamp, EWeatherType::Foggy);
	CurrentWeatherStates.Add(EBiomeType::Forest, EWeatherType::Overcast);
	CurrentWeatherStates.Add(EBiomeType::Savanna, EWeatherType::Clear);
	CurrentWeatherStates.Add(EBiomeType::Desert, EWeatherType::Clear);
	CurrentWeatherStates.Add(EBiomeType::Mountain, EWeatherType::Snowy);
	
	// Initialize biome settings
	FBiomeData SwampData;
	SwampData.Temperature = 25.0f;
	SwampData.Humidity = 90.0f;
	SwampData.WindStrength = 0.2f;
	BiomeSettings.Add(EBiomeType::Swamp, SwampData);
	
	FBiomeData ForestData;
	ForestData.Temperature = 20.0f;
	ForestData.Humidity = 70.0f;
	ForestData.WindStrength = 0.3f;
	BiomeSettings.Add(EBiomeType::Forest, ForestData);
	
	FBiomeData SavannaData;
	SavannaData.Temperature = 30.0f;
	SavannaData.Humidity = 40.0f;
	SavannaData.WindStrength = 0.5f;
	BiomeSettings.Add(EBiomeType::Savanna, SavannaData);
	
	FBiomeData DesertData;
	DesertData.Temperature = 40.0f;
	DesertData.Humidity = 10.0f;
	DesertData.WindStrength = 0.7f;
	BiomeSettings.Add(EBiomeType::Desert, DesertData);
	
	FBiomeData MountainData;
	MountainData.Temperature = -5.0f;
	MountainData.Humidity = 60.0f;
	MountainData.WindStrength = 1.0f;
	BiomeSettings.Add(EBiomeType::Mountain, MountainData);
	
	if (ValidateBiomeConfiguration())
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Successfully initialized all 5 biomes"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BiomeManager: Configuration validation failed"));
	}
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateWeatherSystems(DeltaTime);
}

EBiomeType AEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
	// Check each biome boundary
	for (const auto& BiomePair : BiomeBoundaries)
	{
		if (BiomePair.Value.IsInside(Location))
		{
			return BiomePair.Key;
		}
	}
	
	// Default to Savanna if outside all boundaries
	return EBiomeType::Savanna;
}

bool AEng_BiomeManager::IsValidSpawnLocationForBiome(const FVector& Location, EBiomeType RequiredBiome) const
{
	EBiomeType LocationBiome = GetBiomeAtLocation(Location);
	return LocationBiome == RequiredBiome;
}

FVector AEng_BiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
	if (!BiomeBoundaries.Contains(BiomeType))
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Invalid biome type requested"));
		return FVector::ZeroVector;
	}
	
	const FBox& BiomeBounds = BiomeBoundaries[BiomeType];
	
	float RandomX = FMath::RandRange(BiomeBounds.Min.X, BiomeBounds.Max.X);
	float RandomY = FMath::RandRange(BiomeBounds.Min.Y, BiomeBounds.Max.Y);
	float RandomZ = FMath::RandRange(BiomeBounds.Min.Z, BiomeBounds.Max.Z);
	
	return FVector(RandomX, RandomY, RandomZ);
}

void AEng_BiomeManager::SetWeatherForBiome(EBiomeType BiomeType, EWeatherType WeatherType)
{
	CurrentWeatherStates.Add(BiomeType, WeatherType);
	
	UE_LOG(LogTemp, Log, TEXT("BiomeManager: Set weather for biome %d to %d"), 
		   (int32)BiomeType, (int32)WeatherType);
}

EWeatherType AEng_BiomeManager::GetCurrentWeather(EBiomeType BiomeType) const
{
	if (CurrentWeatherStates.Contains(BiomeType))
	{
		return CurrentWeatherStates[BiomeType];
	}
	
	return EWeatherType::Clear; // Default weather
}

float AEng_BiomeManager::GetBiomeTemperature(EBiomeType BiomeType) const
{
	if (BiomeSettings.Contains(BiomeType))
	{
		return BiomeSettings[BiomeType].Temperature;
	}
	
	return 20.0f; // Default temperature
}

float AEng_BiomeManager::GetBiomeHumidity(EBiomeType BiomeType) const
{
	if (BiomeSettings.Contains(BiomeType))
	{
		return BiomeSettings[BiomeType].Humidity;
	}
	
	return 50.0f; // Default humidity
}

void AEng_BiomeManager::InitializeBiomeBoundaries()
{
	// Based on brain memory coordinates - 157,000 x 153,000 UU map
	// X range: -77,500 to +79,500 | Y range: -76,500 to +76,500
	
	// Swamp (southwest)
	FBox SwampBounds(FVector(-77500, -76500, -100), FVector(-25000, -15000, 1000));
	BiomeBoundaries.Add(EBiomeType::Swamp, SwampBounds);
	
	// Forest (northwest) 
	FBox ForestBounds(FVector(-77500, 15000, -100), FVector(-15000, 76500, 1000));
	BiomeBoundaries.Add(EBiomeType::Forest, ForestBounds);
	
	// Savanna (center)
	FBox SavannaBounds(FVector(-20000, -20000, -100), FVector(20000, 20000, 1000));
	BiomeBoundaries.Add(EBiomeType::Savanna, SavannaBounds);
	
	// Desert (east)
	FBox DesertBounds(FVector(25000, -30000, -100), FVector(79500, 30000, 1000));
	BiomeBoundaries.Add(EBiomeType::Desert, DesertBounds);
	
	// Mountain (northeast)
	FBox MountainBounds(FVector(15000, 20000, 0), FVector(79500, 76500, 2000));
	BiomeBoundaries.Add(EBiomeType::Mountain, MountainBounds);
	
	UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d biome boundaries"), BiomeBoundaries.Num());
}

void AEng_BiomeManager::UpdateWeatherSystems(float DeltaTime)
{
	WeatherUpdateTimer += DeltaTime;
	
	if (WeatherUpdateTimer >= WeatherUpdateInterval)
	{
		WeatherUpdateTimer = 0.0f;
		
		// Simple weather cycling logic
		for (auto& WeatherPair : CurrentWeatherStates)
		{
			EBiomeType BiomeType = WeatherPair.Key;
			EWeatherType CurrentWeather = WeatherPair.Value;
			
			// Random weather change (20% chance)
			if (FMath::RandRange(0.0f, 1.0f) < 0.2f)
			{
				// Cycle to next weather type
				int32 WeatherIndex = (int32)CurrentWeather;
				WeatherIndex = (WeatherIndex + 1) % 6; // 6 weather types
				EWeatherType NewWeather = (EWeatherType)WeatherIndex;
				
				CurrentWeatherStates[BiomeType] = NewWeather;
				
				UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather changed in biome %d to %d"), 
					   (int32)BiomeType, (int32)NewWeather);
			}
		}
	}
}

bool AEng_BiomeManager::ValidateBiomeConfiguration() const
{
	// Check that all 5 biomes are configured
	if (BiomeBoundaries.Num() != 5 || BiomeSettings.Num() != 5 || CurrentWeatherStates.Num() != 5)
	{
		UE_LOG(LogTemp, Error, TEXT("BiomeManager: Missing biome configurations"));
		return false;
	}
	
	// Validate each biome has valid bounds
	for (const auto& BiomePair : BiomeBoundaries)
	{
		const FBox& Bounds = BiomePair.Value;
		if (!Bounds.IsValid || Bounds.GetSize().SizeSquared() < 1000000.0f) // Minimum 1km²
		{
			UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid bounds for biome %d"), (int32)BiomePair.Key);
			return false;
		}
	}
	
	return true;
}