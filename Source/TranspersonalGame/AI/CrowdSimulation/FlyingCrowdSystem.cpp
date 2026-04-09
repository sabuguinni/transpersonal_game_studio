#include "FlyingCrowdSystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// ========================================
// UFlyingFlockProcessor
// ========================================

UFlyingFlockProcessor::UFlyingFlockProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UFlyingFlockProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FFlyingCreatureFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
}

void UFlyingFlockProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto FlyingList = Context.GetMutableFragmentView<FFlyingCreatureFragment>();

        // Coletar posições e velocidades para cálculos de bando
        TArray<FVector> Positions;
        TArray<FVector> Velocities;
        TArray<int32> FlockIDs;
        
        Positions.Reserve(NumEntities);
        Velocities.Reserve(NumEntities);
        FlockIDs.Reserve(NumEntities);

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            const FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FFlyingCreatureFragment& Flying = FlyingList[EntityIndex];

            Positions.Add(Transform.GetTransform().GetLocation());
            Velocities.Add(Velocity.Value);
            FlockIDs.Add(Flying.FlockID);
        }

        // Processar cada entidade
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FFlyingCreatureFragment& Flying = FlyingList[EntityIndex];

            FVector CurrentPosition = Transform.GetTransform().GetLocation();
            FVector CurrentVelocity = Velocity.Value;

            // Coletar vizinhos do mesmo bando
            TArray<FVector> FlockMatePositions;
            TArray<FVector> FlockMateVelocities;
            FVector FlockCenter = FVector::ZeroVector;
            int32 FlockMateCount = 0;

            for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
            {
                if (OtherIndex == EntityIndex) continue;
                
                if (FlockIDs[OtherIndex] == Flying.FlockID)
                {
                    float Distance = FVector::Dist(CurrentPosition, Positions[OtherIndex]);
                    if (Distance <= Flying.FlockRadius)
                    {
                        FlockMatePositions.Add(Positions[OtherIndex]);
                        FlockMateVelocities.Add(Velocities[OtherIndex]);
                        FlockCenter += Positions[OtherIndex];
                        FlockMateCount++;
                    }
                }
            }

            // Calcular centro do bando
            if (FlockMateCount > 0)
            {
                FlockCenter /= FlockMateCount;
            }
            else
            {
                FlockCenter = CurrentPosition;
            }

            // Calcular velocidade média do bando
            FVector AverageFlockVelocity = FVector::ZeroVector;
            if (FlockMateVelocities.Num() > 0)
            {
                for (const FVector& FlockVel : FlockMateVelocities)
                {
                    AverageFlockVelocity += FlockVel;
                }
                AverageFlockVelocity /= FlockMateVelocities.Num();
            }

            // Calcular forças do bando
            FVector CohesionForce = CalculateFlockCohesion(CurrentPosition, FlockCenter, Flying.FlockRadius);
            FVector SeparationForce = CalculateFlockSeparation(CurrentPosition, FlockMatePositions, Flying.FlockRadius * 0.3f);
            FVector AlignmentForce = CalculateFlockAlignment(CurrentVelocity, AverageFlockVelocity);
            FVector AltitudeForce = CalculateAltitudeCorrection(CurrentPosition, Flying.PreferredAltitude, Flying.AltitudeVariance);

            // Combinar forças
            FVector TotalForce = 
                CohesionForce * Flying.FlockCohesion +
                SeparationForce * Flying.FlockSeparation +
                AlignmentForce * Flying.FlockAlignment +
                AltitudeForce * 1.5f; // Força de altitude sempre importante

            // Aplicar força à velocidade
            FVector NewVelocity = CurrentVelocity + TotalForce * Context.GetDeltaTimeSeconds();
            
            // Limitar velocidade
            if (NewVelocity.Size() > Flying.FlyingSpeed)
            {
                NewVelocity = NewVelocity.GetSafeNormal() * Flying.FlyingSpeed;
            }

            // Atualizar velocidade
            Velocity.Value = NewVelocity;

            // Atualizar posição
            FVector NewPosition = CurrentPosition + NewVelocity * Context.GetDeltaTimeSeconds();
            FTransform NewTransform = Transform.GetTransform();
            NewTransform.SetLocation(NewPosition);
            
            // Orientar na direção do movimento
            if (!NewVelocity.IsNearlyZero())
            {
                FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, NewVelocity);
                NewTransform.SetRotation(NewRotation.Quaternion());
            }
            
            Transform.SetTransform(NewTransform);
        }
    });
}

