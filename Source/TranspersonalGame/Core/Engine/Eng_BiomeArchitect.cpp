#include "Eng_BiomeArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_BiomeArchitect::UEng_BiomeArchitect()
{
    GlobalMaxActors = 20000;
    MaxDinosaursTotal = 150;
    DefaultBiomeRadius = 5000.0f;
}

void UEng_BiomeArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome Architecture System Initializing"));
    
    InitializeBiomeDefinitions();
    SetupDefaultBiomes();
    SetupDefaultTransitions();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome Architecture System Ready - %d biomes defined"), BiomeDefinitions.Num());
}

void UEng_BiomeArchitect::Deinitialize()
{
    BiomeDefinitions.Empty();
    BiomeTransitions.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome Architecture System Shutdown"));
    
    Super::Deinitialize();
}

void UEng_BiomeArchitect::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();
    BiomeTransitions.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Initializing biome definitions"));
}

FEng_BiomeDefinition UEng_BiomeArchitect::GetBiomeDefinition(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return *Found;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome definition not found for type %d"), (int32)BiomeType);
    return FEng_BiomeDefinition();
}

void UEng_BiomeArchitect::RegisterBiomeDefinition(const FEng_BiomeDefinition& BiomeDefinition)
{
    if (ValidateBiomeDefinition(BiomeDefinition))
    {
        BiomeDefinitions.Add(BiomeDefinition.BiomeType, BiomeDefinition);
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered biome %s"), *BiomeDefinition.BiomeName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Invalid biome definition for %s"), *BiomeDefinition.BiomeName);
    }
}

TArray<FEng_BiomeDefinition> UEng_BiomeArchitect::GetAllBiomeDefinitions() const
{
    TArray<FEng_BiomeDefinition> Definitions;
    BiomeDefinitions.GenerateValueArray(Definitions);
    return Definitions;
}

void UEng_BiomeArchitect::RegisterBiomeTransition(const FEng_BiomeTransition& Transition)
{
    // Check if transition already exists
    for (const FEng_BiomeTransition& ExistingTransition : BiomeTransitions)
    {
        if (ExistingTransition.FromBiome == Transition.FromBiome && 
            ExistingTransition.ToBiome == Transition.ToBiome)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Transition already exists from %d to %d"), 
                   (int32)Transition.FromBiome, (int32)Transition.ToBiome);
            return;
        }
    }
    
    BiomeTransitions.Add(Transition);
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered biome transition from %d to %d"), 
           (int32)Transition.FromBiome, (int32)Transition.ToBiome);
}

FEng_BiomeTransition UEng_BiomeArchitect::GetBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const
{
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == FromBiome && Transition.ToBiome == ToBiome)
        {
            return Transition;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: No transition found from %d to %d"), (int32)FromBiome, (int32)ToBiome);
    return FEng_BiomeTransition();
}

bool UEng_BiomeArchitect::HasBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const
{
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == FromBiome && Transition.ToBiome == ToBiome)
        {
            return true;
        }
    }
    return false;
}

bool UEng_BiomeArchitect::ValidateBiomeLayout(const TArray<EBiomeType>& BiomeLayout) const
{
    if (BiomeLayout.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Empty biome layout"));
        return false;
    }
    
    // Check if all biomes are defined
    for (EBiomeType BiomeType : BiomeLayout)
    {
        if (!BiomeDefinitions.Contains(BiomeType))
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Undefined biome type %d in layout"), (int32)BiomeType);
            return false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Biome layout validated - %d biomes"), BiomeLayout.Num());
    return true;
}

int32 UEng_BiomeArchitect::GetMaxActorsForBiome(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->MaxActorsPerBiome;
    }
    return 4000; // Default limit
}

float UEng_BiomeArchitect::GetBiomeRadius(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->BiomeRadius;
    }
    return DefaultBiomeRadius;
}

bool UEng_BiomeArchitect::CanDinosaurSpawnInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->NativeDinosaurs.Contains(Species);
    }
    return false;
}

bool UEng_BiomeArchitect::CanVegetationSpawnInBiome(EVegetationType VegetationType, EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->NativeVegetation.Contains(VegetationType);
    }
    return false;
}

TArray<EDinosaurSpecies> UEng_BiomeArchitect::GetNativeDinosaursForBiome(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->NativeDinosaurs;
    }
    return TArray<EDinosaurSpecies>();
}

TArray<EVegetationType> UEng_BiomeArchitect::GetNativeVegetationForBiome(EBiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = BiomeDefinitions.Find(BiomeType))
    {
        return Found->NativeVegetation;
    }
    return TArray<EVegetationType>();
}

bool UEng_BiomeArchitect::IsActorLimitReached(EBiomeType BiomeType, int32 CurrentActorCount) const
{
    int32 MaxActors = GetMaxActorsForBiome(BiomeType);
    return CurrentActorCount >= MaxActors;
}

int32 UEng_BiomeArchitect::GetRemainingActorBudget(EBiomeType BiomeType, int32 CurrentActorCount) const
{
    int32 MaxActors = GetMaxActorsForBiome(BiomeType);
    return FMath::Max(0, MaxActors - CurrentActorCount);
}

