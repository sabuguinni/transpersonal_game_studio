// BiomeManager.cpp — Transpersonal Game Studio
// Agent #02 — Engine Architect — Cycle 004
// Full implementation of BiomeManager: biome classification, vegetation density, weather state

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
UBiomeManager::UBiomeManager()
{
    // Default biome parameters
    CurrentBiomeType = EEng_BiomeType::TropicalJungle;
    CurrentTemperature = 28.0f;
    CurrentHumidity = 0.85f;
    CurrentWindSpeed = 0.3f;
    CurrentWeatherState = EEng_WeatherState::Clear;
    TimeOfDay = 12.0f;
    DayDurationSeconds = 1200.0f; // 20-minute day cycle
    bDayNightCycleEnabled = true;
    VegetationDensityMultiplier = 1.0f;
    DinosaurSpawnMultiplier = 1.0f;
}

// ============================================================
// Initialize
// ============================================================
void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register default biome zones
    RegisterDefaultBiomes();
    
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with %d biome zones"), BiomeZones.Num());
}

// ============================================================
// Deinitialize
// ============================================================
void UBiomeManager::Deinitialize()
{
    BiomeZones.Empty();
    Super::Deinitialize();
}

// ============================================================
// Tick (called via GameInstance tick if registered)
// ============================================================
void UBiomeManager::Tick(float DeltaTime)
{
    if (bDayNightCycleEnabled)
    {
        UpdateDayNightCycle(DeltaTime);
    }
    
    UpdateWeatherTransition(DeltaTime);
}

// ============================================================
// GetBiomeAtLocation
// ============================================================
EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Find the closest biome zone that contains this location
    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float DistSq = FVector::DistSquared2D(WorldLocation, Zone.CenterLocation);
        if (DistSq <= (Zone.Radius * Zone.Radius))
        {
            return Zone.BiomeType;
        }
    }
    
    // Default to current biome if no zone found
    return CurrentBiomeType;
}

// ============================================================
// GetVegetationDensityAtLocation
// ============================================================
float UBiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    float BaseDensity = GetBaseDensityForBiome(Biome);
    
    // Modify by weather
    float WeatherMod = 1.0f;
    switch (CurrentWeatherState)
    {
        case EEng_WeatherState::Rain:       WeatherMod = 1.2f; break;
        case EEng_WeatherState::Storm:      WeatherMod = 0.8f; break;
        case EEng_WeatherState::Drought:    WeatherMod = 0.5f; break;
        default:                            WeatherMod = 1.0f; break;
    }
    
    return FMath::Clamp(BaseDensity * WeatherMod * VegetationDensityMultiplier, 0.0f, 1.0f);
}

// ============================================================
// GetTemperatureAtLocation
// ============================================================
float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    float BaseTemp = GetBaseTemperatureForBiome(Biome);
    
    // Modify by time of day
    float TimeModifier = FMath::Sin((TimeOfDay / 24.0f) * PI * 2.0f - PI * 0.5f) * 8.0f;
    
    return BaseTemp + TimeModifier;
}

// ============================================================
// SetWeatherState
// ============================================================
void UBiomeManager::SetWeatherState(EEng_WeatherState NewWeather)
{
    if (CurrentWeatherState != NewWeather)
    {
        EEng_WeatherState OldWeather = CurrentWeatherState;
        CurrentWeatherState = NewWeather;
        
        UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Weather changed: %d -> %d"), 
               (int32)OldWeather, (int32)NewWeather);
        
        OnWeatherChanged.Broadcast(NewWeather);
        ApplyWeatherEffects(NewWeather);
    }
}

// ============================================================
// RegisterBiomeZone
// ============================================================
void UBiomeManager::RegisterBiomeZone(const FEng_BiomeZone& Zone)
{
    // Check for duplicate zone IDs
    for (const FEng_BiomeZone& Existing : BiomeZones)
    {
        if (Existing.ZoneID == Zone.ZoneID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BiomeManager] Zone ID %s already registered, skipping"), *Zone.ZoneID);
            return;
        }
    }
    
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Registered biome zone: %s (%s)"), 
           *Zone.ZoneID, *Zone.ZoneName);
}

