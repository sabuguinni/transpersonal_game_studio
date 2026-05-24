#include "World_BiomeSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogBiomeSystem);

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize biome definitions with coordinates from brain memories
    InitializeBiomeDefinitions();
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBiomeSystem, Log, TEXT("BiomeSystem: BeginPlay started"));
    
    // Validate biome setup
    ValidateBiomeConfiguration();
    
    UE_LOG(LogBiomeSystem, Log, TEXT("BiomeSystem: BeginPlay completed with %d biomes"), BiomeDefinitions.Num());
}

void UWorld_BiomeSystem::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();
    
    // PANTANO (Swamp) - Southwest
    FEnvArt_BiomeDefinition SwampBiome;
    SwampBiome.BiomeType = EEnvArt_BiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Pantano");
    SwampBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampBiome.BoundingBox = FBox(
        FVector(-77500.0f, -76500.0f, -500.0f),  // Min
        FVector(-25000.0f, -15000.0f, 500.0f)    // Max
    );
    SwampBiome.Temperature = 25.0f;
    SwampBiome.Humidity = 0.9f;
    SwampBiome.Elevation = -50.0f;
    SwampBiome.VegetationDensity = 0.8f;
    SwampBiome.WaterLevel = 0.3f;
    SwampBiome.SoilType = EEnvArt_SoilType::Mud;
    SwampBiome.DominantVegetation.Add(TEXT("SwampTrees"));
    SwampBiome.DominantVegetation.Add(TEXT("Reeds"));
    SwampBiome.DominantVegetation.Add(TEXT("Moss"));
    BiomeDefinitions.Add(EEnvArt_BiomeType::Swamp, SwampBiome);
    
    // FLORESTA (Forest) - Northwest
    FEnvArt_BiomeDefinition ForestBiome;
    ForestBiome.BiomeType = EEnvArt_BiomeType::Forest;
    ForestBiome.BiomeName = TEXT("Floresta");
    ForestBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestBiome.BoundingBox = FBox(
        FVector(-77500.0f, 15000.0f, -200.0f),   // Min
        FVector(-15000.0f, 76500.0f, 800.0f)     // Max
    );
    ForestBiome.Temperature = 18.0f;
    ForestBiome.Humidity = 0.7f;
    ForestBiome.Elevation = 100.0f;
    ForestBiome.VegetationDensity = 0.9f;
    ForestBiome.WaterLevel = 0.1f;
    ForestBiome.SoilType = EEnvArt_SoilType::Loam;
    ForestBiome.DominantVegetation.Add(TEXT("TallTrees"));
    ForestBiome.DominantVegetation.Add(TEXT("Ferns"));
    ForestBiome.DominantVegetation.Add(TEXT("FallenLogs"));
    BiomeDefinitions.Add(EEnvArt_BiomeType::Forest, ForestBiome);
    
    // SAVANA (Savanna) - Center
    FEnvArt_BiomeDefinition SavannaBiome;
    SavannaBiome.BiomeType = EEnvArt_BiomeType::Savanna;
    SavannaBiome.BiomeName = TEXT("Savana");
    SavannaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.BoundingBox = FBox(
        FVector(-20000.0f, -20000.0f, -100.0f),  // Min
        FVector(20000.0f, 20000.0f, 300.0f)      // Max
    );
    SavannaBiome.Temperature = 28.0f;
    SavannaBiome.Humidity = 0.4f;
    SavannaBiome.Elevation = 50.0f;
    SavannaBiome.VegetationDensity = 0.4f;
    SavannaBiome.WaterLevel = 0.05f;
    SavannaBiome.SoilType = EEnvArt_SoilType::Clay;
    SavannaBiome.DominantVegetation.Add(TEXT("TallGrass"));
    SavannaBiome.DominantVegetation.Add(TEXT("AcaciaTrees"));
    SavannaBiome.DominantVegetation.Add(TEXT("Rocks"));
    BiomeDefinitions.Add(EEnvArt_BiomeType::Savanna, SavannaBiome);
    
    // DESERTO (Desert) - East
    FEnvArt_BiomeDefinition DesertBiome;
    DesertBiome.BiomeType = EEnvArt_BiomeType::Desert;
    DesertBiome.BiomeName = TEXT("Deserto");
    DesertBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertBiome.BoundingBox = FBox(
        FVector(25000.0f, -30000.0f, -100.0f),   // Min
        FVector(79500.0f, 30000.0f, 400.0f)      // Max
    );
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.1f;
    DesertBiome.Elevation = 200.0f;
    DesertBiome.VegetationDensity = 0.1f;
    DesertBiome.WaterLevel = 0.0f;
    DesertBiome.SoilType = EEnvArt_SoilType::Sand;
    DesertBiome.DominantVegetation.Add(TEXT("Cacti"));
    DesertBiome.DominantVegetation.Add(TEXT("DesertShrubs"));
    DesertBiome.DominantVegetation.Add(TEXT("SandDunes"));
    BiomeDefinitions.Add(EEnvArt_BiomeType::Desert, DesertBiome);
    
    // MONTANHA NEVADA (Snowy Mountain) - Northeast
    FEnvArt_BiomeDefinition MountainBiome;
    MountainBiome.BiomeType = EEnvArt_BiomeType::Mountain;
    MountainBiome.BiomeName = TEXT("Montanha Nevada");
    MountainBiome.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    MountainBiome.BoundingBox = FBox(
        FVector(15000.0f, 20000.0f, 200.0f),     // Min
        FVector(79500.0f, 76500.0f, 2000.0f)     // Max
    );
    MountainBiome.Temperature = 5.0f;
    MountainBiome.Humidity = 0.6f;
    MountainBiome.Elevation = 800.0f;
    MountainBiome.VegetationDensity = 0.2f;
    MountainBiome.WaterLevel = 0.0f;
    MountainBiome.SoilType = EEnvArt_SoilType::Rock;
    MountainBiome.DominantVegetation.Add(TEXT("PineTrees"));
    MountainBiome.DominantVegetation.Add(TEXT("RockFormations"));
    MountainBiome.DominantVegetation.Add(TEXT("Snow"));
    BiomeDefinitions.Add(EEnvArt_BiomeType::Mountain, MountainBiome);
    
    UE_LOG(LogBiomeSystem, Log, TEXT("BiomeSystem: Initialized %d biome definitions"), BiomeDefinitions.Num());
}

