#include "Eng_BiomeSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

UEng_BiomeSystemManager::UEng_BiomeSystemManager()
{
    bAutoInitializeOnStartup = true;
    BiomeTransitionZone = 2000.0f;
}

void UEng_BiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing..."));
    
    if (bAutoInitializeOnStartup)
    {
        InitializeBiomes();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initialization complete"));
}

void UEng_BiomeSystemManager::Deinitialize()
{
    BiomeActors.Empty();
    BiomeDefinitions.Empty();
    DistributionRules.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Deinitialized"));
    
    Super::Deinitialize();
}

void UEng_BiomeSystemManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Setting up biomes..."));
    
    SetupDefaultBiomes();
    SetupDefaultDistributionRules();
    
    // Clear existing actor tracking
    BiomeActors.Empty();
    for (const auto& BiomePair : BiomeDefinitions)
    {
        BiomeActors.Add(BiomePair.Key, TArray<TWeakObjectPtr<AActor>>());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: %d biomes initialized"), BiomeDefinitions.Num());
}

void UEng_BiomeSystemManager::SetupDefaultBiomes()
{
    // Savanna Biome (Center of map)
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    Savanna.Radius = 15000.0f;
    Savanna.TemperatureRange = 30.0f;
    Savanna.HumidityLevel = 0.3f;
    Savanna.DangerLevel = 0.4f;
    Savanna.NativeDinosaurs = {EDinosaurSpecies::TRex, EDinosaurSpecies::Triceratops, EDinosaurSpecies::Velociraptor};
    Savanna.VegetationTypes = {TEXT("GrassPlains"), TEXT("AcaciaTrees"), TEXT("SavannaFlowers")};
    BiomeDefinitions.Add(EBiomeType::Savanna, Savanna);

    // Swamp Biome
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    Swamp.Radius = 15000.0f;
    Swamp.TemperatureRange = 28.0f;
    Swamp.HumidityLevel = 0.9f;
    Swamp.DangerLevel = 0.7f;
    Swamp.NativeDinosaurs = {EDinosaurSpecies::Parasaurolophus, EDinosaurSpecies::Ankylosaurus, EDinosaurSpecies::Velociraptor};
    Swamp.VegetationTypes = {TEXT("SwampVegetation"), TEXT("Mangroves"), TEXT("WaterLilies")};
    BiomeDefinitions.Add(EBiomeType::Swamp, Swamp);

    // Forest Biome
    FEng_BiomeDefinition Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    Forest.Radius = 15000.0f;
    Forest.TemperatureRange = 22.0f;
    Forest.HumidityLevel = 0.7f;
    Forest.DangerLevel = 0.5f;
    Forest.NativeDinosaurs = {EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Parasaurolophus, EDinosaurSpecies::Protoceratops};
    Forest.VegetationTypes = {TEXT("ConiferTrees"), TEXT("Ferns"), TEXT("ForestFlowers")};
    BiomeDefinitions.Add(EBiomeType::Forest, Forest);

    // Desert Biome
    FEng_BiomeDefinition Desert;
    Desert.BiomeType = EBiomeType::Desert;
    Desert.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    Desert.Radius = 15000.0f;
    Desert.TemperatureRange = 45.0f;
    Desert.HumidityLevel = 0.1f;
    Desert.DangerLevel = 0.6f;
    Desert.NativeDinosaurs = {EDinosaurSpecies::Pachycephalosaurus, EDinosaurSpecies::Protoceratops, EDinosaurSpecies::Velociraptor};
    Desert.VegetationTypes = {TEXT("CactusPlants"), TEXT("DesertShrubs"), TEXT("SandDunes")};
    BiomeDefinitions.Add(EBiomeType::Desert, Desert);

    // Mountain Biome
    FEng_BiomeDefinition Mountain;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    Mountain.Radius = 15000.0f;
    Mountain.TemperatureRange = 15.0f;
    Mountain.HumidityLevel = 0.4f;
    Mountain.DangerLevel = 0.8f;
    Mountain.NativeDinosaurs = {EDinosaurSpecies::TRex, EDinosaurSpecies::Pachycephalosaurus, EDinosaurSpecies::Ankylosaurus};
    Mountain.VegetationTypes = {TEXT("AlpinePlants"), TEXT("RockFormations"), TEXT("MountainFlowers")};
    BiomeDefinitions.Add(EBiomeType::Mountain, Mountain);
}