// ============================================================
// GetDinosaurSpawnWeightForBiome
// ============================================================
float UBiomeManager::GetDinosaurSpawnWeightForBiome(EEng_BiomeType BiomeType, const FName& DinosaurSpecies) const
{
    // Base spawn weights by biome
    switch (BiomeType)
    {
        case EEng_BiomeType::TropicalJungle:
            if (DinosaurSpecies == "Velociraptor") return 0.9f;
            if (DinosaurSpecies == "TRex")         return 0.6f;
            if (DinosaurSpecies == "Triceratops")  return 0.4f;
            return 0.5f;
            
        case EEng_BiomeType::OpenSavanna:
            if (DinosaurSpecies == "TRex")              return 0.8f;
            if (DinosaurSpecies == "Triceratops")       return 0.9f;
            if (DinosaurSpecies == "Brachiosaurus")     return 0.7f;
            if (DinosaurSpecies == "Parasaurolophus")   return 0.8f;
            return 0.6f;
            
        case EEng_BiomeType::RiverDelta:
            if (DinosaurSpecies == "Brachiosaurus")     return 0.9f;
            if (DinosaurSpecies == "Parasaurolophus")   return 0.8f;
            return 0.4f;
            
        case EEng_BiomeType::VolcanicPlains:
            if (DinosaurSpecies == "Ankylosaurus")      return 0.7f;
            if (DinosaurSpecies == "TRex")              return 0.5f;
            return 0.3f;
            
        case EEng_BiomeType::CoastalCliffs:
            return 0.3f;
            
        default:
            return 0.5f;
    }
}

// ============================================================
// GetCurrentDayPhase
// ============================================================
EEng_DayPhase UBiomeManager::GetCurrentDayPhase() const
{
    if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f)   return EEng_DayPhase::Dawn;
    if (TimeOfDay >= 7.0f && TimeOfDay < 11.0f)  return EEng_DayPhase::Morning;
    if (TimeOfDay >= 11.0f && TimeOfDay < 15.0f) return EEng_DayPhase::Midday;
    if (TimeOfDay >= 15.0f && TimeOfDay < 18.0f) return EEng_DayPhase::Afternoon;
    if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f) return EEng_DayPhase::Dusk;
    if (TimeOfDay >= 20.0f && TimeOfDay < 22.0f) return EEng_DayPhase::Evening;
    return EEng_DayPhase::Night;
}

// ============================================================
// Private: UpdateDayNightCycle
// ============================================================
void UBiomeManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time of day
    float TimeAdvance = (24.0f / DayDurationSeconds) * DeltaTime;
    TimeOfDay = FMath::Fmod(TimeOfDay + TimeAdvance, 24.0f);
    
    // Update sun angle (broadcast for lighting system)
    float SunAngle = ((TimeOfDay - 6.0f) / 12.0f) * 180.0f - 90.0f;
    OnTimeOfDayChanged.Broadcast(TimeOfDay, SunAngle);
}

// ============================================================
// Private: UpdateWeatherTransition
// ============================================================
void UBiomeManager::UpdateWeatherTransition(float DeltaTime)
{
    // Simple stochastic weather transitions (placeholder for full system)
    // In production, this would use noise-based weather patterns
    WeatherTransitionTimer += DeltaTime;
    
    if (WeatherTransitionTimer > 300.0f) // Check every 5 minutes
    {
        WeatherTransitionTimer = 0.0f;
        
        // Random weather event based on biome
        float Roll = FMath::FRand();
        if (CurrentBiomeType == EEng_BiomeType::TropicalJungle)
        {
            if (Roll < 0.3f && CurrentWeatherState == EEng_WeatherState::Clear)
            {
                SetWeatherState(EEng_WeatherState::Rain);
            }
            else if (Roll < 0.1f && CurrentWeatherState == EEng_WeatherState::Rain)
            {
                SetWeatherState(EEng_WeatherState::Storm);
            }
            else if (CurrentWeatherState != EEng_WeatherState::Clear && Roll > 0.7f)
            {
                SetWeatherState(EEng_WeatherState::Clear);
            }
        }
    }
}

