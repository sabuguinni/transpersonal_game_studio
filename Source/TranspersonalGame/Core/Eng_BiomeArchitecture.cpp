#include "Eng_BiomeArchitecture.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// Static instance for event dispatcher
UEng_BiomeEventDispatcher* UEng_BiomeEventDispatcher::Instance = nullptr;

//=============================================================================
// UEng_BiomeArchitectureSubsystem Implementation
//=============================================================================

void UEng_BiomeArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome Architecture Subsystem Initialized"));
    
    InitializeDefaultConfigurations();
    
    // Set default global performance profile
    GlobalPerformanceProfile = FEng_BiomePerformanceProfile();
}

void UEng_BiomeArchitectureSubsystem::Deinitialize()
{
    BiomeConfigurations.Empty();
    Super::Deinitialize();
}

bool UEng_BiomeArchitectureSubsystem::RegisterBiomeConfiguration(EEng_BiomeType BiomeType, const FEng_ExtendedBiomeConfig& Config)
{
    if (!ValidateBiomeConfiguration(Config))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register biome configuration - validation failed"));
        return false;
    }
    
    BiomeConfigurations.Add(BiomeType, Config);
    UE_LOG(LogTemp, Log, TEXT("Registered biome configuration for type: %d"), (int32)BiomeType);
    return true;
}

