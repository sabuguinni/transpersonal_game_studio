#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "WorldGenerationTypes.generated.h"

// World Generation Enums and Structs

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Grassland       UMETA(DisplayName = "Grassland"),
    Forest          UMETA(DisplayName = "Forest"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Desert          UMETA(DisplayName = "Desert"),
    Wetland         UMETA(DisplayName = "Wetland"),
    Tundra          UMETA(DisplayName = "Tundra"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Plateau         UMETA(DisplayName = "Plateau"),
    Valley          UMETA(DisplayName = "Valley")
};

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Flat            UMETA(DisplayName = "Flat"),
    Rolling         UMETA(DisplayName = "Rolling Hills"),
    Mountainous     UMETA(DisplayName = "Mountainous"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Plateau         UMETA(DisplayName = "Plateau"),
    Valley          UMETA(DisplayName = "Valley"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Island          UMETA(DisplayName = "Island")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Cloudy          UMETA(DisplayName = "Cloudy"),
    Rainy           UMETA(DisplayName = "Rainy"),
    Stormy          UMETA(DisplayName = "Stormy"),
    Foggy           UMETA(DisplayName = "Foggy"),
    Snowy           UMETA(DisplayName = "Snowy"),
    Windy           UMETA(DisplayName = "Windy"),
    Misty           UMETA(DisplayName = "Misty")
};

UENUM(BlueprintType)
enum class EWorld_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Noon            UMETA(DisplayName = "Noon"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night"),
    Midnight        UMETA(DisplayName = "Midnight"),
    PreDawn         UMETA(DisplayName = "Pre-Dawn")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> NativeSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FWorld_BiomeParameters()
    {
        BiomeType = EWorld_BiomeType::Grassland;
        ElevationMin = 0.0f;
        ElevationMax = 200.0f;
        VegetationDensity = 0.7f;
        WaterProximity = 0.3f;
        Temperature = 0.6f;
        Humidity = 0.5f;
        WindStrength = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType PrimaryBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<EWorld_BiomeType> SecondaryBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsGenerated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsLoaded;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(1000.0f, 1000.0f, 1000.0f);
        TerrainType = EWorld_TerrainType::Flat;
        PrimaryBiome = EWorld_BiomeType::Grassland;
        HeightScale = 100.0f;
        NoiseScale = 0.01f;
        bIsGenerated = false;
        bIsLoaded = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Precipitation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility;

    FWorld_WeatherState()
    {
        CurrentWeather = EWorld_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
        TransitionTime = 60.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindSpeed = 5.0f;
        Precipitation = 0.0f;
        CloudCoverage = 0.3f;
        Visibility = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EWorld_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDayFloat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float MoonPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor MoonColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SkyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float StarVisibility;

    FWorld_DayNightCycle()
    {
        CurrentTimeOfDay = EWorld_TimeOfDay::Noon;
        TimeOfDayFloat = 0.5f;
        DayDuration = 1200.0f; // 20 minutes real time
        SunAngle = 45.0f;
        MoonPhase = 0.5f;
        SunColor = FLinearColor::Yellow;
        MoonColor = FLinearColor::White;
        SkyColor = FLinearColor::Blue;
        StarVisibility = 0.0f;
    }
};