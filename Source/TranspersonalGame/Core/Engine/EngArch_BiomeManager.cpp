#include "EngArch_BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    bDebugDrawEnabled = false;
    DefaultBiomeRadius = 15000.0f;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system"));
    
    SetupDefaultBiomes();
    SetupBiomeTransitions();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d biomes"), BiomeDataMap.Num());
}

void UEng_BiomeManager::Deinitialize()
{
    BiomeDataMap.Empty();
    BiomeTransitions.Empty();
    
    Super::Deinitialize();
}

bool UEng_BiomeManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_BiomeManager::InitializeBiomes()
{
    SetupDefaultBiomes();
    ValidateBiomeConfiguration();
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    EBiomeType ClosestBiome = EBiomeType::Savana;
    float MinDistance = FLT_MAX;

    for (const auto& BiomePair : BiomeDataMap)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        if (!BiomeData.bIsActive)
            continue;

        float Distance = FVector::Dist(Location, BiomeData.CenterLocation);
        
        if (Distance <= BiomeData.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default savana data if not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EBiomeType::Savana;
    return DefaultData;
}

TArray<FEng_BiomeData> UEng_BiomeManager::GetAllBiomes() const
{
    TArray<FEng_BiomeData> AllBiomes;
    
    for (const auto& BiomePair : BiomeDataMap)
    {
        if (BiomePair.Value.bIsActive)
        {
            AllBiomes.Add(BiomePair.Value);
        }
    }
    
    return AllBiomes;
}

bool UEng_BiomeManager::IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const
{
    const FEng_BiomeData* BiomeData = BiomeDataMap.Find(BiomeType);
    if (!BiomeData || !BiomeData->bIsActive)
    {
        return false;
    }

    float Distance = FVector::Dist(Location, BiomeData->CenterLocation);
    return Distance <= BiomeData->Radius;
}

float UEng_BiomeManager::GetDistanceToBiomeCenter(const FVector& Location, EBiomeType BiomeType) const
{
    const FEng_BiomeData* BiomeData = BiomeDataMap.Find(BiomeType);
    if (!BiomeData)
    {
        return FLT_MAX;
    }

    return FVector::Dist(Location, BiomeData->CenterLocation);
}

FEng_BiomeTransition UEng_BiomeManager::GetBiomeTransition(const FVector& Location) const
{
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        const FEng_BiomeData* FromBiome = BiomeDataMap.Find(Transition.FromBiome);
        const FEng_BiomeData* ToBiome = BiomeDataMap.Find(Transition.ToBiome);
        
        if (!FromBiome || !ToBiome)
            continue;

        float DistanceFromSource = FVector::Dist(Location, FromBiome->CenterLocation);
        float DistanceToTarget = FVector::Dist(Location, ToBiome->CenterLocation);
        
        // Check if we're in the transition zone
        if (DistanceFromSource > (FromBiome->Radius - Transition.TransitionDistance) &&
            DistanceFromSource <= FromBiome->Radius &&
            DistanceToTarget <= (ToBiome->Radius + Transition.TransitionDistance))
        {
            return Transition;
        }
    }
    
    // Return default transition
    FEng_BiomeTransition DefaultTransition;
    return DefaultTransition;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(Location);
    const FEng_BiomeData& BiomeData = GetBiomeData(CurrentBiome);
    
    // Apply elevation and transition modifiers
    float BaseTemperature = BiomeData.Temperature;
    float ElevationModifier = -Location.Z * 0.01f; // Temperature drops with altitude
    
    return BaseTemperature + ElevationModifier;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(Location);
    const FEng_BiomeData& BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.Humidity;
}

bool UEng_BiomeManager::CanDinosaurSpawnAtLocation(const FString& DinosaurType, const FVector& Location) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(Location);
    const FEng_BiomeData& BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.AllowedDinosaurTypes.Contains(DinosaurType);
}

void UEng_BiomeManager::RegisterBiome(const FEng_BiomeData& BiomeData)
{
    BiomeDataMap.Add(BiomeData.BiomeType, BiomeData);
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Registered biome %d"), (int32)BiomeData.BiomeType);
}

