#include "BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UBiomeManager::UBiomeManager()
{
    BiomeTransitionRadius = 200.0f;
    UpdateFrequency = 1.0f;
    bEnableDebugDraw = false;
    LastUpdateTime = 0.0f;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system"));
    
    // Setup default biome configurations
    SetupDefaultBiomes();
    
    // Initialize biome zones
    InitializeBiomes();
    
    // Start update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            UpdateTimer,
            FTimerDelegate::CreateUObject(this, &UBiomeManager::UpdateBiomeTransitions, UpdateFrequency),
            UpdateFrequency,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialization complete"));
}

void UBiomeManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimer);
    }
    
    BiomeZones.Empty();
    BiomeDataMap.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Deinitialized"));
    
    Super::Deinitialize();
}

bool UBiomeManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UBiomeManager::SetupDefaultBiomes()
{
    // Forest Biome
    FEng_BiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.BiomeName = TEXT("Dense Forest");
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 80.0f;
    ForestBiome.Fertility = 90.0f;
    ForestBiome.BiomeColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
    ForestBiome.VegetationTypes.Add(TEXT("Oak Trees"));
    ForestBiome.VegetationTypes.Add(TEXT("Ferns"));
    ForestBiome.VegetationTypes.Add(TEXT("Moss"));
    BiomeDataMap.Add(EBiomeType::Forest, ForestBiome);

    // Desert Biome
    FEng_BiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.BiomeName = TEXT("Arid Desert");
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 15.0f;
    DesertBiome.Fertility = 20.0f;
    DesertBiome.BiomeColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    DesertBiome.VegetationTypes.Add(TEXT("Cacti"));
    DesertBiome.VegetationTypes.Add(TEXT("Desert Shrubs"));
    BiomeDataMap.Add(EBiomeType::Desert, DesertBiome);

    // Swamp Biome
    FEng_BiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Murky Swampland");
    SwampBiome.Temperature = 26.0f;
    SwampBiome.Humidity = 95.0f;
    SwampBiome.Fertility = 70.0f;
    SwampBiome.BiomeColor = FLinearColor(0.1f, 0.5f, 0.2f, 1.0f);
    SwampBiome.VegetationTypes.Add(TEXT("Cypress Trees"));
    SwampBiome.VegetationTypes.Add(TEXT("Water Lilies"));
    SwampBiome.VegetationTypes.Add(TEXT("Swamp Grass"));
    BiomeDataMap.Add(EBiomeType::Swamp, SwampBiome);

    // Plains Biome
    FEng_BiomeData PlainsBiome;
    PlainsBiome.BiomeType = EBiomeType::Plains;
    PlainsBiome.BiomeName = TEXT("Open Plains");
    PlainsBiome.Temperature = 24.0f;
    PlainsBiome.Humidity = 45.0f;
    PlainsBiome.Fertility = 60.0f;
    PlainsBiome.BiomeColor = FLinearColor(0.4f, 0.7f, 0.2f, 1.0f);
    PlainsBiome.VegetationTypes.Add(TEXT("Tall Grass"));
    PlainsBiome.VegetationTypes.Add(TEXT("Wildflowers"));
    BiomeDataMap.Add(EBiomeType::Plains, PlainsBiome);

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Default biomes configured (%d types)"), BiomeDataMap.Num());
}

void UBiomeManager::InitializeBiomes()
{
    // Clear existing zones
    BiomeZones.Empty();
    
    // Create default biome zones for testing
    CreateTestBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d biome zones"), BiomeZones.Num());
}

