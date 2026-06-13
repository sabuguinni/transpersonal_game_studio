#include "Eng_BiomeSystemManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UEng_BiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing biome system"));
    SetupDefaultBiomeConfigurations();
}

void UEng_BiomeSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Shutting down biome system"));
    BiomeConfigurations.Empty();
    BiomeTransitions.Empty();
    Super::Deinitialize();
}

void UEng_BiomeSystemManager::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing biome system with %d biome types"), BiomeConfigurations.Num());
    
    // Validate all biome configurations
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        if (Config.MaxDinosaursPerBiome <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeSystemManager: Invalid MaxDinosaursPerBiome for biome %d"), (int32)BiomePair.Key);
        }
    }
}

EBiomeType UEng_BiomeSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Generate noise-based biome determination
    float NoiseValue = GenerateBiomeNoise(WorldLocation, 12345);
    float Elevation = WorldLocation.Z;
    
    return DetermineBiomeFromNoise(NoiseValue, Elevation);
}

FEng_BiomeConfiguration UEng_BiomeSystemManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: No configuration found for biome type %d, returning default"), (int32)BiomeType);
    return FEng_BiomeConfiguration();
}

void UEng_BiomeSystemManager::SetBiomeConfiguration(EBiomeType BiomeType, const FEng_BiomeConfiguration& Configuration)
{
    BiomeConfigurations.Add(BiomeType, Configuration);
    UE_LOG(LogTemp, Log, TEXT("BiomeSystemManager: Updated configuration for biome type %d"), (int32)BiomeType);
}

TArray<EBiomeType> UEng_BiomeSystemManager::GetAvailableBiomes() const
{
    TArray<EBiomeType> AvailableBiomes;
    BiomeConfigurations.GetKeys(AvailableBiomes);
    return AvailableBiomes;
}

float UEng_BiomeSystemManager::GetBiomeBlendFactor(const FVector& WorldLocation, EBiomeType& PrimaryBiome, EBiomeType& SecondaryBiome) const
{
    PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    
    // Check for nearby biome transitions
    float MinDistance = BiomeGridSize;
    SecondaryBiome = PrimaryBiome;
    
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == PrimaryBiome)
        {
            // Calculate distance to transition boundary
            float Distance = FVector::Dist2D(WorldLocation, FVector::ZeroVector); // Simplified
            if (Distance < Transition.TransitionDistance && Distance < MinDistance)
            {
                MinDistance = Distance;
                SecondaryBiome = Transition.ToBiome;
            }
        }
    }
    
    if (SecondaryBiome != PrimaryBiome && MinDistance < BiomeGridSize)
    {
        return FMath::Clamp(MinDistance / BiomeGridSize, 0.0f, 1.0f);
    }
    
    return 1.0f; // Pure primary biome
}

void UEng_BiomeSystemManager::AddBiomeTransition(const FEng_BiomeTransition& Transition)
{
    BiomeTransitions.Add(Transition);
    UE_LOG(LogTemp, Log, TEXT("BiomeSystemManager: Added transition from %d to %d"), (int32)Transition.FromBiome, (int32)Transition.ToBiome);
}

void UEng_BiomeSystemManager::RemoveBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome)
{
    BiomeTransitions.RemoveAll([FromBiome, ToBiome](const FEng_BiomeTransition& Transition)
    {
        return Transition.FromBiome == FromBiome && Transition.ToBiome == ToBiome;
    });
}

float UEng_BiomeSystemManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    
    // Apply elevation-based temperature modification
    float ElevationModifier = -WorldLocation.Z * 0.01f; // Cooler at higher elevations
    return Config.Temperature + ElevationModifier;
}

float UEng_BiomeSystemManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.Humidity;
}

TArray<EDinosaurSpecies> UEng_BiomeSystemManager::GetNativeDinosaursAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.NativeDinosaurs;
}

TArray<EVegetationType> UEng_BiomeSystemManager::GetVegetationTypesAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.VegetationTypes;
}

bool UEng_BiomeSystemManager::CanSpawnDinosaurAtLocation(const FVector& WorldLocation, EDinosaurSpecies Species) const
{
    TArray<EDinosaurSpecies> NativeDinosaurs = GetNativeDinosaursAtLocation(WorldLocation);
    return NativeDinosaurs.Contains(Species);
}

int32 UEng_BiomeSystemManager::GetMaxDinosaursForBiome(EBiomeType BiomeType) const
{
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.MaxDinosaursPerBiome;
}

float UEng_BiomeSystemManager::GetSpawnRateMultiplierAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.DinosaurSpawnRate;
}

void UEng_BiomeSystemManager::DebugDrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Draw biome grid visualization
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            FVector Location(X * BiomeGridSize, Y * BiomeGridSize, 0.0f);
            EBiomeType BiomeType = GetBiomeAtLocation(Location);
            
            FColor BiomeColor = FColor::White;
            switch (BiomeType)
            {
                case EBiomeType::Tropical: BiomeColor = FColor::Green; break;
                case EBiomeType::Temperate: BiomeColor = FColor::Blue; break;
                case EBiomeType::Desert: BiomeColor = FColor::Yellow; break;
                case EBiomeType::Arctic: BiomeColor = FColor::Cyan; break;
                case EBiomeType::Volcanic: BiomeColor = FColor::Red; break;
                default: BiomeColor = FColor::White; break;
            }
            
            DrawDebugSphere(World, Location, 500.0f, 12, BiomeColor, false, 10.0f);
        }
    }
}

