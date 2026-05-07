#include "Crowd_MassiveCrowdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Crowd_SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdMassiveSystem, Log, All);

UCrowd_MassiveCrowdSystem::UCrowd_MassiveCrowdSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for crowd updates
    
    // Configuração padrão para multidões massivas
    MaxCrowdEntities = 50000;
    ActiveCrowdEntities = 0;
    CrowdDensityPerBiome = 1000;
    LODUpdateInterval = 0.5f;
    CullingDistance = 10000.0f;
    
    // Configuração de biomas
    BiomeConfigurations.Add(ECrowd_BiomeType::Savana, FCrowd_BiomeConfig());
    BiomeConfigurations.Add(ECrowd_BiomeType::Floresta, FCrowd_BiomeConfig());
    BiomeConfigurations.Add(ECrowd_BiomeType::Pantano, FCrowd_BiomeConfig());
    BiomeConfigurations.Add(ECrowd_BiomeType::Deserto, FCrowd_BiomeConfig());
    BiomeConfigurations.Add(ECrowd_BiomeType::Montanha, FCrowd_BiomeConfig());
    
    // Configurar biomas específicos
    SetupBiomeConfigurations();
}

void UCrowd_MassiveCrowdSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Massive Crowd System initialized - Max entities: %d"), MaxCrowdEntities);
    
    // Inicializar Mass Entity Framework
    InitializeMassEntityFramework();
    
    // Spawnar multidões iniciais
    SpawnInitialCrowds();
}

void UCrowd_MassiveCrowdSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Atualizar LOD das multidões
    UpdateCrowdLOD(DeltaTime);
    
    // Atualizar densidade baseada na posição do jogador
    UpdateCrowdDensity(DeltaTime);
    
    // Processar comportamento de massa
    ProcessMassBehavior(DeltaTime);
}

void UCrowd_MassiveCrowdSystem::InitializeMassEntityFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCrowdMassiveSystem, Error, TEXT("World not found for Mass Entity initialization"));
        return;
    }
    
    // Obter subsistema Mass Entity
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdMassiveSystem, Error, TEXT("MassEntitySubsystem not found"));
        return;
    }
    
    // Obter subsistema Mass Spawner
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdMassiveSystem, Error, TEXT("MassSpawnerSubsystem not found"));
        return;
    }
    
    // Obter subsistema Mass Simulation
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    if (!MassSimulationSubsystem)
    {
        UE_LOG(LogCrowdMassiveSystem, Error, TEXT("MassSimulationSubsystem not found"));
        return;
    }
    
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Mass Entity Framework initialized successfully"));
}

void UCrowd_MassiveCrowdSystem::SpawnInitialCrowds()
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogCrowdMassiveSystem, Error, TEXT("Mass subsystems not initialized"));
        return;
    }
    
    // Spawnar multidões em cada bioma
    for (auto& BiomePair : BiomeConfigurations)
    {
        ECrowd_BiomeType BiomeType = BiomePair.Key;
        FCrowd_BiomeConfig& BiomeConfig = BiomePair.Value;
        
        SpawnCrowdInBiome(BiomeType, BiomeConfig);
    }
    
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Initial crowds spawned - Active entities: %d"), ActiveCrowdEntities);
}

void UCrowd_MassiveCrowdSystem::SpawnCrowdInBiome(ECrowd_BiomeType BiomeType, const FCrowd_BiomeConfig& BiomeConfig)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    int32 EntitiesToSpawn = FMath::Min(CrowdDensityPerBiome, MaxCrowdEntities - ActiveCrowdEntities);
    
    if (EntitiesToSpawn <= 0)
    {
        UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Cannot spawn more entities - limit reached"));
        return;
    }
    
    // Criar entidades Mass para este bioma
    TArray<FMassEntityHandle> SpawnedEntities;
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        // Posição aleatória dentro do bioma
        FVector SpawnLocation = BiomeCenter + FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(0.0f, 100.0f)
        );
        
        // Criar entidade Mass
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        if (EntityHandle.IsValid())
        {
            SpawnedEntities.Add(EntityHandle);
            ActiveCrowdEntities++;
        }
    }
    
    UE_LOG(LogCrowdMassiveSystem, Log, TEXT("Spawned %d entities in biome %d at center %s"), 
           SpawnedEntities.Num(), (int32)BiomeType, *BiomeCenter.ToString());
}

