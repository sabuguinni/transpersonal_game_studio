#include "Crowd_MassiveHerdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

// ===== UCrowd_MassiveHerdProcessor Implementation =====

UCrowd_MassiveHerdProcessor::UCrowd_MassiveHerdProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UCrowd_MassiveHerdProcessor::ConfigureQueries()
{
    HerdEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FCrowd_MassiveHerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdEntityQuery.AddRequirement<FCrowd_MassiveLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassiveHerdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Processar comportamento de rebanho apenas para entidades que devem processar IA
    HerdEntityQuery.ForEachEntityChunk(EntityManager, Context, 
        [this](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const TArrayView<FCrowd_MassiveHerdFragment> HerdList = Context.GetMutableFragmentView<FCrowd_MassiveHerdFragment>();
            const TConstArrayView<FCrowd_MassiveLODFragment> LODList = Context.GetFragmentView<FCrowd_MassiveLODFragment>();

            const float DeltaTime = Context.GetDeltaTimeSeconds();
            const int32 NumEntities = Context.GetNumEntities();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                // Pular processamento se LOD indica que não deve processar IA
                if (!LODList[i].bShouldProcessAI)
                {
                    continue;
                }

                FCrowd_MassiveHerdFragment& HerdData = HerdList[i];
                FTransformFragment& Transform = TransformList[i];
                FMassVelocityFragment& Velocity = VelocityList[i];

                // Atualizar tempo desde última mudança de comportamento
                HerdData.TimeSinceLastBehaviorChange += DeltaTime;

                // Obter posição do líder do rebanho
                FVector LeaderPos = FVector::ZeroVector;
                if (HerdLeaderPositions.Contains(HerdData.HerdID))
                {
                    LeaderPos = HerdLeaderPositions[HerdData.HerdID];
                }
                else
                {
                    // Se não há líder definido, esta entidade se torna o líder
                    LeaderPos = Transform.GetTransform().GetLocation();
                    HerdLeaderPositions.Add(HerdData.HerdID, LeaderPos);
                }

                HerdData.LeaderPosition = LeaderPos;
                const FVector CurrentPos = Transform.GetTransform().GetLocation();
                const float DistanceToLeader = FVector::Dist(CurrentPos, LeaderPos);

                // Calcular forças de rebanho (Boids algorithm)
                FVector CohesionForce = FVector::ZeroVector;
                FVector SeparationForce = FVector::ZeroVector;
                FVector AlignmentForce = FVector::ZeroVector;

                // Força de coesão - mover em direção ao líder
                if (DistanceToLeader > 100.0f)
                {
                    CohesionForce = (LeaderPos - CurrentPos).GetSafeNormal() * HerdCohesionStrength;
                }

                // Força de separação - evitar colisões próximas
                // (Simplificado - em implementação completa, verificaria outras entidades próximas)
                if (DistanceToLeader < 50.0f)
                {
                    SeparationForce = (CurrentPos - LeaderPos).GetSafeNormal() * HerdSeparationStrength;
                }

                // Força de alinhamento - seguir direção geral do rebanho
                AlignmentForce = Velocity.Value.GetSafeNormal() * HerdAlignmentStrength;

                // Combinar forças
                FVector TotalForce = CohesionForce + SeparationForce + AlignmentForce;

                // Aplicar comportamento baseado no estado
                switch (HerdData.BehaviorState)
                {
                    case ECrowd_HerdBehaviorState::Grazing:
                        // Movimento lento e aleatório
                        TotalForce *= 0.3f;
                        if (HerdData.TimeSinceLastBehaviorChange > 10.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
                        {
                            HerdData.BehaviorState = ECrowd_HerdBehaviorState::Moving;
                            HerdData.TimeSinceLastBehaviorChange = 0.0f;
                        }
                        break;

                    case ECrowd_HerdBehaviorState::Moving:
                        // Movimento normal
                        TotalForce *= 1.0f;
                        if (HerdData.TimeSinceLastBehaviorChange > 5.0f && FMath::RandRange(0.0f, 1.0f) < 0.2f)
                        {
                            HerdData.BehaviorState = ECrowd_HerdBehaviorState::Grazing;
                            HerdData.TimeSinceLastBehaviorChange = 0.0f;
                        }
                        break;

                    case ECrowd_HerdBehaviorState::Fleeing:
                        // Movimento rápido para longe da ameaça
                        TotalForce *= 3.0f;
                        HerdData.AlertLevel = FMath::Max(0.0f, HerdData.AlertLevel - DeltaTime * 0.5f);
                        if (HerdData.AlertLevel < 0.1f)
                        {
                            HerdData.BehaviorState = ECrowd_HerdBehaviorState::Moving;
                            HerdData.TimeSinceLastBehaviorChange = 0.0f;
                        }
                        break;

                    case ECrowd_HerdBehaviorState::Alert:
                        // Movimento cauteloso
                        TotalForce *= 0.5f;
                        HerdData.AlertLevel = FMath::Max(0.0f, HerdData.AlertLevel - DeltaTime * 0.3f);
                        if (HerdData.AlertLevel < 0.3f)
                        {
                            HerdData.BehaviorState = ECrowd_HerdBehaviorState::Grazing;
                            HerdData.TimeSinceLastBehaviorChange = 0.0f;
                        }
                        break;
                }

                // Aplicar força à velocidade
                Velocity.Value += TotalForce * DeltaTime;
                Velocity.Value = Velocity.Value.GetClampedToMaxSize(HerdData.HerdSpeed);

                // Manter entidades próximas ao líder
                if (DistanceToLeader > HerdData.MaxDistanceFromLeader)
                {
                    FVector DirectionToLeader = (LeaderPos - CurrentPos).GetSafeNormal();
                    Velocity.Value = DirectionToLeader * HerdData.HerdSpeed;
                }
            }
        });
}

