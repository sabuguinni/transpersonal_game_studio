#include "MassEntityProcessors.h"
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

// ===== UCrowd_MassMovementProcessor =====

UCrowd_MassMovementProcessor::UCrowd_MassMovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    
    // Configurar query
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Parâmetros de movimento
    MaxSpeed = 800.0f;        // 8 m/s
    MaxAcceleration = 400.0f; // 4 m/s²
    TurnRate = 180.0f;        // 180 graus/segundo
}

void UCrowd_MassMovementProcessor::ConfigureQueries()
{
    Super::ConfigureQueries();
}

void UCrowd_MassMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TArrayView<FMassRepresentationLODFragment> LODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            const FMassRepresentationLODFragment& LOD = LODList[EntityIndex];
            
            // Apenas processar se estiver em LOD visível
            if (LOD.LODLevel > EMassLOD::Medium)
            {
                continue;
            }
            
            // Aplicar aceleração baseada na velocidade desejada
            const FVector DesiredVelocity = Movement.DesiredDirection * Movement.DesiredSpeed;
            const FVector VelocityDifference = DesiredVelocity - Velocity.Value;
            
            // Limitar aceleração
            FVector Acceleration = VelocityDifference;
            if (Acceleration.SizeSquared() > FMath::Square(MaxAcceleration * DeltaTime))
            {
                Acceleration = Acceleration.GetSafeNormal() * MaxAcceleration * DeltaTime;
            }
            
            // Aplicar aceleração
            Velocity.Value += Acceleration;
            
            // Limitar velocidade máxima
            if (Velocity.Value.SizeSquared() > FMath::Square(MaxSpeed))
            {
                Velocity.Value = Velocity.Value.GetSafeNormal() * MaxSpeed;
            }
            
            // Actualizar posição
            FTransform CurrentTransform = Transform.GetTransform();
            FVector NewLocation = CurrentTransform.GetLocation() + Velocity.Value * DeltaTime;
            
            // Actualizar rotação para seguir a direcção do movimento
            if (Velocity.Value.SizeSquared() > 1.0f)
            {
                const FVector ForwardDirection = Velocity.Value.GetSafeNormal();
                const FRotator TargetRotation = ForwardDirection.Rotation();
                const FRotator CurrentRotation = CurrentTransform.GetRotation().Rotator();
                
                // Interpolar rotação suavemente
                const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnRate);
                CurrentTransform.SetRotation(NewRotation.Quaternion());
            }
            
            CurrentTransform.SetLocation(NewLocation);
            Transform.SetTransform(CurrentTransform);
        }
    });
}

// ===== UCrowd_MassAvoidanceProcessor =====

UCrowd_MassAvoidanceProcessor::UCrowd_MassAvoidanceProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    
    // Configurar query
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Parâmetros de evitamento
    AvoidanceRadius = 300.0f;  // 3 metros
    AvoidanceStrength = 500.0f;
    MaxNeighbors = 10;
}

void UCrowd_MassAvoidanceProcessor::ConfigureQueries()
{
    Super::ConfigureQueries();
}

void UCrowd_MassAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Obter todas as entidades para comparação
    TArray<FVector> AllPositions;
    TArray<FMassEntityHandle> AllEntities;
    
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassEntityHandle> EntityList = Context.GetEntities();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            AllPositions.Add(TransformList[EntityIndex].GetTransform().GetLocation());
            AllEntities.Add(EntityList[EntityIndex]);
        }
    });
    
    // Processar evitamento para cada entidade
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TArrayView<FMassRepresentationLODFragment> LODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
        const TConstArrayView<FMassEntityHandle> EntityList = Context.GetEntities();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FMassRepresentationLODFragment& LOD = LODList[EntityIndex];
            
            // Apenas processar se estiver em LOD visível
            if (LOD.LODLevel > EMassLOD::Medium)
            {
                continue;
            }
            
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            const FMassEntityHandle CurrentEntity = EntityList[EntityIndex];
            
            const FVector CurrentPosition = Transform.GetTransform().GetLocation();
            FVector AvoidanceForce = FVector::ZeroVector;
            int32 NeighborCount = 0;
            
            // Verificar vizinhos próximos
            for (int32 OtherIndex = 0; OtherIndex < AllPositions.Num() && NeighborCount < MaxNeighbors; ++OtherIndex)
            {
                if (AllEntities[OtherIndex] == CurrentEntity)
                {
                    continue; // Ignorar a própria entidade
                }
                
                const FVector OtherPosition = AllPositions[OtherIndex];
                const FVector ToOther = OtherPosition - CurrentPosition;
                const float Distance = ToOther.Size();
                
                if (Distance < AvoidanceRadius && Distance > 1.0f)
                {
                    // Calcular força de evitamento
                    const FVector AvoidDirection = -ToOther.GetSafeNormal();
                    const float AvoidanceWeight = (AvoidanceRadius - Distance) / AvoidanceRadius;
                    AvoidanceForce += AvoidDirection * AvoidanceWeight * AvoidanceStrength;
                    NeighborCount++;
                }
            }
            
            // Aplicar força de evitamento à direcção desejada
            if (!AvoidanceForce.IsNearlyZero())
            {
                const FVector ModifiedDirection = (Movement.DesiredDirection + AvoidanceForce.GetSafeNormal()).GetSafeNormal();
                Movement.DesiredDirection = ModifiedDirection;
            }
        }
    });
}