// ============================================================
// Private: RegisterDefaultBiomes
// ============================================================
void UBiomeManager::RegisterDefaultBiomes()
{
    // MinPlayableMap biome zones
    FEng_BiomeZone JungleZone;
    JungleZone.ZoneID = "zone_jungle_central";
    JungleZone.ZoneName = "Cretaceous Jungle";
    JungleZone.BiomeType = EEng_BiomeType::TropicalJungle;
    JungleZone.CenterLocation = FVector(2200.0f, 2500.0f, 0.0f);
    JungleZone.Radius = 3000.0f;
    JungleZone.VegetationDensity = 0.9f;
    JungleZone.Temperature = 32.0f;
    JungleZone.Humidity = 0.9f;
    BiomeZones.Add(JungleZone);
    
    FEng_BiomeZone SavannaZone;
    SavannaZone.ZoneID = "zone_savanna_north";
    SavannaZone.ZoneName = "Northern Savanna";
    SavannaZone.BiomeType = EEng_BiomeType::OpenSavanna;
    SavannaZone.CenterLocation = FVector(-2000.0f, 0.0f, 0.0f);
    SavannaZone.Radius = 4000.0f;
    SavannaZone.VegetationDensity = 0.4f;
    SavannaZone.Temperature = 38.0f;
    SavannaZone.Humidity = 0.3f;
    BiomeZones.Add(SavannaZone);
    
    FEng_BiomeZone RiverZone;
    RiverZone.ZoneID = "zone_river_delta";
    RiverZone.ZoneName = "River Delta";
    RiverZone.BiomeType = EEng_BiomeType::RiverDelta;
    RiverZone.CenterLocation = FVector(5000.0f, -1000.0f, 0.0f);
    RiverZone.Radius = 2500.0f;
    RiverZone.VegetationDensity = 0.7f;
    RiverZone.Temperature = 28.0f;
    RiverZone.Humidity = 0.95f;
    BiomeZones.Add(RiverZone);
    
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Registered %d default biome zones"), BiomeZones.Num());
}

// ============================================================
// Private: ApplyWeatherEffects
// ============================================================
void UBiomeManager::ApplyWeatherEffects(EEng_WeatherState NewWeather)
{
    switch (NewWeather)
    {
        case EEng_WeatherState::Rain:
            CurrentHumidity = FMath::Min(CurrentHumidity + 0.2f, 1.0f);
            CurrentTemperature -= 4.0f;
            break;
        case EEng_WeatherState::Storm:
            CurrentHumidity = 1.0f;
            CurrentTemperature -= 8.0f;
            CurrentWindSpeed = 0.9f;
            break;
        case EEng_WeatherState::Clear:
            CurrentWindSpeed = 0.2f;
            break;
        case EEng_WeatherState::Drought:
            CurrentHumidity = FMath::Max(CurrentHumidity - 0.3f, 0.0f);
            CurrentTemperature += 6.0f;
            break;
        default:
            break;
    }
}

// ============================================================
// Private: GetBaseDensityForBiome
// ============================================================
float UBiomeManager::GetBaseDensityForBiome(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::TropicalJungle:    return 0.95f;
        case EEng_BiomeType::RiverDelta:        return 0.75f;
        case EEng_BiomeType::OpenSavanna:       return 0.35f;
        case EEng_BiomeType::VolcanicPlains:    return 0.15f;
        case EEng_BiomeType::CoastalCliffs:     return 0.25f;
        case EEng_BiomeType::DeepForest:        return 0.85f;
        default:                                return 0.5f;
    }
}

// ============================================================
// Private: GetBaseTemperatureForBiome
// ============================================================
float UBiomeManager::GetBaseTemperatureForBiome(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::TropicalJungle:    return 32.0f;
        case EEng_BiomeType::RiverDelta:        return 28.0f;
        case EEng_BiomeType::OpenSavanna:       return 38.0f;
        case EEng_BiomeType::VolcanicPlains:    return 45.0f;
        case EEng_BiomeType::CoastalCliffs:     return 22.0f;
        case EEng_BiomeType::DeepForest:        return 26.0f;
        default:                                return 30.0f;
    }
}
