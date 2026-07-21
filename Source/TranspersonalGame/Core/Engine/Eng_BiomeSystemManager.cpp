#include "Eng_BiomeSystemManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UEng_BiomeSystemManager::UEng_BiomeSystemManager()
{
    NoiseScale = 0.001f;
    TemperatureNoiseScale = 0.0005f;
    HumidityNoiseScale = 0.0008f;
    ElevationInfluence = 0.1f;
}

void UEng_BiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Initializing Biome System"));
    
    InitializeBiomeData();
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Biome System initialized with %d biome types"), BiomeDataMap.Num());
}

void UEng_BiomeSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Deinitializing Biome System"));
    
    BiomeDataMap.Empty();
    BiomeTransitions.Empty();
    BiomeOverrides.Empty();
    OverrideRadii.Empty();
    
    Super::Deinitialize();
}

bool UEng_BiomeSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_BiomeSystemManager::InitializeBiomeData()
{
    // Grassland biome
    FEng_BiomeData GrasslandData;
    GrasslandData.BiomeType = EBiomeType::Grassland;
    GrasslandData.Temperature = 25.0f;
    GrasslandData.Humidity = 40.0f;
    GrasslandData.Elevation = 100.0f;
    GrasslandData.AllowedVegetation = {TEXT("Grass"), TEXT("Wildflowers"), TEXT("SmallShrubs")};
    GrasslandData.AllowedDinosaurs = {TEXT("Triceratops"), TEXT("Parasaurolophus"), TEXT("Ankylosaurus")};
    BiomeDataMap.Add(EBiomeType::Grassland, GrasslandData);

    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.Temperature = 20.0f;
    ForestData.Humidity = 70.0f;
    ForestData.Elevation = 200.0f;
    ForestData.AllowedVegetation = {TEXT("Trees"), TEXT("Ferns"), TEXT("Moss"), TEXT("Undergrowth")};
    ForestData.AllowedDinosaurs = {TEXT("Compsognathus"), TEXT("Dryosaurus"), TEXT("Dilophosaurus")};
    BiomeDataMap.Add(EBiomeType::Forest, ForestData);

    // Desert biome
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.Temperature = 40.0f;
    DesertData.Humidity = 15.0f;
    DesertData.Elevation = 50.0f;
    DesertData.AllowedVegetation = {TEXT("Cacti"), TEXT("DesertShrubs"), TEXT("DriedGrass")};
    DesertData.AllowedDinosaurs = {TEXT("Coelophysis"), TEXT("Plateosaurus"), TEXT("Herrerasaurus")};
    BiomeDataMap.Add(EBiomeType::Desert, DesertData);

    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.Temperature = 28.0f;
    SwampData.Humidity = 90.0f;
    SwampData.Elevation = 10.0f;
    SwampData.WaterProximity = 100.0f;
    SwampData.AllowedVegetation = {TEXT("SwampTrees"), TEXT("Reeds"), TEXT("Algae"), TEXT("WaterLilies")};
    SwampData.AllowedDinosaurs = {TEXT("Spinosaurus"), TEXT("Baryonyx"), TEXT("Suchomimus")};
    BiomeDataMap.Add(EBiomeType::Swamp, SwampData);

    // Mountain biome
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EBiomeType::Mountain;
    MountainData.Temperature = 10.0f;
    MountainData.Humidity = 30.0f;
    MountainData.Elevation = 800.0f;
    MountainData.AllowedVegetation = {TEXT("PineTrees"), TEXT("MountainGrass"), TEXT("Rocks")};
    MountainData.AllowedDinosaurs = {TEXT("Allosaurus"), TEXT("Ceratosaurus"), TEXT("Stegosaurus")};
    BiomeDataMap.Add(EBiomeType::Mountain, MountainData);

    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Initialized %d biome types"), BiomeDataMap.Num());
}

EBiomeType UEng_BiomeSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Check for manual overrides first
    EBiomeType OverrideBiome;
    if (IsLocationInBiomeOverride(WorldLocation, OverrideBiome))
    {
        return OverrideBiome;
    }

    // Generate biome based on environmental factors
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    float Humidity = GetHumidityAtLocation(WorldLocation);
    float Elevation = WorldLocation.Z;

    return CalculateBiomeFromFactors(Temperature, Humidity, Elevation);
}

FEng_BiomeData UEng_BiomeSystemManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }

    // Return default grassland if biome not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EBiomeType::Grassland;
    return DefaultData;
}

float UEng_BiomeSystemManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    float NoiseValue = GeneratePerlinNoise(WorldLocation.X, WorldLocation.Y, TemperatureNoiseScale);
    float ElevationFactor = FMath::Clamp(WorldLocation.Z * ElevationInfluence, -10.0f, 10.0f);
    
    // Base temperature 25°C, modified by noise and elevation
    float Temperature = 25.0f + (NoiseValue * 30.0f) - ElevationFactor;
    
    return FMath::Clamp(Temperature, -10.0f, 50.0f);
}

float UEng_BiomeSystemManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    float NoiseValue = GeneratePerlinNoise(WorldLocation.X, WorldLocation.Y, HumidityNoiseScale);
    
    // Base humidity 50%, modified by noise
    float Humidity = 50.0f + (NoiseValue * 40.0f);
    
    return FMath::Clamp(Humidity, 0.0f, 100.0f);
}

bool UEng_BiomeSystemManager::CanSpawnVegetationAtLocation(const FVector& WorldLocation, const FString& VegetationType) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.AllowedVegetation.Contains(VegetationType);
}

