#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get Mass Entity references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    // Initialize default herd configurations
    InitializeDefaultConfigurations();
    
    // Start update timers
    GetWorld()->GetTimerManager().SetTimer(HerdUpdateTimer, 
        this, &UCrowdSimulationSubsystem::UpdateHerdBehaviors, 
        0.1f, true); // 10 FPS update rate for performance
        
    GetWorld()->GetTimerManager().SetTimer(EcosystemUpdateTimer,
        this, &UCrowdSimulationSubsystem::ProcessTimeOfDayChanges,
        1.0f, true); // 1 second intervals for time-based changes
        
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem initialized - Ready for prehistoric ecosystem simulation"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HerdUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UCrowdSimulationSubsystem::InitializeDefaultConfigurations()
{
    // Small Herbivores (Compsognathus, small dinosaurs)
    FHerdConfiguration SmallHerbConfig;
    SmallHerbConfig.HerdType = EHerdType::SmallHerbivore;
    SmallHerbConfig.MinGroupSize = 50;
    SmallHerbConfig.MaxGroupSize = 200;
    SmallHerbConfig.MovementSpeed = 400.0f;
    SmallHerbConfig.CohesionRadius = 300.0f;
    SmallHerbConfig.SeparationRadius = 100.0f;
    SmallHerbConfig.AlignmentRadius = 200.0f;
    SmallHerbConfig.FleeRadius = 800.0f;
    SmallHerbConfig.WanderRadius = 1500.0f;
    SmallHerbConfig.bIsNocturnal = false;
    SmallHerbConfig.ActivityLevel = 1.0f;
    HerdConfigurations.Add(EHerdType::SmallHerbivore, SmallHerbConfig);

    // Medium Herbivores (Triceratops, Stegosaurus)
    FHerdConfiguration MediumHerbConfig;
    MediumHerbConfig.HerdType = EHerdType::MediumHerbivore;
    MediumHerbConfig.MinGroupSize = 10;
    MediumHerbConfig.MaxGroupSize = 30;
    MediumHerbConfig.MovementSpeed = 250.0f;
    MediumHerbConfig.CohesionRadius = 500.0f;
    MediumHerbConfig.SeparationRadius = 300.0f;
    MediumHerbConfig.AlignmentRadius = 400.0f;
    MediumHerbConfig.FleeRadius = 1200.0f;
    MediumHerbConfig.WanderRadius = 2000.0f;
    MediumHerbConfig.bIsNocturnal = false;
    MediumHerbConfig.ActivityLevel = 0.8f;
    HerdConfigurations.Add(EHerdType::MediumHerbivore, MediumHerbConfig);

    // Large Herbivores (Brontosaurus, Diplodocus)
    FHerdConfiguration LargeHerbConfig;
    LargeHerbConfig.HerdType = EHerdType::LargeHerbivore;
    LargeHerbConfig.MinGroupSize = 3;
    LargeHerbConfig.MaxGroupSize = 8;
    LargeHerbConfig.MovementSpeed = 150.0f;
    LargeHerbConfig.CohesionRadius = 800.0f;
    LargeHerbConfig.SeparationRadius = 500.0f;
    LargeHerbConfig.AlignmentRadius = 600.0f;
    LargeHerbConfig.FleeRadius = 1000.0f;
    LargeHerbConfig.WanderRadius = 3000.0f;
    LargeHerbConfig.bIsNocturnal = false;
    LargeHerbConfig.ActivityLevel = 0.6f;
    HerdConfigurations.Add(EHerdType::LargeHerbivore, LargeHerbConfig);

    // Pack Carnivores (Velociraptors, Deinonychus)
    FHerdConfiguration PackCarnConfig;
    PackCarnConfig.HerdType = EHerdType::PackCarnivore;
    PackCarnConfig.MinGroupSize = 3;
    PackCarnConfig.MaxGroupSize = 12;
    PackCarnConfig.MovementSpeed = 500.0f;
    PackCarnConfig.CohesionRadius = 400.0f;
    PackCarnConfig.SeparationRadius = 200.0f;
    PackCarnConfig.AlignmentRadius = 300.0f;
    PackCarnConfig.FleeRadius = 600.0f;
    PackCarnConfig.WanderRadius = 4000.0f;
    PackCarnConfig.bIsNocturnal = true;
    PackCarnConfig.ActivityLevel = 1.2f;
    HerdConfigurations.Add(EHerdType::PackCarnivore, PackCarnConfig);

    // Solitary Carnivores (T-Rex, Allosaurus)
    FHerdConfiguration SolitaryConfig;
    SolitaryConfig.HerdType = EHerdType::SolitaryCarnivore;
    SolitaryConfig.MinGroupSize = 1;
    SolitaryConfig.MaxGroupSize = 2;
    SolitaryConfig.MovementSpeed = 300.0f;
    SolitaryConfig.CohesionRadius = 1000.0f;
    SolitaryConfig.SeparationRadius = 800.0f;
    SolitaryConfig.AlignmentRadius = 900.0f;
    SolitaryConfig.FleeRadius = 0.0f; // Apex predators don't flee
    SolitaryConfig.WanderRadius = 5000.0f;
    SolitaryConfig.bIsNocturnal = false;
    SolitaryConfig.ActivityLevel = 0.7f;
    HerdConfigurations.Add(EHerdType::SolitaryCarnivore, SolitaryConfig);

    // Flying Creatures (Pteranodons)
    FHerdConfiguration FlyingConfig;
    FlyingConfig.HerdType = EHerdType::FlyingCreatures;
    FlyingConfig.MinGroupSize = 20;
    FlyingConfig.MaxGroupSize = 100;
    FlyingConfig.MovementSpeed = 600.0f;
    FlyingConfig.CohesionRadius = 400.0f;
    FlyingConfig.SeparationRadius = 150.0f;
    FlyingConfig.AlignmentRadius = 300.0f;
    FlyingConfig.FleeRadius = 1500.0f;
    FlyingConfig.WanderRadius = 8000.0f;
    FlyingConfig.bIsNocturnal = false;
    FlyingConfig.ActivityLevel = 1.0f;
    HerdConfigurations.Add(EHerdType::FlyingCreatures, FlyingConfig);

    // Aquatic Life (Plesiosaurs)
    FHerdConfiguration AquaticConfig;
    AquaticConfig.HerdType = EHerdType::AquaticLife;
    AquaticConfig.MinGroupSize = 5;
    AquaticConfig.MaxGroupSize = 15;
    AquaticConfig.MovementSpeed = 350.0f;
    AquaticConfig.CohesionRadius = 600.0f;
    AquaticConfig.SeparationRadius = 300.0f;
    AquaticConfig.AlignmentRadius = 450.0f;
    AquaticConfig.FleeRadius = 1000.0f;
    AquaticConfig.WanderRadius = 6000.0f;
    AquaticConfig.bIsNocturnal = false;
    AquaticConfig.ActivityLevel = 0.8f;
    HerdConfigurations.Add(EHerdType::AquaticLife, AquaticConfig);
}