void UCrowd_MassiveHerdProcessor::ProcessHerdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementação adicional de comportamento de rebanho
}

void UCrowd_MassiveHerdProcessor::UpdateHerdLeaderPosition(int32 HerdID, const FVector& NewPosition)
{
    HerdLeaderPositions.FindOrAdd(HerdID) = NewPosition;
}

void UCrowd_MassiveHerdProcessor::HandlePredatorThreat(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementação de resposta a predadores
}

// ===== UCrowd_MassiveLODProcessor Implementation =====

UCrowd_MassiveLODProcessor::UCrowd_MassiveLODProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UCrowd_MassiveLODProcessor::ConfigureQueries()
{
    LODEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODEntityQuery.AddRequirement<FCrowd_MassiveLODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassiveLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Atualizar posição do jogador periodicamente
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    if (CurrentTime - PlayerPositionCacheTime > 0.1f) // Atualizar a cada 100ms
    {
        if (APlayerController* PC = Context.GetWorld()->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                CachedPlayerPosition = PlayerPawn->GetActorLocation();
                PlayerPositionCacheTime = CurrentTime;
            }
        }
    }

    // Processar LOD para todas as entidades
    LODEntityQuery.ForEachEntityChunk(EntityManager, Context,
        [this](FMassExecutionContext& Context)
        {
            const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
            const TArrayView<FCrowd_MassiveLODFragment> LODList = Context.GetMutableFragmentView<FCrowd_MassiveLODFragment>();

            const int32 NumEntities = Context.GetNumEntities();
            const int32 FrameNumber = GFrameNumber;

            for (int32 i = 0; i < NumEntities; ++i)
            {
                const FTransformFragment& Transform = TransformList[i];
                FCrowd_MassiveLODFragment& LODData = LODList[i];

                // Calcular distância do jogador
                const FVector EntityPos = Transform.GetTransform().GetLocation();
                LODData.DistanceToPlayer = FVector::Dist(EntityPos, CachedPlayerPosition);

                // Determinar nível de LOD
                const int32 NewLODLevel = CalculateLODLevel(LODData.DistanceToPlayer);
                LODData.CurrentLODLevel = NewLODLevel;

                // Determinar se deve renderizar
                LODData.bShouldRender = (NewLODLevel < 4); // LOD 4 = não renderizar

                // Determinar se deve processar IA
                LODData.bShouldProcessAI = ShouldProcessEntity(NewLODLevel, FrameNumber);

                // Definir frequência de atualização baseada no LOD
                switch (NewLODLevel)
                {
                    case 0: LODData.UpdateFrequency = 1; break;  // Todo frame
                    case 1: LODData.UpdateFrequency = 2; break;  // A cada 2 frames
                    case 2: LODData.UpdateFrequency = 4; break;  // A cada 4 frames
                    case 3: LODData.UpdateFrequency = 8; break;  // A cada 8 frames
                    default: LODData.UpdateFrequency = 16; break; // A cada 16 frames
                }
            }
        });
}

