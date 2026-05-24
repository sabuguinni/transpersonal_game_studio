#include "Crowd_HerdBehaviorManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// UCrowd_HerdBehaviorProcessor Implementation
UCrowd_HerdBehaviorProcessor::UCrowd_HerdBehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UCrowd_HerdBehaviorProcessor::ConfigureQueries()
{
    // Query for herd members
    HerdMemberQuery.AddRequirement<FCrowd_HerdMemberFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerdMemberQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);

    // Query for herd leaders
    HerdLeaderQuery.AddRequirement<FCrowd_HerdMemberFragment>(EMassFragmentAccess::ReadOnly);
    HerdLeaderQuery.AddRequirement<FCrowd_HerdStateFragment>(EMassFragmentAccess::ReadWrite);
    HerdLeaderQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    HerdLeaderQuery.AddChunkRequirement<FCrowd_HerdMemberFragment>([](const FCrowd_HerdMemberFragment& Fragment)
    {
        return Fragment.bIsHerdLeader;
    });
}

void UCrowd_HerdBehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Update herd states first
    UpdateHerdState(EntityManager, Context);
    
    // Process threat responses
    ProcessThreatResponse(EntityManager, Context);

    // Process individual herd member behaviors
    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FCrowd_HerdMemberFragment> HerdMembers = Context.GetMutableFragmentView<FCrowd_HerdMemberFragment>();
        const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            FCrowd_HerdMemberFragment& HerdMember = HerdMembers[i];
            FTransformFragment& Transform = Transforms[i];
            FMassVelocityFragment& Velocity = Velocities[i];

            // Skip if not time to update
            if (CurrentTime - HerdMember.LastUpdateTime < (1.0f / UpdateFrequency))
            {
                continue;
            }

            HerdMember.LastUpdateTime = CurrentTime;
            FMassEntityHandle CurrentEntity = Context.GetEntity(i);

            // Calculate flocking forces
            FVector CohesionForce = CalculateCohesion(EntityManager, Context, CurrentEntity, HerdMember);
            FVector SeparationForce = CalculateSeparation(EntityManager, Context, CurrentEntity, HerdMember);
            FVector AlignmentForce = CalculateAlignment(EntityManager, Context, CurrentEntity, HerdMember);

            // Combine forces
            FVector TotalForce = (CohesionForce * HerdMember.CohesionStrength) +
                               (SeparationForce * HerdMember.SeparationStrength) +
                               (AlignmentForce * HerdMember.AlignmentStrength);

            // Apply force to velocity
            if (!TotalForce.IsNearlyZero())
            {
                TotalForce.Normalize();
                Velocity.Value += TotalForce * 100.0f * DeltaTime; // Base movement speed
                
                // Limit velocity
                if (Velocity.Value.Size() > 200.0f)
                {
                    Velocity.Value = Velocity.Value.GetSafeNormal() * 200.0f;
                }

                // Update rotation to face movement direction
                if (!Velocity.Value.IsNearlyZero())
                {
                    FRotator NewRotation = FRotationMatrix::MakeFromX(Velocity.Value.GetSafeNormal()).Rotator();
                    Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
                }
            }
        }
    });
}

FVector UCrowd_HerdBehaviorProcessor::CalculateCohesion(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context, 
                                                       FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    const FTransformFragment* MyTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
    if (!MyTransform)
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = MyTransform->GetTransform().GetLocation();

    // Find nearby herd members of the same species
    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkContext)
    {
        const TArrayView<FCrowd_HerdMemberFragment> HerdMembers = ChunkContext.GetFragmentView<FCrowd_HerdMemberFragment>();
        const TArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            FMassEntityHandle OtherEntity = ChunkContext.GetEntity(i);
            if (OtherEntity == Entity) continue;

            const FCrowd_HerdMemberFragment& OtherHerdData = HerdMembers[i];
            if (OtherHerdData.Species != HerdData.Species) continue;

            FVector OtherLocation = Transforms[i].GetTransform().GetLocation();
            float Distance = FVector::Dist(MyLocation, OtherLocation);

            if (Distance < NeighborSearchRadius)
            {
                CenterOfMass += OtherLocation;
                NeighborCount++;
            }
        }
    });

    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        return (CenterOfMass - MyLocation).GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector UCrowd_HerdBehaviorProcessor::CalculateSeparation(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context,
                                                         FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData)
{
    FVector SeparationForce = FVector::ZeroVector;
    
    const FTransformFragment* MyTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
    if (!MyTransform)
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = MyTransform->GetTransform().GetLocation();

    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkContext)
    {
        const TArrayView<FCrowd_HerdMemberFragment> HerdMembers = ChunkContext.GetFragmentView<FCrowd_HerdMemberFragment>();
        const TArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            FMassEntityHandle OtherEntity = ChunkContext.GetEntity(i);
            if (OtherEntity == Entity) continue;

            const FCrowd_HerdMemberFragment& OtherHerdData = HerdMembers[i];
            if (OtherHerdData.Species != HerdData.Species) continue;

            FVector OtherLocation = Transforms[i].GetTransform().GetLocation();
            float Distance = FVector::Dist(MyLocation, OtherLocation);

            if (Distance < HerdData.PreferredDistance && Distance > 0.1f)
            {
                FVector AwayVector = (MyLocation - OtherLocation).GetSafeNormal();
                float Strength = (HerdData.PreferredDistance - Distance) / HerdData.PreferredDistance;
                SeparationForce += AwayVector * Strength;
            }
        }
    });

    return SeparationForce.GetSafeNormal();
}

