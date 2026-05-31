#include "Eng_BiomeArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Static biome center definitions from memory
const FVector UEng_BiomeArchitect::SavannaCenter = FVector(0.0f, 0.0f, 100.0f);
const FVector UEng_BiomeArchitect::SwampCenter = FVector(-50000.0f, -45000.0f, 100.0f);
const FVector UEng_BiomeArchitect::ForestCenter = FVector(-45000.0f, 40000.0f, 100.0f);
const FVector UEng_BiomeArchitect::DesertCenter = FVector(55000.0f, 0.0f, 100.0f);
const FVector UEng_BiomeArchitect::MountainCenter = FVector(40000.0f, 50000.0f, 100.0f);
const float UEng_BiomeArchitect::BiomeRadius = 15000.0f;

UEng_BiomeArchitect::UEng_BiomeArchitect()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UEng_BiomeArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultBiomes();
    SetupEnvironmentalConstraints();
    ConfigureSpeciesDistribution();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitect: Initialized with %d biome definitions"), BiomeDefinitions.Num());
}

void UEng_BiomeArchitect::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savanna biome
    FBiomeDefinition Savanna;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.BiomeName = TEXT("African Savanna");
    Savanna.CenterLocation = SavannaCenter;
    Savanna.Radius = BiomeRadius;
    Savanna.TemperatureRange = FVector2D(25.0f, 35.0f);
    Savanna.HumidityRange = FVector2D(0.3f, 0.6f);
    Savanna.VegetationDensity = 0.4f;
    BiomeDefinitions.Add(Savanna);
    
    // Swamp biome
    FBiomeDefinition Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.BiomeName = TEXT("Prehistoric Wetlands");
    Swamp.CenterLocation = SwampCenter;
    Swamp.Radius = BiomeRadius;
    Swamp.TemperatureRange = FVector2D(20.0f, 30.0f);
    Swamp.HumidityRange = FVector2D(0.8f, 1.0f);
    Swamp.VegetationDensity = 0.8f;
    BiomeDefinitions.Add(Swamp);
    
    // Forest biome
    FBiomeDefinition Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.BiomeName = TEXT("Dense Prehistoric Forest");
    Forest.CenterLocation = ForestCenter;
    Forest.Radius = BiomeRadius;
    Forest.TemperatureRange = FVector2D(15.0f, 25.0f);
    Forest.HumidityRange = FVector2D(0.6f, 0.9f);
    Forest.VegetationDensity = 0.9f;
    BiomeDefinitions.Add(Forest);
    
    // Desert biome
    FBiomeDefinition Desert;
    Desert.BiomeType = EBiomeType::Desert;
    Desert.BiomeName = TEXT("Arid Badlands");
    Desert.CenterLocation = DesertCenter;
    Desert.Radius = BiomeRadius;
    Desert.TemperatureRange = FVector2D(30.0f, 45.0f);
    Desert.HumidityRange = FVector2D(0.1f, 0.3f);
    Desert.VegetationDensity = 0.1f;
    BiomeDefinitions.Add(Desert);
    
    // Mountain biome
    FBiomeDefinition Mountain;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.BiomeName = TEXT("Rocky Highlands");
    Mountain.CenterLocation = MountainCenter;
    Mountain.Radius = BiomeRadius;
    Mountain.TemperatureRange = FVector2D(5.0f, 20.0f);
    Mountain.HumidityRange = FVector2D(0.4f, 0.7f);
    Mountain.VegetationDensity = 0.3f;
    BiomeDefinitions.Add(Mountain);
}

void UEng_BiomeArchitect::SetupEnvironmentalConstraints()
{
    BiomeConstraints.Empty();
    
    // Savanna constraints
    FEnvironmentalConstraints SavannaConstraints;
    SavannaConstraints.MaxWindSpeed = 15.0f;
    SavannaConstraints.RainProbability = 0.2f;
    SavannaConstraints.MaxVisibility = 10000.0f;
    SavannaConstraints.AllowedTimeOfDay = ETimeOfDay::All;
    BiomeConstraints.Add(EBiomeType::Savanna, SavannaConstraints);
    
    // Swamp constraints
    FEnvironmentalConstraints SwampConstraints;
    SwampConstraints.MaxWindSpeed = 8.0f;
    SwampConstraints.RainProbability = 0.6f;
    SwampConstraints.MaxVisibility = 3000.0f;
    SwampConstraints.AllowedTimeOfDay = ETimeOfDay::All;
    BiomeConstraints.Add(EBiomeType::Swamp, SwampConstraints);
    
    // Forest constraints
    FEnvironmentalConstraints ForestConstraints;
    ForestConstraints.MaxWindSpeed = 12.0f;
    ForestConstraints.RainProbability = 0.4f;
    ForestConstraints.MaxVisibility = 2000.0f;
    ForestConstraints.AllowedTimeOfDay = ETimeOfDay::All;
    BiomeConstraints.Add(EBiomeType::Forest, ForestConstraints);
    
    // Desert constraints
    FEnvironmentalConstraints DesertConstraints;
    DesertConstraints.MaxWindSpeed = 25.0f;
    DesertConstraints.RainProbability = 0.05f;
    DesertConstraints.MaxVisibility = 15000.0f;
    DesertConstraints.AllowedTimeOfDay = ETimeOfDay::All;
    BiomeConstraints.Add(EBiomeType::Desert, DesertConstraints);
    
    // Mountain constraints
    FEnvironmentalConstraints MountainConstraints;
    MountainConstraints.MaxWindSpeed = 30.0f;
    MountainConstraints.RainProbability = 0.3f;
    MountainConstraints.MaxVisibility = 12000.0f;
    MountainConstraints.AllowedTimeOfDay = ETimeOfDay::All;
    BiomeConstraints.Add(EBiomeType::Mountain, MountainConstraints);
}

