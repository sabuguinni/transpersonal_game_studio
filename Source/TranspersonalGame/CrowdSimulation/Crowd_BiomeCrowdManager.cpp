#include "Crowd_BiomeCrowdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

ACrowd_BiomeCrowdManager::ACrowd_BiomeCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente root
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Configurações padrão
    BiomeType = ECrowd_BiomeType::Savana;
    CurrentEntityCount = 0;
    LastSpawnTime = 0.0f;
    bIsInitialized = false;

    // Configurar densidade padrão baseada no bioma
    DensityConfig.MaxEntities = 1000;
    DensityConfig.SpawnRadius = 5000.0f;
    DensityConfig.SpeciesVariety = 3;
    DensityConfig.DensityMultiplier = 1.0f;
    DensityConfig.bEnableDynamicSpawning = true;

    // Configurar espécies padrão
    SpeciesConfig.HerbivoreSpecies.Add(TEXT("Triceratops"));
    SpeciesConfig.HerbivoreSpecies.Add(TEXT("Brachiosaurus"));
    SpeciesConfig.HerbivoreSpecies.Add(TEXT("Stegosaurus"));
    
    SpeciesConfig.CarnivoreSpecies.Add(TEXT("TRex"));
    SpeciesConfig.CarnivoreSpecies.Add(TEXT("Velociraptor"));
    
    SpeciesConfig.OmnivoreSpecies.Add(TEXT("Compsognathus"));

    // Configurações de comportamento
    bEnableFlocking = true;
    bEnablePredatorAvoidance = true;
    bEnableHerdBehavior = true;

    // Configurações de performance
    LODDistance1 = 2000.0f;
    LODDistance2 = 5000.0f;
    CullingDistance = 10000.0f;
}

void ACrowd_BiomeCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeCrowdManager: BeginPlay iniciado para bioma %d"), static_cast<int32>(BiomeType));
    
    // Inicializar sistema de crowd após um pequeno delay
    FTimerHandle InitTimer;
    GetWorld()->GetTimerManager().SetTimer(InitTimer, this, &ACrowd_BiomeCrowdManager::InitializeBiomeCrowd, 1.0f, false);
}

void ACrowd_BiomeCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsInitialized)
        return;

    // Actualizar comportamento da multidão
    UpdateCrowdBehavior(DeltaTime);
    
    // Gerir sistema LOD
    ManageLODSystem();
    
    // Spawning dinâmico se activado
    if (DensityConfig.bEnableDynamicSpawning)
    {
        LastSpawnTime += DeltaTime;
        if (LastSpawnTime >= 5.0f && CurrentEntityCount < DensityConfig.MaxEntities)
        {
            int32 EntitiesToSpawn = FMath::Min(50, DensityConfig.MaxEntities - CurrentEntityCount);
            SpawnCrowdEntities(EntitiesToSpawn);
            LastSpawnTime = 0.0f;
        }
    }
}

void ACrowd_BiomeCrowdManager::InitializeBiomeCrowd()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeCrowdManager: Inicializando crowd para bioma %d"), static_cast<int32>(BiomeType));
    
    // Configurar densidade baseada no tipo de bioma
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Pantano:
            DensityConfig.MaxEntities = 500;
            DensityConfig.SpawnRadius = 5000.0f;
            DensityConfig.SpeciesVariety = 3;
            break;
            
        case ECrowd_BiomeType::Floresta:
            DensityConfig.MaxEntities = 800;
            DensityConfig.SpawnRadius = 7000.0f;
            DensityConfig.SpeciesVariety = 5;
            break;
            
        case ECrowd_BiomeType::Savana:
            DensityConfig.MaxEntities = 2000;
            DensityConfig.SpawnRadius = 10000.0f;
            DensityConfig.SpeciesVariety = 4;
            break;
            
        case ECrowd_BiomeType::Deserto:
            DensityConfig.MaxEntities = 300;
            DensityConfig.SpawnRadius = 8000.0f;
            DensityConfig.SpeciesVariety = 2;
            break;
            
        case ECrowd_BiomeType::Montanha:
            DensityConfig.MaxEntities = 200;
            DensityConfig.SpawnRadius = 4000.0f;
            DensityConfig.SpeciesVariety = 2;
            break;
    }

    // Gerar pontos de spawn
    GenerateSpawnPoints();
    
    // Spawnar entidades iniciais (25% da capacidade máxima)
    int32 InitialSpawnCount = FMath::FloorToInt(DensityConfig.MaxEntities * 0.25f);
    SpawnCrowdEntities(InitialSpawnCount);
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("BiomeCrowdManager: Inicialização completa. Spawned %d entidades"), InitialSpawnCount);
}

