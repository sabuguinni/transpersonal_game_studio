#include "EngArch_BiomeSystemCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UEngArch_BiomeSystemCore::UEngArch_BiomeSystemCore()
{
    bIsInitialized = false;
}

void UEngArch_BiomeSystemCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Initializing Biome System Core"));
    
    InitializeBiomeSystem();
}

void UEngArch_BiomeSystemCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Deinitializing Biome System Core"));
    
    RegisteredBiomes.Empty();
    BiomeTransitions.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UEngArch_BiomeSystemCore::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEngArch_BiomeSystemCore::InitializeBiomeSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Biome system already initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Starting biome system initialization"));

    // Clear existing data
    RegisteredBiomes.Empty();
    BiomeTransitions.Empty();

    // Setup default biomes
    SetupDefaultBiomes();

    // Setup default transitions
    SetupDefaultTransitions();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Biome system initialization complete - %d biomes registered"), RegisteredBiomes.Num());
}

void UEngArch_BiomeSystemCore::RegisterBiome(const FEngArch_BiomeProperties& BiomeProperties)
{
    RegisteredBiomes.Add(BiomeProperties.BiomeType, BiomeProperties);
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Registered biome type %d at location (%f, %f, %f)"), 
           (int32)BiomeProperties.BiomeType, 
           BiomeProperties.BiomeCenter.X, 
           BiomeProperties.BiomeCenter.Y, 
           BiomeProperties.BiomeCenter.Z);
}

void UEngArch_BiomeSystemCore::UnregisterBiome(EEngArch_BiomeType BiomeType)
{
    if (RegisteredBiomes.Remove(BiomeType) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Unregistered biome type %d"), (int32)BiomeType);
    }
}

EEngArch_BiomeType UEngArch_BiomeSystemCore::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bIsInitialized || RegisteredBiomes.Num() == 0)
    {
        return EEngArch_BiomeType::None;
    }

    return FindClosestBiome(WorldLocation);
}

FEngArch_BiomeProperties UEngArch_BiomeSystemCore::GetBiomeProperties(EEngArch_BiomeType BiomeType) const
{
    if (const FEngArch_BiomeProperties* FoundBiome = RegisteredBiomes.Find(BiomeType))
    {
        return *FoundBiome;
    }

    return FEngArch_BiomeProperties();
}

TArray<EEngArch_BiomeType> UEngArch_BiomeSystemCore::GetAllRegisteredBiomes() const
{
    TArray<EEngArch_BiomeType> BiomeTypes;
    RegisteredBiomes.GetKeys(BiomeTypes);
    return BiomeTypes;
}

void UEngArch_BiomeSystemCore::AddBiomeTransition(const FEngArch_BiomeTransition& Transition)
{
    BiomeTransitions.Add(Transition);
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Added transition from biome %d to %d"), 
           (int32)Transition.FromBiome, (int32)Transition.ToBiome);
}

FEngArch_BiomeTransition UEngArch_BiomeSystemCore::GetBiomeTransition(const FVector& WorldLocation) const
{
    // Find if location is in a transition zone
    for (const FEngArch_BiomeTransition& Transition : BiomeTransitions)
    {
        const FEngArch_BiomeProperties* FromBiome = RegisteredBiomes.Find(Transition.FromBiome);
        const FEngArch_BiomeProperties* ToBiome = RegisteredBiomes.Find(Transition.ToBiome);

        if (FromBiome && ToBiome)
        {
            float DistanceFromCenter = FVector::Dist(WorldLocation, FromBiome->BiomeCenter);
            float DistanceToTarget = FVector::Dist(WorldLocation, ToBiome->BiomeCenter);

            // Check if we're in the transition zone
            if (DistanceFromCenter > (FromBiome->BiomeRadius - Transition.TransitionDistance) &&
                DistanceFromCenter < FromBiome->BiomeRadius &&
                DistanceToTarget < ToBiome->BiomeRadius)
            {
                return Transition;
            }
        }
    }

    return FEngArch_BiomeTransition();
}

bool UEngArch_BiomeSystemCore::IsInTransitionZone(const FVector& WorldLocation) const
{
    FEngArch_BiomeTransition Transition = GetBiomeTransition(WorldLocation);
    return Transition.FromBiome != EEngArch_BiomeType::None && Transition.ToBiome != EEngArch_BiomeType::None;
}

float UEngArch_BiomeSystemCore::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEngArch_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEngArch_BiomeProperties BiomeProps = GetBiomeProperties(BiomeType);
    
    // Apply elevation-based temperature modification
    float ElevationModifier = -BiomeProps.Elevation * 0.006f; // 6°C per 1000m elevation
    
    return BiomeProps.Temperature + ElevationModifier;
}

float UEngArch_BiomeSystemCore::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEngArch_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEngArch_BiomeProperties BiomeProps = GetBiomeProperties(BiomeType);
    
    return BiomeProps.Humidity;
}

float UEngArch_BiomeSystemCore::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    EEngArch_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEngArch_BiomeProperties BiomeProps = GetBiomeProperties(BiomeType);
    
    return BiomeProps.VegetationDensity;
}

TArray<FString> UEngArch_BiomeSystemCore::GetDinosaurSpeciesAtLocation(const FVector& WorldLocation) const
{
    EEngArch_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEngArch_BiomeProperties BiomeProps = GetBiomeProperties(BiomeType);
    
    return BiomeProps.DinosaurSpecies;
}