void UCrowd_MassiveCrowdSystem::UpdateCrowdLOD(float DeltaTime)
{
    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer < LODUpdateInterval)
    {
        return;
    }
    LODUpdateTimer = 0.0f;
    
    // Obter posição do jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Atualizar LOD baseado na distância do jogador
    for (auto& BiomePair : BiomeConfigurations)
    {
        FVector BiomeCenter = GetBiomeCenter(BiomePair.Key);
        float DistanceToPlayer = FVector::Dist(PlayerLocation, BiomeCenter);
        
        ECrowd_LODLevel NewLOD = CalculateLODLevel(DistanceToPlayer);
        BiomePair.Value.CurrentLOD = NewLOD;
    }
}

void UCrowd_MassiveCrowdSystem::UpdateCrowdDensity(float DeltaTime)
{
    // Obter posição do jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Determinar bioma mais próximo do jogador
    ECrowd_BiomeType ClosestBiome = GetClosestBiome(PlayerLocation);
    
    // Aumentar densidade no bioma mais próximo
    if (BiomeConfigurations.Contains(ClosestBiome))
    {
        FCrowd_BiomeConfig& BiomeConfig = BiomeConfigurations[ClosestBiome];
        BiomeConfig.DensityMultiplier = FMath::FInterpTo(BiomeConfig.DensityMultiplier, 2.0f, DeltaTime, 1.0f);
    }
    
    // Diminuir densidade em biomas distantes
    for (auto& BiomePair : BiomeConfigurations)
    {
        if (BiomePair.Key != ClosestBiome)
        {
            BiomePair.Value.DensityMultiplier = FMath::FInterpTo(BiomePair.Value.DensityMultiplier, 0.5f, DeltaTime, 0.5f);
        }
    }
}

void UCrowd_MassiveCrowdSystem::ProcessMassBehavior(float DeltaTime)
{
    if (!MassSimulationSubsystem)
    {
        return;
    }
    
    // Processar comportamentos de massa (flocking, pathfinding, etc.)
    // Isto seria normalmente feito pelos Mass Processors, mas aqui fazemos uma simulação básica
    
    for (auto& BiomePair : BiomeConfigurations)
    {
        ECrowd_BiomeType BiomeType = BiomePair.Key;
        FCrowd_BiomeConfig& BiomeConfig = BiomePair.Value;
        
        // Simular movimento de massa baseado no tipo de bioma
        ProcessBiomeBehavior(BiomeType, BiomeConfig, DeltaTime);
    }
}

void UCrowd_MassiveCrowdSystem::ProcessBiomeBehavior(ECrowd_BiomeType BiomeType, FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Savana:
            // Comportamento de manada na savana
            ProcessHerdBehavior(BiomeConfig, DeltaTime);
            break;
            
        case ECrowd_BiomeType::Floresta:
            // Comportamento disperso na floresta
            ProcessScatteredBehavior(BiomeConfig, DeltaTime);
            break;
            
        case ECrowd_BiomeType::Pantano:
            // Comportamento cauteloso no pantano
            ProcessCautiousBehavior(BiomeConfig, DeltaTime);
            break;
            
        case ECrowd_BiomeType::Deserto:
            // Comportamento de conservação no deserto
            ProcessConservationBehavior(BiomeConfig, DeltaTime);
            break;
            
        case ECrowd_BiomeType::Montanha:
            // Comportamento de escalada na montanha
            ProcessClimbingBehavior(BiomeConfig, DeltaTime);
            break;
    }
}

void UCrowd_MassiveCrowdSystem::ProcessHerdBehavior(FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    // Simular comportamento de manada (agrupamento, movimento coordenado)
    BiomeConfig.MovementSpeed = FMath::FInterpTo(BiomeConfig.MovementSpeed, 300.0f, DeltaTime, 1.0f);
    BiomeConfig.GroupCohesion = FMath::FInterpTo(BiomeConfig.GroupCohesion, 0.8f, DeltaTime, 0.5f);
}