void UWorld_BiomeSystem::ValidateBiomeConfiguration()
{
    for (const auto& BiomePair : BiomeDefinitions)
    {
        const FEnvArt_BiomeDefinition& Biome = BiomePair.Value;
        
        // Check if bounding box is valid
        if (!Biome.BoundingBox.IsValid)
        {
            UE_LOG(LogBiomeSystem, Warning, TEXT("Invalid bounding box for biome: %s"), *Biome.BiomeName);
            continue;
        }
        
        // Check if center is within bounds
        if (!Biome.BoundingBox.IsInside(Biome.CenterLocation))
        {
            UE_LOG(LogBiomeSystem, Warning, TEXT("Center location outside bounds for biome: %s"), *Biome.BiomeName);
        }
        
        // Validate parameter ranges
        if (Biome.Temperature < -50.0f || Biome.Temperature > 60.0f)
        {
            UE_LOG(LogBiomeSystem, Warning, TEXT("Temperature out of range for biome: %s"), *Biome.BiomeName);
        }
        
        if (Biome.Humidity < 0.0f || Biome.Humidity > 1.0f)
        {
            UE_LOG(LogBiomeSystem, Warning, TEXT("Humidity out of range for biome: %s"), *Biome.BiomeName);
        }
        
        UE_LOG(LogBiomeSystem, Log, TEXT("Biome validated: %s at %s"), 
               *Biome.BiomeName, *Biome.CenterLocation.ToString());
    }
}

EEnvArt_BiomeType UWorld_BiomeSystem::GetBiomeTypeAtLocation(const FVector& Location) const
{
    // Find which biome contains this location
    for (const auto& BiomePair : BiomeDefinitions)
    {
        const FEnvArt_BiomeDefinition& Biome = BiomePair.Value;
        
        if (Biome.BoundingBox.IsInside(Location))
        {
            return Biome.BiomeType;
        }
    }
    
    // Default to savanna if no biome found
    return EEnvArt_BiomeType::Savanna;
}

FEnvArt_BiomeDefinition UWorld_BiomeSystem::GetBiomeDefinition(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return *FoundBiome;
    }
    
    // Return default biome if not found
    FEnvArt_BiomeDefinition DefaultBiome;
    DefaultBiome.BiomeType = EEnvArt_BiomeType::Savanna;
    DefaultBiome.BiomeName = TEXT("Default");
    return DefaultBiome;
}