FVector UCrowd_HerdBehaviorProcessor::CalculateAlignment(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context,
                                                        FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    const FTransformFragment* MyTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
    if (!MyTransform)
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = MyTransform->GetTransform().GetLocation();

    HerdMemberQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkContext)
    {
        const TArrayView<FCrowd_HerdMemberFragment> HerdMembers = ChunkContext.GetFragmentView<FCrowd_HerdMemberFragment>();
        const TArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> Velocities = ChunkContext.GetFragmentView<FMassVelocityFragment>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            FMassEntityHandle OtherEntity = ChunkContext.GetEntity(i);
            if (OtherEntity == Entity) continue;

            const FCrowd_HerdMemberFragment& OtherHerdData = HerdMembers[i];
            if (OtherHerdData.Species != HerdData.Species) continue;

            FVector OtherLocation = Transforms[i].GetTransform().GetLocation();
            float Distance = FVector::Dist(MyLocation, OtherLocation);

            if (Distance < NeighborSearchRadius)
            {
                AverageVelocity += Velocities[i].Value;
                NeighborCount++;
            }
        }
    });

    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

void UCrowd_HerdBehaviorProcessor::UpdateHerdState(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Update herd leaders' state information
    HerdLeaderQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FCrowd_HerdMemberFragment> HerdMembers = Context.GetFragmentView<FCrowd_HerdMemberFragment>();
        const TArrayView<FCrowd_HerdStateFragment> HerdStates = Context.GetMutableFragmentView<FCrowd_HerdStateFragment>();
        const TArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            if (!HerdMembers[i].bIsHerdLeader) continue;

            FCrowd_HerdStateFragment& HerdState = HerdStates[i];
            FVector LeaderLocation = Transforms[i].GetTransform().GetLocation();

            // Calculate herd center and size
            FVector HerdCenter = FVector::ZeroVector;
            int32 HerdSize = 0;
            float MaxDistance = 0.0f;

            // This would need to be optimized with spatial partitioning in a real implementation
            // For now, simple iteration through all entities
            HerdState.HerdCenter = LeaderLocation;
            HerdState.HerdSize = 1; // At least the leader
            HerdState.HerdRadius = 300.0f; // Default radius
        }
    });
}

void UCrowd_HerdBehaviorProcessor::ProcessThreatResponse(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implement threat detection and response logic
    // This would involve checking for predators, player proximity, loud noises, etc.
    // For now, placeholder implementation
}

// UCrowd_HerdBehaviorComponent Implementation
UCrowd_HerdBehaviorComponent::UCrowd_HerdBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateInterval;
}

void UCrowd_HerdBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateHerdMembers();
}

void UCrowd_HerdBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        LastUpdateTime = CurrentTime;
        UpdateHerdMembers();
        UpdateHerdCenter();
        DetectThreats();

        // Apply flocking behavior
        if (HerdMembers.Num() > 1)
        {
            FVector DesiredDirection = GetDesiredMovementDirection();
            if (!DesiredDirection.IsNearlyZero())
            {
                // Apply movement to owner actor
                if (AActor* Owner = GetOwner())
                {
                    FVector NewLocation = Owner->GetActorLocation() + (DesiredDirection * MovementSpeed * DeltaTime);
                    Owner->SetActorLocation(NewLocation);
                    
                    // Face movement direction
                    FRotator NewRotation = FRotationMatrix::MakeFromX(DesiredDirection).Rotator();
                    Owner->SetActorRotation(NewRotation);
                }
            }
        }
    }
}

void UCrowd_HerdBehaviorComponent::SetHerdBehavior(ECrowd_HerdBehavior NewBehavior)
{
    CurrentBehavior = NewBehavior;
}

void UCrowd_HerdBehaviorComponent::AddHerdMember(AActor* NewMember)
{
    if (NewMember && !HerdMembers.Contains(NewMember))
    {
        HerdMembers.Add(NewMember);
    }
}

void UCrowd_HerdBehaviorComponent::RemoveHerdMember(AActor* Member)
{
    HerdMembers.Remove(Member);
}

FVector UCrowd_HerdBehaviorComponent::GetDesiredMovementDirection() const
{
    return CalculateFlockingForce();
}

void UCrowd_HerdBehaviorComponent::DetectThreats()
{
    // Placeholder threat detection
    // In a full implementation, this would check for predators, player proximity, etc.
}

void UCrowd_HerdBehaviorComponent::UpdateHerdMembers()
{
    // Find nearby actors with herd behavior components
    if (AActor* Owner = GetOwner())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

        HerdMembers.Empty();
        FVector MyLocation = Owner->GetActorLocation();

        for (AActor* Actor : FoundActors)
        {
            if (Actor == Owner) continue;

            UCrowd_HerdBehaviorComponent* OtherHerdComp = Actor->FindComponentByClass<UCrowd_HerdBehaviorComponent>();
            if (OtherHerdComp && OtherHerdComp->Species == Species)
            {
                float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
                if (Distance <= HerdRadius)
                {
                    HerdMembers.Add(Actor);
                }
            }
        }
    }
}