void UBiomeManager::CreateTestBiomes()
{
    // Forest zone (southwest)
    FEng_BiomeZone ForestZone;
    ForestZone.Center = FVector(-1000.0f, -1000.0f, 0.0f);
    ForestZone.Radius = 800.0f;
    ForestZone.BiomeData = BiomeDataMap[EBiomeType::Forest];
    ForestZone.bIsActive = true;
    BiomeZones.Add(ForestZone);

    // Desert zone (northeast)
    FEng_BiomeZone DesertZone;
    DesertZone.Center = FVector(1000.0f, 1000.0f, 0.0f);
    DesertZone.Radius = 800.0f;
    DesertZone.BiomeData = BiomeDataMap[EBiomeType::Desert];
    DesertZone.bIsActive = true;
    BiomeZones.Add(DesertZone);

    // Swamp zone (northwest)
    FEng_BiomeZone SwampZone;
    SwampZone.Center = FVector(-1000.0f, 1000.0f, 0.0f);
    SwampZone.Radius = 600.0f;
    SwampZone.BiomeData = BiomeDataMap[EBiomeType::Swamp];
    SwampZone.bIsActive = true;
    BiomeZones.Add(SwampZone);

    // Plains zone (center)
    FEng_BiomeZone PlainsZone;
    PlainsZone.Center = FVector(0.0f, 0.0f, 0.0f);
    PlainsZone.Radius = 1200.0f;
    PlainsZone.BiomeData = BiomeDataMap[EBiomeType::Plains];
    PlainsZone.bIsActive = true;
    BiomeZones.Add(PlainsZone);

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Created test biomes - Forest, Desert, Swamp, Plains"));
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Plains; // Default fallback

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        if (!Zone.bIsActive) continue;

        float Distance = FVector::Dist(Location, Zone.Center);
        
        // If within zone radius, return this biome
        if (Distance <= Zone.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Zone.BiomeData.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData UBiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }

    // Return default plains data if not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EBiomeType::Plains;
    DefaultData.BiomeName = TEXT("Unknown Biome");
    return DefaultData;
}

void UBiomeManager::AddBiomeZone(const FEng_BiomeZone& NewZone)
{
    BiomeZones.Add(NewZone);
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Added biome zone at (%f, %f, %f)"), 
           NewZone.Center.X, NewZone.Center.Y, NewZone.Center.Z);
}

void UBiomeManager::RemoveBiomeZone(const FVector& Location)
{
    for (int32 i = BiomeZones.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(Location, BiomeZones[i].Center);
        if (Distance <= BiomeZones[i].Radius)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Removed biome zone at (%f, %f, %f)"), 
                   BiomeZones[i].Center.X, BiomeZones[i].Center.Y, BiomeZones[i].Center.Z);
            BiomeZones.RemoveAt(i);
            break;
        }
    }
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.Temperature;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.Humidity;
}

float UBiomeManager::GetFertilityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.Fertility;
}

void UBiomeManager::UpdateBiomeTransitions(float DeltaTime)
{
    LastUpdateTime += DeltaTime;
    
    // Periodic biome system updates
    if (LastUpdateTime >= UpdateFrequency)
    {
        // Debug drawing if enabled
        if (bEnableDebugDraw)
        {
            DebugDrawBiomes();
        }
        
        LastUpdateTime = 0.0f;
    }
}

bool UBiomeManager::IsLocationInBiomeTransition(const FVector& Location) const
{
    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        if (!Zone.bIsActive) continue;

        float Distance = FVector::Dist(Location, Zone.Center);
        float TransitionStart = Zone.Radius - BiomeTransitionRadius;
        
        // Check if in transition zone (near edge of biome)
        if (Distance >= TransitionStart && Distance <= Zone.Radius + BiomeTransitionRadius)
        {
            return true;
        }
    }
    
    return false;
}

void UBiomeManager::DebugDrawBiomes() const
{
    if (UWorld* World = GetWorld())
    {
        for (const FEng_BiomeZone& Zone : BiomeZones)
        {
            if (!Zone.bIsActive) continue;

            // Draw biome zone boundary
            DrawDebugCircle(
                World,
                Zone.Center,
                Zone.Radius,
                32,
                Zone.BiomeData.BiomeColor.ToFColor(true),
                false,
                UpdateFrequency + 0.1f,
                0,
                5.0f,
                FVector(0, 1, 0),
                FVector(1, 0, 0)
            );

            // Draw biome name
            DrawDebugString(
                World,
                Zone.Center + FVector(0, 0, 100),
                Zone.BiomeData.BiomeName,
                nullptr,
                Zone.BiomeData.BiomeColor.ToFColor(true),
                UpdateFrequency + 0.1f
            );
        }
    }
}

FEng_BiomeZone* UBiomeManager::FindBiomeZoneAtLocation(const FVector& Location)
{
    for (FEng_BiomeZone& Zone : BiomeZones)
    {
        if (!Zone.bIsActive) continue;

        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius)
        {
            return &Zone;
        }
    }
    
    return nullptr;
}

float UBiomeManager::CalculateDistanceWeight(const FVector& Location, const FEng_BiomeZone& Zone) const
{
    float Distance = FVector::Dist(Location, Zone.Center);
    if (Distance >= Zone.Radius)
    {
        return 0.0f;
    }
    
    // Linear falloff from center to edge
    return 1.0f - (Distance / Zone.Radius);
}