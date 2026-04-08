#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Initializing..."));
    
    // Obter referências aos subsistemas Mass
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: Failed to get Mass subsystems!"));
        return;
    }
    
    // Configurar configurações padrão para biomas
    SetupDefaultBiomeConfigurations();
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Initialized successfully"));
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    // Limpar timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PopulationUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(MigrationUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Só criar em mundos de jogo
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::InitializeCrowdSimulation()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationSubsystem: No valid world!"));
        return;
    }
    
    // Iniciar timers de atualização
    GetWorld()->GetTimerManager().SetTimer(
        PopulationUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::UpdatePopulationTick,
        UpdateFrequency,
        true
    );
    
    GetWorld()->GetTimerManager().SetTimer(
        MigrationUpdateTimer,
        this,
        &UCrowdSimulationSubsystem::UpdateMigrationTick,
        60.0f, // Atualizar migrações a cada minuto
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Crowd simulation initialized"));
}

void UCrowdSimulationSubsystem::SetBiomeConfiguration(const FString& BiomeName, const FBiomePopulationConfig& Config)
{
    BiomeConfigurations.Add(BiomeName, Config);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Set configuration for biome %s"), *BiomeName);
}

void UCrowdSimulationSubsystem::SpawnHerdInArea(const FVector& Location, float Radius, const FDinosaurHerdData& HerdData)
{
    if (!CanSpawnInArea(Location, Radius, HerdData.MaxHerdSize))
    {
        return;
    }
    
    // Determinar tamanho da manada
    int32 HerdSize = FMath::RandRange(HerdData.MinHerdSize, HerdData.MaxHerdSize);
    
    // Encontrar localização adequada
    FVector SpawnLocation = FindSuitableSpawnLocation(Location, Radius);
    
    // Spawnar entidades usando Mass Entity
    for (int32 i = 0; i < HerdSize; ++i)
    {
        // Calcular posição individual dentro da manada
        FVector IndividualOffset = FVector(
            FMath::RandRange(-HerdData.FlockingRadius, HerdData.FlockingRadius),
            FMath::RandRange(-HerdData.FlockingRadius, HerdData.FlockingRadius),
            0.0f
        );
        
        FVector IndividualLocation = SpawnLocation + IndividualOffset;
        
        // TODO: Integrar com Mass Entity spawning
        // Esta parte será completada quando o sistema Mass estiver totalmente configurado
    }
    
    // Atualizar contadores
    int32* CurrentCount = ActiveHerdCounts.Find(HerdData.SpeciesName);
    if (CurrentCount)
    {
        *CurrentCount += HerdSize;
    }
    else
    {
        ActiveHerdCounts.Add(HerdData.SpeciesName, HerdSize);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned herd of %d %s at %s"), 
           HerdSize, *HerdData.SpeciesName, *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::SpawnPredatorPackInArea(const FVector& Location, float Radius, const FPredatorPackData& PackData)
{
    if (!CanSpawnInArea(Location, Radius, PackData.MaxPackSize))
    {
        return;
    }
    
    // Determinar tamanho do grupo
    int32 PackSize = FMath::RandRange(PackData.MinPackSize, PackData.MaxPackSize);
    
    // Encontrar localização adequada
    FVector SpawnLocation = FindSuitableSpawnLocation(Location, Radius);
    
    // Spawnar entidades usando Mass Entity
    for (int32 i = 0; i < PackSize; ++i)
    {
        // Calcular posição individual dentro do grupo
        FVector IndividualOffset = FVector(
            FMath::RandRange(-PackData.CoordinationRadius, PackData.CoordinationRadius),
            FMath::RandRange(-PackData.CoordinationRadius, PackData.CoordinationRadius),
            0.0f
        );
        
        FVector IndividualLocation = SpawnLocation + IndividualOffset;
        
        // TODO: Integrar com Mass Entity spawning
        // Esta parte será completada quando o sistema Mass estiver totalmente configurado
    }
    
    // Atualizar contadores
    int32* CurrentCount = ActivePackCounts.Find(PackData.SpeciesName);
    if (CurrentCount)
    {
        *CurrentCount += PackSize;
    }
    else
    {
        ActivePackCounts.Add(PackData.SpeciesName, PackSize);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned pack of %d %s at %s"), 
           PackSize, *PackData.SpeciesName, *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::UpdatePopulationDensity(const FVector& PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;
    
    // Determinar bioma atual baseado na localização do jogador
    FString CurrentBiome = DetermineBiomeAtLocation(PlayerLocation);
    
    const FBiomePopulationConfig* BiomeConfig = BiomeConfigurations.Find(CurrentBiome);
    if (!BiomeConfig)
    {
        return;
    }
    
    // Verificar densidade atual
    float CurrentDensity = GetCurrentPopulationDensity(PlayerLocation, 10000.0f); // 10km radius
    
    // Spawnar manadas se densidade estiver baixa
    for (const FDinosaurHerdData& HerdData : BiomeConfig->HerbivoreHerds)
    {
        int32* CurrentHerdCount = ActiveHerdCounts.Find(HerdData.SpeciesName);
        int32 ActualCount = CurrentHerdCount ? *CurrentHerdCount : 0;
        
        float DesiredDensity = HerdData.PopulationDensityPerKm2;
        float AreaKm2 = (10000.0f * 10000.0f) / (100.0f * 100.0f * 100.0f); // Converter cm² para km²
        int32 DesiredCount = FMath::RoundToInt(DesiredDensity * AreaKm2);
        
        if (ActualCount < DesiredCount)
        {
            // Spawnar nova manada
            FVector SpawnArea = PlayerLocation + FVector(
                FMath::RandRange(-BiomeConfig->MaxSpawnDistanceFromPlayer, BiomeConfig->MaxSpawnDistanceFromPlayer),
                FMath::RandRange(-BiomeConfig->MaxSpawnDistanceFromPlayer, BiomeConfig->MaxSpawnDistanceFromPlayer),
                0.0f
            );
            
            SpawnHerdInArea(SpawnArea, 5000.0f, HerdData);
        }
    }
    
    // Spawnar grupos de predadores se necessário
    for (const FPredatorPackData& PackData : BiomeConfig->PredatorPacks)
    {
        int32* CurrentPackCount = ActivePackCounts.Find(PackData.SpeciesName);
        int32 ActualCount = CurrentPackCount ? *CurrentPackCount : 0;
        
        // Predadores têm densidade muito menor
        if (ActualCount < 2) // Máximo 2 grupos por espécie na área
        {
            FVector SpawnArea = PlayerLocation + FVector(
                FMath::RandRange(-BiomeConfig->MaxSpawnDistanceFromPlayer, BiomeConfig->MaxSpawnDistanceFromPlayer),
                FMath::RandRange(-BiomeConfig->MaxSpawnDistanceFromPlayer, BiomeConfig->MaxSpawnDistanceFromPlayer),
                0.0f
            );
            
            SpawnPredatorPackInArea(SpawnArea, 5000.0f, PackData);
        }
    }
}

int32 UCrowdSimulationSubsystem::GetActiveEntityCount() const
{
    int32 TotalCount = 0;
    
    for (const auto& HerdPair : ActiveHerdCounts)
    {
        TotalCount += HerdPair.Value;
    }
    
    for (const auto& PackPair : ActivePackCounts)
    {
        TotalCount += PackPair.Value;
    }
    
    return TotalCount;
}

float UCrowdSimulationSubsystem::GetCurrentPopulationDensity(const FVector& Location, float Radius) const
{
    // TODO: Implementar cálculo real baseado em entidades Mass na área
    // Por agora, retornar estimativa baseada nos contadores
    
    int32 TotalInArea = 0;
    float AreaKm2 = (Radius * Radius * PI) / (100.0f * 100.0f * 100.0f); // Converter cm² para km²
    
    // Estimativa simples baseada nos contadores ativos
    TotalInArea = GetActiveEntityCount() / 4; // Assumir que 1/4 das entidades estão na área
    
    return TotalInArea / AreaKm2;
}

void UCrowdSimulationSubsystem::TriggerPredatorHuntBehavior(const FVector& Location, float Radius)
{
    // TODO: Implementar comportamento de caça coordenada
    // Enviar sinal para todos os predadores na área para iniciar comportamento de caça
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Triggered predator hunt behavior at %s"), *Location.ToString());
}

void UCrowdSimulationSubsystem::TriggerHerdMigrationBehavior(const FString& SpeciesName)
{
    // TODO: Implementar comportamento de migração
    // Enviar sinal para todas as manadas da espécie para iniciar migração
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Triggered migration for species %s"), *SpeciesName);
}

void UCrowdSimulationSubsystem::ToggleCrowdDebugVisualization(bool bEnabled)
{
    // TODO: Implementar visualização de debug
    // Mostrar densidade populacional, rotas de migração, territórios de predadores, etc.
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Debug visualization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCrowdSimulationSubsystem::UpdatePopulationTick()
{
    if (LastPlayerLocation.IsZero())
    {
        return;
    }
    
    // Atualizar densidade populacional
    UpdatePopulationDensity(LastPlayerLocation);
    
    // Remover entidades distantes
    CullDistantEntities(LastPlayerLocation);
    
    // Verificar limite máximo de entidades
    int32 CurrentTotal = GetActiveEntityCount();
    if (CurrentTotal > MaxTotalEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Entity count (%d) exceeds maximum (%d)"), 
               CurrentTotal, MaxTotalEntities);
        // TODO: Implementar culling inteligente
    }
}

void UCrowdSimulationSubsystem::UpdateMigrationTick()
{
    // TODO: Implementar lógica de migração sazonal
    // Verificar se é época de migração para cada espécie
    // Iniciar comportamentos de migração conforme necessário
    
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationSubsystem: Migration update tick"));
}

void UCrowdSimulationSubsystem::CullDistantEntities(const FVector& PlayerLocation)
{
    // TODO: Implementar culling de entidades distantes
    // Remover entidades que estão além da distância de culling
    // Atualizar contadores apropriadamente
    
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationSubsystem: Culling distant entities from %s"), 
           *PlayerLocation.ToString());
}

bool UCrowdSimulationSubsystem::CanSpawnInArea(const FVector& Location, float Radius, int32 RequestedCount) const
{
    // Verificar se não excederemos o limite máximo
    if (GetActiveEntityCount() + RequestedCount > MaxTotalEntities)
    {
        return false;
    }
    
    // Verificar densidade local
    float CurrentDensity = GetCurrentPopulationDensity(Location, Radius);
    float MaxDensity = 100.0f; // entidades por km²
    
    return CurrentDensity < MaxDensity;
}

FVector UCrowdSimulationSubsystem::FindSuitableSpawnLocation(const FVector& Center, float Radius) const
{
    // TODO: Implementar verificação de terreno adequado
    // Por agora, retornar localização aleatória dentro do raio
    
    FVector RandomOffset = FVector(
        FMath::RandRange(-Radius, Radius),
        FMath::RandRange(-Radius, Radius),
        0.0f
    );
    
    return Center + RandomOffset;
}

FString UCrowdSimulationSubsystem::DetermineBiomeAtLocation(const FVector& Location) const
{
    // TODO: Implementar detecção real de bioma baseada no sistema de mundo
    // Por agora, retornar bioma padrão
    return TEXT("Forest");
}

void UCrowdSimulationSubsystem::SetupDefaultBiomeConfigurations()
{
    // Configuração para Floresta
    FBiomePopulationConfig ForestConfig;
    ForestConfig.BiomeName = TEXT("Forest");
    ForestConfig.MaxEntitiesPerSquareKm = 150;
    ForestConfig.MinSpawnDistanceFromPlayer = 3000.0f;
    ForestConfig.MaxSpawnDistanceFromPlayer = 15000.0f;
    
    // Herbívoros da floresta
    FDinosaurHerdData Triceratops;
    Triceratops.SpeciesName = TEXT("Triceratops");
    Triceratops.MinHerdSize = 8;
    Triceratops.MaxHerdSize = 25;
    Triceratops.PopulationDensityPerKm2 = 15.0f;
    Triceratops.MovementSpeed = 400.0f;
    Triceratops.FlockingRadius = 3000.0f;
    Triceratops.SeparationRadius = 800.0f;
    Triceratops.bMigratory = true;
    ForestConfig.HerbivoreHerds.Add(Triceratops);
    
    FDinosaurHerdData Parasaurolophus;
    Parasaurolophus.SpeciesName = TEXT("Parasaurolophus");
    Parasaurolophus.MinHerdSize = 12;
    Parasaurolophus.MaxHerdSize = 40;
    Parasaurolophus.PopulationDensityPerKm2 = 25.0f;
    Parasaurolophus.MovementSpeed = 600.0f;
    Parasaurolophus.FlockingRadius = 2500.0f;
    Parasaurolophus.SeparationRadius = 600.0f;
    Parasaurolophus.bMigratory = true;
    ForestConfig.HerbivoreHerds.Add(Parasaurolophus);
    
    // Predadores da floresta
    FPredatorPackData Allosaurus;
    Allosaurus.SpeciesName = TEXT("Allosaurus");
    Allosaurus.MinPackSize = 2;
    Allosaurus.MaxPackSize = 5;
    Allosaurus.TerritoryRadius = 8000.0f;
    Allosaurus.HuntingSpeed = 1200.0f;
    Allosaurus.DetectionRange = 4000.0f;
    Allosaurus.CoordinationRadius = 2000.0f;
    ForestConfig.PredatorPacks.Add(Allosaurus);
    
    FPredatorPackData Compsognathus;
    Compsognathus.SpeciesName = TEXT("Compsognathus");
    Compsognathus.MinPackSize = 8;
    Compsognathus.MaxPackSize = 20;
    Compsognathus.TerritoryRadius = 3000.0f;
    Compsognathus.HuntingSpeed = 800.0f;
    Compsognathus.DetectionRange = 2000.0f;
    Compsognathus.CoordinationRadius = 1000.0f;
    ForestConfig.PredatorPacks.Add(Compsognathus);
    
    BiomeConfigurations.Add(TEXT("Forest"), ForestConfig);
    
    // Configuração para Planícies
    FBiomePopulationConfig PlainsConfig;
    PlainsConfig.BiomeName = TEXT("Plains");
    PlainsConfig.MaxEntitiesPerSquareKm = 200;
    PlainsConfig.MinSpawnDistanceFromPlayer = 5000.0f;
    PlainsConfig.MaxSpawnDistanceFromPlayer = 25000.0f;
    
    // Herbívoros das planícies (manadas maiores)
    FDinosaurHerdData Brachiosaurus;
    Brachiosaurus.SpeciesName = TEXT("Brachiosaurus");
    Brachiosaurus.MinHerdSize = 5;
    Brachiosaurus.MaxHerdSize = 15;
    Brachiosaurus.PopulationDensityPerKm2 = 8.0f;
    Brachiosaurus.MovementSpeed = 300.0f;
    Brachiosaurus.FlockingRadius = 5000.0f;
    Brachiosaurus.SeparationRadius = 1500.0f;
    Brachiosaurus.bMigratory = true;
    PlainsConfig.HerbivoreHerds.Add(Brachiosaurus);
    
    FDinosaurHerdData Edmontosaurus;
    Edmontosaurus.SpeciesName = TEXT("Edmontosaurus");
    Edmontosaurus.MinHerdSize = 20;
    Edmontosaurus.MaxHerdSize = 80;
    Edmontosaurus.PopulationDensityPerKm2 = 35.0f;
    Edmontosaurus.MovementSpeed = 700.0f;
    Edmontosaurus.FlockingRadius = 4000.0f;
    Edmontosaurus.SeparationRadius = 700.0f;
    Edmontosaurus.bMigratory = true;
    PlainsConfig.HerbivoreHerds.Add(Edmontosaurus);
    
    // Predadores das planícies
    FPredatorPackData TyrannosaurusRex;
    TyrannosaurusRex.SpeciesName = TEXT("TyrannosaurusRex");
    TyrannosaurusRex.MinPackSize = 1;
    TyrannosaurusRex.MaxPackSize = 3;
    TyrannosaurusRex.TerritoryRadius = 15000.0f;
    TyrannosaurusRex.HuntingSpeed = 1000.0f;
    TyrannosaurusRex.DetectionRange = 6000.0f;
    TyrannosaurusRex.CoordinationRadius = 3000.0f;
    PlainsConfig.PredatorPacks.Add(TyrannosaurusRex);
    
    BiomeConfigurations.Add(TEXT("Plains"), PlainsConfig);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Default biome configurations set up"));
}