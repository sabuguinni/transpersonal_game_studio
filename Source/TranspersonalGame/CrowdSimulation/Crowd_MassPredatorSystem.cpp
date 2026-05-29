#include "Crowd_MassPredatorSystem.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "MassSignalSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UCrowd_MassPredatorSystem::UCrowd_MassPredatorSystem()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    
    // Configurar query para predadores
    PredatorQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddRequirement<FCrowd_PredatorFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
    PredatorQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Configurar query para presas
    PreyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FCrowd_HerbivoreFragment>(EMassFragmentAccess::ReadOnly);
    
    // Configurar parâmetros de caça
    HuntingRange = 5000.0f; // 50 metros
    AttackRange = 200.0f;   // 2 metros
    MaxHuntSpeed = 1500.0f; // 15 m/s
    PatrolSpeed = 500.0f;   // 5 m/s
    HuntCooldown = 10.0f;   // 10 segundos
}

void UCrowd_MassPredatorSystem::ConfigureQueries()
{
    Super::ConfigureQueries();
}

void UCrowd_MassPredatorSystem::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Processar comportamento de predadores
    PredatorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_PredatorFragment> PredatorList = Context.GetMutableFragmentView<FCrowd_PredatorFragment>();
        const TArrayView<FMassRepresentationLODFragment> LODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_PredatorFragment& Predator = PredatorList[EntityIndex];
            const FMassRepresentationLODFragment& LOD = LODList[EntityIndex];
            
            // Apenas processar se estiver em LOD visível
            if (LOD.LODLevel > EMassLOD::Medium)
            {
                continue;
            }
            
            // Actualizar cooldowns
            if (Predator.HuntCooldownTimer > 0.0f)
            {
                Predator.HuntCooldownTimer -= DeltaTime;
            }
            
            // Máquina de estados do predador
            switch (Predator.BehaviorState)
            {
                case ECrowd_PredatorBehavior::Patrolling:
                    ProcessPatrolling(Transform, Velocity, Predator, DeltaTime);
                    break;
                    
                case ECrowd_PredatorBehavior::Hunting:
                    ProcessHunting(Transform, Velocity, Predator, DeltaTime, EntityManager);
                    break;
                    
                case ECrowd_PredatorBehavior::Attacking:
                    ProcessAttacking(Transform, Velocity, Predator, DeltaTime);
                    break;
                    
                case ECrowd_PredatorBehavior::Feeding:
                    ProcessFeeding(Transform, Velocity, Predator, DeltaTime);
                    break;
                    
                case ECrowd_PredatorBehavior::Resting:
                    ProcessResting(Transform, Velocity, Predator, DeltaTime);
                    break;
            }
            
            // Detectar presas próximas
            if (Predator.BehaviorState == ECrowd_PredatorBehavior::Patrolling && 
                Predator.HuntCooldownTimer <= 0.0f)
            {
                DetectNearbyPrey(Transform, Predator, EntityManager);
            }
        }
    });
}

void UCrowd_MassPredatorSystem::ProcessPatrolling(FTransformFragment& Transform, FMassVelocityFragment& Velocity, 
                                                 FCrowd_PredatorFragment& Predator, float DeltaTime)
{
    // Movimento de patrulha aleatório
    if (Predator.StateTimer <= 0.0f)
    {
        // Escolher nova direcção aleatória
        const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        Predator.TargetDirection = FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
        Predator.StateTimer = FMath::RandRange(3.0f, 8.0f); // 3-8 segundos numa direcção
    }
    
    // Aplicar movimento
    Velocity.Value = Predator.TargetDirection * PatrolSpeed;
    Predator.StateTimer -= DeltaTime;
}