FVector UFlyingFlockProcessor::CalculateFlockCohesion(const FVector& Position, const FVector& FlockCenter, float FlockRadius)
{
    FVector ToCenter = FlockCenter - Position;
    float Distance = ToCenter.Size();
    
    if (Distance > FlockRadius * 0.5f) // Só aplicar coesão se estiver longe do centro
    {
        return ToCenter.GetSafeNormal() * FMath::Clamp(Distance / FlockRadius, 0.0f, 1.0f);
    }
    
    return FVector::ZeroVector;
}

FVector UFlyingFlockProcessor::CalculateFlockSeparation(const FVector& Position, const TArray<FVector>& NearbyPositions, float SeparationRadius)
{
    FVector SeparationForce = FVector::ZeroVector;
    
    for (const FVector& NearbyPos : NearbyPositions)
    {
        FVector ToOther = NearbyPos - Position;
        float Distance = ToOther.Size();
        
        if (Distance < SeparationRadius && Distance > 0.1f)
        {
            // Força inversamente proporcional à distância
            FVector AwayForce = -ToOther.GetSafeNormal() * (SeparationRadius - Distance) / SeparationRadius;
            SeparationForce += AwayForce;
        }
    }
    
    return SeparationForce;
}

FVector UFlyingFlockProcessor::CalculateFlockAlignment(const FVector& Velocity, const FVector& AverageFlockVelocity)
{
    if (AverageFlockVelocity.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }
    
    FVector VelocityDiff = AverageFlockVelocity - Velocity;
    return VelocityDiff.GetSafeNormal() * FMath::Clamp(VelocityDiff.Size() / 1000.0f, 0.0f, 1.0f);
}

FVector UFlyingFlockProcessor::CalculateAltitudeCorrection(const FVector& Position, float PreferredAltitude, float AltitudeVariance)
{
    float CurrentAltitude = Position.Z;
    float TargetAltitude = PreferredAltitude + FMath::RandRange(-AltitudeVariance, AltitudeVariance);
    
    float AltitudeDiff = TargetAltitude - CurrentAltitude;
    
    if (FMath::Abs(AltitudeDiff) > AltitudeVariance * 0.5f)
    {
        return FVector(0, 0, FMath::Sign(AltitudeDiff)) * FMath::Clamp(FMath::Abs(AltitudeDiff) / AltitudeVariance, 0.0f, 2.0f);
    }
    
    return FVector::ZeroVector;
}

// ========================================
// UFlyingMigrationProcessor
// ========================================

UFlyingMigrationProcessor::UFlyingMigrationProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UFlyingMigrationProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FFlyingCreatureFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMigrationPatternFragment>(EMassFragmentAccess::ReadWrite);
}

void UFlyingMigrationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto FlyingList = Context.GetFragmentView<FFlyingCreatureFragment>();
        const auto MigrationList = Context.GetMutableFragmentView<FMigrationPatternFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FFlyingCreatureFragment& Flying = FlyingList[EntityIndex];
            FMigrationPatternFragment& Migration = MigrationList[EntityIndex];

            // Verificar se está em estado de migração
            if (Flying.FlightState != 3) // 3 = Migrating
            {
                continue;
            }

            FVector CurrentPosition = Transform.GetTransform().GetLocation();

            // Verificar se há waypoints
            if (Migration.MigrationWaypoints.Num() == 0)
            {
                continue;
            }

            // Verificar se está descansando
            if (Migration.CurrentRestTime > 0.0f)
            {
                Migration.CurrentRestTime -= Context.GetDeltaTimeSeconds();
                Velocity.Value = FVector::ZeroVector; // Parar durante o descanso
                continue;
            }

            // Obter waypoint atual
            int32 WaypointIndex = FMath::Clamp(Migration.CurrentWaypointIndex, 0, Migration.MigrationWaypoints.Num() - 1);
            FVector TargetWaypoint = Migration.MigrationWaypoints[WaypointIndex];

            // Calcular direção para o waypoint
            FVector ToWaypoint = TargetWaypoint - CurrentPosition;
            float DistanceToWaypoint = ToWaypoint.Size();

            // Verificar se chegou ao waypoint
            if (DistanceToWaypoint <= Migration.WaypointReachDistance)
            {
                // Avançar para próximo waypoint
                Migration.CurrentWaypointIndex = (Migration.CurrentWaypointIndex + 1) % Migration.MigrationWaypoints.Num();
                
                // Iniciar período de descanso
                Migration.CurrentRestTime = Migration.RestDuration;
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Flying creature reached waypoint %d, starting rest"), WaypointIndex);
                continue;
            }

            // Mover em direção ao waypoint
            FVector MigrationVelocity = ToWaypoint.GetSafeNormal() * Migration.MigrationSpeed;
            Velocity.Value = MigrationVelocity;
        }
    });
}

