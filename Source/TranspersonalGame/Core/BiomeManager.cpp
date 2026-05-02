#include "BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    // Initialize grid parameters
    GridSizeX = 100;
    GridSizeY = 100;
    CellSize = 1000.0f;
    WorldOrigin = FVector::ZeroVector;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initializing biome system"));
    
    // Initialize default biome data
    InitializeDefaultBiomeData();
    
    // Setup biome transitions
    SetupBiomeTransitions();
    
    // Generate initial biome map
    GenerateBiomeMap(GridSizeX, GridSizeY, 1);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialization complete"));
}

void UEng_BiomeManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Shutting down biome system"));
    
    // Clear biome grid
    BiomeGrid.Empty();
    BiomeDataMap.Empty();
    BiomeTransitions.Empty();
    
    Super::Deinitialize();
}

EEng_BiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    FIntPoint GridCoords = WorldLocationToGridCoords(WorldLocation);
    
    if (!IsValidGridCoords(GridCoords))
    {
        return EEng_BiomeType::Forest; // Default fallback
    }
    
    return BiomeGrid[GridCoords.X][GridCoords.Y];
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default forest biome if not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EEng_BiomeType::Forest;
    DefaultData.BiomeName = TEXT("Default Forest");
    return DefaultData;
}

void UEng_BiomeManager::SetBiomeAtLocation(const FVector& WorldLocation, EEng_BiomeType BiomeType)
{
    FIntPoint GridCoords = WorldLocationToGridCoords(WorldLocation);
    
    if (IsValidGridCoords(GridCoords))
    {
        BiomeGrid[GridCoords.X][GridCoords.Y] = BiomeType;
    }
}

TArray<EEng_BiomeType> UEng_BiomeManager::GetNearbyBiomes(const FVector& WorldLocation, float Radius) const
{
    TArray<EEng_BiomeType> NearbyBiomes;
    TSet<EEng_BiomeType> UniqueBiomes;
    
    FIntPoint CenterCoords = WorldLocationToGridCoords(WorldLocation);
    int32 GridRadius = FMath::CeilToInt(Radius / CellSize);
    
    for (int32 X = CenterCoords.X - GridRadius; X <= CenterCoords.X + GridRadius; X++)
    {
        for (int32 Y = CenterCoords.Y - GridRadius; Y <= CenterCoords.Y + GridRadius; Y++)
        {
            FIntPoint CheckCoords(X, Y);
            if (IsValidGridCoords(CheckCoords))
            {
                EEng_BiomeType BiomeType = BiomeGrid[X][Y];
                UniqueBiomes.Add(BiomeType);
            }
        }
    }
    
    return UniqueBiomes.Array();
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add some variation based on location
    float BaseTemp = BiomeData.TemperatureRange;
    float NoiseVariation = GenerateBiomeNoise(WorldLocation.X * 0.001f, WorldLocation.Y * 0.001f, 2) * 10.0f;
    
    return BaseTemp + NoiseVariation;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add some variation based on location
    float BaseHumidity = BiomeData.HumidityLevel;
    float NoiseVariation = GenerateBiomeNoise(WorldLocation.X * 0.0005f, WorldLocation.Y * 0.0005f, 3) * 0.2f;
    
    return FMath::Clamp(BaseHumidity + NoiseVariation, 0.0f, 1.0f);
}

float UEng_BiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.VegetationDensity;
}

bool UEng_BiomeManager::IsInTransitionZone(const FVector& WorldLocation) const
{
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    
    // Check nearby cells for different biomes
    TArray<EEng_BiomeType> NearbyBiomes = GetNearbyBiomes(WorldLocation, CellSize * 2.0f);
    
    for (EEng_BiomeType NearbyBiome : NearbyBiomes)
    {
        if (NearbyBiome != CurrentBiome)
        {
            return true;
        }
    }
    
    return false;
}

FEng_BiomeTransition UEng_BiomeManager::GetTransitionData(const FVector& WorldLocation) const
{
    // Find the most relevant transition for this location
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    TArray<EEng_BiomeType> NearbyBiomes = GetNearbyBiomes(WorldLocation, CellSize * 3.0f);
    
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == CurrentBiome && NearbyBiomes.Contains(Transition.ToBiome))
        {
            return Transition;
        }
    }
    
    // Return default transition
    FEng_BiomeTransition DefaultTransition;
    DefaultTransition.FromBiome = CurrentBiome;
    DefaultTransition.ToBiome = CurrentBiome;
    DefaultTransition.TransitionWidth = 500.0f;
    DefaultTransition.BlendFactor = 0.5f;
    return DefaultTransition;
}

