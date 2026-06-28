// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Manages biome zones, transitions, and environmental conditions for the prehistoric world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// ============================================================
// UBiomeDataAsset
// ============================================================

UBiomeDataAsset::UBiomeDataAsset()
{
    BiomeName = TEXT("Unknown Biome");
    BiomeType = EBiomeType::Forest;
    AmbientTemperature = 25.f;
    Humidity = 0.5f;
    FoliageDensity = 0.7f;
    DangerLevel = 1;
    bHasWater = false;
    FogDensity = 0.02f;
    WindIntensity = 0.3f;
}

// ============================================================
// ABiomeZone
// ============================================================

ABiomeZone::ABiomeZone()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    ZoneRadius = 5000.f;
    BiomeType = EBiomeType::Forest;
    BiomeData = nullptr;

#if WITH_EDITORONLY_DATA
    // Billboard for editor visibility
    UBillboardComponent* Billboard = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    if (Billboard)
    {
        Billboard->SetupAttachment(SceneRoot);
    }
#endif
}

void ABiomeZone::BeginPlay()
{
    Super::BeginPlay();
}

bool ABiomeZone::IsLocationInZone(const FVector& WorldLocation) const
{
    float Dist2D = FVector::Dist2D(WorldLocation, GetActorLocation());
    return Dist2D <= ZoneRadius;
}

float ABiomeZone::GetBlendWeight(const FVector& WorldLocation) const
{
    float Dist2D = FVector::Dist2D(WorldLocation, GetActorLocation());
    if (Dist2D >= ZoneRadius) return 0.f;

    // Smooth falloff at zone edges (20% blend zone)
    float BlendStart = ZoneRadius * 0.8f;
    if (Dist2D <= BlendStart) return 1.f;

    float BlendRange = ZoneRadius - BlendStart;
    float DistInBlend = Dist2D - BlendStart;
    return 1.f - (DistInBlend / BlendRange);
}

// ============================================================
// ABiomeManager
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Default time of day — midday
    CurrentTimeOfDay = 12.f;
    DayDurationSeconds = 1200.f; // 20 real minutes = 1 game day
    bDayNightCycleEnabled = true;
    GlobalTemperatureModifier = 1.0f;
    GlobalHumidityModifier = 1.0f;
    CurrentWeather = EWeatherType::Clear;
    WeatherTransitionProgress = 0.f;
    bIsTransitioningWeather = false;

    bReplicates = true;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Discover all BiomeZones in the world
    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABiomeZone::StaticClass(), FoundZones);
    for (AActor* ZoneActor : FoundZones)
    {
        if (ABiomeZone* Zone = Cast<ABiomeZone>(ZoneActor))
        {
            RegisteredBiomeZones.Add(Zone);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Discovered %d biome zones"), RegisteredBiomeZones.Num());

    // Start weather update timer
    GetWorldTimerManager().SetTimer(
        WeatherUpdateTimer,
        this,
        &ABiomeManager::UpdateWeather,
        60.f,
        true
    );
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayNightCycleEnabled)
    {
        AdvanceTimeOfDay(DeltaTime);
    }

    if (bIsTransitioningWeather)
    {
        WeatherTransitionProgress += DeltaTime / 30.f; // 30s transition
        if (WeatherTransitionProgress >= 1.f)
        {
            WeatherTransitionProgress = 1.f;
            bIsTransitioningWeather = false;
            CurrentWeather = TargetWeather;
        }
    }
}

void ABiomeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABiomeManager, CurrentTimeOfDay);
    DOREPLIFETIME(ABiomeManager, CurrentWeather);
    DOREPLIFETIME(ABiomeManager, WeatherTransitionProgress);
}

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    ABiomeZone* DominantZone = GetDominantBiomeZone(WorldLocation);
    if (DominantZone)
    {
        return DominantZone->BiomeType;
    }
    return EBiomeType::Plains; // Default biome
}

ABiomeZone* ABiomeManager::GetDominantBiomeZone(const FVector& WorldLocation) const
{
    ABiomeZone* BestZone = nullptr;
    float BestWeight = 0.f;

    for (ABiomeZone* Zone : RegisteredBiomeZones)
    {
        if (!Zone) continue;
        float Weight = Zone->GetBlendWeight(WorldLocation);
        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            BestZone = Zone;
        }
    }
    return BestZone;
}