void UEng_BiomeSystemManager::SetupDefaultDistributionRules()
{
    // Equal distribution across all biomes (20% each)
    for (const auto& BiomePair : BiomeDefinitions)
    {
        FEng_BiomeDistributionRule Rule;
        Rule.TargetBiome = BiomePair.Key;
        Rule.SpawnPercentage = 0.2f;
        Rule.MaxActorsPerBiome = 1000;
        Rule.bRequiresBiomeCompatibility = true;
        DistributionRules.Add(Rule);
    }
}

EBiomeType UEng_BiomeSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;

    for (const auto& BiomePair : BiomeDefinitions)
    {
        const FEng_BiomeDefinition& Biome = BiomePair.Value;
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        
        if (Distance <= Biome.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

FEng_BiomeDefinition UEng_BiomeSystemManager::GetBiomeDefinition(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return *Found;
    }
    
    // Return default savanna if not found
    return BiomeDefinitions.FindRef(EBiomeType::Savanna);
}

TArray<FVector> UEng_BiomeSystemManager::GetDistributedSpawnLocations(int32 TotalActors, float SpawnRadius) const
{
    TArray<FVector> SpawnLocations;
    
    for (const FEng_BiomeDistributionRule& Rule : DistributionRules)
    {
        int32 ActorsForThisBiome = FMath::RoundToInt(TotalActors * Rule.SpawnPercentage);
        ActorsForThisBiome = FMath::Min(ActorsForThisBiome, Rule.MaxActorsPerBiome);
        
        const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(Rule.TargetBiome);
        if (!BiomeDef)
        {
            continue;
        }
        
        for (int32 i = 0; i < ActorsForThisBiome; i++)
        {
            FVector SpawnLocation = GetRandomLocationInBiome(Rule.TargetBiome, SpawnRadius);
            SpawnLocations.Add(SpawnLocation);
        }
    }
    
    return SpawnLocations;
}

FVector UEng_BiomeSystemManager::GetRandomLocationInBiome(EBiomeType BiomeType, float MinDistanceFromCenter) const
{
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    if (!BiomeDef)
    {
        return FVector::ZeroVector;
    }
    
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(MinDistanceFromCenter, BiomeDef->Radius * 0.8f);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        100.0f // Default spawn height
    );
    
    return BiomeDef->CenterLocation + Offset;
}

bool UEng_BiomeSystemManager::IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const
{
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    if (!BiomeDef)
    {
        return false;
    }
    
    float Distance = FVector::Dist2D(Location, BiomeDef->CenterLocation);
    return Distance <= BiomeDef->Radius;
}

void UEng_BiomeSystemManager::SetDistributionRules(const TArray<FEng_BiomeDistributionRule>& Rules)
{
    DistributionRules = Rules;
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Updated distribution rules - %d rules active"), Rules.Num());
}

TArray<FEng_BiomeDistributionRule> UEng_BiomeSystemManager::GetCurrentDistributionRules() const
{
    return DistributionRules;
}

void UEng_BiomeSystemManager::RegisterActorInBiome(AActor* Actor, EBiomeType BiomeType)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<TWeakObjectPtr<AActor>>* BiomeActorList = BiomeActors.Find(BiomeType);
    if (BiomeActorList)
    {
        BiomeActorList->AddUnique(Actor);
        UE_LOG(LogTemp, Log, TEXT("BiomeSystemManager: Registered actor %s in biome %d"), 
               *Actor->GetName(), (int32)BiomeType);
    }
}

