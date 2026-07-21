#include "EngArch_BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"

UEngArch_BiomeManager::UEngArch_BiomeManager()
{
    TimeOfDay = 12.0f;
    DayDurationMinutes = 24.0f;
    TimeScale = 1.0f;
    bWeatherOverrideActive = false;
}

void UEngArch_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system"));
    
    SetupDefaultBiomes();
    
    // Initialize weather
    CurrentWeather.Temperature = 25.0f;
    CurrentWeather.Humidity = 0.5f;
    CurrentWeather.WindSpeed = 5.0f;
    CurrentWeather.WindDirection = FVector(1, 0, 0);
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.TimeOfDay = TimeOfDay;
}

void UEngArch_BiomeManager::Deinitialize()
{
    BiomeDatabase.Empty();
    Super::Deinitialize();
}

void UEngArch_BiomeManager::InitializeBiomes()
{
    SetupDefaultBiomes();
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Biomes initialized with %d biome types"), BiomeDatabase.Num());
}

void UEngArch_BiomeManager::SetupDefaultBiomes()
{
    BiomeDatabase.Empty();

    // Savanna Biome (Central)
    FEng_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EBiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0, 0, 0);
    SavannaBiome.Radius = 30000.0f;
    SavannaBiome.Temperature = 28.0f;
    SavannaBiome.Humidity = 0.4f;
    SavannaBiome.Fertility = 0.6f;
    SavannaBiome.NativeDinosaurs.Add(EDinosaurSpecies::TRex);
    SavannaBiome.NativeDinosaurs.Add(EDinosaurSpecies::Velociraptor);
    SavannaBiome.NativeDinosaurs.Add(EDinosaurSpecies::Triceratops);
    SavannaBiome.VegetationTypes.Add("Acacia_Trees");
    SavannaBiome.VegetationTypes.Add("Tall_Grass");
    BiomeDatabase.Add(EBiomeType::Savanna, SavannaBiome);

    // Forest Biome (Northwest)
    FEng_BiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-45000, 40000, 0);
    ForestBiome.Radius = 25000.0f;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.Fertility = 0.9f;
    ForestBiome.NativeDinosaurs.Add(EDinosaurSpecies::Brachiosaurus);
    ForestBiome.NativeDinosaurs.Add(EDinosaurSpecies::Parasaurolophus);
    ForestBiome.VegetationTypes.Add("Dense_Forest");
    ForestBiome.VegetationTypes.Add("Ferns");
    ForestBiome.VegetationTypes.Add("Moss");
    BiomeDatabase.Add(EBiomeType::Forest, ForestBiome);

    // Desert Biome (Southeast)
    FEng_BiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.CenterLocation = FVector(50000, -40000, 0);
    DesertBiome.Radius = 20000.0f;
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.2f;
    DesertBiome.Fertility = 0.3f;
    DesertBiome.NativeDinosaurs.Add(EDinosaurSpecies::Ankylosaurus);
    DesertBiome.VegetationTypes.Add("Cacti");
    DesertBiome.VegetationTypes.Add("Desert_Shrubs");
    BiomeDatabase.Add(EBiomeType::Desert, DesertBiome);

    // Mountain Biome (North)
    FEng_BiomeData MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0, 60000, 2000);
    MountainBiome.Radius = 15000.0f;
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.6f;
    MountainBiome.Fertility = 0.4f;
    MountainBiome.NativeDinosaurs.Add(EDinosaurSpecies::Pachycephalosaurus);
    MountainBiome.VegetationTypes.Add("Pine_Trees");
    MountainBiome.VegetationTypes.Add("Rocky_Vegetation");
    BiomeDatabase.Add(EBiomeType::Mountain, MountainBiome);

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Setup %d default biomes"), BiomeDatabase.Num());
}

EBiomeType UEngArch_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;

    for (const auto& BiomePair : BiomeDatabase)
    {
        float Distance = CalculateDistanceToBiome(Location, BiomePair.Key);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData UEngArch_BiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeDatabase.Find(BiomeType))
    {
        return *BiomeData;
    }
    
    // Return default savanna if not found
    FEng_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = EBiomeType::Savanna;
    return DefaultBiome;
}

TArray<FEng_BiomeData> UEngArch_BiomeManager::GetAllBiomes() const
{
    TArray<FEng_BiomeData> AllBiomes;
    for (const auto& BiomePair : BiomeDatabase)
    {
        AllBiomes.Add(BiomePair.Value);
    }
    return AllBiomes;
}

bool UEngArch_BiomeManager::IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeDatabase.Find(BiomeType))
    {
        float Distance = FVector::Dist(Location, BiomeData->CenterLocation);
        return Distance <= BiomeData->Radius;
    }
    return false;
}

