// BiomeManager.cpp — Biome management system for prehistoric survival game
// Agent #3 — Core Systems Programmer | Transpersonal Game Studio

#include "Dinosaurs/BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Update every 5 seconds for performance

    CurrentBiomeType = ECore_BiomeType::Grassland;
    CurrentTemperature = 25.0f;
    CurrentHumidity = 0.5f;
    CurrentWindSpeed = 2.0f;
    TimeOfDay = 12.0f; // Noon default
    bDayNightCycleActive = true;
    DayDurationSeconds = 1200.0f; // 20 real minutes = 1 game day
    CurrentWeatherType = ECore_WeatherType::Clear;
    WeatherTransitionTime = 0.0f;
    WeatherTransitionDuration = 30.0f;
    bIsTransitioningWeather = false;

    // Initialize biome data table
    InitializeBiomeData();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Start weather update timer
    GetWorldTimerManager().SetTimer(
        WeatherUpdateTimer,
        this,
        &ABiomeManager::UpdateWeatherSystem,
        60.0f, // Check weather change every 60 seconds
        true
    );

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with biome %d"), (int32)CurrentBiomeType);
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Day/night cycle
    if (bDayNightCycleActive)
    {
        AdvanceDayNightCycle(DeltaTime);
    }

    // Weather transition
    if (bIsTransitioningWeather)
    {
        WeatherTransitionTime += DeltaTime;
        float Alpha = FMath::Clamp(WeatherTransitionTime / WeatherTransitionDuration, 0.0f, 1.0f);

        if (Alpha >= 1.0f)
        {
            bIsTransitioningWeather = false;
            WeatherTransitionTime = 0.0f;
            OnWeatherChanged.Broadcast(CurrentWeatherType);
        }
    }
}

void ABiomeManager::InitializeBiomeData()
{
    // Grassland
    FCore_BiomeData Grassland;
    Grassland.BiomeType = ECore_BiomeType::Grassland;
    Grassland.DisplayName = FText::FromString(TEXT("Prehistoric Grassland"));
    Grassland.BaseTemperature = 22.0f;
    Grassland.TemperatureVariance = 8.0f;
    Grassland.BaseHumidity = 0.45f;
    Grassland.HumidityVariance = 0.15f;
    Grassland.FoliageDensity = 0.6f;
    Grassland.DangerLevel = 0.4f;
    Grassland.bCanSnow = false;
    Grassland.bHasWater = true;
    BiomeDataMap.Add(ECore_BiomeType::Grassland, Grassland);

    // Forest
    FCore_BiomeData Forest;
    Forest.BiomeType = ECore_BiomeType::Forest;
    Forest.DisplayName = FText::FromString(TEXT("Prehistoric Forest"));
    Forest.BaseTemperature = 18.0f;
    Forest.TemperatureVariance = 6.0f;
    Forest.BaseHumidity = 0.75f;
    Forest.HumidityVariance = 0.1f;
    Forest.FoliageDensity = 0.95f;
    Forest.DangerLevel = 0.6f;
    Forest.bCanSnow = false;
    Forest.bHasWater = true;
    BiomeDataMap.Add(ECore_BiomeType::Forest, Forest);

    // Desert
    FCore_BiomeData Desert;
    Desert.BiomeType = ECore_BiomeType::Desert;
    Desert.DisplayName = FText::FromString(TEXT("Arid Badlands"));
    Desert.BaseTemperature = 38.0f;
    Desert.TemperatureVariance = 15.0f;
    Desert.BaseHumidity = 0.1f;
    Desert.HumidityVariance = 0.05f;
    Desert.FoliageDensity = 0.1f;
    Desert.DangerLevel = 0.7f;
    Desert.bCanSnow = false;
    Desert.bHasWater = false;
    BiomeDataMap.Add(ECore_BiomeType::Desert, Desert);

    // Swamp
    FCore_BiomeData Swamp;
    Swamp.BiomeType = ECore_BiomeType::Swamp;
    Swamp.DisplayName = FText::FromString(TEXT("Primordial Swamp"));
    Swamp.BaseTemperature = 28.0f;
    Swamp.TemperatureVariance = 5.0f;
    Swamp.BaseHumidity = 0.95f;
    Swamp.HumidityVariance = 0.05f;
    Swamp.FoliageDensity = 0.8f;
    Swamp.DangerLevel = 0.8f;
    Swamp.bCanSnow = false;
    Swamp.bHasWater = true;
    BiomeDataMap.Add(ECore_BiomeType::Swamp, Swamp);

    // Mountain
    FCore_BiomeData Mountain;
    Mountain.BiomeType = ECore_BiomeType::Mountain;
    Mountain.DisplayName = FText::FromString(TEXT("Ancient Mountains"));
    Mountain.BaseTemperature = 5.0f;
    Mountain.TemperatureVariance = 12.0f;
    Mountain.BaseHumidity = 0.4f;
    Mountain.HumidityVariance = 0.2f;
    Mountain.FoliageDensity = 0.3f;
    Mountain.DangerLevel = 0.65f;
    Mountain.bCanSnow = true;
    Mountain.bHasWater = true;
    BiomeDataMap.Add(ECore_BiomeType::Mountain, Mountain);

    // Volcanic
    FCore_BiomeData Volcanic;
    Volcanic.BiomeType = ECore_BiomeType::Volcanic;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Wasteland"));
    Volcanic.BaseTemperature = 55.0f;
    Volcanic.TemperatureVariance = 20.0f;
    Volcanic.BaseHumidity = 0.2f;
    Volcanic.HumidityVariance = 0.1f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.DangerLevel = 0.95f;
    Volcanic.bCanSnow = false;
    Volcanic.bHasWater = false;
    BiomeDataMap.Add(ECore_BiomeType::Volcanic, Volcanic);

    // Coastal
    FCore_BiomeData Coastal;
    Coastal.BiomeType = ECore_BiomeType::Coastal;
    Coastal.DisplayName = FText::FromString(TEXT("Prehistoric Coast"));
    Coastal.BaseTemperature = 24.0f;
    Coastal.TemperatureVariance = 7.0f;
    Coastal.BaseHumidity = 0.7f;
    Coastal.HumidityVariance = 0.15f;
    Coastal.FoliageDensity = 0.5f;
    Coastal.DangerLevel = 0.5f;
    Coastal.bCanSnow = false;
    Coastal.bHasWater = true;
    BiomeDataMap.Add(ECore_BiomeType::Coastal, Coastal);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome types"), BiomeDataMap.Num());
}