void UEng_BiomeSystemManager::ValidateBiomeConfiguration()
{
    bool bIsValid = true;
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        
        if (Config.MaxDinosaursPerBiome <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeSystemManager: Invalid MaxDinosaursPerBiome for biome %d"), (int32)BiomePair.Key);
            bIsValid = false;
        }
        
        if (Config.DinosaurSpawnRate <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeSystemManager: Invalid DinosaurSpawnRate for biome %d"), (int32)BiomePair.Key);
            bIsValid = false;
        }
        
        if (Config.NativeDinosaurs.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: No native dinosaurs defined for biome %d"), (int32)BiomePair.Key);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Configuration validation %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
}

void UEng_BiomeSystemManager::LogBiomeSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Status - %d biomes configured, %d transitions defined"), 
           BiomeConfigurations.Num(), BiomeTransitions.Num());
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        UE_LOG(LogTemp, Log, TEXT("Biome %d: Temp=%.1f, Humidity=%.2f, MaxDinos=%d, SpawnRate=%.2f"), 
               (int32)BiomePair.Key, Config.Temperature, Config.Humidity, 
               Config.MaxDinosaursPerBiome, Config.DinosaurSpawnRate);
    }
}

void UEng_BiomeSystemManager::SetupDefaultBiomeConfigurations()
{
    // Tropical Biome
    FEng_BiomeConfiguration TropicalConfig;
    TropicalConfig.BiomeType = EBiomeType::Tropical;
    TropicalConfig.Temperature = 28.0f;
    TropicalConfig.Humidity = 0.8f;
    TropicalConfig.MaxDinosaursPerBiome = 30;
    TropicalConfig.DinosaurSpawnRate = 1.2f;
    TropicalConfig.NativeDinosaurs = {EDinosaurSpecies::TRex, EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Parasaurolophus};
    TropicalConfig.VegetationTypes = {EVegetationType::Fern, EVegetationType::PalmTree};
    BiomeConfigurations.Add(EBiomeType::Tropical, TropicalConfig);
    
    // Temperate Biome
    FEng_BiomeConfiguration TemperateConfig;
    TemperateConfig.BiomeType = EBiomeType::Temperate;
    TemperateConfig.Temperature = 18.0f;
    TemperateConfig.Humidity = 0.6f;
    TemperateConfig.MaxDinosaursPerBiome = 25;
    TemperateConfig.DinosaurSpawnRate = 1.0f;
    TemperateConfig.NativeDinosaurs = {EDinosaurSpecies::Triceratops, EDinosaurSpecies::Parasaurolophus};
    TemperateConfig.VegetationTypes = {EVegetationType::Conifer, EVegetationType::Fern};
    BiomeConfigurations.Add(EBiomeType::Temperate, TemperateConfig);
    
    // Desert Biome
    FEng_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.Temperature = 35.0f;
    DesertConfig.Humidity = 0.2f;
    DesertConfig.MaxDinosaursPerBiome = 15;
    DesertConfig.DinosaurSpawnRate = 0.7f;
    DesertConfig.NativeDinosaurs = {EDinosaurSpecies::Velociraptor, EDinosaurSpecies::Ankylosaurus};
    DesertConfig.VegetationTypes = {EVegetationType::Cactus};
    BiomeConfigurations.Add(EBiomeType::Desert, DesertConfig);
    
    // Arctic Biome
    FEng_BiomeConfiguration ArcticConfig;
    ArcticConfig.BiomeType = EBiomeType::Arctic;
    ArcticConfig.Temperature = -5.0f;
    ArcticConfig.Humidity = 0.4f;
    ArcticConfig.MaxDinosaursPerBiome = 10;
    ArcticConfig.DinosaurSpawnRate = 0.5f;
    ArcticConfig.NativeDinosaurs = {EDinosaurSpecies::Ankylosaurus};
    ArcticConfig.VegetationTypes = {EVegetationType::Conifer};
    BiomeConfigurations.Add(EBiomeType::Arctic, ArcticConfig);
    
    // Volcanic Biome
    FEng_BiomeConfiguration VolcanicConfig;
    VolcanicConfig.BiomeType = EBiomeType::Volcanic;
    VolcanicConfig.Temperature = 45.0f;
    VolcanicConfig.Humidity = 0.3f;
    VolcanicConfig.MaxDinosaursPerBiome = 8;
    VolcanicConfig.DinosaurSpawnRate = 0.4f;
    VolcanicConfig.NativeDinosaurs = {EDinosaurSpecies::TRex};
    VolcanicConfig.VegetationTypes = {};
    BiomeConfigurations.Add(EBiomeType::Volcanic, VolcanicConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initialized %d default biome configurations"), BiomeConfigurations.Num());
}

float UEng_BiomeSystemManager::GenerateBiomeNoise(const FVector& Location, int32 Seed) const
{
    // Simple noise generation for biome determination
    float X = Location.X * BiomeNoiseScale;
    float Y = Location.Y * BiomeNoiseScale;
    
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    
    for (int32 i = 0; i < BiomeNoiseOctaves; i++)
    {
        NoiseValue += FMath::Sin(X * Frequency + Seed) * FMath::Cos(Y * Frequency + Seed) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(NoiseValue, -1.0f, 1.0f);
}

EBiomeType UEng_BiomeSystemManager::DetermineBiomeFromNoise(float NoiseValue, float Elevation) const
{
    // High elevation = Arctic/Volcanic
    if (Elevation > 2000.0f)
    {
        return NoiseValue > 0.5f ? EBiomeType::Volcanic : EBiomeType::Arctic;
    }
    
    // Low elevation biomes based on noise
    if (NoiseValue < -0.5f)
        return EBiomeType::Desert;
    else if (NoiseValue < 0.0f)
        return EBiomeType::Temperate;
    else if (NoiseValue < 0.5f)
        return EBiomeType::Tropical;
    else
        return EBiomeType::Temperate;
}