void UCrowdSimulationSubsystem::SpawnHerd(EHerdType HerdType, FVector Location, int32 GroupSize)
{
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassSpawnerSubsystem not available"));
        return;
    }

    const FHerdConfiguration* Config = HerdConfigurations.Find(HerdType);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No configuration found for HerdType: %d"), (int32)HerdType);
        return;
    }

    // Use provided group size or random within config range
    int32 ActualGroupSize = (GroupSize > 0) ? GroupSize : 
        FMath::RandRange(Config->MinGroupSize, Config->MaxGroupSize);

    // Apply population density multiplier
    ActualGroupSize = FMath::RoundToInt(ActualGroupSize * PopulationDensityMultiplier);
    ActualGroupSize = FMath::Max(1, ActualGroupSize); // Ensure at least 1 entity

    // Track this herd center
    if (!ActiveHerdCenters.Contains(HerdType))
    {
        ActiveHerdCenters.Add(HerdType, TArray<FVector>());
    }
    ActiveHerdCenters[HerdType].Add(Location);

    UE_LOG(LogTemp, Warning, TEXT("Spawned %s herd of %d entities at location %s"), 
        *UEnum::GetValueAsString(HerdType), ActualGroupSize, *Location.ToString());

    // TODO: Implement actual Mass Entity spawning
    // This will be connected to Mass Entity Framework in the next iteration
}

void UCrowdSimulationSubsystem::SpawnEcosystemInRegion(FVector Center, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Spawning ecosystem in region - Center: %s, Radius: %.2f"), 
        *Center.ToString(), Radius);

    // Calculate population based on area and density
    float Area = PI * Radius * Radius;
    float BasePopulation = Area / 1000000.0f; // 1 creature per km²

    // Spawn different types of herds with ecological balance
    int32 SmallHerbivores = FMath::RoundToInt(BasePopulation * 0.4f * PopulationDensityMultiplier);
    int32 MediumHerbivores = FMath::RoundToInt(BasePopulation * 0.3f * PopulationDensityMultiplier);
    int32 LargeHerbivores = FMath::RoundToInt(BasePopulation * 0.15f * PopulationDensityMultiplier);
    int32 PackCarnivores = FMath::RoundToInt(BasePopulation * 0.1f * PopulationDensityMultiplier);
    int32 SolitaryCarnivores = FMath::RoundToInt(BasePopulation * 0.03f * PopulationDensityMultiplier);
    int32 FlyingCreatures = FMath::RoundToInt(BasePopulation * 0.02f * PopulationDensityMultiplier);

    // Spawn herds at random locations within the region
    for (int32 i = 0; i < SmallHerbivores; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::SmallHerbivore, RandomLocation);
    }

    for (int32 i = 0; i < MediumHerbivores; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::MediumHerbivore, RandomLocation);
    }

    for (int32 i = 0; i < LargeHerbivores; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::LargeHerbivore, RandomLocation);
    }

    for (int32 i = 0; i < PackCarnivores; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::PackCarnivore, RandomLocation);
    }

    for (int32 i = 0; i < SolitaryCarnivores; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::SolitaryCarnivore, RandomLocation);
    }

    for (int32 i = 0; i < FlyingCreatures; i++)
    {
        FVector RandomLocation = Center + FMath::VRand() * FMath::RandRange(0.0f, Radius);
        SpawnHerd(EHerdType::FlyingCreatures, RandomLocation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Ecosystem spawned: %d Small, %d Medium, %d Large herbivores, %d Pack, %d Solitary carnivores, %d Flying"),
        SmallHerbivores, MediumHerbivores, LargeHerbivores, PackCarnivores, SolitaryCarnivores, FlyingCreatures);
}