ECore_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple height-based biome determination
    // In a full implementation this would sample a biome map texture
    float Height = WorldLocation.Z;

    if (Height > 8000.0f)
    {
        return ECore_BiomeType::Mountain;
    }
    else if (Height > 5000.0f)
    {
        return ECore_BiomeType::Forest;
    }
    else if (Height < -500.0f)
    {
        return ECore_BiomeType::Swamp;
    }

    // Use noise-like XY position for variety
    float NoiseVal = FMath::PerlinNoise2D(FVector2D(WorldLocation.X * 0.0001f, WorldLocation.Y * 0.0001f));

    if (NoiseVal > 0.6f)
        return ECore_BiomeType::Desert;
    else if (NoiseVal > 0.2f)
        return ECore_BiomeType::Grassland;
    else if (NoiseVal > -0.2f)
        return ECore_BiomeType::Forest;
    else if (NoiseVal > -0.6f)
        return ECore_BiomeType::Coastal;
    else
        return ECore_BiomeType::Swamp;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    ECore_BiomeType BiomeAtLoc = GetBiomeAtLocation(WorldLocation);

    if (const FCore_BiomeData* Data = BiomeDataMap.Find(BiomeAtLoc))
    {
        // Base temperature + time of day modifier
        float TimeModifier = FMath::Sin((TimeOfDay / 24.0f) * PI * 2.0f - PI * 0.5f) * 5.0f;
        float HeightModifier = -WorldLocation.Z * 0.001f; // Colder at altitude
        return Data->BaseTemperature + TimeModifier + HeightModifier;
    }

    return CurrentTemperature;
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    ECore_BiomeType BiomeAtLoc = GetBiomeAtLocation(WorldLocation);

    if (const FCore_BiomeData* Data = BiomeDataMap.Find(BiomeAtLoc))
    {
        return Data->BaseHumidity;
    }

    return CurrentHumidity;
}

bool ABiomeManager::IsLocationDangerous(const FVector& WorldLocation) const
{
    ECore_BiomeType BiomeAtLoc = GetBiomeAtLocation(WorldLocation);

    if (const FCore_BiomeData* Data = BiomeDataMap.Find(BiomeAtLoc))
    {
        return Data->DangerLevel > 0.7f;
    }

    return false;
}

FCore_BiomeData ABiomeManager::GetBiomeData(ECore_BiomeType BiomeType) const
{
    if (const FCore_BiomeData* Data = BiomeDataMap.Find(BiomeType))
    {
        return *Data;
    }

    return FCore_BiomeData();
}