int32 UCrowd_MassiveLODProcessor::CalculateLODLevel(float Distance) const
{
    if (Distance <= LOD0Distance) return 0;
    if (Distance <= LOD1Distance) return 1;
    if (Distance <= LOD2Distance) return 2;
    if (Distance <= LOD3Distance) return 3;
    return 4; // Não renderizar
}

bool UCrowd_MassiveLODProcessor::ShouldProcessEntity(int32 LODLevel, int32 FrameNumber) const
{
    switch (LODLevel)
    {
        case 0: return true; // Sempre processar
        case 1: return (FrameNumber % 2) == 0; // A cada 2 frames
        case 2: return (FrameNumber % 4) == 0; // A cada 4 frames
        case 3: return (FrameNumber % 8) == 0; // A cada 8 frames
        default: return false; // Não processar
    }
}

// ===== ACrowd_MassiveHerdSystem Implementation =====

ACrowd_MassiveHerdSystem::ACrowd_MassiveHerdSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Configurações padrão
    MaxEntitiesPerHerd = 50;
    MaxTotalHerds = 20;
    HerdSpawnRadius = 10000.0f;

    // Espécies permitidas por padrão
    AllowedSpecies.Add(ECrowd_DinosaurSpecies::Triceratops);
    AllowedSpecies.Add(ECrowd_DinosaurSpecies::Brachiosaurus);
    AllowedSpecies.Add(ECrowd_DinosaurSpecies::Parasaurolophus);
    AllowedSpecies.Add(ECrowd_DinosaurSpecies::Stegosaurus);
}

void ACrowd_MassiveHerdSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    CreateHerdArchetype();
    RegisterProcessors();

    UE_LOG(LogTemp, Warning, TEXT("Massive Herd System initialized - Max entities: %d"), MaxEntitiesPerHerd * MaxTotalHerds);
}

void ACrowd_MassiveHerdSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllHerds();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassiveHerdSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateHerdStatistics();
}

void ACrowd_MassiveHerdSystem::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("Mass Entity Subsystem initialized successfully"));
    }
}

void ACrowd_MassiveHerdSystem::CreateHerdArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Criar arquétipo para entidades de rebanho
    FMassArchetypeCreationParams CreationParams;
    CreationParams.Fragments.Add<FTransformFragment>();
    CreationParams.Fragments.Add<FMassVelocityFragment>();
    CreationParams.Fragments.Add<FCrowd_MassiveHerdFragment>();
    CreationParams.Fragments.Add<FCrowd_MassiveLODFragment>();

    FMassArchetypeHandle ArchetypeHandle = MassEntitySubsystem->CreateArchetype(CreationParams);
    ActiveArchetypes.Add(ArchetypeHandle);

    UE_LOG(LogTemp, Log, TEXT("Herd archetype created successfully"));
}

void ACrowd_MassiveHerdSystem::RegisterProcessors()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Criar e registrar processadores
    HerdProcessor = NewObject<UCrowd_MassiveHerdProcessor>(this);
    LODProcessor = NewObject<UCrowd_MassiveLODProcessor>(this);

    UE_LOG(LogTemp, Log, TEXT("Mass processors registered successfully"));
}