// ===== UCrowd_MassFlockingProcessor =====

UCrowd_MassFlockingProcessor::UCrowd_MassFlockingProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    
    // Configurar query
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_FlockingFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Parâmetros de flocking
    FlockingRadius = 1000.0f;  // 10 metros
    SeparationWeight = 1.5f;
    AlignmentWeight = 1.0f;
    CohesionWeight = 0.8f;
    MaxNeighbors = 15;
}

void UCrowd_MassFlockingProcessor::ConfigureQueries()
{
    Super::ConfigureQueries();
}

void UCrowd_MassFlockingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementação similar ao avoidance mas com regras de flocking
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_FlockingFragment> FlockingList = Context.GetMutableFragmentView<FCrowd_FlockingFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TArrayView<FMassRepresentationLODFragment> LODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FMassRepresentationLODFragment& LOD = LODList[EntityIndex];
            
            // Apenas processar se estiver em LOD visível
            if (LOD.LODLevel > EMassLOD::High)
            {
                continue;
            }
            
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_FlockingFragment& Flocking = FlockingList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            
            const FVector CurrentPosition = Transform.GetTransform().GetLocation();
            const FVector CurrentVelocity = Velocity.Value;
            
            // Aplicar comportamento de flocking básico
            FVector Separation = FVector::ZeroVector;
            FVector Alignment = FVector::ZeroVector;
            FVector Cohesion = FVector::ZeroVector;
            int32 NeighborCount = 0;
            
            // Simular vizinhos próximos (versão simplificada)
            for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
            {
                if (OtherIndex == EntityIndex)
                {
                    continue;
                }
                
                const FVector OtherPosition = TransformList[OtherIndex].GetTransform().GetLocation();
                const FVector OtherVelocity = VelocityList[OtherIndex].Value;
                const FVector ToOther = OtherPosition - CurrentPosition;
                const float Distance = ToOther.Size();
                
                if (Distance < FlockingRadius && Distance > 1.0f && NeighborCount < MaxNeighbors)
                {
                    // Separation - evitar colisões
                    if (Distance < FlockingRadius * 0.3f)
                    {
                        Separation += -ToOther.GetSafeNormal() / Distance;
                    }
                    
                    // Alignment - alinhar velocidades
                    Alignment += OtherVelocity;
                    
                    // Cohesion - mover-se para o centro do grupo
                    Cohesion += OtherPosition;
                    
                    NeighborCount++;
                }
            }
            
            if (NeighborCount > 0)
            {
                // Normalizar forças
                Alignment = (Alignment / NeighborCount).GetSafeNormal();
                Cohesion = ((Cohesion / NeighborCount) - CurrentPosition).GetSafeNormal();
                
                // Aplicar pesos
                const FVector FlockingForce = (Separation * SeparationWeight + 
                                             Alignment * AlignmentWeight + 
                                             Cohesion * CohesionWeight).GetSafeNormal();
                
                // Combinar com direcção desejada
                Movement.DesiredDirection = (Movement.DesiredDirection + FlockingForce).GetSafeNormal();
            }
        }
    });
}