bool UEng_BiomeSystemManager::CanSpawnDinosaurAtLocation(const FVector& WorldLocation, const FString& DinosaurType) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.AllowedDinosaurs.Contains(DinosaurType);
}

TArray<FString> UEng_BiomeSystemManager::GetAllowedVegetationAtLocation(const FVector& WorldLocation) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.AllowedVegetation;
}

TArray<FString> UEng_BiomeSystemManager::GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.AllowedDinosaurs;
}

void UEng_BiomeSystemManager::RegisterBiomeTransition(const FEng_BiomeTransition& Transition)
{
    BiomeTransitions.Add(Transition);
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Registered biome transition from %d to %d"), 
           (int32)Transition.FromBiome, (int32)Transition.ToBiome);
}

float UEng_BiomeSystemManager::CalculateBiomeBlendFactor(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const
{
    // Find transition data for these biomes
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if ((Transition.FromBiome == BiomeA && Transition.ToBiome == BiomeB) ||
            (Transition.FromBiome == BiomeB && Transition.ToBiome == BiomeA))
        {
            return Transition.BlendFactor;
        }
    }
    
    // Default blend factor if no specific transition found
    return 0.5f;
}

void UEng_BiomeSystemManager::DebugPrintBiomeAtLocation(const FVector& WorldLocation)
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    float Humidity = GetHumidityAtLocation(WorldLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome at location (%.1f, %.1f, %.1f): Type=%d, Temp=%.1f°C, Humidity=%.1f%%"), 
           WorldLocation.X, WorldLocation.Y, WorldLocation.Z, 
           (int32)CurrentBiome, Temperature, Humidity);
    
    if (GEngine)
    {
        FString BiomeNames[] = {TEXT("Grassland"), TEXT("Forest"), TEXT("Desert"), TEXT("Swamp"), TEXT("Mountain")};
        FString BiomeName = (int32)CurrentBiome < 5 ? BiomeNames[(int32)CurrentBiome] : TEXT("Unknown");
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Biome: %s (Temp: %.1f°C, Humidity: %.1f%%)"), 
                           *BiomeName, Temperature, Humidity));
    }
}

void UEng_BiomeSystemManager::GenerateBiomeMap(int32 MapSize)
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Generating biome map of size %d x %d"), MapSize, MapSize);
    
    for (int32 X = 0; X < MapSize; X++)
    {
        for (int32 Y = 0; Y < MapSize; Y++)
        {
            FVector TestLocation(X * 1000.0f, Y * 1000.0f, 0.0f);
            EBiomeType BiomeAtLocation = GetBiomeAtLocation(TestLocation);
            
            // Log every 10th sample to avoid spam
            if (X % 10 == 0 && Y % 10 == 0)
            {
                UE_LOG(LogTemp, Log, TEXT("Biome map [%d,%d]: %d"), X, Y, (int32)BiomeAtLocation);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Biome map generation complete"));
}

void UEng_BiomeSystemManager::SetBiomeOverride(const FVector& WorldLocation, EBiomeType NewBiome, float Radius)
{
    BiomeOverrides.Add(WorldLocation, NewBiome);
    OverrideRadii.Add(WorldLocation, Radius);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Set biome override at (%.1f, %.1f, %.1f) to type %d with radius %.1f"), 
           WorldLocation.X, WorldLocation.Y, WorldLocation.Z, (int32)NewBiome, Radius);
}

void UEng_BiomeSystemManager::ClearBiomeOverrides()
{
    int32 ClearedCount = BiomeOverrides.Num();
    BiomeOverrides.Empty();
    OverrideRadii.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeSystemManager: Cleared %d biome overrides"), ClearedCount);
}

float UEng_BiomeSystemManager::GeneratePerlinNoise(float X, float Y, float Scale) const
{
    // Simple noise implementation using UE5's FMath functions
    float ScaledX = X * Scale;
    float ScaledY = Y * Scale;
    
    // Generate pseudo-random value based on coordinates
    float NoiseValue = FMath::Sin(ScaledX * 12.9898f + ScaledY * 78.233f) * 43758.5453f;
    NoiseValue = NoiseValue - FMath::FloorToFloat(NoiseValue);
    
    // Convert from [0,1] to [-1,1]
    return (NoiseValue * 2.0f) - 1.0f;
}

EBiomeType UEng_BiomeSystemManager::CalculateBiomeFromFactors(float Temperature, float Humidity, float Elevation) const
{
    // Biome determination logic based on environmental factors
    if (Elevation > 500.0f)
    {
        return EBiomeType::Mountain;
    }
    
    if (Humidity > 80.0f && Elevation < 50.0f)
    {
        return EBiomeType::Swamp;
    }
    
    if (Temperature > 35.0f && Humidity < 25.0f)
    {
        return EBiomeType::Desert;
    }
    
    if (Humidity > 60.0f && Temperature < 30.0f)
    {
        return EBiomeType::Forest;
    }
    
    // Default to grassland
    return EBiomeType::Grassland;
}

bool UEng_BiomeSystemManager::IsLocationInBiomeOverride(const FVector& WorldLocation, EBiomeType& OutBiome) const
{
    for (const auto& Override : BiomeOverrides)
    {
        const FVector& OverrideLocation = Override.Key;
        const EBiomeType& OverrideBiome = Override.Value;
        
        if (const float* RadiusPtr = OverrideRadii.Find(OverrideLocation))
        {
            float Distance = FVector::Dist(WorldLocation, OverrideLocation);
            if (Distance <= *RadiusPtr)
            {
                OutBiome = OverrideBiome;
                return true;
            }
        }
    }
    
    return false;
}