FVector UEngArch_BiomeManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeDatabase.Find(BiomeType))
    {
        return BiomeData->CenterLocation;
    }
    return FVector::ZeroVector;
}

void UEngArch_BiomeManager::UpdateWeather(float DeltaTime)
{
    if (!bWeatherOverrideActive)
    {
        // Natural weather progression based on time of day and biome
        CurrentWeather.TimeOfDay = TimeOfDay;
        
        // Temperature varies with time of day
        float DayProgress = GetDayProgress();
        float TempVariation = FMath::Sin(DayProgress * PI) * 5.0f; // ±5°C variation
        CurrentWeather.Temperature = 25.0f + TempVariation;
        
        // Humidity changes with temperature
        CurrentWeather.Humidity = FMath::Clamp(0.8f - (CurrentWeather.Temperature - 20.0f) * 0.02f, 0.2f, 0.9f);
        
        // Simple cloud coverage simulation
        CurrentWeather.CloudCoverage = FMath::Clamp(CurrentWeather.Humidity + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
        
        // Rain based on cloud coverage
        if (CurrentWeather.CloudCoverage > 0.7f)
        {
            CurrentWeather.RainIntensity = FMath::Clamp((CurrentWeather.CloudCoverage - 0.7f) * 3.33f, 0.0f, 1.0f);
        }
        else
        {
            CurrentWeather.RainIntensity = 0.0f;
        }
    }
    
    UpdateEnvironmentalEffects();
}

FEng_WeatherData UEngArch_BiomeManager::GetCurrentWeather() const
{
    return CurrentWeather;
}

void UEngArch_BiomeManager::SetWeatherOverride(const FEng_WeatherData& NewWeather)
{
    CurrentWeather = NewWeather;
    bWeatherOverrideActive = true;
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Weather override activated"));
}

void UEngArch_BiomeManager::ClearWeatherOverride()
{
    bWeatherOverrideActive = false;
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Weather override cleared"));
}

void UEngArch_BiomeManager::UpdateDayNightCycle(float DeltaTime)
{
    float TimeIncrement = (DeltaTime * TimeScale) / (DayDurationMinutes * 60.0f) * 24.0f;
    TimeOfDay += TimeIncrement;
    
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    CurrentWeather.TimeOfDay = TimeOfDay;
}

float UEngArch_BiomeManager::GetTimeOfDay() const
{
    return TimeOfDay;
}

void UEngArch_BiomeManager::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    CurrentWeather.TimeOfDay = TimeOfDay;
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Time of day set to %.2f"), TimeOfDay);
}

bool UEngArch_BiomeManager::IsNightTime() const
{
    return TimeOfDay < 6.0f || TimeOfDay > 18.0f;
}

float UEngArch_BiomeManager::GetDayProgress() const
{
    return TimeOfDay / 24.0f;
}

float UEngArch_BiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Modify base biome temperature with current weather
    float BaseTemp = BiomeData.Temperature;
    float WeatherModifier = CurrentWeather.Temperature - 25.0f; // 25°C is baseline
    
    return BaseTemp + WeatherModifier;
}

float UEngArch_BiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Blend biome humidity with current weather
    return (BiomeData.Humidity + CurrentWeather.Humidity) * 0.5f;
}

bool UEngArch_BiomeManager::CanDinosaurSurviveInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeDatabase.Find(BiomeType))
    {
        return BiomeData->NativeDinosaurs.Contains(Species);
    }
    return false;
}

void UEngArch_BiomeManager::UpdateEnvironmentalEffects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update directional light intensity based on time of day
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* LightActor : DirectionalLights)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(LightActor))
        {
            if (UDirectionalLightComponent* LightComp = DirLight->GetComponent())
            {
                float DayProgress = GetDayProgress();
                float SunAngle = (DayProgress - 0.5f) * 180.0f; // -90 to +90 degrees
                
                // Calculate light intensity based on sun angle
                float Intensity = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle)), 0.1f, 1.0f);
                LightComp->SetIntensity(Intensity * 3.0f); // Scale for visibility
                
                // Set light rotation to simulate sun movement
                FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
                DirLight->SetActorRotation(SunRotation);
            }
        }
    }
}

float UEngArch_BiomeManager::CalculateDistanceToBiome(const FVector& Location, EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeDatabase.Find(BiomeType))
    {
        float Distance = FVector::Dist(Location, BiomeData->CenterLocation);
        // If within radius, return 0, otherwise return distance beyond radius
        return FMath::Max(0.0f, Distance - BiomeData->Radius);
    }
    return FLT_MAX;
}