int32 UEng_BiomeSystemManager::GetActorCountInBiome(EBiomeType BiomeType) const
{
    const TArray<TWeakObjectPtr<AActor>>* BiomeActorList = BiomeActors.Find(BiomeType);
    if (!BiomeActorList)
    {
        return 0;
    }
    
    // Count valid actors (remove stale weak pointers)
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<AActor>& ActorPtr : *BiomeActorList)
    {
        if (ActorPtr.IsValid())
        {
            ValidCount++;
        }
    }
    
    return ValidCount;
}

TArray<AActor*> UEng_BiomeSystemManager::GetActorsInBiome(EBiomeType BiomeType) const
{
    TArray<AActor*> ValidActors;
    
    const TArray<TWeakObjectPtr<AActor>>* BiomeActorList = BiomeActors.Find(BiomeType);
    if (!BiomeActorList)
    {
        return ValidActors;
    }
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : *BiomeActorList)
    {
        if (ActorPtr.IsValid())
        {
            ValidActors.Add(ActorPtr.Get());
        }
    }
    
    return ValidActors;
}

float UEng_BiomeSystemManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    
    if (BiomeDef)
    {
        // Add some random variation
        float Variation = FMath::RandRange(-5.0f, 5.0f);
        return BiomeDef->TemperatureRange + Variation;
    }
    
    return 25.0f; // Default temperature
}

float UEng_BiomeSystemManager::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    
    if (BiomeDef)
    {
        // Add some random variation
        float Variation = FMath::RandRange(-0.1f, 0.1f);
        return FMath::Clamp(BiomeDef->HumidityLevel + Variation, 0.0f, 1.0f);
    }
    
    return 0.5f; // Default humidity
}

float UEng_BiomeSystemManager::GetDangerLevelAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    
    if (BiomeDef)
    {
        return BiomeDef->DangerLevel;
    }
    
    return 0.5f; // Default danger level
}

void UEng_BiomeSystemManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME CONFIGURATION VALIDATION ==="));
    
    // Check if all biomes are properly configured
    for (const auto& BiomePair : BiomeDefinitions)
    {
        const FEng_BiomeDefinition& Biome = BiomePair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: Center(%s) Radius(%.1f) Temp(%.1f) Humidity(%.2f) Danger(%.2f)"),
               (int32)BiomePair.Key,
               *Biome.CenterLocation.ToString(),
               Biome.Radius,
               Biome.TemperatureRange,
               Biome.HumidityLevel,
               Biome.DangerLevel);
    }
    
    // Check distribution rules
    float TotalPercentage = 0.0f;
    for (const FEng_BiomeDistributionRule& Rule : DistributionRules)
    {
        TotalPercentage += Rule.SpawnPercentage;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Distribution Rules: %d rules, Total Percentage: %.2f"), 
           DistributionRules.Num(), TotalPercentage);
    
    if (FMath::Abs(TotalPercentage - 1.0f) > 0.01f)
    {
        UE_LOG(LogTemp, Error, TEXT("WARNING: Distribution percentages do not sum to 1.0!"));
    }
}

void UEng_BiomeSystemManager::LogBiomeStatistics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME STATISTICS ==="));
    
    for (const auto& BiomePair : BiomeDefinitions)
    {
        int32 ActorCount = GetActorCountInBiome(BiomePair.Key);
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: %d actors registered"),
               (int32)BiomePair.Key, ActorCount);
    }
}

float UEng_BiomeSystemManager::CalculateDistanceToBiomeCenter(const FVector& Location, EBiomeType BiomeType) const
{
    const FEng_BiomeDefinition* BiomeDef = BiomeDefinitions.Find(BiomeType);
    if (!BiomeDef)
    {
        return FLT_MAX;
    }
    
    return FVector::Dist2D(Location, BiomeDef->CenterLocation);
}