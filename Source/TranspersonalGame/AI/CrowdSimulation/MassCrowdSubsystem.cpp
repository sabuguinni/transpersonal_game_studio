#include "MassCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UMassCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Initializing..."));
    
    InitializeMassFramework();
    SetupDefaultConfigurations();
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Initialization complete"));
}

void UMassCrowdSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Deinitializing..."));
    
    // Limpar todas as entidades ativas
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& Entity : ActiveEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(Entity))
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
        ActiveEntities.Empty();
    }
    
    Super::Deinitialize();
}

bool UMassCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Só criar em mundos de jogo, não no editor
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UMassCrowdSubsystem::InitializeMassFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: No valid world found"));
        return;
    }
    
    // Obter referências aos subsistemas Mass
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Failed to get MassEntitySubsystem"));
        return;
    }
    
    if (!MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Failed to get MassSimulationSubsystem"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Mass framework initialized successfully"));
}

void UMassCrowdSubsystem::SetupDefaultConfigurations()
{
    // Configuração padrão para manada de Triceratops
    FDinosaurHerdConfig TriceratopsHerd;
    TriceratopsHerd.SpeciesName = "Triceratops";
    TriceratopsHerd.HerdSize = 15;
    TriceratopsHerd.CohesionRadius = 1200.0f;
    TriceratopsHerd.MovementSpeed = 250.0f;
    TriceratopsHerd.GrazingRadius = 2500.0f;
    TriceratopsHerd.ActiveHourStart = 6;
    TriceratopsHerd.ActiveHourEnd = 18;
    RegisterHerdConfig(TriceratopsHerd);
    
    // Configuração padrão para manada de Parasaurolophus
    FDinosaurHerdConfig ParasaurolophusHerd;
    ParasaurolophusHerd.SpeciesName = "Parasaurolophus";
    ParasaurolophusHerd.HerdSize = 25;
    ParasaurolophusHerd.CohesionRadius = 1000.0f;
    ParasaurolophusHerd.MovementSpeed = 300.0f;
    ParasaurolophusHerd.GrazingRadius = 2000.0f;
    ParasaurolophusHerd.ActiveHourStart = 5;
    ParasaurolophusHerd.ActiveHourEnd = 19;
    RegisterHerdConfig(ParasaurolophusHerd);
    
    // Configuração padrão para território de T-Rex
    FPredatorTerritoryConfig TRexTerritory;
    TRexTerritory.SpeciesName = "TRex";
    TRexTerritory.TerritoryRadius = 8000.0f;
    TRexTerritory.PatrolSpeed = 400.0f;
    TRexTerritory.HuntingRange = 2500.0f;
    TRexTerritory.HuntingHourStart = 5;
    TRexTerritory.HuntingHourEnd = 10;
    RegisterPredatorTerritory(TRexTerritory);
    
    // Configuração padrão para território de Allosaurus
    FPredatorTerritoryConfig AllosaurusTerritory;
    AllosaurusTerritory.SpeciesName = "Allosaurus";
    AllosaurusTerritory.TerritoryRadius = 6000.0f;
    AllosaurusTerritory.PatrolSpeed = 450.0f;
    AllosaurusTerritory.HuntingRange = 2000.0f;
    AllosaurusTerritory.HuntingHourStart = 18;
    AllosaurusTerritory.HuntingHourEnd = 22;
    RegisterPredatorTerritory(AllosaurusTerritory);
    
    // Rota de migração sazonal
    FMigrationRouteConfig SeasonalMigration;
    SeasonalMigration.RouteName = "GreatMigration";
    SeasonalMigration.WayPoints = {
        FVector(0, 0, 0),
        FVector(10000, 5000, 0),
        FVector(15000, 15000, 0),
        FVector(5000, 20000, 0),
        FVector(-5000, 15000, 0),
        FVector(-10000, 5000, 0)
    };
    SeasonalMigration.MigratingSpecies = {"Triceratops", "Parasaurolophus", "Stegosaurus"};
    SeasonalMigration.MigrationDurationDays = 10;
    SeasonalMigration.MigrationStartDay = 90;  // Primavera
    SeasonalMigration.ReturnMigrationDay = 270; // Outono
    RegisterMigrationRoute(SeasonalMigration);
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Default configurations loaded"));
}

void UMassCrowdSubsystem::RegisterHerdConfig(const FDinosaurHerdConfig& HerdConfig)
{
    // Verificar se já existe uma configuração para esta espécie
    for (int32 i = 0; i < HerdConfigs.Num(); i++)
    {
        if (HerdConfigs[i].SpeciesName == HerdConfig.SpeciesName)
        {
            HerdConfigs[i] = HerdConfig;
            UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Updated herd config for %s"), *HerdConfig.SpeciesName.ToString());
            return;
        }
    }
    
    // Adicionar nova configuração
    HerdConfigs.Add(HerdConfig);
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Registered new herd config for %s"), *HerdConfig.SpeciesName.ToString());
}

