#include "Crowd_MassEntitySpawner.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntitySpawner::UCrowd_MassEntitySpawner()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick a cada segundo para performance
    
    // Configuração padrão de dinossauros
    DinosaurCounts.Add(ECrowd_DinosaurType::Herbivore_Small, 20000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Herbivore_Medium, 15000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Herbivore_Large, 8000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Carnivore_Small, 3000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Carnivore_Medium, 2000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Carnivore_Large, 1000);
    DinosaurCounts.Add(ECrowd_DinosaurType::Flying, 1000);
    
    // Probabilidades de spawn
    SpawnProbabilities.Add(ECrowd_DinosaurType::Herbivore_Small, 0.4f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Herbivore_Medium, 0.3f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Herbivore_Large, 0.16f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Carnivore_Small, 0.06f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Carnivore_Medium, 0.04f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Carnivore_Large, 0.02f);
    SpawnProbabilities.Add(ECrowd_DinosaurType::Flying, 0.02f);
}

void UCrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSubsystems();
    
    if (bAutoSpawnOnStart)
    {
        // Delay inicial para garantir que tudo está carregado
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UCrowd_MassEntitySpawner::SpawnMassEntities, 2.0f, false);
    }
}

void UCrowd_MassEntitySpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentEntityCount > 0)
    {
        UpdateEntityLOD();
        CleanupDistantEntities();
    }
}

void UCrowd_MassEntitySpawner::InitializeMassSubsystems()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySpawner: MassEntitySubsystem não encontrado"));
        }
        
        if (!MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySpawner: MassSpawnerSubsystem não encontrado"));
        }
    }
}

void UCrowd_MassEntitySpawner::SpawnMassEntities()
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntitySpawner: Subsistemas Mass não inicializados"));
        return;
    }
    
    if (bIsSpawning)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntitySpawner: Spawn já em progresso"));
        return;
    }
    
    bIsSpawning = true;
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySpawner: Iniciando spawn de %d entidades"), MaxEntities);
    
    // Spawn por lotes para evitar lag
    const int32 BatchSize = 1000;
    SpawnEntitiesInBatch(BatchSize);
}

void UCrowd_MassEntitySpawner::SpawnEntitiesInBatch(int32 BatchSize)
{
    if (CurrentEntityCount >= MaxEntities)
    {
        bIsSpawning = false;
        OnMassSpawnCompleted.Broadcast(CurrentEntityCount);
        UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySpawner: Spawn completo - %d entidades"), CurrentEntityCount);
        return;
    }
    
    int32 EntitiesToSpawn = FMath::Min(BatchSize, MaxEntities - CurrentEntityCount);
    FVector BiomeCenter = GetBiomeSpawnLocation(TargetBiome);
    
    for (int32 i = 0; i < EntitiesToSpawn; i++)
    {
        // Seleccionar tipo de dinossauro baseado em probabilidades
        ECrowd_DinosaurType SelectedType = ECrowd_DinosaurType::Herbivore_Small;
        float RandomValue = FMath::RandRange(0.0f, 1.0f);
        float CumulativeProbability = 0.0f;
        
        for (const auto& Pair : SpawnProbabilities)
        {
            CumulativeProbability += Pair.Value;
            if (RandomValue <= CumulativeProbability)
            {
                SelectedType = Pair.Key;
                break;
            }
        }
        
        // Gerar posição aleatória dentro do raio
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        
        // Ajustar altura ao terreno (simplificado)
        SpawnLocation.Z = 100.0f; // Altura base
        
        // Spawn da entidade Mass (simplificado - requer configuração adicional)
        // Aqui seria feita a criação real da entidade Mass
        CurrentEntityCount++;
    }
    
    OnEntityCountChanged.Broadcast(CurrentEntityCount);
    
    // Continuar com próximo lote
    if (CurrentEntityCount < MaxEntities)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, BatchSize]()
        {
            SpawnEntitiesInBatch(BatchSize);
        }, 0.1f, false);
    }
    else
    {
        bIsSpawning = false;
        OnMassSpawnCompleted.Broadcast(CurrentEntityCount);
    }
}

FVector UCrowd_MassEntitySpawner::GetBiomeSpawnLocation(ECrowd_BiomeType Biome) const
{
    switch (Biome)
    {
        case ECrowd_BiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 0.0f);
        case ECrowd_BiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case ECrowd_BiomeType::Savana:
            return FVector(0.0f, 0.0f, 0.0f);
        case ECrowd_BiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 0.0f);
        case ECrowd_BiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 500.0f);
        default:
            return FVector::ZeroVector;
    }
}

void UCrowd_MassEntitySpawner::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Aqui seria feita a remoção de todas as entidades Mass
    CurrentEntityCount = 0;
    OnEntityCountChanged.Broadcast(CurrentEntityCount);
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySpawner: Todas as entidades removidas"));
}

void UCrowd_MassEntitySpawner::SpawnDinosaurType(ECrowd_DinosaurType DinosaurType, int32 Count, FVector CenterLocation)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            0.0f
        );
        
        FVector SpawnLocation = CenterLocation + RandomOffset;
        
        // Spawn específico do tipo
        // Configuração específica seria feita aqui
        CurrentEntityCount++;
    }
    
    OnEntityCountChanged.Broadcast(CurrentEntityCount);
}

int32 UCrowd_MassEntitySpawner::GetCurrentEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassEntitySpawner::UpdateLODSettings(float NewLOD1, float NewLOD2, float NewCulling)
{
    LODDistance1 = NewLOD1;
    LODDistance2 = NewLOD2;
    CullingDistance = NewCulling;
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_MassEntitySpawner: LOD atualizado - LOD1: %.0f, LOD2: %.0f, Culling: %.0f"), 
           LODDistance1, LODDistance2, CullingDistance);
}

void UCrowd_MassEntitySpawner::UpdateEntityLOD()
{
    // Sistema de LOD baseado na distância ao jogador
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Aqui seria implementado o sistema de LOD para as entidades Mass
        // Baseado na distância ao jogador
    }
}

void UCrowd_MassEntitySpawner::CleanupDistantEntities()
{
    // Limpar entidades muito distantes para manter performance
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Aqui seria implementada a lógica de culling de entidades distantes
    }
}

void UCrowd_MassEntitySpawner::SetupMassEntityConfig()
{
    // Configuração dos templates de entidades Mass
    // Seria implementado quando o sistema Mass estiver totalmente configurado
}

void UCrowd_MassEntitySpawner::ConfigureDinosaurFragments(ECrowd_DinosaurType DinosaurType)
{
    // Configuração dos fragmentos específicos para cada tipo de dinossauro
    // Movimento, comportamento, rendering, etc.
}