void UEng_BiomeArchitect::SetupDefaultBiomes()
{
    // Forest Biome
    FEng_BiomeDefinition ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.BiomeName = TEXT("Dense Forest");
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.Elevation = 150.0f;
    ForestBiome.NativeDinosaurs = {EDinosaurSpecies::Parasaurolophus, EDinosaurSpecies::Triceratops, EDinosaurSpecies::Velociraptor};
    ForestBiome.NativeVegetation = {EVegetationType::Tree, EVegetationType::Fern, EVegetationType::Moss};
    ForestBiome.MaxActorsPerBiome = 4000;
    ForestBiome.BiomeRadius = 6000.0f;
    RegisterBiomeDefinition(ForestBiome);

    // Desert Biome
    FEng_BiomeDefinition DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.BiomeName = TEXT("Arid Desert");
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.2f;
    DesertBiome.Elevation = 50.0f;
    DesertBiome.NativeDinosaurs = {EDinosaurSpecies::TRex, EDinosaurSpecies::Ankylosaurus};
    DesertBiome.NativeVegetation = {EVegetationType::Cactus, EVegetationType::DeadTree};
    DesertBiome.MaxActorsPerBiome = 3000;
    DesertBiome.BiomeRadius = 7000.0f;
    RegisterBiomeDefinition(DesertBiome);

    // Plains Biome
    FEng_BiomeDefinition PlainsBiome;
    PlainsBiome.BiomeType = EBiomeType::Plains;
    PlainsBiome.BiomeName = TEXT("Open Plains");
    PlainsBiome.Temperature = 25.0f;
    PlainsBiome.Humidity = 0.5f;
    PlainsBiome.Elevation = 100.0f;
    PlainsBiome.NativeDinosaurs = {EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Parasaurolophus, EDinosaurSpecies::TRex};
    PlainsBiome.NativeVegetation = {EVegetationType::Grass, EVegetationType::Bush};
    PlainsBiome.MaxActorsPerBiome = 5000;
    PlainsBiome.BiomeRadius = 8000.0f;
    RegisterBiomeDefinition(PlainsBiome);

    // Swamp Biome
    FEng_BiomeDefinition SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Murky Swamp");
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 0.9f;
    SwampBiome.Elevation = 20.0f;
    SwampBiome.NativeDinosaurs = {EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Parasaurolophus};
    SwampBiome.NativeVegetation = {EVegetationType::Tree, EVegetationType::Fern, EVegetationType::Moss, EVegetationType::Vine};
    SwampBiome.MaxActorsPerBiome = 3500;
    SwampBiome.BiomeRadius = 5500.0f;
    RegisterBiomeDefinition(SwampBiome);

    // Mountain Biome
    FEng_BiomeDefinition MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.BiomeName = TEXT("Rocky Mountains");
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.4f;
    MountainBiome.Elevation = 500.0f;
    MountainBiome.NativeDinosaurs = {EDinosaurSpecies::Ankylosaurus, EDinosaurSpecies::TRex};
    MountainBiome.NativeVegetation = {EVegetationType::Bush, EVegetationType::DeadTree};
    MountainBiome.MaxActorsPerBiome = 2500;
    MountainBiome.BiomeRadius = 4500.0f;
    RegisterBiomeDefinition(MountainBiome);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Setup %d default biomes"), BiomeDefinitions.Num());
}

void UEng_BiomeArchitect::SetupDefaultTransitions()
{
    // Forest to Plains
    FEng_BiomeTransition ForestToPlains;
    ForestToPlains.FromBiome = EBiomeType::Forest;
    ForestToPlains.ToBiome = EBiomeType::Plains;
    ForestToPlains.TransitionWidth = 1500.0f;
    ForestToPlains.BlendFactor = 0.6f;
    RegisterBiomeTransition(ForestToPlains);

    // Plains to Desert
    FEng_BiomeTransition PlainsToDesert;
    PlainsToDesert.FromBiome = EBiomeType::Plains;
    PlainsToDesert.ToBiome = EBiomeType::Desert;
    PlainsToDesert.TransitionWidth = 2000.0f;
    PlainsToDesert.BlendFactor = 0.7f;
    RegisterBiomeTransition(PlainsToDesert);

    // Forest to Swamp
    FEng_BiomeTransition ForestToSwamp;
    ForestToSwamp.FromBiome = EBiomeType::Forest;
    ForestToSwamp.ToBiome = EBiomeType::Swamp;
    ForestToSwamp.TransitionWidth = 1200.0f;
    ForestToSwamp.BlendFactor = 0.8f;
    RegisterBiomeTransition(ForestToSwamp);

    // Plains to Mountain
    FEng_BiomeTransition PlainsToMountain;
    PlainsToMountain.FromBiome = EBiomeType::Plains;
    PlainsToMountain.ToBiome = EBiomeType::Mountain;
    PlainsToMountain.TransitionWidth = 1800.0f;
    PlainsToMountain.BlendFactor = 0.5f;
    RegisterBiomeTransition(PlainsToMountain);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Setup %d biome transitions"), BiomeTransitions.Num());
}

bool UEng_BiomeArchitect::ValidateBiomeDefinition(const FEng_BiomeDefinition& BiomeDefinition) const
{
    if (BiomeDefinition.BiomeName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Biome definition has empty name"));
        return false;
    }
    
    if (BiomeDefinition.MaxActorsPerBiome <= 0 || BiomeDefinition.MaxActorsPerBiome > 10000)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Invalid actor limit %d for biome %s"), 
               BiomeDefinition.MaxActorsPerBiome, *BiomeDefinition.BiomeName);
        return false;
    }
    
    if (BiomeDefinition.BiomeRadius <= 0.0f || BiomeDefinition.BiomeRadius > 20000.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Invalid radius %f for biome %s"), 
               BiomeDefinition.BiomeRadius, *BiomeDefinition.BiomeName);
        return false;
    }
    
    return true;
}