void UMassCrowdSubsystem::RegisterPredatorTerritory(const FPredatorTerritoryConfig& TerritoryConfig)
{
    // Verificar se já existe uma configuração para esta espécie
    for (int32 i = 0; i < PredatorConfigs.Num(); i++)
    {
        if (PredatorConfigs[i].SpeciesName == TerritoryConfig.SpeciesName)
        {
            PredatorConfigs[i] = TerritoryConfig;
            UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Updated predator config for %s"), *TerritoryConfig.SpeciesName.ToString());
            return;
        }
    }
    
    // Adicionar nova configuração
    PredatorConfigs.Add(TerritoryConfig);
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Registered new predator config for %s"), *TerritoryConfig.SpeciesName.ToString());
}

void UMassCrowdSubsystem::RegisterMigrationRoute(const FMigrationRouteConfig& RouteConfig)
{
    // Verificar se já existe uma rota com este nome
    for (int32 i = 0; i < MigrationRoutes.Num(); i++)
    {
        if (MigrationRoutes[i].RouteName == RouteConfig.RouteName)
        {
            MigrationRoutes[i] = RouteConfig;
            UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Updated migration route %s"), *RouteConfig.RouteName);
            return;
        }
    }
    
    // Adicionar nova rota
    MigrationRoutes.Add(RouteConfig);
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Registered new migration route %s"), *RouteConfig.RouteName);
}

void UMassCrowdSubsystem::SpawnHerd(const FName& SpeciesName, const FVector& Location, int32 Count)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Cannot spawn herd - MassEntitySubsystem not available"));
        return;
    }
    
    // Encontrar configuração da espécie
    const FDinosaurHerdConfig* HerdConfig = nullptr;
    for (const FDinosaurHerdConfig& Config : HerdConfigs)
    {
        if (Config.SpeciesName == SpeciesName)
        {
            HerdConfig = &Config;
            break;
        }
    }
    
    if (!HerdConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: No herd config found for species %s"), *SpeciesName.ToString());
        return;
    }
    
    // Spawnar entidades da manada
    for (int32 i = 0; i < Count; i++)
    {
        // Calcular posição aleatória dentro do raio de coesão
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, HerdConfig->CohesionRadius);
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Criar entidade Mass
        FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity();
        if (MassEntitySubsystem->IsEntityValid(Entity))
        {
            ActiveEntities.Add(Entity);
            
            // Adicionar fragmentos básicos
            // Nota: Aqui adicionaríamos os fragmentos específicos do Mass AI
            // como FMassTransformFragment, FMassVelocityFragment, etc.
            
            UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawned %s entity at %s"), 
                *SpeciesName.ToString(), *SpawnLocation.ToString());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Spawned herd of %d %s at %s"), 
        Count, *SpeciesName.ToString(), *Location.ToString());
}

void UMassCrowdSubsystem::SpawnPredator(const FName& SpeciesName, const FVector& Location)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Cannot spawn predator - MassEntitySubsystem not available"));
        return;
    }
    
    // Encontrar configuração do predador
    const FPredatorTerritoryConfig* PredatorConfig = nullptr;
    for (const FPredatorTerritoryConfig& Config : PredatorConfigs)
    {
        if (Config.SpeciesName == SpeciesName)
        {
            PredatorConfig = &Config;
            break;
        }
    }
    
    if (!PredatorConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: No predator config found for species %s"), *SpeciesName.ToString());
        return;
    }
    
    // Criar entidade Mass para o predador
    FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity();
    if (MassEntitySubsystem->IsEntityValid(Entity))
    {
        ActiveEntities.Add(Entity);
        
        // Adicionar fragmentos específicos do predador
        // Nota: Aqui adicionaríamos fragmentos como território, estado de caça, etc.
        
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Spawned predator %s at %s"), 
            *SpeciesName.ToString(), *Location.ToString());
    }
}

void UMassCrowdSubsystem::StartMigration(const FString& RouteName)
{
    // Encontrar rota de migração
    const FMigrationRouteConfig* Route = nullptr;
    for (const FMigrationRouteConfig& Config : MigrationRoutes)
    {
        if (Config.RouteName == RouteName)
        {
            Route = &Config;
            break;
        }
    }
    
    if (!Route)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Migration route %s not found"), *RouteName);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Starting migration %s with %d waypoints"), 
        *RouteName, Route->WayPoints.Num());
    
    // Implementar lógica de migração
    // Nota: Aqui implementaríamos o sistema de migração usando Mass AI
}

void UMassCrowdSubsystem::PauseSimulation()
{
    bSimulationPaused = true;
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Simulation paused"));
}

void UMassCrowdSubsystem::ResumeSimulation()
{
    bSimulationPaused = false;
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Simulation resumed"));
}

int32 UMassCrowdSubsystem::GetActiveEntityCount() const
{
    if (!MassEntitySubsystem)
    {
        return 0;
    }
    
    int32 ValidCount = 0;
    for (const FMassEntityHandle& Entity : ActiveEntities)
    {
        if (MassEntitySubsystem->IsEntityValid(Entity))
        {
            ValidCount++;
        }
    }
    
    return ValidCount;
}

float UMassCrowdSubsystem::GetSimulationPerformance() const
{
    // Retornar métrica de performance simples
    // Em implementação real, calcularia FPS médio, tempo de processamento, etc.
    return bSimulationPaused ? 0.0f : 60.0f;
}