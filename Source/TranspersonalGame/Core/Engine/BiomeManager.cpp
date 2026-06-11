#include "BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UBiomeManager::UBiomeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for weather updates
    
    // Initialize default values
    BiomeTransitionSmoothness = 2.0f;
    WeatherUpdateFrequency = 30.0f;
    MaxBiomeInfluenceRadius = 10000.0f;
    TimeSinceWeatherUpdate = 0.0f;
    bIsBiomeSystemInitialized = false;
    ActiveTransitionCount = 0;
}

void UBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome system on begin play
    InitializeBiomeSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: System initialized with %d biome types"), BiomeDataMap.Num());
}

void UBiomeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsBiomeSystemInitialized)
    {
        return;
    }
    
    // Update weather progression
    UpdateWeatherProgression(DeltaTime);
}

void UBiomeManager::InitializeBiomeSystem()
{
    if (bIsBiomeSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: System already initialized"));
        return;
    }
    
    // Clear existing data
    BiomeDataMap.Empty();
    ActiveBiomes.Empty();
    BiomeWeatherStates.Empty();
    
    // Initialize default biome data
    InitializeDefaultBiomeData();
    
    // Set system as initialized
    bIsBiomeSystemInitialized = true;
    TimeSinceWeatherUpdate = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Biome system initialized successfully"));
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bIsBiomeSystemInitialized || BiomeDataMap.Num() == 0)
    {
        return EEng_BiomeType::Grassland; // Default fallback
    }
    
    // Simple biome determination based on location
    // This is a basic implementation - can be expanded with noise functions, height maps, etc.
    
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    float Z = WorldLocation.Z;
    
    // Determine biome based on location patterns
    if (Z > 2000.0f)
    {
        return EEng_BiomeType::Mountain;
    }
    else if (FMath::Abs(X) < 5000.0f && FMath::Abs(Y) < 5000.0f)
    {
        return EEng_BiomeType::Grassland; // Central area
    }
    else if (X > 10000.0f || X < -10000.0f)
    {
        return EEng_BiomeType::Desert; // Far east/west
    }
    else if (Y > 8000.0f)
    {
        return EEng_BiomeType::Forest; // North
    }
    else if (Y < -8000.0f)
    {
        return EEng_BiomeType::Swamp; // South
    }
    else
    {
        return EEng_BiomeType::Savanna; // Default
    }
}

FEng_BiomeData UBiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default biome data if not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = BiomeType;
    DefaultData.Temperature = 20.0f;
    DefaultData.Humidity = 0.5f;
    DefaultData.DangerLevel = 0.3f;
    DefaultData.BiomeName = TEXT("Unknown Biome");
    
    return DefaultData;
}

void UBiomeManager::UpdateBiomeTransition(const FVector& Location, float TransitionRadius)
{
    if (!bIsBiomeSystemInitialized)
    {
        return;
    }
    
    // Get current biome at location
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(Location);
    
    // Add to active biomes if not already present
    ActiveBiomes.AddUnique(CurrentBiome);
    
    // Update transition count
    ActiveTransitionCount = FMath::Max(1, ActiveBiomes.Num() - 1);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Updated transition at location (%f, %f, %f) - Biome: %d"), 
           Location.X, Location.Y, Location.Z, (int32)CurrentBiome);
}

FEng_WeatherState UBiomeManager::GetCurrentWeather() const
{
    FEng_WeatherState DefaultWeather;
    DefaultWeather.WeatherType = EEng_WeatherType::Clear;
    DefaultWeather.Intensity = 0.0f;
    DefaultWeather.WindSpeed = 5.0f;
    DefaultWeather.Temperature = 20.0f;
    DefaultWeather.Humidity = 0.5f;
    
    // Return weather for the first active biome, or default
    if (ActiveBiomes.Num() > 0)
    {
        if (const FEng_WeatherState* FoundWeather = BiomeWeatherStates.Find(ActiveBiomes[0]))
        {
            return *FoundWeather;
        }
    }
    
    return DefaultWeather;
}