void UEng_BiomeArchitect::ConfigureSpeciesDistribution()
{
    AllowedSpeciesPerBiome.Empty();
    
    // Savanna species
    TArray<EDinosaurSpecies> SavannaSpecies;
    SavannaSpecies.Add(EDinosaurSpecies::TRex);
    SavannaSpecies.Add(EDinosaurSpecies::Velociraptor);
    SavannaSpecies.Add(EDinosaurSpecies::Triceratops);
    SavannaSpecies.Add(EDinosaurSpecies::Parasaurolophus);
    AllowedSpeciesPerBiome.Add(EBiomeType::Savanna, SavannaSpecies);
    
    // Swamp species
    TArray<EDinosaurSpecies> SwampSpecies;
    SwampSpecies.Add(EDinosaurSpecies::TRex);
    SwampSpecies.Add(EDinosaurSpecies::Triceratops);
    SwampSpecies.Add(EDinosaurSpecies::Parasaurolophus);
    SwampSpecies.Add(EDinosaurSpecies::Ankylosaurus);
    AllowedSpeciesPerBiome.Add(EBiomeType::Swamp, SwampSpecies);
    
    // Forest species
    TArray<EDinosaurSpecies> ForestSpecies;
    ForestSpecies.Add(EDinosaurSpecies::Velociraptor);
    ForestSpecies.Add(EDinosaurSpecies::Brachiosaurus);
    ForestSpecies.Add(EDinosaurSpecies::Parasaurolophus);
    ForestSpecies.Add(EDinosaurSpecies::Compsognathus);
    AllowedSpeciesPerBiome.Add(EBiomeType::Forest, ForestSpecies);
    
    // Desert species
    TArray<EDinosaurSpecies> DesertSpecies;
    DesertSpecies.Add(EDinosaurSpecies::TRex);
    DesertSpecies.Add(EDinosaurSpecies::Velociraptor);
    DesertSpecies.Add(EDinosaurSpecies::Ankylosaurus);
    DesertSpecies.Add(EDinosaurSpecies::Compsognathus);
    AllowedSpeciesPerBiome.Add(EBiomeType::Desert, DesertSpecies);
    
    // Mountain species
    TArray<EDinosaurSpecies> MountainSpecies;
    MountainSpecies.Add(EDinosaurSpecies::TRex);
    MountainSpecies.Add(EDinosaurSpecies::Velociraptor);
    MountainSpecies.Add(EDinosaurSpecies::Ankylosaurus);
    MountainSpecies.Add(EDinosaurSpecies::Triceratops);
    AllowedSpeciesPerBiome.Add(EBiomeType::Mountain, MountainSpecies);
}

bool UEng_BiomeArchitect::ValidateBiomeConfiguration()
{
    if (BiomeDefinitions.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeArchitect: Invalid biome count. Expected 5, got %d"), BiomeDefinitions.Num());
        return false;
    }
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.Radius <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeArchitect: Invalid radius for biome %s"), *Biome.BiomeName);
            return false;
        }
        
        if (Biome.VegetationDensity < 0.0f || Biome.VegetationDensity > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeArchitect: Invalid vegetation density for biome %s"), *Biome.BiomeName);
            return false;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitect: Configuration validation passed"));
    return true;
}

FBiomeDefinition UEng_BiomeArchitect::GetBiomeDefinition(EBiomeType BiomeType)
{
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitect: Biome definition not found for type %d"), (int32)BiomeType);
    return FBiomeDefinition();
}

bool UEng_BiomeArchitect::CanSpawnSpeciesInBiome(EDinosaurSpecies Species, EBiomeType BiomeType)
{
    if (AllowedSpeciesPerBiome.Contains(BiomeType))
    {
        const TArray<EDinosaurSpecies>& AllowedSpecies = AllowedSpeciesPerBiome[BiomeType];
        return AllowedSpecies.Contains(Species);
    }
    
    return false;
}

FVector UEng_BiomeArchitect::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna: return SavannaCenter;
        case EBiomeType::Swamp: return SwampCenter;
        case EBiomeType::Forest: return ForestCenter;
        case EBiomeType::Desert: return DesertCenter;
        case EBiomeType::Mountain: return MountainCenter;
        default: return FVector::ZeroVector;
    }
}

EBiomeType UEng_BiomeArchitect::GetBiomeAtLocation(FVector WorldLocation)
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    for (const FBiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

int32 UEng_BiomeArchitect::GetActorCountInBiome(EBiomeType BiomeType)
{
    if (!GetWorld())
    {
        return 0;
    }
    
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    int32 ActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && FVector::Dist2D(Actor->GetActorLocation(), BiomeCenter) <= BiomeRadius)
        {
            ActorCount++;
        }
    }
    
    return ActorCount;
}

bool UEng_BiomeArchitect::IsWithinPerformanceLimits(EBiomeType BiomeType)
{
    int32 ActorCount = GetActorCountInBiome(BiomeType);
    return ActorCount <= MaxActorsPerBiome;
}