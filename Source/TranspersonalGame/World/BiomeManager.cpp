// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Manages biome detection, transitions, and environmental parameters
// for the prehistoric survival world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

UBiomeManager::UBiomeManager()
{
    // Default biome parameters
    CurrentBiome = EBiomeType::Forest;
    PreviousBiome = EBiomeType::Forest;
    BiomeTransitionAlpha = 1.f;
    bIsTransitioning = false;
    TransitionDuration = 5.f;
    BiomeCheckInterval = 2.f;

    // Default weather
    CurrentWeather = EWeatherType::Clear;
    WeatherIntensity = 0.f;
    TemperatureCelsius = 28.f;
    HumidityPercent = 60.f;
    WindSpeed = 5.f;
    WindDirection = FVector(1.f, 0.f, 0.f);

    // Day/night
    TimeOfDay = 12.f;
    DayLengthMinutes = 20.f;
    bIsDaytime = true;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Start biome check timer
    GetWorld()->GetTimerManager().SetTimer(
        BiomeCheckTimerHandle,
        this,
        &UBiomeManager::UpdateBiomeForPlayerPosition,
        BiomeCheckInterval,
        true
    );

    // Start day/night cycle timer (updates every real second)
    GetWorld()->GetTimerManager().SetTimer(
        DayNightTimerHandle,
        this,
        &UBiomeManager::AdvanceDayNightCycle,
        1.f,
        true
    );

    // Start weather change timer (every 5 real minutes)
    GetWorld()->GetTimerManager().SetTimer(
        WeatherTimerHandle,
        this,
        &UBiomeManager::RandomizeWeather,
        300.f,
        true
    );

    // Initialize biome data table
    InitializeBiomeData();

    UE_LOG(LogTemp, Log, TEXT("BiomeManager initialized — prehistoric world active"));
}