void UEng_BiomeManager::UpdateBiomeData(EBiomeType BiomeType, const FEng_BiomeData& NewData)
{
    BiomeDataMap.Add(BiomeType, NewData);
}

void UEng_BiomeManager::SetBiomeActive(EBiomeType BiomeType, bool bActive)
{
    if (FEng_BiomeData* BiomeData = BiomeDataMap.Find(BiomeType))
    {
        BiomeData->bIsActive = bActive;
    }
}

void UEng_BiomeManager::ValidateBiomeConfiguration()
{
    int32 ActiveBiomes = 0;
    
    for (const auto& BiomePair : BiomeDataMap)
    {
        if (BiomePair.Value.bIsActive)
        {
            ActiveBiomes++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validation complete - %d active biomes"), ActiveBiomes);
    
    if (ActiveBiomes == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: No active biomes found!"));
    }
}

void UEng_BiomeManager::DrawBiomeBounds(bool bDraw)
{
    bDebugDrawEnabled = bDraw;
    
    if (!bDebugDrawEnabled)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    for (const auto& BiomePair : BiomeDataMap)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        if (!BiomeData.bIsActive)
            continue;

        FColor BiomeColor = FColor::Green;
        switch (BiomePair.Key)
        {
            case EBiomeType::Savana: BiomeColor = FColor::Yellow; break;
            case EBiomeType::Floresta: BiomeColor = FColor::Green; break;
            case EBiomeType::Deserto: BiomeColor = FColor::Orange; break;
            case EBiomeType::Montanha: BiomeColor = FColor::Blue; break;
            case EBiomeType::Pantano: BiomeColor = FColor::Purple; break;
            default: BiomeColor = FColor::White; break;
        }

        DrawDebugSphere(World, BiomeData.CenterLocation, BiomeData.Radius, 32, BiomeColor, false, 5.0f, 0, 100.0f);
    }
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    BiomeDataMap.Empty();

    // Savana (Central area)
    FEng_BiomeData SavanaData;
    SavanaData.BiomeType = EBiomeType::Savana;
    SavanaData.CenterLocation = FVector(0, 0, 0);
    SavanaData.Radius = 20000.0f;
    SavanaData.Temperature = 28.0f;
    SavanaData.Humidity = 0.3f;
    SavanaData.Rainfall = 50.0f;
    SavanaData.AllowedDinosaurTypes = {"TRex", "Velociraptor", "Triceratops"};
    SavanaData.VegetationTypes = {"Grass", "AcaciaTrees", "Bushes"};
    SavanaData.MaxDinosaurPopulation = 25;
    BiomeDataMap.Add(EBiomeType::Savana, SavanaData);

    // Floresta (Northwest)
    FEng_BiomeData FlorestaData;
    FlorestaData.BiomeType = EBiomeType::Floresta;
    FlorestaData.CenterLocation = FVector(-45000, 40000, 200);
    FlorestaData.Radius = 18000.0f;
    FlorestaData.Temperature = 22.0f;
    FlorestaData.Humidity = 0.8f;
    FlorestaData.Rainfall = 200.0f;
    FlorestaData.AllowedDinosaurTypes = {"Brachiosaurus", "Parasaurolophus", "Protoceratops"};
    FlorestaData.VegetationTypes = {"DenseTrees", "Ferns", "Vines"};
    FlorestaData.MaxDinosaurPopulation = 15;
    BiomeDataMap.Add(EBiomeType::Floresta, FlorestaData);

    // Deserto (Southeast)
    FEng_BiomeData DesertoData;
    DesertoData.BiomeType = EBiomeType::Deserto;
    DesertoData.CenterLocation = FVector(50000, -40000, -100);
    DesertoData.Radius = 22000.0f;
    DesertoData.Temperature = 35.0f;
    DesertoData.Humidity = 0.1f;
    DesertoData.Rainfall = 10.0f;
    DesertoData.AllowedDinosaurTypes = {"Ankylosaurus", "Pachycephalo"};
    DesertoData.VegetationTypes = {"Cacti", "DesertShrubs"};
    DesertoData.MaxDinosaurPopulation = 10;
    BiomeDataMap.Add(EBiomeType::Deserto, DesertoData);

    // Montanha (Northeast)
    FEng_BiomeData MontanhaData;
    MontanhaData.BiomeType = EBiomeType::Montanha;
    MontanhaData.CenterLocation = FVector(30000, 50000, 1000);
    MontanhaData.Radius = 15000.0f;
    MontanhaData.Temperature = 15.0f;
    MontanhaData.Humidity = 0.6f;
    MontanhaData.Rainfall = 150.0f;
    MontanhaData.AllowedDinosaurTypes = {"Tsintaosaurus"};
    MontanhaData.VegetationTypes = {"PineTrees", "RockFormations"};
    MontanhaData.MaxDinosaurPopulation = 8;
    BiomeDataMap.Add(EBiomeType::Montanha, MontanhaData);

    // Pantano (Southwest)
    FEng_BiomeData PantanoData;
    PantanoData.BiomeType = EBiomeType::Pantano;
    PantanoData.CenterLocation = FVector(-30000, -35000, -50);
    PantanoData.Radius = 12000.0f;
    PantanoData.Temperature = 26.0f;
    PantanoData.Humidity = 0.9f;
    PantanoData.Rainfall = 300.0f;
    PantanoData.AllowedDinosaurTypes = {"Brachiosaurus", "Parasaurolophus"};
    PantanoData.VegetationTypes = {"SwampTrees", "Reeds", "Moss"};
    PantanoData.MaxDinosaurPopulation = 12;
    BiomeDataMap.Add(EBiomeType::Pantano, PantanoData);
}

void UEng_BiomeManager::SetupBiomeTransitions()
{
    BiomeTransitions.Empty();

    // Savana to Floresta
    FEng_BiomeTransition SavanaToFloresta;
    SavanaToFloresta.FromBiome = EBiomeType::Savana;
    SavanaToFloresta.ToBiome = EBiomeType::Floresta;
    SavanaToFloresta.TransitionDistance = 3000.0f;
    SavanaToFloresta.BlendFactor = 0.5f;
    BiomeTransitions.Add(SavanaToFloresta);

    // Savana to Deserto
    FEng_BiomeTransition SavanaToDeserto;
    SavanaToDeserto.FromBiome = EBiomeType::Savana;
    SavanaToDeserto.ToBiome = EBiomeType::Deserto;
    SavanaToDeserto.TransitionDistance = 2500.0f;
    SavanaToDeserto.BlendFactor = 0.4f;
    BiomeTransitions.Add(SavanaToDeserto);

    // Floresta to Montanha
    FEng_BiomeTransition FlorestaToMontanha;
    FlorestaToMontanha.FromBiome = EBiomeType::Floresta;
    FlorestaToMontanha.ToBiome = EBiomeType::Montanha;
    FlorestaToMontanha.TransitionDistance = 2000.0f;
    FlorestaToMontanha.BlendFactor = 0.6f;
    BiomeTransitions.Add(FlorestaToMontanha);

    // Savana to Pantano
    FEng_BiomeTransition SavanaToPantano;
    SavanaToPantano.FromBiome = EBiomeType::Savana;
    SavanaToPantano.ToBiome = EBiomeType::Pantano;
    SavanaToPantano.TransitionDistance = 2200.0f;
    SavanaToPantano.BlendFactor = 0.3f;
    BiomeTransitions.Add(SavanaToPantano);
}

float UEng_BiomeManager::CalculateBlendFactor(const FVector& Location, const FEng_BiomeData& BiomeA, const FEng_BiomeData& BiomeB) const
{
    float DistanceA = FVector::Dist(Location, BiomeA.CenterLocation);
    float DistanceB = FVector::Dist(Location, BiomeB.CenterLocation);
    
    float TotalDistance = DistanceA + DistanceB;
    if (TotalDistance == 0.0f)
        return 0.5f;
    
    return DistanceB / TotalDistance;
}

EBiomeType UEng_BiomeManager::FindClosestBiome(const FVector& Location) const
{
    EBiomeType ClosestBiome = EBiomeType::Savana;
    float MinDistance = FLT_MAX;

    for (const auto& BiomePair : BiomeDataMap)
    {
        if (!BiomePair.Value.bIsActive)
            continue;

        float Distance = FVector::Dist(Location, BiomePair.Value.CenterLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}