void UCrowd_MassiveCrowdSystem::ProcessScatteredBehavior(FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    // Comportamento disperso entre árvores
    BiomeConfig.MovementSpeed = FMath::FInterpTo(BiomeConfig.MovementSpeed, 150.0f, DeltaTime, 1.0f);
    BiomeConfig.GroupCohesion = FMath::FInterpTo(BiomeConfig.GroupCohesion, 0.3f, DeltaTime, 0.5f);
}

void UCrowd_MassiveCrowdSystem::ProcessCautiousBehavior(FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    // Comportamento cauteloso no pantano
    BiomeConfig.MovementSpeed = FMath::FInterpTo(BiomeConfig.MovementSpeed, 100.0f, DeltaTime, 1.0f);
    BiomeConfig.GroupCohesion = FMath::FInterpTo(BiomeConfig.GroupCohesion, 0.9f, DeltaTime, 0.5f);
}

void UCrowd_MassiveCrowdSystem::ProcessConservationBehavior(FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    // Comportamento de conservação de energia no deserto
    BiomeConfig.MovementSpeed = FMath::FInterpTo(BiomeConfig.MovementSpeed, 80.0f, DeltaTime, 1.0f);
    BiomeConfig.GroupCohesion = FMath::FInterpTo(BiomeConfig.GroupCohesion, 0.6f, DeltaTime, 0.5f);
}

void UCrowd_MassiveCrowdSystem::ProcessClimbingBehavior(FCrowd_BiomeConfig& BiomeConfig, float DeltaTime)
{
    // Comportamento de escalada na montanha
    BiomeConfig.MovementSpeed = FMath::FInterpTo(BiomeConfig.MovementSpeed, 120.0f, DeltaTime, 1.0f);
    BiomeConfig.GroupCohesion = FMath::FInterpTo(BiomeConfig.GroupCohesion, 0.5f, DeltaTime, 0.5f);
}

FVector UCrowd_MassiveCrowdSystem::GetBiomeCenter(ECrowd_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Savana:
            return FVector(0, 0, 0);
        case ECrowd_BiomeType::Floresta:
            return FVector(-45000, 40000, 0);
        case ECrowd_BiomeType::Pantano:
            return FVector(-50000, -45000, 0);
        case ECrowd_BiomeType::Deserto:
            return FVector(55000, 0, 0);
        case ECrowd_BiomeType::Montanha:
            return FVector(40000, 50000, 500);
        default:
            return FVector::ZeroVector;
    }
}

ECrowd_BiomeType UCrowd_MassiveCrowdSystem::GetClosestBiome(const FVector& Location) const
{
    ECrowd_BiomeType ClosestBiome = ECrowd_BiomeType::Savana;
    float MinDistance = FLT_MAX;
    
    for (int32 i = 0; i < (int32)ECrowd_BiomeType::Count; i++)
    {
        ECrowd_BiomeType BiomeType = (ECrowd_BiomeType)i;
        FVector BiomeCenter = GetBiomeCenter(BiomeType);
        float Distance = FVector::Dist(Location, BiomeCenter);
        
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeType;
        }
    }
    
    return ClosestBiome;
}

ECrowd_LODLevel UCrowd_MassiveCrowdSystem::CalculateLODLevel(float DistanceToPlayer) const
{
    if (DistanceToPlayer < 1000.0f)
    {
        return ECrowd_LODLevel::High;
    }
    else if (DistanceToPlayer < 5000.0f)
    {
        return ECrowd_LODLevel::Medium;
    }
    else if (DistanceToPlayer < CullingDistance)
    {
        return ECrowd_LODLevel::Low;
    }
    else
    {
        return ECrowd_LODLevel::Culled;
    }
}