void UBiomeManager::Deinitialize()
{
    GetWorld()->GetTimerManager().ClearTimer(BiomeCheckTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(DayNightTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(WeatherTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TransitionTimerHandle);

    Super::Deinitialize();
}

void UBiomeManager::InitializeBiomeData()
{
    // Forest biome
    FBiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.DisplayName = TEXT("Cretaceous Forest");
    ForestData.BaseTemperature = 22.f;
    ForestData.BaseHumidity = 75.f;
    ForestData.TreeDensity = 0.8f;
    ForestData.GrassDensity = 0.6f;
    ForestData.RockDensity = 0.2f;
    ForestData.FogDensity = 0.15f;
    ForestData.AmbientColorTint = FLinearColor(0.2f, 0.5f, 0.1f, 1.f);
    ForestData.CommonDinoSpecies.Add(EDinoSpecies::Raptor);
    ForestData.CommonDinoSpecies.Add(EDinoSpecies::Stegosaurus);
    BiomeDataMap.Add(EBiomeType::Forest, ForestData);

    // Plains biome
    FBiomeData PlainsData;
    PlainsData.BiomeType = EBiomeType::Plains;
    PlainsData.DisplayName = TEXT("Open Savanna");
    PlainsData.BaseTemperature = 32.f;
    PlainsData.BaseHumidity = 40.f;
    PlainsData.TreeDensity = 0.1f;
    PlainsData.GrassDensity = 0.9f;
    PlainsData.RockDensity = 0.1f;
    PlainsData.FogDensity = 0.05f;
    PlainsData.AmbientColorTint = FLinearColor(0.6f, 0.5f, 0.1f, 1.f);
    PlainsData.CommonDinoSpecies.Add(EDinoSpecies::Brachiosaurus);
    PlainsData.CommonDinoSpecies.Add(EDinoSpecies::Triceratops);
    BiomeDataMap.Add(EBiomeType::Plains, PlainsData);

    // Swamp biome
    FBiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.DisplayName = TEXT("Primordial Swamp");
    SwampData.BaseTemperature = 28.f;
    SwampData.BaseHumidity = 95.f;
    SwampData.TreeDensity = 0.5f;
    SwampData.GrassDensity = 0.4f;
    SwampData.RockDensity = 0.05f;
    SwampData.FogDensity = 0.6f;
    SwampData.AmbientColorTint = FLinearColor(0.1f, 0.3f, 0.15f, 1.f);
    SwampData.CommonDinoSpecies.Add(EDinoSpecies::TRex);
    BiomeDataMap.Add(EBiomeType::Swamp, SwampData);

    // Desert biome
    FBiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.DisplayName = TEXT("Arid Badlands");
    DesertData.BaseTemperature = 45.f;
    DesertData.BaseHumidity = 10.f;
    DesertData.TreeDensity = 0.02f;
    DesertData.GrassDensity = 0.05f;
    DesertData.RockDensity = 0.7f;
    DesertData.FogDensity = 0.02f;
    DesertData.AmbientColorTint = FLinearColor(0.8f, 0.6f, 0.2f, 1.f);
    DesertData.CommonDinoSpecies.Add(EDinoSpecies::Pterodactyl);
    BiomeDataMap.Add(EBiomeType::Desert, DesertData);

    // Volcanic biome
    FBiomeData VolcanicData;
    VolcanicData.BiomeType = EBiomeType::Volcanic;
    VolcanicData.DisplayName = TEXT("Volcanic Highlands");
    VolcanicData.BaseTemperature = 55.f;
    VolcanicData.BaseHumidity = 30.f;
    VolcanicData.TreeDensity = 0.05f;
    VolcanicData.GrassDensity = 0.02f;
    VolcanicData.RockDensity = 0.9f;
    VolcanicData.FogDensity = 0.4f;
    VolcanicData.AmbientColorTint = FLinearColor(0.7f, 0.2f, 0.05f, 1.f);
    VolcanicData.CommonDinoSpecies.Add(EDinoSpecies::TRex);
    BiomeDataMap.Add(EBiomeType::Volcanic, VolcanicData);

    // River biome
    FBiomeData RiverData;
    RiverData.BiomeType = EBiomeType::River;
    RiverData.DisplayName = TEXT("River Delta");
    RiverData.BaseTemperature = 24.f;
    RiverData.BaseHumidity = 85.f;
    RiverData.TreeDensity = 0.6f;
    RiverData.GrassDensity = 0.7f;
    RiverData.RockDensity = 0.3f;
    RiverData.FogDensity = 0.2f;
    RiverData.AmbientColorTint = FLinearColor(0.1f, 0.4f, 0.5f, 1.f);
    RiverData.CommonDinoSpecies.Add(EDinoSpecies::Brachiosaurus);
    RiverData.CommonDinoSpecies.Add(EDinoSpecies::Stegosaurus);
    BiomeDataMap.Add(EBiomeType::River, RiverData);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: %d biomes initialized"), BiomeDataMap.Num());
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Biome determination based on world position using noise-like logic
    // In production this would sample a biome map texture or PCG data
    float X = WorldLocation.X / 10000.f;
    float Y = WorldLocation.Y / 10000.f;
    float Z = WorldLocation.Z;

    // Altitude-based biome selection
    if (Z > 3000.f)
    {
        return EBiomeType::Volcanic;
    }

    // Simple grid-based biome zones for prototype
    int32 GridX = FMath::FloorToInt(X);
    int32 GridY = FMath::FloorToInt(Y);
    int32 BiomeIndex = FMath::Abs(GridX * 7 + GridY * 13) % 6;

    switch (BiomeIndex)
    {
    case 0: return EBiomeType::Forest;
    case 1: return EBiomeType::Plains;
    case 2: return EBiomeType::Swamp;
    case 3: return EBiomeType::Desert;
    case 4: return EBiomeType::Volcanic;
    case 5: return EBiomeType::River;
    default: return EBiomeType::Forest;
    }
}

void UBiomeManager::UpdateBiomeForPlayerPosition()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);

    if (NewBiome != CurrentBiome)
    {
        TransitionToBiome(NewBiome);
    }

    // Update environmental conditions
    UpdateEnvironmentalConditions(PlayerLocation);
}

void UBiomeManager::TransitionToBiome(EBiomeType NewBiome)
{
    if (bIsTransitioning) return;

    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    bIsTransitioning = true;
    BiomeTransitionAlpha = 0.f;

    // Start transition timer
    GetWorld()->GetTimerManager().SetTimer(
        TransitionTimerHandle,
        this,
        &UBiomeManager::TickBiomeTransition,
        0.05f,
        true
    );

    OnBiomeChanged.Broadcast(PreviousBiome, CurrentBiome);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transitioning from %d to %d"),
        (int32)PreviousBiome, (int32)CurrentBiome);
}

