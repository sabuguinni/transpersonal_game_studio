#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    BiomeNoiseScale = 0.001f;
    BiomeBlendDistance = 1000.0f;
    BiomeResolution = 128;
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeDatabase();
    GenerateDefaultBiomes();
    UpdateBiomeTransitions();
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update biome system periodically
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f) // Update every 5 seconds
    {
        UpdateBiomeTransitions();
        UpdateTimer = 0.0f;
    }
}

void UWorld_BiomeSystem::InitializeBiomeDatabase()
{
    // Forest biome
    FWorld_BiomeData ForestData;
    ForestData.BiomeType = EWorld_BiomeType::Forest;
    ForestData.BiomeName = TEXT("Dense Forest");
    ForestData.BiomeColor = FLinearColor(0.2f, 0.6f, 0.2f, 1.0f);
    ForestData.Temperature = 18.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 100.0f;
    ForestData.VegetationTypes = {TEXT("Oak_Tree"), TEXT("Pine_Tree"), TEXT("Fern"), TEXT("Moss")};
    ForestData.WildlifeTypes = {TEXT("Deer"), TEXT("Bear"), TEXT("Wolf"), TEXT("Rabbit")};
    ForestData.SpawnDensity = 1.2f;
    BiomeDatabase.Add(EWorld_BiomeType::Forest, ForestData);

    // Plains biome
    FWorld_BiomeData PlainsData;
    PlainsData.BiomeType = EWorld_BiomeType::Plains;
    PlainsData.BiomeName = TEXT("Open Plains");
    PlainsData.BiomeColor = FLinearColor(0.4f, 0.7f, 0.3f, 1.0f);
    PlainsData.Temperature = 22.0f;
    PlainsData.Humidity = 0.4f;
    PlainsData.Elevation = 50.0f;
    PlainsData.VegetationTypes = {TEXT("Grass"), TEXT("Wildflower"), TEXT("Shrub")};
    PlainsData.WildlifeTypes = {TEXT("Bison"), TEXT("Horse"), TEXT("Eagle")};
    PlainsData.SpawnDensity = 0.8f;
    BiomeDatabase.Add(EWorld_BiomeType::Plains, PlainsData);

    // Swampland biome
    FWorld_BiomeData SwampData;
    SwampData.BiomeType = EWorld_BiomeType::Swampland;
    SwampData.BiomeName = TEXT("Misty Swampland");
    SwampData.BiomeColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);
    SwampData.Temperature = 25.0f;
    SwampData.Humidity = 0.95f;
    SwampData.Elevation = -20.0f;
    SwampData.VegetationTypes = {TEXT("Cypress_Tree"), TEXT("Moss"), TEXT("Reed"), TEXT("Lily_Pad")};
    SwampData.WildlifeTypes = {TEXT("Alligator"), TEXT("Frog"), TEXT("Heron")};
    SwampData.SpawnDensity = 1.0f;
    BiomeDatabase.Add(EWorld_BiomeType::Swampland, SwampData);

    // Canyon biome
    FWorld_BiomeData CanyonData;
    CanyonData.BiomeType = EWorld_BiomeType::Canyon;
    CanyonData.BiomeName = TEXT("Red Rock Canyon");
    CanyonData.BiomeColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    CanyonData.Temperature = 30.0f;
    CanyonData.Humidity = 0.2f;
    CanyonData.Elevation = 200.0f;
    CanyonData.VegetationTypes = {TEXT("Cactus"), TEXT("Desert_Shrub"), TEXT("Rock_Formation")};
    CanyonData.WildlifeTypes = {TEXT("Lizard"), TEXT("Snake"), TEXT("Vulture")};
    CanyonData.SpawnDensity = 0.5f;
    BiomeDatabase.Add(EWorld_BiomeType::Canyon, CanyonData);

    // Mountains biome
    FWorld_BiomeData MountainData;
    MountainData.BiomeType = EWorld_BiomeType::Mountains;
    MountainData.BiomeName = TEXT("Rocky Mountains");
    MountainData.BiomeColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    MountainData.Temperature = 10.0f;
    MountainData.Humidity = 0.6f;
    MountainData.Elevation = 500.0f;
    MountainData.VegetationTypes = {TEXT("Pine_Tree"), TEXT("Rock"), TEXT("Alpine_Grass")};
    MountainData.WildlifeTypes = {TEXT("Mountain_Goat"), TEXT("Eagle"), TEXT("Bear")};
    MountainData.SpawnDensity = 0.6f;
    BiomeDatabase.Add(EWorld_BiomeType::Mountains, MountainData);

    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Initialized %d biome types"), BiomeDatabase.Num());
}