void UBiomeManager::SetWeatherState(EEng_BiomeType BiomeType, const FEng_WeatherState& NewWeather)
{
    BiomeWeatherStates.Add(BiomeType, NewWeather);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Set weather for biome %d - Type: %d, Intensity: %f"), 
           (int32)BiomeType, (int32)NewWeather.WeatherType, NewWeather.Intensity);
}

void UBiomeManager::UpdateWeatherProgression(float DeltaTime)
{
    TimeSinceWeatherUpdate += DeltaTime;
    
    if (TimeSinceWeatherUpdate >= WeatherUpdateFrequency)
    {
        // Update weather for each active biome
        for (EEng_BiomeType BiomeType : ActiveBiomes)
        {
            UpdateBiomeWeatherInternal(BiomeType, TimeSinceWeatherUpdate);
        }
        
        TimeSinceWeatherUpdate = 0.0f;
    }
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add some variation based on height
    float HeightModifier = Location.Z * -0.01f; // Temperature decreases with altitude
    
    return BiomeData.Temperature + HeightModifier;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.Humidity;
}

float UBiomeManager::GetBiomeDangerLevel(EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.DangerLevel;
}

bool UBiomeManager::ValidateBiomeConfiguration()
{
    bool bIsValid = true;
    
    // Check if we have biome data
    if (BiomeDataMap.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: No biome data configured"));
        bIsValid = false;
    }
    
    // Validate each biome data entry
    for (const auto& BiomeEntry : BiomeDataMap)
    {
        if (!ValidateSingleBiomeData(BiomeEntry.Value))
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid biome data for type %d"), (int32)BiomeEntry.Key);
            bIsValid = false;
        }
    }
    
    // Check system parameters
    if (BiomeTransitionSmoothness <= 0.0f || WeatherUpdateFrequency <= 0.0f || MaxBiomeInfluenceRadius <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid system parameters"));
        bIsValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Configuration validation %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    return bIsValid;
}

FString UBiomeManager::GetBiomeSystemStatus() const
{
    FString Status = FString::Printf(TEXT("BiomeManager Status:\n"));
    Status += FString::Printf(TEXT("- Initialized: %s\n"), bIsBiomeSystemInitialized ? TEXT("Yes") : TEXT("No"));
    Status += FString::Printf(TEXT("- Biome Types: %d\n"), BiomeDataMap.Num());
    Status += FString::Printf(TEXT("- Active Biomes: %d\n"), ActiveBiomes.Num());
    Status += FString::Printf(TEXT("- Active Transitions: %d\n"), ActiveTransitionCount);
    Status += FString::Printf(TEXT("- Weather States: %d\n"), BiomeWeatherStates.Num());
    Status += FString::Printf(TEXT("- Time Since Weather Update: %.1fs\n"), TimeSinceWeatherUpdate);
    
    return Status;
}