void UEng_BiomeManager::GenerateBiomeMap(int32 WorldSizeX, int32 WorldSizeY, int32 BiomeResolution)
{
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Generating biome map %dx%d"), WorldSizeX, WorldSizeY);
    
    GridSizeX = WorldSizeX;
    GridSizeY = WorldSizeY;
    
    // Initialize the grid
    BiomeGrid.SetNum(GridSizeX);
    for (int32 X = 0; X < GridSizeX; X++)
    {
        BiomeGrid[X].SetNum(GridSizeY);
    }
    
    // Generate biomes using noise
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            float NoiseX = X * 0.05f;
            float NoiseY = Y * 0.05f;
            
            float BiomeNoise = GenerateBiomeNoise(NoiseX, NoiseY, 4);
            float ElevationNoise = GenerateBiomeNoise(NoiseX * 0.5f, NoiseY * 0.5f, 3);
            
            EEng_BiomeType GeneratedBiome = DetermineBiomeFromNoise(BiomeNoise, ElevationNoise);
            BiomeGrid[X][Y] = GeneratedBiome;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Biome map generation complete"));
}

void UEng_BiomeManager::ApplyBiomeEffectsToTerrain(const FVector& Location, float Radius)
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Applying %s biome effects at location %s"), 
           *BiomeData.BiomeName, *Location.ToString());
    
    // This would integrate with terrain generation systems
    // For now, just log the operation
}

void UEng_BiomeManager::DebugDrawBiomeMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Drawing debug biome map"));
    
    // Draw biome grid with different colors
    for (int32 X = 0; X < GridSizeX; X += 5) // Sample every 5th cell for performance
    {
        for (int32 Y = 0; Y < GridSizeY; Y += 5)
        {
            FVector WorldPos = GridCoordsToWorldLocation(FIntPoint(X, Y));
            EEng_BiomeType BiomeType = BiomeGrid[X][Y];
            
            FColor BiomeColor = FColor::Green; // Default
            switch (BiomeType)
            {
                case EEng_BiomeType::Swamp: BiomeColor = FColor::Blue; break;
                case EEng_BiomeType::Forest: BiomeColor = FColor::Green; break;
                case EEng_BiomeType::Savanna: BiomeColor = FColor::Yellow; break;
                case EEng_BiomeType::Desert: BiomeColor = FColor::Orange; break;
                case EEng_BiomeType::SnowyMountain: BiomeColor = FColor::White; break;
            }
            
            DrawDebugBox(World, WorldPos + FVector(0, 0, 100), FVector(CellSize * 0.4f), BiomeColor, false, 30.0f);
        }
    }
}

void UEng_BiomeManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Validating biome configuration"));
    
    bool bIsValid = true;
    
    // Check if all biome types have data
    for (int32 i = 0; i < static_cast<int32>(EEng_BiomeType::Count); i++)
    {
        EEng_BiomeType BiomeType = static_cast<EEng_BiomeType>(i);
        if (!BiomeDataMap.Contains(BiomeType))
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Missing data for biome type %d"), i);
            bIsValid = false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Configuration validation %s"), 
           bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
}

// Private helper functions

FIntPoint UEng_BiomeManager::WorldLocationToGridCoords(const FVector& WorldLocation) const
{
    FVector RelativeLocation = WorldLocation - WorldOrigin;
    int32 GridX = FMath::FloorToInt(RelativeLocation.X / CellSize);
    int32 GridY = FMath::FloorToInt(RelativeLocation.Y / CellSize);
    
    return FIntPoint(GridX, GridY);
}

FVector UEng_BiomeManager::GridCoordsToWorldLocation(const FIntPoint& GridCoords) const
{
    float WorldX = GridCoords.X * CellSize + (CellSize * 0.5f);
    float WorldY = GridCoords.Y * CellSize + (CellSize * 0.5f);
    
    return WorldOrigin + FVector(WorldX, WorldY, 0);
}

bool UEng_BiomeManager::IsValidGridCoords(const FIntPoint& GridCoords) const
{
    return GridCoords.X >= 0 && GridCoords.X < GridSizeX && 
           GridCoords.Y >= 0 && GridCoords.Y < GridSizeY;
}