EWorld_BiomeType UWorld_BiomeSystem::GetBiomeAtLocation(FVector WorldLocation) const
{
    float HighestInfluence = 0.0f;
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::Forest;

    // Check registered biome zones first
    for (const FBiomeZone& Zone : BiomeZones)
    {
        float Influence = CalculateDistanceInfluence(WorldLocation, Zone);
        if (Influence > HighestInfluence)
        {
            HighestInfluence = Influence;
            DominantBiome = Zone.BiomeType;
        }
    }

    // If no strong zone influence, use noise-based determination
    if (HighestInfluence < 0.3f)
    {
        float NoiseValue = CalculateNoiseValue(WorldLocation);
        DominantBiome = DetermineBiomeFromNoise(NoiseValue, WorldLocation);
    }

    return DominantBiome;
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    if (const FWorld_BiomeData* FoundData = BiomeDatabase.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default forest data if not found
    return BiomeDatabase.FindRef(EWorld_BiomeType::Forest);
}

float UWorld_BiomeSystem::GetBiomeInfluence(FVector WorldLocation, EWorld_BiomeType BiomeType) const
{
    float TotalInfluence = 0.0f;

    // Check zone-based influence
    for (const FBiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            TotalInfluence += CalculateDistanceInfluence(WorldLocation, Zone);
        }
    }

    // Add noise-based influence
    float NoiseValue = CalculateNoiseValue(WorldLocation);
    EWorld_BiomeType NoiseBiome = DetermineBiomeFromNoise(NoiseValue, WorldLocation);
    if (NoiseBiome == BiomeType)
    {
        TotalInfluence += 0.5f;
    }

    return FMath::Clamp(TotalInfluence, 0.0f, 1.0f);
}

TArray<EWorld_BiomeType> UWorld_BiomeSystem::GetNearbyBiomes(FVector WorldLocation, float Radius) const
{
    TArray<EWorld_BiomeType> NearbyBiomes;
    TSet<EWorld_BiomeType> UniqueTypes;

    // Sample points in a circle around the location
    int32 SampleCount = 16;
    for (int32 i = 0; i < SampleCount; ++i)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        FVector SamplePoint = WorldLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );

        EWorld_BiomeType BiomeAtPoint = GetBiomeAtLocation(SamplePoint);
        UniqueTypes.Add(BiomeAtPoint);
    }

    // Convert set to array
    for (EWorld_BiomeType BiomeType : UniqueTypes)
    {
        NearbyBiomes.Add(BiomeType);
    }

    return NearbyBiomes;
}

void UWorld_BiomeSystem::RegisterBiomeZone(FVector Center, float Radius, EWorld_BiomeType BiomeType)
{
    FBiomeZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.BiomeType = BiomeType;
    NewZone.Strength = 1.0f;

    BiomeZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Registered %s biome zone at %s with radius %.1f"), 
        *UEnum::GetValueAsString(BiomeType), *Center.ToString(), Radius);
}