void ACrowd_BiomeCrowdManager::SpawnCrowdEntities(int32 EntityCount)
{
    if (EntityCount <= 0 || SpawnPoints.Num() == 0)
        return;

    UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: Spawning %d entidades"), EntityCount);

    for (int32 i = 0; i < EntityCount; i++)
    {
        FVector SpawnLocation = GetRandomSpawnPoint();
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Em implementação real, aqui seria criada uma Mass Entity
            // Por agora, simular o spawn
            CurrentEntityCount++;
            
            // Debug visual (opcional)
            if (GEngine && GEngine->GetWorld())
            {
                DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 8, FColor::Green, false, 2.0f);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: Total entidades activas: %d"), CurrentEntityCount);
}

void ACrowd_BiomeCrowdManager::UpdateCrowdDensity(float NewDensityMultiplier)
{
    DensityConfig.DensityMultiplier = FMath::Clamp(NewDensityMultiplier, 0.1f, 3.0f);
    
    int32 NewMaxEntities = FMath::FloorToInt(DensityConfig.MaxEntities * DensityConfig.DensityMultiplier);
    
    if (NewMaxEntities < CurrentEntityCount)
    {
        // Remover entidades em excesso
        int32 EntitiesToRemove = CurrentEntityCount - NewMaxEntities;
        CurrentEntityCount -= EntitiesToRemove;
        UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: Removidas %d entidades. Total: %d"), EntitiesToRemove, CurrentEntityCount);
    }
}

void ACrowd_BiomeCrowdManager::SetBiomeType(ECrowd_BiomeType NewBiomeType)
{
    if (BiomeType != NewBiomeType)
    {
        BiomeType = NewBiomeType;
        
        // Reinicializar com nova configuração
        if (bIsInitialized)
        {
            bIsInitialized = false;
            CurrentEntityCount = 0;
            SpawnPoints.Empty();
            InitializeBiomeCrowd();
        }
    }
}

void ACrowd_BiomeCrowdManager::EnableDynamicSpawning(bool bEnable)
{
    DensityConfig.bEnableDynamicSpawning = bEnable;
    UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: Dynamic spawning %s"), bEnable ? TEXT("activado") : TEXT("desactivado"));
}

void ACrowd_BiomeCrowdManager::SetLODDistances(float LOD1, float LOD2, float Culling)
{
    LODDistance1 = FMath::Max(LOD1, 100.0f);
    LODDistance2 = FMath::Max(LOD2, LODDistance1 + 100.0f);
    CullingDistance = FMath::Max(Culling, LODDistance2 + 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: LOD distances actualizadas - LOD1: %.1f, LOD2: %.1f, Culling: %.1f"), 
           LODDistance1, LODDistance2, CullingDistance);
}

void ACrowd_BiomeCrowdManager::GenerateSpawnPoints()
{
    SpawnPoints.Empty();
    
    FVector ActorLocation = GetActorLocation();
    int32 NumPoints = FMath::FloorToInt(DensityConfig.SpawnRadius / 500.0f) * 10; // 10 pontos por 500 unidades
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        float Distance = FMath::RandRange(500.0f, DensityConfig.SpawnRadius);
        
        FVector SpawnPoint = ActorLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        SpawnPoints.Add(SpawnPoint);
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeCrowdManager: Gerados %d pontos de spawn"), SpawnPoints.Num());
}

void ACrowd_BiomeCrowdManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (bEnableFlocking)
    {
        // Actualizar comportamento de flocking
        // Implementação simplificada
    }
    
    if (bEnablePredatorAvoidance)
    {
        HandlePredatorInteractions();
    }
    
    if (bEnableHerdBehavior)
    {
        UpdateHerdFormations();
    }
}

void ACrowd_BiomeCrowdManager::ManageLODSystem()
{
    // Implementação simplificada do sistema LOD
    // Em implementação real, ajustaria o nível de detalhe baseado na distância ao jogador
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        
        // Ajustar LOD baseado na distância
        if (DistanceToPlayer > CullingDistance)
        {
            // Culling - desactivar entidades muito distantes
        }
        else if (DistanceToPlayer > LODDistance2)
        {
            // LOD 2 - baixo detalhe
        }
        else if (DistanceToPlayer > LODDistance1)
        {
            // LOD 1 - médio detalhe
        }
        else
        {
            // LOD 0 - alto detalhe
        }
    }
}

void ACrowd_BiomeCrowdManager::HandlePredatorInteractions()
{
    // Implementação simplificada de interacções predador-presa
    // Em implementação real, detectaria predadores próximos e activaria comportamento de fuga
}

void ACrowd_BiomeCrowdManager::UpdateHerdFormations()
{
    // Implementação simplificada de formações de rebanho
    // Em implementação real, manteria entidades próximas em grupos coesos
}

FVector ACrowd_BiomeCrowdManager::GetRandomSpawnPoint() const
{
    if (SpawnPoints.Num() == 0)
        return GetActorLocation();
        
    int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
    FVector BasePoint = SpawnPoints[RandomIndex];
    
    // Adicionar variação aleatória
    FVector RandomOffset = FVector(
        FMath::RandRange(-200.0f, 200.0f),
        FMath::RandRange(-200.0f, 200.0f),
        0.0f
    );
    
    return BasePoint + RandomOffset;
}

bool ACrowd_BiomeCrowdManager::IsValidSpawnLocation(const FVector& Location) const
{
    // Verificação básica de validade
    // Em implementação real, verificaria colisões, navegação, etc.
    
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Verificar se não está muito próximo de outros spawns recentes
    // Implementação simplificada
    return true;
}

void ACrowd_BiomeCrowdManager::CleanupInvalidEntities()
{
    // Implementação para limpar entidades inválidas ou muito distantes
    // Em implementação real, removeria entidades que saíram dos limites ou ficaram presas
}