void ABiomeManager::SetCurrentBiome(ECore_BiomeType NewBiome)
{
    if (CurrentBiomeType != NewBiome)
    {
        ECore_BiomeType OldBiome = CurrentBiomeType;
        CurrentBiomeType = NewBiome;

        // Update environmental conditions
        if (const FCore_BiomeData* Data = BiomeDataMap.Find(NewBiome))
        {
            CurrentTemperature = Data->BaseTemperature;
            CurrentHumidity = Data->BaseHumidity;
        }

        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Biome changed from %d to %d"), (int32)OldBiome, (int32)NewBiome);
    }
}

void ABiomeManager::AdvanceDayNightCycle(float DeltaTime)
{
    // Advance time of day
    float TimeAdvance = (DeltaTime / DayDurationSeconds) * 24.0f;
    TimeOfDay = FMath::Fmod(TimeOfDay + TimeAdvance, 24.0f);

    // Update directional light based on time
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
        if (DirLight)
        {
            // Map time of day to sun angle
            // 6am = sunrise (pitch = 0), 12pm = noon (pitch = -90), 18pm = sunset (pitch = 0), midnight = -180
            float SunAngle = ((TimeOfDay - 6.0f) / 24.0f) * 360.0f - 90.0f;
            FRotator CurrentRot = DirLight->GetActorRotation();
            DirLight->SetActorRotation(FRotator(SunAngle, CurrentRot.Yaw, CurrentRot.Roll));
            break; // Only update first directional light
        }
    }

    // Broadcast time change every game hour
    static float LastBroadcastHour = -1.0f;
    float CurrentHour = FMath::FloorToFloat(TimeOfDay);
    if (CurrentHour != LastBroadcastHour)
    {
        LastBroadcastHour = CurrentHour;
        OnTimeOfDayChanged.Broadcast(TimeOfDay);
    }
}

void ABiomeManager::UpdateWeatherSystem()
{
    if (bIsTransitioningWeather) return;

    // Random weather change based on biome
    float ChangeChance = 0.2f; // 20% chance per minute

    if (const FCore_BiomeData* Data = BiomeDataMap.Find(CurrentBiomeType))
    {
        ChangeChance *= Data->BaseHumidity; // Wetter biomes have more weather changes
    }

    if (FMath::FRand() < ChangeChance)
    {
        // Pick new weather
        TArray<ECore_WeatherType> PossibleWeather;
        PossibleWeather.Add(ECore_WeatherType::Clear);
        PossibleWeather.Add(ECore_WeatherType::Cloudy);
        PossibleWeather.Add(ECore_WeatherType::Rain);

        if (const FCore_BiomeData* Data = BiomeDataMap.Find(CurrentBiomeType))
        {
            if (Data->bCanSnow && CurrentTemperature < 2.0f)
            {
                PossibleWeather.Add(ECore_WeatherType::Snow);
            }
        }

        if (CurrentWeatherType == ECore_WeatherType::Rain || CurrentWeatherType == ECore_WeatherType::Cloudy)
        {
            PossibleWeather.Add(ECore_WeatherType::Storm);
        }

        int32 NewWeatherIdx = FMath::RandRange(0, PossibleWeather.Num() - 1);
        ECore_WeatherType NewWeather = PossibleWeather[NewWeatherIdx];

        if (NewWeather != CurrentWeatherType)
        {
            ECore_WeatherType OldWeather = CurrentWeatherType;
            CurrentWeatherType = NewWeather;
            bIsTransitioningWeather = true;
            WeatherTransitionTime = 0.0f;

            UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather changing from %d to %d"), (int32)OldWeather, (int32)NewWeather);
        }
    }
}

float ABiomeManager::GetTimeOfDay() const
{
    return TimeOfDay;
}

bool ABiomeManager::IsNightTime() const
{
    return TimeOfDay < 6.0f || TimeOfDay > 20.0f;
}

ECore_WeatherType ABiomeManager::GetCurrentWeather() const
{
    return CurrentWeatherType;
}

void ABiomeManager::ForceWeatherChange(ECore_WeatherType NewWeather)
{
    if (CurrentWeatherType != NewWeather)
    {
        CurrentWeatherType = NewWeather;
        bIsTransitioningWeather = true;
        WeatherTransitionTime = 0.0f;
        OnWeatherChanged.Broadcast(NewWeather);
    }
}