TArray<ABiomeZone*> ABiomeManager::GetBiomeZonesAtLocation(const FVector& WorldLocation) const
{
    TArray<ABiomeZone*> Result;
    for (ABiomeZone* Zone : RegisteredBiomeZones)
    {
        if (Zone && Zone->IsLocationInZone(WorldLocation))
        {
            Result.Add(Zone);
        }
    }
    return Result;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    ABiomeZone* Zone = GetDominantBiomeZone(WorldLocation);
    float BaseTemp = 25.f;
    if (Zone && Zone->BiomeData)
    {
        BaseTemp = Zone->BiomeData->AmbientTemperature;
    }

    // Apply time-of-day temperature variation
    float TimeNormalized = CurrentTimeOfDay / 24.f;
    float TempVariation = FMath::Sin(TimeNormalized * PI * 2.f - PI * 0.5f) * 10.f;

    return (BaseTemp + TempVariation) * GlobalTemperatureModifier;
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    ABiomeZone* Zone = GetDominantBiomeZone(WorldLocation);
    float BaseHumidity = 0.5f;
    if (Zone && Zone->BiomeData)
    {
        BaseHumidity = Zone->BiomeData->Humidity;
    }

    // Rain increases humidity
    if (CurrentWeather == EWeatherType::Rain || CurrentWeather == EWeatherType::Storm)
    {
        BaseHumidity = FMath::Min(1.f, BaseHumidity + 0.3f);
    }

    return BaseHumidity * GlobalHumidityModifier;
}

void ABiomeManager::SetWeather(EWeatherType NewWeather, bool bImmediate)
{
    if (CurrentWeather == NewWeather) return;

    TargetWeather = NewWeather;
    if (bImmediate)
    {
        CurrentWeather = NewWeather;
        WeatherTransitionProgress = 1.f;
        bIsTransitioningWeather = false;
    }
    else
    {
        WeatherTransitionProgress = 0.f;
        bIsTransitioningWeather = true;
    }
    OnWeatherChanged(CurrentWeather, NewWeather);
}

void ABiomeManager::AdvanceTimeOfDay(float DeltaTime)
{
    float HoursPerSecond = 24.f / DayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;
    if (CurrentTimeOfDay >= 24.f)
    {
        CurrentTimeOfDay -= 24.f;
        OnNewDay();
    }
}

void ABiomeManager::UpdateWeather()
{
    if (bIsTransitioningWeather) return;

    // Random weather transitions based on current weather
    int32 Roll = FMath::RandRange(0, 9);
    EWeatherType NewWeather = CurrentWeather;

    switch (CurrentWeather)
    {
    case EWeatherType::Clear:
        if (Roll < 2) NewWeather = EWeatherType::Cloudy;
        break;
    case EWeatherType::Cloudy:
        if (Roll < 3) NewWeather = EWeatherType::Rain;
        else if (Roll < 5) NewWeather = EWeatherType::Clear;
        break;
    case EWeatherType::Rain:
        if (Roll < 2) NewWeather = EWeatherType::Storm;
        else if (Roll < 4) NewWeather = EWeatherType::Cloudy;
        break;
    case EWeatherType::Storm:
        if (Roll < 4) NewWeather = EWeatherType::Rain;
        break;
    case EWeatherType::Fog:
        if (Roll < 5) NewWeather = EWeatherType::Clear;
        break;
    default:
        break;
    }

    if (NewWeather != CurrentWeather)
    {
        SetWeather(NewWeather, false);
    }
}

void ABiomeManager::OnWeatherChanged_Implementation(EWeatherType OldWeather, EWeatherType NewWeather)
{
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather changed from %d to %d"), (int32)OldWeather, (int32)NewWeather);
}

void ABiomeManager::OnNewDay_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: New day started. Time: %.1f"), CurrentTimeOfDay);
}

bool ABiomeManager::IsNightTime() const
{
    return CurrentTimeOfDay < 6.f || CurrentTimeOfDay > 20.f;
}

float ABiomeManager::GetDayProgress() const
{
    return CurrentTimeOfDay / 24.f;
}

FString ABiomeManager::GetTimeOfDayString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}