// ========================================
// UFlyingObstacleAvoidanceProcessor
// ========================================

UFlyingObstacleAvoidanceProcessor::UFlyingObstacleAvoidanceProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ObstacleDetectionRange = 1000.0f;
    AvoidanceForce = 2.0f;
    MinGroundClearance = 200.0f;
}

void UFlyingObstacleAvoidanceProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FFlyingCreatureFragment>(EMassFragmentAccess::ReadOnly);
}

void UFlyingObstacleAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = EntityManager.GetWorld();
    if (!World)
    {
        return;
    }

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, World](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const auto TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto FlyingList = Context.GetFragmentView<FFlyingCreatureFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FFlyingCreatureFragment& Flying = FlyingList[EntityIndex];

            FVector CurrentPosition = Transform.GetTransform().GetLocation();
            FVector CurrentVelocity = Velocity.Value;

            if (CurrentVelocity.IsNearlyZero())
            {
                continue;
            }

            FVector AvoidanceForceVector = FVector::ZeroVector;

            // Verificar colisão com o solo
            FVector GroundCheckStart = CurrentPosition;
            FVector GroundCheckEnd = CurrentPosition - FVector(0, 0, MinGroundClearance + 100.0f);
            
            FHitResult GroundHit;
            if (World->LineTraceSingleByChannel(GroundHit, GroundCheckStart, GroundCheckEnd, ECC_WorldStatic))
            {
                float GroundDistance = GroundHit.Distance;
                if (GroundDistance < MinGroundClearance)
                {
                    // Força para subir
                    float UpwardForce = (MinGroundClearance - GroundDistance) / MinGroundClearance;
                    AvoidanceForceVector += FVector(0, 0, UpwardForce * AvoidanceForce);
                }
            }

            // Verificar obstáculos à frente
            FVector ForwardDirection = CurrentVelocity.GetSafeNormal();
            FVector ObstacleCheckStart = CurrentPosition;
            FVector ObstacleCheckEnd = CurrentPosition + ForwardDirection * ObstacleDetectionRange;

            FHitResult ObstacleHit;
            if (World->LineTraceSingleByChannel(ObstacleHit, ObstacleCheckStart, ObstacleCheckEnd, ECC_WorldStatic))
            {
                FVector HitNormal = ObstacleHit.Normal;
                float DistanceToObstacle = ObstacleHit.Distance;
                
                // Calcular força de evitamento
                float AvoidanceStrength = (ObstacleDetectionRange - DistanceToObstacle) / ObstacleDetectionRange;
                FVector AvoidanceDirection = FVector::CrossProduct(ForwardDirection, FVector::UpVector).GetSafeNormal();
                
                // Escolher lado de evitamento baseado na normal do obstáculo
                if (FVector::DotProduct(AvoidanceDirection, HitNormal) < 0)
                {
                    AvoidanceDirection = -AvoidanceDirection;
                }
                
                AvoidanceForceVector += AvoidanceDirection * AvoidanceStrength * AvoidanceForce;
                
                // Adicionar componente para cima se necessário
                if (HitNormal.Z < 0.7f) // Se não é uma superfície muito inclinada
                {
                    AvoidanceForceVector += FVector(0, 0, AvoidanceStrength * AvoidanceForce * 0.5f);
                }
            }

            // Aplicar força de evitamento
            if (!AvoidanceForceVector.IsNearlyZero())
            {
                FVector NewVelocity = CurrentVelocity + AvoidanceForceVector * Context.GetDeltaTimeSeconds();
                
                // Manter velocidade original se possível
                if (NewVelocity.Size() > Flying.FlyingSpeed)
                {
                    NewVelocity = NewVelocity.GetSafeNormal() * Flying.FlyingSpeed;
                }
                
                Velocity.Value = NewVelocity;
            }
        }
    });
}