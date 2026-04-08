#include "JurassicCrowdManager.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AJurassicCrowdManager::AJurassicCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    // Configurações padrão de manadas
    FHerdConfiguration SmallHerbivoreConfig;
    SmallHerbivoreConfig.HerdType = EDinosaurHerdType::SmallHerbivore;
    SmallHerbivoreConfig.MinHerdSize = 15;
    SmallHerbivoreConfig.MaxHerdSize = 50;
    SmallHerbivoreConfig.MaxSpeed = 1200.0f;
    HerdConfigurations.Add(SmallHerbivoreConfig);

    FHerdConfiguration LargeHerbivoreConfig;
    LargeHerbivoreConfig.HerdType = EDinosaurHerdType::LargeHerbivore;
    LargeHerbivoreConfig.MinHerdSize = 3;
    LargeHerbivoreConfig.MaxHerdSize = 12;
    LargeHerbivoreConfig.MaxSpeed = 800.0f;
    HerdConfigurations.Add(LargeHerbivoreConfig);
}

void AJurassicCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    SpawnInitialHerds();
}

void AJurassicCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CurrentGameTime += DeltaTime;
    UpdateSimulation(DeltaTime);
}

void AJurassicCrowdManager::SpawnHerd(EDinosaurHerdType HerdType, FVector Location, int32 HerdSize)
{
    if (!MassEntitySubsystem) return;
    
    FHerdConfiguration* Config = GetHerdConfiguration(HerdType);
    if (!Config) return;

    int32 FinalHerdSize = HerdSize > 0 ? HerdSize : FMath::RandRange(Config->MinHerdSize, Config->MaxHerdSize);
    OnHerdSpawned.Broadcast(HerdType, Location);
}

int32 AJurassicCrowdManager::GetActiveEntityCount() const
{
    return ActiveHerds.Num();
}

FHerdConfiguration* AJurassicCrowdManager::GetHerdConfiguration(EDinosaurHerdType HerdType)
{
    for (FHerdConfiguration& Config : HerdConfigurations)
    {
        if (Config.HerdType == HerdType) return &Config;
    }
    return nullptr;
}

void AJurassicCrowdManager::UpdateSimulation(float DeltaTime)
{
    // Core simulation logic
}

void AJurassicCrowdManager::SpawnInitialHerds()
{
    SpawnHerd(EDinosaurHerdType::SmallHerbivore, FVector(10000, 5000, 0));
    SpawnHerd(EDinosaurHerdType::LargeHerbivore, FVector(-8000, 12000, 0));
}