FEng_ExtendedBiomeConfig UEng_BiomeArchitectureSubsystem::GetBiomeConfiguration(EEng_BiomeType BiomeType) const
{
    if (const FEng_ExtendedBiomeConfig* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default configuration if not found
    FEng_ExtendedBiomeConfig DefaultConfig;
    DefaultConfig.BaseData.BiomeType = BiomeType;
    return DefaultConfig;
}

TArray<EEng_BiomeType> UEng_BiomeArchitectureSubsystem::GetAllRegisteredBiomes() const
{
    TArray<EEng_BiomeType> RegisteredBiomes;
    BiomeConfigurations.GetKeys(RegisteredBiomes);
    return RegisteredBiomes;
}

void UEng_BiomeArchitectureSubsystem::SetGlobalPerformanceProfile(const FEng_BiomePerformanceProfile& Profile)
{
    if (ValidatePerformanceProfile(Profile))
    {
        GlobalPerformanceProfile = Profile;
        UE_LOG(LogTemp, Log, TEXT("Global performance profile updated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid performance profile - not applied"));
    }
}

FEng_BiomePerformanceProfile UEng_BiomeArchitectureSubsystem::GetGlobalPerformanceProfile() const
{
    return GlobalPerformanceProfile;
}

bool UEng_BiomeArchitectureSubsystem::ValidateBiomeConfiguration(const FEng_ExtendedBiomeConfig& Config) const
{
    // Validate performance profile
    if (!ValidatePerformanceProfile(Config.PerformanceProfile))
    {
        return false;
    }
    
    // Validate transition config
    if (!ValidateTransitionConfig(Config.TransitionConfig))
    {
        return false;
    }
    
    // Validate biome data
    if (Config.BaseData.Temperature < -50.0f || Config.BaseData.Temperature > 60.0f)
    {
        return false;
    }
    
    if (Config.BaseData.Humidity < 0.0f || Config.BaseData.Humidity > 100.0f)
    {
        return false;
    }
    
    // Validate dinosaur spawn probabilities
    float TotalProbability = 0.0f;
    for (const auto& SpawnPair : Config.DinosaurSpawnProbabilities)
    {
        if (SpawnPair.Value < 0.0f || SpawnPair.Value > 1.0f)
        {
            return false;
        }
        TotalProbability += SpawnPair.Value;
    }
    
    if (TotalProbability > 1.0f)
    {
        return false;
    }
    
    return true;
}

TArray<FString> UEng_BiomeArchitectureSubsystem::GetConfigurationErrors(const FEng_ExtendedBiomeConfig& Config) const
{
    TArray<FString> Errors;
    
    // Check performance profile
    if (!ValidatePerformanceProfile(Config.PerformanceProfile))
    {
        Errors.Add(TEXT("Invalid performance profile settings"));
    }
    
    // Check transition config
    if (!ValidateTransitionConfig(Config.TransitionConfig))
    {
        Errors.Add(TEXT("Invalid transition configuration"));
    }
    
    // Check temperature range
    if (Config.BaseData.Temperature < -50.0f || Config.BaseData.Temperature > 60.0f)
    {
        Errors.Add(FString::Printf(TEXT("Temperature out of range: %.1f (valid: -50 to 60)"), Config.BaseData.Temperature));
    }
    
    // Check humidity range
    if (Config.BaseData.Humidity < 0.0f || Config.BaseData.Humidity > 100.0f)
    {
        Errors.Add(FString::Printf(TEXT("Humidity out of range: %.1f (valid: 0 to 100)"), Config.BaseData.Humidity));
    }
    
    // Check spawn probabilities
    float TotalProbability = 0.0f;
    for (const auto& SpawnPair : Config.DinosaurSpawnProbabilities)
    {
        if (SpawnPair.Value < 0.0f || SpawnPair.Value > 1.0f)
        {
            Errors.Add(FString::Printf(TEXT("Invalid spawn probability for dinosaur %d: %.2f"), (int32)SpawnPair.Key, SpawnPair.Value));
        }
        TotalProbability += SpawnPair.Value;
    }
    
    if (TotalProbability > 1.0f)
    {
        Errors.Add(FString::Printf(TEXT("Total spawn probabilities exceed 1.0: %.2f"), TotalProbability));
    }
    
    return Errors;
}

void UEng_BiomeArchitectureSubsystem::InitializeDefaultConfigurations()
{
    // Forest Biome
    FEng_ExtendedBiomeConfig ForestConfig;
    ForestConfig.BaseData.BiomeType = EEng_BiomeType::Forest;
    ForestConfig.BaseData.BiomeName = TEXT("Dense Forest");
    ForestConfig.BaseData.Temperature = 22.0f;
    ForestConfig.BaseData.Humidity = 75.0f;
    ForestConfig.BaseData.ThreatLevel = EEng_ThreatLevel::Cautious;
    ForestConfig.BaseData.NativeDinosaurs = {EEng_DinosaurSpecies::Raptor, EEng_DinosaurSpecies::Dilophosaurus, EEng_DinosaurSpecies::Triceratops};
    ForestConfig.AllowedWeatherTypes = {EEng_WeatherType::Clear, EEng_WeatherType::Cloudy, EEng_WeatherType::Rain, EEng_WeatherType::Fog};
    ForestConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Raptor, 0.3f);
    ForestConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Dilophosaurus, 0.2f);
    ForestConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Triceratops, 0.4f);
    RegisterBiomeConfiguration(EEng_BiomeType::Forest, ForestConfig);
    
    // Savanna Biome
    FEng_ExtendedBiomeConfig SavannaConfig;
    SavannaConfig.BaseData.BiomeType = EEng_BiomeType::Savanna;
    SavannaConfig.BaseData.BiomeName = TEXT("Open Savanna");
    SavannaConfig.BaseData.Temperature = 28.0f;
    SavannaConfig.BaseData.Humidity = 45.0f;
    SavannaConfig.BaseData.ThreatLevel = EEng_ThreatLevel::Dangerous;
    SavannaConfig.BaseData.NativeDinosaurs = {EEng_DinosaurSpecies::TRex, EEng_DinosaurSpecies::Brachiosaurus, EEng_DinosaurSpecies::Stegosaurus};
    SavannaConfig.AllowedWeatherTypes = {EEng_WeatherType::Clear, EEng_WeatherType::Cloudy, EEng_WeatherType::Storm};
    SavannaConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::TRex, 0.1f);
    SavannaConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Brachiosaurus, 0.3f);
    SavannaConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Stegosaurus, 0.4f);
    RegisterBiomeConfiguration(EEng_BiomeType::Savanna, SavannaConfig);
    
    // Swamp Biome
    FEng_ExtendedBiomeConfig SwampConfig;
    SwampConfig.BaseData.BiomeType = EEng_BiomeType::Swamp;
    SwampConfig.BaseData.BiomeName = TEXT("Murky Swamp");
    SwampConfig.BaseData.Temperature = 26.0f;
    SwampConfig.BaseData.Humidity = 90.0f;
    SwampConfig.BaseData.ThreatLevel = EEng_ThreatLevel::Deadly;
    SwampConfig.BaseData.NativeDinosaurs = {EEng_DinosaurSpecies::Dilophosaurus, EEng_DinosaurSpecies::Ankylosaurus};
    SwampConfig.AllowedWeatherTypes = {EEng_WeatherType::Cloudy, EEng_WeatherType::Rain, EEng_WeatherType::Fog};
    SwampConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Dilophosaurus, 0.4f);
    SwampConfig.DinosaurSpawnProbabilities.Add(EEng_DinosaurSpecies::Ankylosaurus, 0.3f);
    RegisterBiomeConfiguration(EEng_BiomeType::Swamp, SwampConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Default biome configurations initialized"));
}