void UCrowd_MassPredatorSystem::ProcessHunting(FTransformFragment& Transform, FMassVelocityFragment& Velocity, 
                                              FCrowd_PredatorFragment& Predator, float DeltaTime, 
                                              FMassEntityManager& EntityManager)
{
    if (Predator.TargetEntity.IsSet())
    {
        // Verificar se a presa ainda existe
        if (!EntityManager.IsEntityValid(Predator.TargetEntity))
        {
            // Presa desapareceu - voltar ao patrulhamento
            Predator.BehaviorState = ECrowd_PredatorBehavior::Patrolling;
            Predator.TargetEntity.Reset();
            Predator.StateTimer = 2.0f;
            return;
        }
        
        // Obter posição da presa
        const FTransformFragment* PreyTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Predator.TargetEntity);
        if (PreyTransform)
        {
            const FVector ToTarget = PreyTransform->GetTransform().GetLocation() - Transform.GetTransform().GetLocation();
            const float DistanceToTarget = ToTarget.Size();
            
            // Verificar se está dentro do alcance de ataque
            if (DistanceToTarget <= AttackRange)
            {
                Predator.BehaviorState = ECrowd_PredatorBehavior::Attacking;
                Predator.StateTimer = 2.0f; // Duração do ataque
                Velocity.Value = FVector::ZeroVector; // Parar para atacar
                return;
            }
            
            // Verificar se a presa está muito longe
            if (DistanceToTarget > HuntingRange * 1.5f)
            {
                // Desistir da caça
                Predator.BehaviorState = ECrowd_PredatorBehavior::Patrolling;
                Predator.TargetEntity.Reset();
                Predator.HuntCooldownTimer = HuntCooldown;
                Predator.StateTimer = 3.0f;
                return;
            }
            
            // Perseguir a presa
            const FVector HuntDirection = ToTarget.GetSafeNormal();
            Velocity.Value = HuntDirection * MaxHuntSpeed;
        }
    }
    else
    {
        // Sem alvo - voltar ao patrulhamento
        Predator.BehaviorState = ECrowd_PredatorBehavior::Patrolling;
        Predator.StateTimer = 2.0f;
    }
}

void UCrowd_MassPredatorSystem::ProcessAttacking(FTransformFragment& Transform, FMassVelocityFragment& Velocity, 
                                                FCrowd_PredatorFragment& Predator, float DeltaTime)
{
    // Parar durante o ataque
    Velocity.Value = FVector::ZeroVector;
    
    Predator.StateTimer -= DeltaTime;
    if (Predator.StateTimer <= 0.0f)
    {
        // Ataque concluído - passar para alimentação
        Predator.BehaviorState = ECrowd_PredatorBehavior::Feeding;
        Predator.StateTimer = FMath::RandRange(5.0f, 10.0f); // 5-10 segundos a alimentar-se
        Predator.TargetEntity.Reset(); // A presa foi "consumida"
    }
}

void UCrowd_MassPredatorSystem::ProcessFeeding(FTransformFragment& Transform, FMassVelocityFragment& Velocity, 
                                              FCrowd_PredatorFragment& Predator, float DeltaTime)
{
    // Parar durante a alimentação
    Velocity.Value = FVector::ZeroVector;
    
    Predator.StateTimer -= DeltaTime;
    if (Predator.StateTimer <= 0.0f)
    {
        // Alimentação concluída - descansar
        Predator.BehaviorState = ECrowd_PredatorBehavior::Resting;
        Predator.StateTimer = FMath::RandRange(10.0f, 20.0f); // 10-20 segundos a descansar
        Predator.HuntCooldownTimer = HuntCooldown * 2.0f; // Cooldown mais longo após alimentação
    }
}

void UCrowd_MassPredatorSystem::ProcessResting(FTransformFragment& Transform, FMassVelocityFragment& Velocity, 
                                              FCrowd_PredatorFragment& Predator, float DeltaTime)
{
    // Parar durante o descanso
    Velocity.Value = FVector::ZeroVector;
    
    Predator.StateTimer -= DeltaTime;
    if (Predator.StateTimer <= 0.0f)
    {
        // Descanso concluído - voltar ao patrulhamento
        Predator.BehaviorState = ECrowd_PredatorBehavior::Patrolling;
        Predator.StateTimer = FMath::RandRange(5.0f, 15.0f);
    }
}

void UCrowd_MassPredatorSystem::DetectNearbyPrey(const FTransformFragment& PredatorTransform, 
                                                FCrowd_PredatorFragment& Predator, 
                                                FMassEntityManager& EntityManager)
{
    const FVector PredatorLocation = PredatorTransform.GetTransform().GetLocation();
    float ClosestDistance = HuntingRange;
    FMassEntityHandle ClosestPrey;
    
    // Procurar presas próximas
    PreyQuery.ForEachEntityChunk(EntityManager, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassEntityHandle> EntityList = Context.GetEntities();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& PreyTransform = TransformList[EntityIndex];
            const FVector PreyLocation = PreyTransform.GetTransform().GetLocation();
            
            const float Distance = FVector::Dist(PredatorLocation, PreyLocation);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestPrey = EntityList[EntityIndex];
            }
        }
    });
    
    // Se encontrou uma presa, iniciar caça
    if (ClosestPrey.IsValid())
    {
        Predator.BehaviorState = ECrowd_PredatorBehavior::Hunting;
        Predator.TargetEntity = ClosestPrey;
        Predator.StateTimer = 30.0f; // Máximo 30 segundos de caça
    }
}

FMassEntityQuery& UCrowd_MassPredatorSystem::GetPredatorQuery()
{
    return PredatorQuery;
}

FMassEntityQuery& UCrowd_MassPredatorSystem::GetPreyQuery()
{
    return PreyQuery;
}