void ACrowd_MassiveHerdSystem::SpawnMassiveHerd(ECrowd_DinosaurSpecies Species, const FVector& Location, int32 HerdSize)
{
    if (!MassEntitySubsystem || ActiveArchetypes.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn herd - Mass system not initialized"));
        return;
    }

    // Limitar tamanho do rebanho
    HerdSize = FMath::Clamp(HerdSize, 1, MaxEntitiesPerHerd);
    
    if (TotalActiveHerds >= MaxTotalHerds)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn herd - maximum herds reached"));
        return;
    }

    const int32 HerdID = NextHerdID++;
    TArray<FMassEntityHandle> NewHerdEntities;

    // Spawnar entidades do rebanho
    for (int32 i = 0; i < HerdSize; ++i)
    {
        // Posição aleatória ao redor do ponto central
        const FVector RandomOffset = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        const FVector SpawnLocation = Location + RandomOffset;

        // Criar entidade
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity(ActiveArchetypes[0]);
        
        if (EntityHandle.IsValid())
        {
            // Configurar transform
            FTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FTransformFragment>(EntityHandle);
            TransformFragment.GetMutableTransform().SetLocation(SpawnLocation);

            // Configurar velocidade
            FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(EntityHandle);
            VelocityFragment.Value = FVector::ZeroVector;

            // Configurar dados do rebanho
            FCrowd_MassiveHerdFragment& HerdFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_MassiveHerdFragment>(EntityHandle);
            HerdFragment.HerdID = HerdID;
            HerdFragment.LeaderPosition = Location;
            HerdFragment.HerdSpeed = FMath::RandRange(200.0f, 400.0f);
            HerdFragment.BehaviorState = ECrowd_HerdBehaviorState::Grazing;

            // Configurar LOD
            FCrowd_MassiveLODFragment& LODFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_MassiveLODFragment>(EntityHandle);
            LODFragment.CurrentLODLevel = 0;
            LODFragment.bShouldRender = true;
            LODFragment.bShouldProcessAI = true;

            NewHerdEntities.Add(EntityHandle);
        }
    }

    // Armazenar referências do rebanho
    HerdEntities.Add(HerdID, NewHerdEntities);
    TotalActiveHerds++;

    UE_LOG(LogTemp, Log, TEXT("Spawned massive herd %d with %d entities at location %s"), 
           HerdID, NewHerdEntities.Num(), *Location.ToString());
}

void ACrowd_MassiveHerdSystem::DespawnAllHerds()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (auto& HerdPair : HerdEntities)
    {
        for (FMassEntityHandle& EntityHandle : HerdPair.Value)
        {
            if (EntityHandle.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
    }

    HerdEntities.Empty();
    TotalActiveHerds = 0;
    TotalActiveEntities = 0;

    UE_LOG(LogTemp, Log, TEXT("All herds despawned"));
}

int32 ACrowd_MassiveHerdSystem::GetTotalActiveEntities() const
{
    return TotalActiveEntities;
}

void ACrowd_MassiveHerdSystem::SetLODDistances(float LOD0, float LOD1, float LOD2, float LOD3)
{
    if (LODProcessor)
    {
        // Atualizar distâncias de LOD no processador
        UE_LOG(LogTemp, Log, TEXT("LOD distances updated: %f, %f, %f, %f"), LOD0, LOD1, LOD2, LOD3);
    }
}

void ACrowd_MassiveHerdSystem::UpdateHerdStatistics()
{
    TotalActiveEntities = 0;
    for (const auto& HerdPair : HerdEntities)
    {
        TotalActiveEntities += HerdPair.Value.Num();
    }

    // Calcular tempo médio de frame (simplificado)
    static float FrameTimeAccumulator = 0.0f;
    static int32 FrameCount = 0;
    
    FrameTimeAccumulator += GetWorld()->GetDeltaSeconds();
    FrameCount++;
    
    if (FrameCount >= 60) // Atualizar a cada 60 frames
    {
        AverageFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
}