bool UEng_BiomeArchitectureSubsystem::ValidatePerformanceProfile(const FEng_BiomePerformanceProfile& Profile) const
{
    if (Profile.MaxVegetationActors < 0 || Profile.MaxVegetationActors > 10000)
        return false;
        
    if (Profile.MaxDinosaurActors < 0 || Profile.MaxDinosaurActors > 500)
        return false;
        
    if (Profile.LODDistanceMultiplier < 0.1f || Profile.LODDistanceMultiplier > 10.0f)
        return false;
        
    if (Profile.WeatherUpdateFrequency < 0.1f || Profile.WeatherUpdateFrequency > 60.0f)
        return false;
        
    return true;
}

bool UEng_BiomeArchitectureSubsystem::ValidateTransitionConfig(const FEng_BiomeTransitionConfig& Config) const
{
    if (Config.TransitionDistance < 0.0f || Config.TransitionDistance > 5000.0f)
        return false;
        
    if (Config.BlendRadius < 0.0f || Config.BlendRadius > Config.TransitionDistance)
        return false;
        
    if (Config.WeatherTransitionTime < 0.0f || Config.WeatherTransitionTime > 300.0f)
        return false;
        
    return true;
}

//=============================================================================
// UEng_WorldBiomeManager Implementation
//=============================================================================

void UEng_WorldBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("World Biome Manager Initialized"));
    
    // Get reference to architecture subsystem
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        ArchitectureSubsystem = GameInstance->GetSubsystem<UEng_BiomeArchitectureSubsystem>();
    }
    
    InitializeWorldBiomes();
}

void UEng_WorldBiomeManager::Deinitialize()
{
    ActiveBiomes.Empty();
    ArchitectureSubsystem = nullptr;
    Super::Deinitialize();
}

bool UEng_WorldBiomeManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UEng_WorldBiomeManager::RegisterActiveBiome(FVector Location, EEng_BiomeType BiomeType, float Radius)
{
    FActiveBiomeData NewBiome;
    NewBiome.Location = Location;
    NewBiome.BiomeType = BiomeType;
    NewBiome.Radius = Radius;
    NewBiome.CurrentWeather = EEng_WeatherType::Clear;
    
    ActiveBiomes.Add(NewBiome);
    
    UE_LOG(LogTemp, Log, TEXT("Registered active biome at location: %s"), *Location.ToString());
}

EEng_BiomeType UEng_WorldBiomeManager::GetBiomeAtLocation(FVector Location) const
{
    FActiveBiomeData* ClosestBiome = const_cast<UEng_WorldBiomeManager*>(this)->FindClosestBiome(Location);
    
    if (ClosestBiome && IsLocationInBiome(Location, *ClosestBiome))
    {
        return ClosestBiome->BiomeType;
    }
    
    // Default to forest if no biome found
    return EEng_BiomeType::Forest;
}

TArray<EEng_BiomeType> UEng_WorldBiomeManager::GetNearbyBiomes(FVector Location, float SearchRadius) const
{
    TArray<EEng_BiomeType> NearbyBiomes;
    
    for (const FActiveBiomeData& BiomeData : ActiveBiomes)
    {
        float Distance = FVector::Dist(Location, BiomeData.Location);
        if (Distance <= SearchRadius + BiomeData.Radius)
        {
            NearbyBiomes.AddUnique(BiomeData.BiomeType);
        }
    }
    
    return NearbyBiomes;
}

bool UEng_WorldBiomeManager::IsInBiomeTransitionZone(FVector Location) const
{
    int32 BiomesInRange = 0;
    
    for (const FActiveBiomeData& BiomeData : ActiveBiomes)
    {
        float Distance = FVector::Dist(Location, BiomeData.Location);
        if (Distance <= BiomeData.Radius * 1.2f) // 20% buffer for transition zone
        {
            BiomesInRange++;
            if (BiomesInRange > 1)
            {
                return true;
            }
        }
    }
    
    return false;
}

EEng_WeatherType UEng_WorldBiomeManager::GetCurrentWeather(FVector Location) const
{
    FActiveBiomeData* ClosestBiome = const_cast<UEng_WorldBiomeManager*>(this)->FindClosestBiome(Location);
    
    if (ClosestBiome)
    {
        return ClosestBiome->CurrentWeather;
    }
    
    return EEng_WeatherType::Clear;
}