void UWorld_BiomeSystem::UpdateBiomeTransitions()
{
    // Clear existing transitions
    BiomeTransitions.Empty();

    // Generate transitions between adjacent biome zones
    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        for (int32 j = i + 1; j < BiomeZones.Num(); ++j)
        {
            const FBiomeZone& ZoneA = BiomeZones[i];
            const FBiomeZone& ZoneB = BiomeZones[j];

            float Distance = FVector::Dist(ZoneA.Center, ZoneB.Center);
            float CombinedRadius = ZoneA.Radius + ZoneB.Radius;

            // If zones are close enough, create a transition
            if (Distance < CombinedRadius + BiomeBlendDistance)
            {
                FWorld_BiomeTransition Transition;
                Transition.FromBiome = ZoneA.BiomeType;
                Transition.ToBiome = ZoneB.BiomeType;
                Transition.TransitionWidth = FMath::Min(BiomeBlendDistance, Distance * 0.5f);
                Transition.BlendStrength = 0.5f;

                BiomeTransitions.Add(Transition);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Updated %d biome transitions"), BiomeTransitions.Num());
}

void UWorld_BiomeSystem::GenerateDefaultBiomes()
{
    // Clear existing zones
    BiomeZones.Empty();

    // Create default biome layout
    RegisterBiomeZone(FVector(0, 0, 0), 2000.0f, EWorld_BiomeType::Forest);
    RegisterBiomeZone(FVector(5000, 0, 0), 1500.0f, EWorld_BiomeType::Plains);
    RegisterBiomeZone(FVector(-3000, 3000, -50), 1200.0f, EWorld_BiomeType::Swampland);
    RegisterBiomeZone(FVector(3000, -4000, 300), 1800.0f, EWorld_BiomeType::Canyon);
    RegisterBiomeZone(FVector(-5000, -2000, 600), 2200.0f, EWorld_BiomeType::Mountains);

    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Generated %d default biome zones"), BiomeZones.Num());
}

float UWorld_BiomeSystem::GetTemperatureAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add elevation-based temperature variation
    float ElevationEffect = -WorldLocation.Z * 0.01f; // Cooler at higher elevations
    
    return BiomeData.Temperature + ElevationEffect;
}

float UWorld_BiomeSystem::GetHumidityAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.Humidity;
}

FLinearColor UWorld_BiomeSystem::GetBiomeColorAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.BiomeColor;
}

void UWorld_BiomeSystem::DebugDrawBiomes()
{
    if (!GetWorld()) return;

    // Draw biome zones
    for (const FBiomeZone& Zone : BiomeZones)
    {
        FWorld_BiomeData BiomeData = GetBiomeData(Zone.BiomeType);
        FColor DebugColor = BiomeData.BiomeColor.ToFColor(true);
        
        DrawDebugSphere(GetWorld(), Zone.Center, Zone.Radius, 32, DebugColor, false, 10.0f, 0, 5.0f);
        
        // Draw biome name
        FString BiomeName = BiomeData.BiomeName;
        DrawDebugString(GetWorld(), Zone.Center + FVector(0, 0, Zone.Radius + 100), BiomeName, nullptr, DebugColor, 10.0f);
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Debug drawing %d biome zones"), BiomeZones.Num());
}

void UWorld_BiomeSystem::ClearBiomeZones()
{
    BiomeZones.Empty();
    BiomeTransitions.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Cleared all biome zones"));
}

float UWorld_BiomeSystem::CalculateNoiseValue(FVector WorldLocation) const
{
    // Simple Perlin-like noise using sin/cos functions
    float X = WorldLocation.X * BiomeNoiseScale;
    float Y = WorldLocation.Y * BiomeNoiseScale;
    
    float Noise1 = FMath::Sin(X) * FMath::Cos(Y);
    float Noise2 = FMath::Sin(X * 2.0f) * FMath::Cos(Y * 2.0f) * 0.5f;
    float Noise3 = FMath::Sin(X * 4.0f) * FMath::Cos(Y * 4.0f) * 0.25f;
    
    return (Noise1 + Noise2 + Noise3) * 0.5f + 0.5f; // Normalize to 0-1
}

EWorld_BiomeType UWorld_BiomeSystem::DetermineBiomeFromNoise(float NoiseValue, FVector WorldLocation) const
{
    // Use noise value and elevation to determine biome
    float Elevation = WorldLocation.Z;
    
    if (Elevation > 400.0f)
    {
        return EWorld_BiomeType::Mountains;
    }
    else if (Elevation < -10.0f)
    {
        return EWorld_BiomeType::Swampland;
    }
    else if (NoiseValue > 0.7f)
    {
        return EWorld_BiomeType::Forest;
    }
    else if (NoiseValue > 0.4f)
    {
        return EWorld_BiomeType::Plains;
    }
    else
    {
        return EWorld_BiomeType::Canyon;
    }
}

float UWorld_BiomeSystem::CalculateDistanceInfluence(FVector Location, const FBiomeZone& Zone) const
{
    float Distance = FVector::Dist(Location, Zone.Center);
    
    if (Distance <= Zone.Radius)
    {
        // Inside the zone - full influence
        return Zone.Strength;
    }
    else if (Distance <= Zone.Radius + BiomeBlendDistance)
    {
        // In transition zone - falloff influence
        float BlendFactor = (Distance - Zone.Radius) / BiomeBlendDistance;
        return Zone.Strength * (1.0f - BlendFactor);
    }
    
    // Outside influence range
    return 0.0f;
}