void UBiomeManager::TickBiomeTransition()
{
    BiomeTransitionAlpha += 0.05f / TransitionDuration;

    if (BiomeTransitionAlpha >= 1.f)
    {
        BiomeTransitionAlpha = 1.f;
        bIsTransitioning = false;
        GetWorld()->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }
}

void UBiomeManager::UpdateEnvironmentalConditions(const FVector& PlayerLocation)
{
    const FBiomeData* BiomeData = BiomeDataMap.Find(CurrentBiome);
    if (!BiomeData) return;

    // Blend temperature with time-of-day modifier
    float TimeModifier = FMath::Sin((TimeOfDay / 24.f) * PI * 2.f - PI * 0.5f) * 8.f;
    TemperatureCelsius = BiomeData->BaseTemperature + TimeModifier;

    // Weather affects humidity
    if (CurrentWeather == EWeatherType::Rain || CurrentWeather == EWeatherType::Storm)
    {
        HumidityPercent = FMath::Min(100.f, BiomeData->BaseHumidity + 20.f);
    }
    else
    {
        HumidityPercent = BiomeData->BaseHumidity;
    }
}

void UBiomeManager::AdvanceDayNightCycle()
{
    // DayLengthMinutes = real minutes per full day cycle
    float HoursPerRealSecond = 24.f / (DayLengthMinutes * 60.f);
    TimeOfDay += HoursPerRealSecond;

    if (TimeOfDay >= 24.f)
    {
        TimeOfDay -= 24.f;
    }

    bool bWasDaytime = bIsDaytime;
    bIsDaytime = (TimeOfDay >= 6.f && TimeOfDay < 20.f);

    if (bWasDaytime != bIsDaytime)
    {
        OnDayNightChanged.Broadcast(bIsDaytime);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: %s"), bIsDaytime ? TEXT("Dawn") : TEXT("Dusk"));
    }
}

void UBiomeManager::RandomizeWeather()
{
    // Weather probability based on current biome
    float RainChance = 0.3f;

    const FBiomeData* BiomeData = BiomeDataMap.Find(CurrentBiome);
    if (BiomeData)
    {
        // High humidity biomes have more rain
        RainChance = BiomeData->BaseHumidity / 200.f;
    }

    float Roll = FMath::FRand();

    EWeatherType NewWeather;
    if (Roll < RainChance * 0.3f)
    {
        NewWeather = EWeatherType::Storm;
        WeatherIntensity = FMath::FRandRange(0.6f, 1.f);
    }
    else if (Roll < RainChance)
    {
        NewWeather = EWeatherType::Rain;
        WeatherIntensity = FMath::FRandRange(0.2f, 0.7f);
    }
    else if (Roll < RainChance + 0.15f)
    {
        NewWeather = EWeatherType::Fog;
        WeatherIntensity = FMath::FRandRange(0.3f, 0.8f);
    }
    else
    {
        NewWeather = EWeatherType::Clear;
        WeatherIntensity = 0.f;
    }

    if (NewWeather != CurrentWeather)
    {
        CurrentWeather = NewWeather;
        OnWeatherChanged.Broadcast(CurrentWeather, WeatherIntensity);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather changed to %d (intensity=%.2f)"),
            (int32)CurrentWeather, WeatherIntensity);
    }
}

FBiomeData UBiomeManager::GetCurrentBiomeData() const
{
    const FBiomeData* Data = BiomeDataMap.Find(CurrentBiome);
    if (Data)
    {
        return *Data;
    }
    return FBiomeData();
}

float UBiomeManager::GetSunAngle() const
{
    // Returns sun pitch angle based on time of day
    // 6:00 = sunrise (0 deg), 12:00 = noon (-90 deg), 20:00 = sunset (0 deg)
    if (TimeOfDay >= 6.f && TimeOfDay <= 20.f)
    {
        float DayProgress = (TimeOfDay - 6.f) / 14.f;
        return FMath::Lerp(0.f, -180.f, DayProgress);
    }
    return -10.f; // Night — below horizon
}