void UEng_WorldBiomeManager::SetWeatherForBiome(EEng_BiomeType BiomeType, EEng_WeatherType WeatherType)
{
    for (FActiveBiomeData& BiomeData : ActiveBiomes)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            EEng_WeatherType OldWeather = BiomeData.CurrentWeather;
            BiomeData.CurrentWeather = WeatherType;
            
            // Broadcast weather change event
            if (UEng_BiomeEventDispatcher* EventDispatcher = UEng_BiomeEventDispatcher::GetBiomeEventDispatcher(this))
            {
                EventDispatcher->BroadcastWeatherChanged(OldWeather, WeatherType);
            }
        }
    }
}

int32 UEng_WorldBiomeManager::GetActiveBiomeCount() const
{
    return ActiveBiomes.Num();
}

float UEng_WorldBiomeManager::GetBiomeSystemPerformanceMetric() const
{
    // Simple performance metric based on active biomes and their settings
    float PerformanceScore = 1.0f;
    
    if (ArchitectureSubsystem)
    {
        FEng_BiomePerformanceProfile GlobalProfile = ArchitectureSubsystem->GetGlobalPerformanceProfile();
        
        // Reduce score based on number of active biomes
        PerformanceScore -= (ActiveBiomes.Num() * 0.1f);
        
        // Factor in vegetation and dinosaur limits
        PerformanceScore -= (GlobalProfile.MaxVegetationActors / 10000.0f) * 0.3f;
        PerformanceScore -= (GlobalProfile.MaxDinosaurActors / 500.0f) * 0.2f;
    }
    
    return FMath::Clamp(PerformanceScore, 0.0f, 1.0f);
}

void UEng_WorldBiomeManager::InitializeWorldBiomes()
{
    // Register default biomes for the world
    RegisterActiveBiome(FVector(0, 0, 0), EEng_BiomeType::Forest, 2000.0f);
    RegisterActiveBiome(FVector(5000, 0, 0), EEng_BiomeType::Savanna, 3000.0f);
    RegisterActiveBiome(FVector(-3000, 3000, 0), EEng_BiomeType::Swamp, 1500.0f);
    
    UE_LOG(LogTemp, Log, TEXT("World biomes initialized with %d active biomes"), ActiveBiomes.Num());
}

FEng_WorldBiomeManager::FActiveBiomeData* UEng_WorldBiomeManager::FindClosestBiome(FVector Location)
{
    FActiveBiomeData* ClosestBiome = nullptr;
    float ClosestDistance = MAX_FLT;
    
    for (FActiveBiomeData& BiomeData : ActiveBiomes)
    {
        float Distance = FVector::Dist(Location, BiomeData.Location);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = &BiomeData;
        }
    }
    
    return ClosestBiome;
}

bool UEng_WorldBiomeManager::IsLocationInBiome(FVector Location, const FActiveBiomeData& BiomeData) const
{
    float Distance = FVector::Dist(Location, BiomeData.Location);
    return Distance <= BiomeData.Radius;
}

//=============================================================================
// UEng_BiomeEventDispatcher Implementation
//=============================================================================

void UEng_BiomeEventDispatcher::BroadcastBiomeChanged(EEng_BiomeType OldBiome, EEng_BiomeType NewBiome)
{
    OnBiomeChanged.Broadcast(OldBiome, NewBiome);
    UE_LOG(LogTemp, Log, TEXT("Biome changed from %d to %d"), (int32)OldBiome, (int32)NewBiome);
}

void UEng_BiomeEventDispatcher::BroadcastWeatherChanged(EEng_WeatherType OldWeather, EEng_WeatherType NewWeather)
{
    OnWeatherChanged.Broadcast(OldWeather, NewWeather);
    UE_LOG(LogTemp, Log, TEXT("Weather changed from %d to %d"), (int32)OldWeather, (int32)NewWeather);
}

void UEng_BiomeEventDispatcher::BroadcastTransitionStarted(EEng_BiomeType TargetBiome)
{
    OnBiomeTransitionStarted.Broadcast(TargetBiome);
    UE_LOG(LogTemp, Log, TEXT("Biome transition started to %d"), (int32)TargetBiome);
}

void UEng_BiomeEventDispatcher::BroadcastTransitionCompleted()
{
    OnBiomeTransitionCompleted.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Biome transition completed"));
}

UEng_BiomeEventDispatcher* UEng_BiomeEventDispatcher::GetBiomeEventDispatcher(const UObject* WorldContext)
{
    if (!Instance)
    {
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
        {
            Instance = NewObject<UEng_BiomeEventDispatcher>();
            Instance->AddToRoot(); // Prevent garbage collection
        }
    }
    
    return Instance;
}