void UCrowdSimulationSubsystem::TriggerMassFleeEvent(FVector ThreatLocation, float Radius, float Intensity)
{
    UE_LOG(LogTemp, Warning, TEXT("MASS FLEE EVENT triggered at %s, radius %.2f, intensity %.2f"), 
        *ThreatLocation.ToString(), Radius, Intensity);

    // This will send flee commands to all entities within the radius
    // Implementation will use Mass Entity messaging system
    
    // For now, log the event for debugging
    if (bDebugVisualizationEnabled && GetWorld())
    {
        DrawDebugSphere(GetWorld(), ThreatLocation, Radius, 32, 
            FColor::Red, false, 5.0f, 0, 10.0f);
    }
}

void UCrowdSimulationSubsystem::TriggerPredatorHuntEvent(FVector PreyLocation, EHerdType PredatorType)
{
    UE_LOG(LogTemp, Warning, TEXT("PREDATOR HUNT EVENT triggered - %s hunting at %s"), 
        *UEnum::GetValueAsString(PredatorType), *PreyLocation.ToString());

    // This will coordinate pack hunting behavior
    // Implementation will use Mass Entity coordination system
}

void UCrowdSimulationSubsystem::SetPopulationDensity(float DensityMultiplier)
{
    PopulationDensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 5.0f);
    UE_LOG(LogTemp, Warning, TEXT("Population density set to %.2f"), PopulationDensityMultiplier);
}

int32 UCrowdSimulationSubsystem::GetActiveEntityCount() const
{
    // TODO: Return actual count from Mass Entity system
    return 0; // Placeholder
}

void UCrowdSimulationSubsystem::SetTimeOfDay(float TimeNormalized)
{
    CurrentTimeOfDay = FMath::Clamp(TimeNormalized, 0.0f, 1.0f);
    ProcessTimeOfDayChanges();
}

void UCrowdSimulationSubsystem::SetDebugVisualization(bool bEnabled)
{
    bDebugVisualizationEnabled = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Debug visualization %s"), 
        bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCrowdSimulationSubsystem::SetHerdConfiguration(EHerdType HerdType, const FHerdConfiguration& Config)
{
    HerdConfigurations.Add(HerdType, Config);
    UE_LOG(LogTemp, Warning, TEXT("Updated configuration for %s"), 
        *UEnum::GetValueAsString(HerdType));
}

FHerdConfiguration UCrowdSimulationSubsystem::GetHerdConfiguration(EHerdType HerdType) const
{
    const FHerdConfiguration* Config = HerdConfigurations.Find(HerdType);
    return Config ? *Config : FHerdConfiguration();
}

TArray<FVector> UCrowdSimulationSubsystem::GetHerdCenters(EHerdType HerdType) const
{
    const TArray<FVector>* Centers = ActiveHerdCenters.Find(HerdType);
    return Centers ? *Centers : TArray<FVector>();
}

void UCrowdSimulationSubsystem::UpdateHerdBehaviors()
{
    // This will be the main update loop for all herd behaviors
    // Called at 10 FPS for performance
    
    // TODO: Implement Mass Entity behavior updates
    // - Flocking behavior (cohesion, separation, alignment)
    // - Predator avoidance
    // - Food seeking
    // - Territorial behavior
    // - Migration patterns
}

void UCrowdSimulationSubsystem::ProcessTimeOfDayChanges()
{
    // Adjust activity levels based on time of day
    for (auto& ConfigPair : HerdConfigurations)
    {
        FHerdConfiguration& Config = ConfigPair.Value;
        
        if (Config.bIsNocturnal)
        {
            // More active at night (time 0.0-0.2 and 0.8-1.0)
            if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f)
            {
                Config.ActivityLevel = FMath::Lerp(Config.ActivityLevel, 1.2f, 0.1f);
            }
            else
            {
                Config.ActivityLevel = FMath::Lerp(Config.ActivityLevel, 0.3f, 0.1f);
            }
        }
        else
        {
            // More active during day (time 0.3-0.7)
            if (CurrentTimeOfDay > 0.3f && CurrentTimeOfDay < 0.7f)
            {
                Config.ActivityLevel = FMath::Lerp(Config.ActivityLevel, 1.0f, 0.1f);
            }
            else
            {
                Config.ActivityLevel = FMath::Lerp(Config.ActivityLevel, 0.5f, 0.1f);
            }
        }
    }
}