void UEng_BiomeManager::InitializeDefaultBiomeData()
{
    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.BiomeName = TEXT("Swamp");
    SwampData.TemperatureRange = 28.0f;
    SwampData.HumidityLevel = 0.9f;
    SwampData.VegetationDensity = 0.8f;
    SwampData.DinosaurSpecies = {TEXT("Spinosaurus"), TEXT("Parasaurolophus"), TEXT("Dracorex")};
    BiomeDataMap.Add(EEng_BiomeType::Swamp, SwampData);
    
    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.BiomeName = TEXT("Forest");
    ForestData.TemperatureRange = 22.0f;
    ForestData.HumidityLevel = 0.7f;
    ForestData.VegetationDensity = 0.9f;
    ForestData.DinosaurSpecies = {TEXT("Triceratops"), TEXT("Stegosaurus"), TEXT("Compsognathus")};
    BiomeDataMap.Add(EEng_BiomeType::Forest, ForestData);
    
    // Savanna biome
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.BiomeName = TEXT("Savanna");
    SavannaData.TemperatureRange = 30.0f;
    SavannaData.HumidityLevel = 0.4f;
    SavannaData.VegetationDensity = 0.3f;
    SavannaData.DinosaurSpecies = {TEXT("Brachiosaurus"), TEXT("Allosaurus"), TEXT("Gallimimus")};
    BiomeDataMap.Add(EEng_BiomeType::Savanna, SavannaData);
    
    // Desert biome
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EEng_BiomeType::Desert;
    DesertData.BiomeName = TEXT("Desert");
    DesertData.TemperatureRange = 40.0f;
    DesertData.HumidityLevel = 0.1f;
    DesertData.VegetationDensity = 0.1f;
    DesertData.DinosaurSpecies = {TEXT("Carnotaurus"), TEXT("Dilophosaurus"), TEXT("Ouranosaurus")};
    BiomeDataMap.Add(EEng_BiomeType::Desert, DesertData);
    
    // Snowy Mountain biome
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::SnowyMountain;
    MountainData.BiomeName = TEXT("Snowy Mountain");
    MountainData.TemperatureRange = -5.0f;
    MountainData.HumidityLevel = 0.6f;
    MountainData.VegetationDensity = 0.2f;
    MountainData.DinosaurSpecies = {TEXT("Utahraptor"), TEXT("Therizinosaurus"), TEXT("Cryolophosaurus")};
    BiomeDataMap.Add(EEng_BiomeType::SnowyMountain, MountainData);
}

void UEng_BiomeManager::SetupBiomeTransitions()
{
    // Forest to Savanna
    FEng_BiomeTransition ForestToSavanna;
    ForestToSavanna.FromBiome = EEng_BiomeType::Forest;
    ForestToSavanna.ToBiome = EEng_BiomeType::Savanna;
    ForestToSavanna.TransitionWidth = 1500.0f;
    ForestToSavanna.BlendFactor = 0.6f;
    BiomeTransitions.Add(ForestToSavanna);
    
    // Savanna to Desert
    FEng_BiomeTransition SavannaToDesert;
    SavannaToDesert.FromBiome = EEng_BiomeType::Savanna;
    SavannaToDesert.ToBiome = EEng_BiomeType::Desert;
    SavannaToDesert.TransitionWidth = 2000.0f;
    SavannaToDesert.BlendFactor = 0.7f;
    BiomeTransitions.Add(SavannaToDesert);
    
    // Forest to Swamp
    FEng_BiomeTransition ForestToSwamp;
    ForestToSwamp.FromBiome = EEng_BiomeType::Forest;
    ForestToSwamp.ToBiome = EEng_BiomeType::Swamp;
    ForestToSwamp.TransitionWidth = 1000.0f;
    ForestToSwamp.BlendFactor = 0.5f;
    BiomeTransitions.Add(ForestToSwamp);
    
    // Forest to Mountain
    FEng_BiomeTransition ForestToMountain;
    ForestToMountain.FromBiome = EEng_BiomeType::Forest;
    ForestToMountain.ToBiome = EEng_BiomeType::SnowyMountain;
    ForestToMountain.TransitionWidth = 2500.0f;
    ForestToMountain.BlendFactor = 0.8f;
    BiomeTransitions.Add(ForestToMountain);
}

float UEng_BiomeManager::GenerateBiomeNoise(float X, float Y, int32 Octaves) const
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Value / MaxValue;
}

EEng_BiomeType UEng_BiomeManager::DetermineBiomeFromNoise(float NoiseValue, float Elevation) const
{
    // Normalize noise value to 0-1 range
    float NormalizedNoise = (NoiseValue + 1.0f) * 0.5f;
    
    // Use elevation to influence biome selection
    if (Elevation > 0.7f)
    {
        return EEng_BiomeType::SnowyMountain;
    }
    else if (Elevation < -0.3f)
    {
        return EEng_BiomeType::Swamp;
    }
    else if (NormalizedNoise < 0.3f)
    {
        return EEng_BiomeType::Desert;
    }
    else if (NormalizedNoise < 0.6f)
    {
        return EEng_BiomeType::Savanna;
    }
    else
    {
        return EEng_BiomeType::Forest;
    }
}