void UCrowd_MassiveCrowdSystem::SetupBiomeConfigurations()
{
    // Configurar Savana
    FCrowd_BiomeConfig& SavanaConfig = BiomeConfigurations[ECrowd_BiomeType::Savana];
    SavanaConfig.MaxEntities = 15000;
    SavanaConfig.DensityMultiplier = 1.0f;
    SavanaConfig.MovementSpeed = 250.0f;
    SavanaConfig.GroupCohesion = 0.7f;
    SavanaConfig.CurrentLOD = ECrowd_LODLevel::Medium;
    
    // Configurar Floresta
    FCrowd_BiomeConfig& FlorestaConfig = BiomeConfigurations[ECrowd_BiomeType::Floresta];
    FlorestaConfig.MaxEntities = 8000;
    FlorestaConfig.DensityMultiplier = 0.8f;
    FlorestaConfig.MovementSpeed = 150.0f;
    FlorestaConfig.GroupCohesion = 0.4f;
    FlorestaConfig.CurrentLOD = ECrowd_LODLevel::Medium;
    
    // Configurar Pantano
    FCrowd_BiomeConfig& PantanoConfig = BiomeConfigurations[ECrowd_BiomeType::Pantano];
    PantanoConfig.MaxEntities = 5000;
    PantanoConfig.DensityMultiplier = 0.6f;
    PantanoConfig.MovementSpeed = 100.0f;
    PantanoConfig.GroupCohesion = 0.9f;
    PantanoConfig.CurrentLOD = ECrowd_LODLevel::Low;
    
    // Configurar Deserto
    FCrowd_BiomeConfig& DesertoConfig = BiomeConfigurations[ECrowd_BiomeType::Deserto];
    DesertoConfig.MaxEntities = 3000;
    DesertoConfig.DensityMultiplier = 0.4f;
    DesertoConfig.MovementSpeed = 80.0f;
    DesertoConfig.GroupCohesion = 0.6f;
    DesertoConfig.CurrentLOD = ECrowd_LODLevel::Low;
    
    // Configurar Montanha
    FCrowd_BiomeConfig& MontanhaConfig = BiomeConfigurations[ECrowd_BiomeType::Montanha];
    MontanhaConfig.MaxEntities = 4000;
    MontanhaConfig.DensityMultiplier = 0.5f;
    MontanhaConfig.MovementSpeed = 120.0f;
    MontanhaConfig.GroupCohesion = 0.5f;
    MontanhaConfig.CurrentLOD = ECrowd_LODLevel::Low;
}

void UCrowd_MassiveCrowdSystem::SetMaxCrowdEntities(int32 NewMaxEntities)
{
    MaxCrowdEntities = FMath::Clamp(NewMaxEntities, 1000, 100000);
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Max crowd entities set to: %d"), MaxCrowdEntities);
}

void UCrowd_MassiveCrowdSystem::SetCrowdDensityPerBiome(int32 NewDensity)
{
    CrowdDensityPerBiome = FMath::Clamp(NewDensity, 100, 20000);
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Crowd density per biome set to: %d"), CrowdDensityPerBiome);
}

void UCrowd_MassiveCrowdSystem::SetCullingDistance(float NewDistance)
{
    CullingDistance = FMath::Clamp(NewDistance, 1000.0f, 50000.0f);
    UE_LOG(LogCrowdMassiveSystem, Warning, TEXT("Culling distance set to: %f"), CullingDistance);
}

int32 UCrowd_MassiveCrowdSystem::GetActiveCrowdEntities() const
{
    return ActiveCrowdEntities;
}

float UCrowd_MassiveCrowdSystem::GetCrowdDensityInBiome(ECrowd_BiomeType BiomeType) const
{
    if (BiomeConfigurations.Contains(BiomeType))
    {
        const FCrowd_BiomeConfig& Config = BiomeConfigurations[BiomeType];
        return Config.DensityMultiplier;
    }
    return 0.0f;
}

ECrowd_LODLevel UCrowd_MassiveCrowdSystem::GetBiomeLOD(ECrowd_BiomeType BiomeType) const
{
    if (BiomeConfigurations.Contains(BiomeType))
    {
        const FCrowd_BiomeConfig& Config = BiomeConfigurations[BiomeType];
        return Config.CurrentLOD;
    }
    return ECrowd_LODLevel::Culled;
}