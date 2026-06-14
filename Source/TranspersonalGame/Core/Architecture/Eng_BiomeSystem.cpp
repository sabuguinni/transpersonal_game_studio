#include "Eng_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    BiomeTransitionDistance = 1000.0f;
    BiomeMapResolution = 512;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeBiomeDatabase();
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager initialized"));
}

void UEng_BiomeManager::Deinitialize()
{
    BiomeDatabase.Empty();
    BiomeRegions.Empty();
    Super::Deinitialize();
}

void UEng_BiomeManager::InitializeBiomeDatabase()
{
    // Plains biome
    FEng_BiomeData PlainsData;
    PlainsData.BiomeType = EBiomeType::Plains;
    PlainsData.Temperature = 22.0f;
    PlainsData.Humidity = 0.4f;
    PlainsData.Elevation = 100.0f;
    PlainsData.VegetationDensity = 0.6f;
    PlainsData.DinosaurSpawnRate = 0.3f;
    BiomeDatabase.Add(EBiomeType::Plains, PlainsData);

    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.Temperature = 18.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 200.0f;
    ForestData.VegetationDensity = 1.5f;
    ForestData.DinosaurSpawnRate = 0.5f;
    BiomeDatabase.Add(EBiomeType::Forest, ForestData);

    // Desert biome
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.1f;
    DesertData.Elevation = 50.0f;
    DesertData.VegetationDensity = 0.1f;
    DesertData.DinosaurSpawnRate = 0.2f;
    BiomeDatabase.Add(EBiomeType::Desert, DesertData);

    // Mountain biome
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EBiomeType::Mountain;
    MountainData.Temperature = 5.0f;
    MountainData.Humidity = 0.6f;
    MountainData.Elevation = 800.0f;
    MountainData.VegetationDensity = 0.3f;
    MountainData.DinosaurSpawnRate = 0.1f;
    BiomeDatabase.Add(EBiomeType::Mountain, MountainData);

    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.Temperature = 28.0f;
    SwampData.Humidity = 0.95f;
    SwampData.Elevation = -20.0f;
    SwampData.VegetationDensity = 1.2f;
    SwampData.DinosaurSpawnRate = 0.4f;
    BiomeDatabase.Add(EBiomeType::Swamp, SwampData);
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation)
{
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    float Humidity = GetHumidityAtLocation(WorldLocation);
    float Elevation = WorldLocation.Z;

    return CalculateBiomeFromEnvironment(Temperature, Humidity, Elevation);
}

EBiomeType UEng_BiomeManager::CalculateBiomeFromEnvironment(float Temperature, float Humidity, float Elevation)
{
    // High elevation = Mountain
    if (Elevation > 600.0f)
    {
        return EBiomeType::Mountain;
    }
    
    // Low elevation + high humidity = Swamp
    if (Elevation < 50.0f && Humidity > 0.8f)
    {
        return EBiomeType::Swamp;
    }
    
    // High temperature + low humidity = Desert
    if (Temperature > 30.0f && Humidity < 0.3f)
    {
        return EBiomeType::Desert;
    }
    
    // High humidity + moderate temperature = Forest
    if (Humidity > 0.6f && Temperature > 15.0f && Temperature < 25.0f)
    {
        return EBiomeType::Forest;
    }
    
    // Default to Plains
    return EBiomeType::Plains;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EBiomeType BiomeType)
{
    if (BiomeDatabase.Contains(BiomeType))
    {
        return BiomeDatabase[BiomeType];
    }
    
    return FEng_BiomeData(); // Return default
}

void UEng_BiomeManager::SetBiomeData(EBiomeType BiomeType, const FEng_BiomeData& BiomeData)
{
    BiomeDatabase.Add(BiomeType, BiomeData);
}

TArray<EBiomeType> UEng_BiomeManager::GetAllBiomeTypes()
{
    TArray<EBiomeType> BiomeTypes;
    BiomeDatabase.GetKeys(BiomeTypes);
    return BiomeTypes;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation)
{
    float BaseTemp = 25.0f;
    float ElevationEffect = -0.006f * WorldLocation.Z; // Temperature drops with elevation
    float LatitudeEffect = GetNoiseValue(FVector2D(WorldLocation.X, WorldLocation.Y), 0.0001f) * 20.0f;
    
    return BaseTemp + ElevationEffect + LatitudeEffect;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation)
{
    float BaseHumidity = 0.5f;
    float NoiseVariation = GetNoiseValue(FVector2D(WorldLocation.X, WorldLocation.Y), 0.0005f) * 0.4f;
    
    return FMath::Clamp(BaseHumidity + NoiseVariation, 0.0f, 1.0f);
}

float UEng_BiomeManager::GetNoiseValue(const FVector2D& Location, float Scale)
{
    // Simple Perlin-like noise using sine waves
    float X = Location.X * Scale;
    float Y = Location.Y * Scale;
    
    float Noise = FMath::Sin(X * 2.1f + Y * 1.7f) * 0.5f +
                  FMath::Sin(X * 1.3f - Y * 2.3f) * 0.3f +
                  FMath::Sin(X * 3.7f + Y * 0.9f) * 0.2f;
    
    return (Noise + 1.0f) * 0.5f; // Normalize to 0-1
}

void UEng_BiomeManager::GenerateBiomeMap(int32 WorldSize)
{
    BiomeRegions.Empty();
    
    int32 RegionCount = FMath::Max(1, WorldSize / 2000); // One region per 2km
    
    for (int32 i = 0; i < RegionCount; i++)
    {
        FVector2D RegionCenter;
        RegionCenter.X = FMath::RandRange(-WorldSize / 2, WorldSize / 2);
        RegionCenter.Y = FMath::RandRange(-WorldSize / 2, WorldSize / 2);
        BiomeRegions.Add(RegionCenter);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d biome regions for world size %d"), RegionCount, WorldSize);
}

void UEng_BiomeManager::UpdateBiomeTransitions()
{
    // Update transition zones between biomes
    // This would be called periodically to smooth biome boundaries
    UE_LOG(LogTemp, Log, TEXT("Biome transitions updated"));
}

// BiomeComponent implementation
UEng_BiomeComponent::UEng_BiomeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    UpdateInterval = 1.0f;
    CurrentBiome = EBiomeType::Plains;
    TimeSinceLastUpdate = 0.0f;
}

void UEng_BiomeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    BiomeManager = GetWorld()->GetGameInstance()->GetSubsystem<UEng_BiomeManager>();
    if (BiomeManager)
    {
        UpdateCurrentBiome();
    }
}

void UEng_BiomeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdateCurrentBiome();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UEng_BiomeComponent::UpdateCurrentBiome()
{
    if (!BiomeManager || !GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    EBiomeType NewBiome = BiomeManager->GetBiomeAtLocation(OwnerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        EBiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
        
        UE_LOG(LogTemp, Log, TEXT("Biome changed from %d to %d at location %s"), 
               (int32)OldBiome, (int32)NewBiome, *OwnerLocation.ToString());
    }
}

FEng_BiomeData UEng_BiomeComponent::GetCurrentBiomeData() const
{
    if (BiomeManager)
    {
        return BiomeManager->GetBiomeData(CurrentBiome);
    }
    
    return FEng_BiomeData();
}