void UBiomeManager::InitializeDefaultBiomeData()
{
    // Grassland biome
    FEng_BiomeData GrasslandData;
    GrasslandData.BiomeType = EEng_BiomeType::Grassland;
    GrasslandData.Temperature = 22.0f;
    GrasslandData.Humidity = 0.6f;
    GrasslandData.DangerLevel = 0.2f;
    GrasslandData.BiomeName = TEXT("Grassland");
    BiomeDataMap.Add(EEng_BiomeType::Grassland, GrasslandData);
    
    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.Temperature = 18.0f;
    ForestData.Humidity = 0.8f;
    ForestData.DangerLevel = 0.4f;
    ForestData.BiomeName = TEXT("Forest");
    BiomeDataMap.Add(EEng_BiomeType::Forest, ForestData);
    
    // Desert biome
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EEng_BiomeType::Desert;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.1f;
    DesertData.DangerLevel = 0.6f;
    DesertData.BiomeName = TEXT("Desert");
    BiomeDataMap.Add(EEng_BiomeType::Desert, DesertData);
    
    // Mountain biome
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::Mountain;
    MountainData.Temperature = 5.0f;
    MountainData.Humidity = 0.3f;
    MountainData.DangerLevel = 0.7f;
    MountainData.BiomeName = TEXT("Mountain");
    BiomeDataMap.Add(EEng_BiomeType::Mountain, MountainData);
    
    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.Temperature = 25.0f;
    SwampData.Humidity = 0.9f;
    SwampData.DangerLevel = 0.8f;
    SwampData.BiomeName = TEXT("Swamp");
    BiomeDataMap.Add(EEng_BiomeType::Swamp, SwampData);
    
    // Savanna biome
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.Temperature = 28.0f;
    SavannaData.Humidity = 0.4f;
    SavannaData.DangerLevel = 0.5f;
    SavannaData.BiomeName = TEXT("Savanna");
    BiomeDataMap.Add(EEng_BiomeType::Savanna, SavannaData);
    
    // Initialize default weather states
    for (const auto& BiomeEntry : BiomeDataMap)
    {
        FEng_WeatherState DefaultWeather;
        DefaultWeather.WeatherType = EEng_WeatherType::Clear;
        DefaultWeather.Intensity = 0.0f;
        DefaultWeather.WindSpeed = 5.0f;
        DefaultWeather.Temperature = BiomeEntry.Value.Temperature;
        DefaultWeather.Humidity = BiomeEntry.Value.Humidity;
        
        BiomeWeatherStates.Add(BiomeEntry.Key, DefaultWeather);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d default biome configurations"), BiomeDataMap.Num());
}

float UBiomeManager::CalculateBiomeInfluence(const FVector& Location, EEng_BiomeType BiomeType) const
{
    // Simple distance-based influence calculation
    // In a full implementation, this would use noise functions, heightmaps, etc.
    
    float Distance = FVector::Dist(Location, FVector::ZeroVector);
    float NormalizedDistance = FMath::Clamp(Distance / MaxBiomeInfluenceRadius, 0.0f, 1.0f);
    
    // Apply smoothing
    float Influence = FMath::Pow(1.0f - NormalizedDistance, BiomeTransitionSmoothness);
    
    return FMath::Clamp(Influence, 0.0f, 1.0f);
}

void UBiomeManager::UpdateBiomeWeatherInternal(EEng_BiomeType BiomeType, float DeltaTime)
{
    FEng_WeatherState* CurrentWeather = BiomeWeatherStates.Find(BiomeType);
    if (!CurrentWeather)
    {
        return;
    }
    
    // Simple weather progression - randomly change weather occasionally
    float RandomChance = FMath::RandRange(0.0f, 1.0f);
    
    if (RandomChance < 0.1f) // 10% chance to change weather
    {
        // Randomly select new weather type
        int32 WeatherTypeCount = (int32)EEng_WeatherType::Count;
        int32 NewWeatherIndex = FMath::RandRange(0, WeatherTypeCount - 1);
        CurrentWeather->WeatherType = (EEng_WeatherType)NewWeatherIndex;
        
        // Set appropriate intensity
        switch (CurrentWeather->WeatherType)
        {
        case EEng_WeatherType::Clear:
            CurrentWeather->Intensity = 0.0f;
            break;
        case EEng_WeatherType::Rain:
            CurrentWeather->Intensity = FMath::RandRange(0.3f, 0.8f);
            break;
        case EEng_WeatherType::Storm:
            CurrentWeather->Intensity = FMath::RandRange(0.6f, 1.0f);
            break;
        case EEng_WeatherType::Fog:
            CurrentWeather->Intensity = FMath::RandRange(0.2f, 0.6f);
            break;
        case EEng_WeatherType::Snow:
            CurrentWeather->Intensity = FMath::RandRange(0.2f, 0.7f);
            break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather changed for biome %d to %d (intensity: %f)"), 
               (int32)BiomeType, (int32)CurrentWeather->WeatherType, CurrentWeather->Intensity);
    }
}

bool UBiomeManager::ValidateSingleBiomeData(const FEng_BiomeData& BiomeData) const
{
    // Check temperature range
    if (BiomeData.Temperature < -50.0f || BiomeData.Temperature > 60.0f)
    {
        return false;
    }
    
    // Check humidity range
    if (BiomeData.Humidity < 0.0f || BiomeData.Humidity > 1.0f)
    {
        return false;
    }
    
    // Check danger level range
    if (BiomeData.DangerLevel < 0.0f || BiomeData.DangerLevel > 1.0f)
    {
        return false;
    }
    
    // Check name is not empty
    if (BiomeData.BiomeName.IsEmpty())
    {
        return false;
    }
    
    return true;
}