int32 UEngArch_BiomeSystemCore::GetRegisteredBiomeCount() const
{
    return RegisteredBiomes.Num();
}

void UEngArch_BiomeSystemCore::SetupDefaultBiomes()
{
    // Savanna Biome (Central)
    FEngArch_BiomeProperties SavannaBiome;
    SavannaBiome.BiomeType = EEngArch_BiomeType::Savanna;
    SavannaBiome.Temperature = 28.0f;
    SavannaBiome.Humidity = 0.4f;
    SavannaBiome.Elevation = 500.0f;
    SavannaBiome.VegetationDensity = 0.3f;
    SavannaBiome.WaterAvailability = 0.4f;
    SavannaBiome.BiomeCenter = FVector(0.0f, 0.0f, 500.0f);
    SavannaBiome.BiomeRadius = 30000.0f;
    SavannaBiome.DinosaurSpecies.Add(TEXT("Trex"));
    SavannaBiome.DinosaurSpecies.Add(TEXT("Velociraptor"));
    SavannaBiome.DinosaurSpecies.Add(TEXT("Triceratops"));
    RegisterBiome(SavannaBiome);

    // Forest Biome (North-West)
    FEngArch_BiomeProperties ForestBiome;
    ForestBiome.BiomeType = EEngArch_BiomeType::Forest;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.Elevation = 300.0f;
    ForestBiome.VegetationDensity = 0.9f;
    ForestBiome.WaterAvailability = 0.8f;
    ForestBiome.BiomeCenter = FVector(-45000.0f, 40000.0f, 300.0f);
    ForestBiome.BiomeRadius = 25000.0f;
    ForestBiome.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    ForestBiome.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    ForestBiome.DinosaurSpecies.Add(TEXT("Protoceratops"));
    RegisterBiome(ForestBiome);

    // Desert Biome (South-East)
    FEngArch_BiomeProperties DesertBiome;
    DesertBiome.BiomeType = EEngArch_BiomeType::Desert;
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.1f;
    DesertBiome.Elevation = 200.0f;
    DesertBiome.VegetationDensity = 0.1f;
    DesertBiome.WaterAvailability = 0.1f;
    DesertBiome.BiomeCenter = FVector(50000.0f, -40000.0f, 200.0f);
    DesertBiome.BiomeRadius = 20000.0f;
    DesertBiome.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    DesertBiome.DinosaurSpecies.Add(TEXT("Pachycephalo"));
    RegisterBiome(DesertBiome);

    // Mountain Biome (North-East)
    FEngArch_BiomeProperties MountainBiome;
    MountainBiome.BiomeType = EEngArch_BiomeType::Mountain;
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.6f;
    MountainBiome.Elevation = 1500.0f;
    MountainBiome.VegetationDensity = 0.4f;
    MountainBiome.WaterAvailability = 0.6f;
    MountainBiome.BiomeCenter = FVector(40000.0f, 45000.0f, 1500.0f);
    MountainBiome.BiomeRadius = 18000.0f;
    MountainBiome.DinosaurSpecies.Add(TEXT("Tsintaosaurus"));
    RegisterBiome(MountainBiome);

    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Setup %d default biomes"), RegisteredBiomes.Num());
}

void UEngArch_BiomeSystemCore::SetupDefaultTransitions()
{
    // Savanna to Forest transition
    FEngArch_BiomeTransition SavannaToForest;
    SavannaToForest.FromBiome = EEngArch_BiomeType::Savanna;
    SavannaToForest.ToBiome = EEngArch_BiomeType::Forest;
    SavannaToForest.TransitionDistance = 8000.0f;
    SavannaToForest.BlendFactor = 0.5f;
    AddBiomeTransition(SavannaToForest);

    // Savanna to Desert transition
    FEngArch_BiomeTransition SavannaToDesert;
    SavannaToDesert.FromBiome = EEngArch_BiomeType::Savanna;
    SavannaToDesert.ToBiome = EEngArch_BiomeType::Desert;
    SavannaToDesert.TransitionDistance = 6000.0f;
    SavannaToDesert.BlendFactor = 0.4f;
    AddBiomeTransition(SavannaToDesert);

    // Forest to Mountain transition
    FEngArch_BiomeTransition ForestToMountain;
    ForestToMountain.FromBiome = EEngArch_BiomeType::Forest;
    ForestToMountain.ToBiome = EEngArch_BiomeType::Mountain;
    ForestToMountain.TransitionDistance = 5000.0f;
    ForestToMountain.BlendFactor = 0.6f;
    AddBiomeTransition(ForestToMountain);

    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemCore: Setup %d default transitions"), BiomeTransitions.Num());
}

float UEngArch_BiomeSystemCore::CalculateDistanceToBiome(const FVector& WorldLocation, const FEngArch_BiomeProperties& BiomeProps) const
{
    return FVector::Dist(WorldLocation, BiomeProps.BiomeCenter);
}

EEngArch_BiomeType UEngArch_BiomeSystemCore::FindClosestBiome(const FVector& WorldLocation) const
{
    EEngArch_BiomeType ClosestBiome = EEngArch_BiomeType::None;
    float ClosestDistance = FLT_MAX;

    for (const auto& BiomePair : RegisteredBiomes)
    {
        float Distance = CalculateDistanceToBiome(WorldLocation, BiomePair.Value);
        
        // Check if location is within biome radius
        if (Distance <= BiomePair.Value.BiomeRadius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}