TArray<EEnvArt_BiomeType> UWorld_BiomeSystem::GetAllBiomeTypes() const
{
    TArray<EEnvArt_BiomeType> BiomeTypes;
    BiomeDefinitions.GetKeys(BiomeTypes);
    return BiomeTypes;
}

FVector UWorld_BiomeSystem::GetRandomLocationInBiome(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (!FoundBiome)
    {
        UE_LOG(LogBiomeSystem, Warning, TEXT("Biome type not found: %d"), (int32)BiomeType);
        return FVector::ZeroVector;
    }
    
    // Generate random location within biome bounds
    FVector MinBounds = FoundBiome->BoundingBox.Min;
    FVector MaxBounds = FoundBiome->BoundingBox.Max;
    
    float RandomX = FMath::RandRange(MinBounds.X, MaxBounds.X);
    float RandomY = FMath::RandRange(MinBounds.Y, MaxBounds.Y);
    float RandomZ = FMath::RandRange(MinBounds.Z, MaxBounds.Z);
    
    return FVector(RandomX, RandomY, RandomZ);
}

FVector UWorld_BiomeSystem::GetBiomeCenter(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->CenterLocation;
    }
    
    return FVector::ZeroVector;
}

float UWorld_BiomeSystem::GetBiomeTemperature(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->Temperature;
    }
    
    return 20.0f; // Default temperature
}

float UWorld_BiomeSystem::GetBiomeHumidity(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->Humidity;
    }
    
    return 0.5f; // Default humidity
}

float UWorld_BiomeSystem::GetBiomeVegetationDensity(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->VegetationDensity;
    }
    
    return 0.5f; // Default vegetation density
}

TArray<FString> UWorld_BiomeSystem::GetBiomeDominantVegetation(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->DominantVegetation;
    }
    
    return TArray<FString>();
}

bool UWorld_BiomeSystem::IsLocationInBiome(const FVector& Location, EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->BoundingBox.IsInside(Location);
    }
    
    return false;
}

float UWorld_BiomeSystem::GetDistanceToBiomeCenter(const FVector& Location, EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FVector::Dist(Location, FoundBiome->CenterLocation);
    }
    
    return 0.0f;
}

EEnvArt_SoilType UWorld_BiomeSystem::GetBiomeSoilType(EEnvArt_BiomeType BiomeType) const
{
    const FEnvArt_BiomeDefinition* FoundBiome = BiomeDefinitions.Find(BiomeType);
    if (FoundBiome)
    {
        return FoundBiome->SoilType;
    }
    
    return EEnvArt_SoilType::Loam; // Default soil type
}

FEnvArt_BiomeTransitionData UWorld_BiomeSystem::CalculateBiomeTransition(const FVector& Location) const
{
    FEnvArt_BiomeTransitionData TransitionData;
    TransitionData.PrimaryBiome = GetBiomeTypeAtLocation(Location);
    TransitionData.BlendFactor = 1.0f;
    TransitionData.SecondaryBiome = TransitionData.PrimaryBiome;
    
    // Find the closest biome boundary for transition calculations
    float MinDistanceToBoundary = FLT_MAX;
    EEnvArt_BiomeType ClosestSecondaryBiome = TransitionData.PrimaryBiome;
    
    for (const auto& BiomePair : BiomeDefinitions)
    {
        if (BiomePair.Key == TransitionData.PrimaryBiome) continue;
        
        const FEnvArt_BiomeDefinition& Biome = BiomePair.Value;
        float DistanceToCenter = FVector::Dist(Location, Biome.CenterLocation);
        
        if (DistanceToCenter < MinDistanceToBoundary)
        {
            MinDistanceToBoundary = DistanceToCenter;
            ClosestSecondaryBiome = Biome.BiomeType;
        }
    }
    
    // Calculate blend factor based on distance to boundary
    const float TransitionDistance = 5000.0f; // 5km transition zone
    if (MinDistanceToBoundary < TransitionDistance)
    {
        TransitionData.SecondaryBiome = ClosestSecondaryBiome;
        TransitionData.BlendFactor = MinDistanceToBoundary / TransitionDistance;
    }
    
    return TransitionData;
}