void UCrowd_HerdBehaviorComponent::UpdateHerdCenter()
{
    if (HerdMembers.Num() == 0)
    {
        HerdCenter = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        return;
    }

    FVector TotalLocation = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (AActor* Member : HerdMembers)
    {
        if (IsValid(Member))
        {
            TotalLocation += Member->GetActorLocation();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        HerdCenter = TotalLocation / ValidMembers;
    }
}

FVector UCrowd_HerdBehaviorComponent::CalculateFlockingForce() const
{
    FVector CohesionForce = CalculateCohesionForce();
    FVector SeparationForce = CalculateSeparationForce();
    FVector AlignmentForce = CalculateAlignmentForce();

    return (CohesionForce * CohesionWeight) + 
           (SeparationForce * SeparationWeight) + 
           (AlignmentForce * AlignmentWeight);
}

FVector UCrowd_HerdBehaviorComponent::CalculateCohesionForce() const
{
    if (HerdMembers.Num() == 0 || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    return (HerdCenter - GetOwner()->GetActorLocation()).GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::CalculateSeparationForce() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector SeparationForce = FVector::ZeroVector;
    FVector MyLocation = GetOwner()->GetActorLocation();

    for (AActor* Member : HerdMembers)
    {
        if (IsValid(Member))
        {
            float Distance = FVector::Dist(MyLocation, Member->GetActorLocation());
            if (Distance < PreferredDistance && Distance > 0.1f)
            {
                FVector AwayVector = (MyLocation - Member->GetActorLocation()).GetSafeNormal();
                float Strength = (PreferredDistance - Distance) / PreferredDistance;
                SeparationForce += AwayVector * Strength;
            }
        }
    }

    return SeparationForce.GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::CalculateAlignmentForce() const
{
    if (HerdMembers.Num() == 0 || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector AverageDirection = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (AActor* Member : HerdMembers)
    {
        if (IsValid(Member))
        {
            AverageDirection += Member->GetActorForwardVector();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        AverageDirection /= ValidMembers;
        return AverageDirection.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

// ACrowd_HerdSpawner Implementation
ACrowd_HerdSpawner::ACrowd_HerdSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACrowd_HerdSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnOnBeginPlay)
    {
        SpawnHerd();
    }
}

void ACrowd_HerdSpawner::SpawnHerd()
{
    if (bUseMassEntity)
    {
        SpawnMassEntityHerd();
    }
    else
    {
        SpawnTraditionalHerd();
    }
}

void ACrowd_HerdSpawner::SpawnMassEntityHerd()
{
    // Mass Entity spawning implementation
    UMassEntitySubsystem* MassSubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem not available, falling back to traditional spawning"));
        SpawnTraditionalHerd();
        return;
    }

    // Create Mass Entity configuration and spawn entities
    // This would require proper Mass Entity setup which is complex
    // For now, fall back to traditional spawning
    SpawnTraditionalHerd();
}

void ACrowd_HerdSpawner::SpawnTraditionalHerd()
{
    DestroyHerd(); // Clear existing herd

    if (!DinosaurClass)
    {
        // Use default static mesh actor if no specific class is set
        DinosaurClass = AStaticMeshActor::StaticClass();
    }

    FVector SpawnerLocation = GetActorLocation();
    
    for (int32 i = 0; i < HerdSize; ++i)
    {
        FVector SpawnLocation = GetRandomSpawnLocation();
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(DinosaurClass, SpawnLocation, SpawnRotation);
        if (SpawnedActor)
        {
            // Add herd behavior component
            UCrowd_HerdBehaviorComponent* HerdComp = NewObject<UCrowd_HerdBehaviorComponent>(SpawnedActor);
            SpawnedActor->AddInstanceComponent(HerdComp);
            HerdComp->Species = SpeciesToSpawn;
            HerdComp->bIsHerdLeader = (i == 0); // First spawned is leader
            HerdComp->RegisterComponent();

            SpawnedHerd.Add(SpawnedActor);
            
            // Set actor name for identification
            SpawnedActor->SetActorLabel(FString::Printf(TEXT("%s_Herd_%d"), 
                *UEnum::GetValueAsString(SpeciesToSpawn), i));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned traditional herd of %d %s"), 
           SpawnedHerd.Num(), *UEnum::GetValueAsString(SpeciesToSpawn));
}

void ACrowd_HerdSpawner::DestroyHerd()
{
    for (AActor* Actor : SpawnedHerd)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedHerd.Empty();
}

FVector ACrowd_HerdSpawner::GetRandomSpawnLocation() const
{
    FVector SpawnerLocation = GetActorLocation();
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(50.0f, SpawnRadius);
    
    FVector Offset = FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        FMath::RandRange(-20.0f, 20.0f)
    );

    return SpawnerLocation + Offset;
}