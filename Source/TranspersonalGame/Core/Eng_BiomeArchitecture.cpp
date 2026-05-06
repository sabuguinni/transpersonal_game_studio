#include "Eng_BiomeArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UEng_BiomeArchitecture::UEng_BiomeArchitecture()
{
    // Initialize biome zones on construction
}

void UEng_BiomeArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeBiomeZones();
    LogBiomeConfiguration();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome Architecture System Initialized"));
}

void UEng_BiomeArchitecture::InitializeBiomeZones()
{
    // Clear existing zones
    BiomeZones.Empty();
    
    // PANTANO (sudoeste)
    FEng_BiomeZone SwampZone;
    SwampZone.BiomeName = TEXT("Pantano");
    SwampZone.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampZone.MinBounds = FVector(-77500.0f, -76500.0f, -100.0f);
    SwampZone.MaxBounds = FVector(-25000.0f, -15000.0f, 200.0f);
    SwampZone.Temperature = 28.0f;
    SwampZone.Humidity = 0.9f;
    BiomeZones.Add(EEng_BiomeType::Swamp, SwampZone);
    
    // FLORESTA (noroeste)
    FEng_BiomeZone ForestZone;
    ForestZone.BiomeName = TEXT("Floresta");
    ForestZone.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestZone.MinBounds = FVector(-77500.0f, 15000.0f, -50.0f);
    ForestZone.MaxBounds = FVector(-15000.0f, 76500.0f, 300.0f);
    ForestZone.Temperature = 22.0f;
    ForestZone.Humidity = 0.8f;
    BiomeZones.Add(EEng_BiomeType::Forest, ForestZone);
    
    // SAVANA (centro)
    FEng_BiomeZone SavannaZone;
    SavannaZone.BiomeName = TEXT("Savana");
    SavannaZone.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaZone.MinBounds = FVector(-20000.0f, -20000.0f, -50.0f);
    SavannaZone.MaxBounds = FVector(20000.0f, 20000.0f, 150.0f);
    SavannaZone.Temperature = 30.0f;
    SavannaZone.Humidity = 0.4f;
    BiomeZones.Add(EEng_BiomeType::Savanna, SavannaZone);
    
    // DESERTO (leste)
    FEng_BiomeZone DesertZone;
    DesertZone.BiomeName = TEXT("Deserto");
    DesertZone.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertZone.MinBounds = FVector(25000.0f, -30000.0f, -50.0f);
    DesertZone.MaxBounds = FVector(79500.0f, 30000.0f, 200.0f);
    DesertZone.Temperature = 40.0f;
    DesertZone.Humidity = 0.1f;
    BiomeZones.Add(EEng_BiomeType::Desert, DesertZone);
    
    // MONTANHA NEVADA (nordeste)
    FEng_BiomeZone MountainZone;
    MountainZone.BiomeName = TEXT("Montanha Nevada");
    MountainZone.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    MountainZone.MinBounds = FVector(15000.0f, 20000.0f, 200.0f);
    MountainZone.MaxBounds = FVector(79500.0f, 76500.0f, 1000.0f);
    MountainZone.Temperature = -5.0f;
    MountainZone.Humidity = 0.7f;
    BiomeZones.Add(EEng_BiomeType::Mountain, MountainZone);
}

FEng_BiomeZone UEng_BiomeArchitecture::GetBiomeZone(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeZone* Zone = BiomeZones.Find(BiomeType))
    {
        return *Zone;
    }
    
    // Return default zone if not found
    FEng_BiomeZone DefaultZone;
    DefaultZone.BiomeName = TEXT("Invalid");
    return DefaultZone;
}

EEng_BiomeType UEng_BiomeArchitecture::GetBiomeTypeAtLocation(const FVector& Location) const
{
    for (const auto& BiomePair : BiomeZones)
    {
        const FEng_BiomeZone& Zone = BiomePair.Value;
        if (Location.X >= Zone.MinBounds.X && Location.X <= Zone.MaxBounds.X &&
            Location.Y >= Zone.MinBounds.Y && Location.Y <= Zone.MaxBounds.Y)
        {
            return BiomePair.Key;
        }
    }
    
    // Default to Savanna if outside all zones
    return EEng_BiomeType::Savanna;
}

FVector UEng_BiomeArchitecture::GetRandomLocationInBiome(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeZone Zone = GetBiomeZone(BiomeType);
    
    if (Zone.BiomeName == TEXT("Invalid"))
    {
        return FVector::ZeroVector;
    }
    
    // Generate random location within biome bounds
    float RandomX = FMath::RandRange(Zone.MinBounds.X, Zone.MaxBounds.X);
    float RandomY = FMath::RandRange(Zone.MinBounds.Y, Zone.MaxBounds.Y);
    float RandomZ = FMath::RandRange(Zone.MinBounds.Z, Zone.MaxBounds.Z);
    
    return FVector(RandomX, RandomY, RandomZ);
}

bool UEng_BiomeArchitecture::IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const
{
    const FEng_BiomeZone Zone = GetBiomeZone(BiomeType);
    
    return (Location.X >= Zone.MinBounds.X && Location.X <= Zone.MaxBounds.X &&
            Location.Y >= Zone.MinBounds.Y && Location.Y <= Zone.MaxBounds.Y &&
            Location.Z >= Zone.MinBounds.Z && Location.Z <= Zone.MaxBounds.Z);
}

TArray<FEng_BiomeZone> UEng_BiomeArchitecture::GetAllBiomeZones() const
{
    TArray<FEng_BiomeZone> AllZones;
    for (const auto& BiomePair : BiomeZones)
    {
        AllZones.Add(BiomePair.Value);
    }
    return AllZones;
}

bool UEng_BiomeArchitecture::ValidateSpawnLocation(const FVector& Location, const FString& ActorType) const
{
    // Check if location is at origin (forbidden)
    if (Location.Equals(FVector::ZeroVector, 1.0f))
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: FORBIDDEN spawn at origin (0,0,0) for %s"), *ActorType);
        return false;
    }
    
    // Check if location is within any biome
    EEng_BiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    const FEng_BiomeZone Zone = GetBiomeZone(BiomeType);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: %s spawning at %s in biome %s"), 
           *ActorType, *Location.ToString(), *Zone.BiomeName);
    
    return true;
}

void UEng_BiomeArchitecture::ValidateAllActorsInMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: No world found for validation"));
        return;
    }
    
    int32 ValidActors = 0;
    int32 InvalidActors = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && !Actor->IsA<AWorldSettings>())
        {
            FVector ActorLocation = Actor->GetActorLocation();
            FString ActorName = Actor->GetClass()->GetName();
            
            if (ValidateSpawnLocation(ActorLocation, ActorName))
            {
                ValidActors++;
            }
            else
            {
                InvalidActors++;
                UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Invalid actor %s at %s"), 
                       *ActorName, *ActorLocation.ToString());
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Map validation complete - %d valid, %d invalid actors"), 
           ValidActors, InvalidActors);
}

void UEng_BiomeArchitecture::LogBiomeConfiguration() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - BIOME CONFIGURATION ==="));
    
    for (const auto& BiomePair : BiomeZones)
    {
        const FEng_BiomeZone& Zone = BiomePair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Biome: %s | Center: %s | Bounds: %s to %s | Temp: %.1f°C | Humidity: %.1f"), 
               *Zone.BiomeName, 
               *Zone.CenterLocation.ToString(),
               *Zone.MinBounds.ToString(),
               *Zone.MaxBounds.ToString(),
               Zone.Temperature,
               Zone.Humidity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME CONFIGURATION COMPLETE ==="));
}