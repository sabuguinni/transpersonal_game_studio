#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MassEntityProcessor::UCrowd_MassEntityProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UCrowd_MassEntityProcessor::ConfigureQueries()
{
    // Configure tribal behavior query
    TribalBehaviorQuery.AddRequirement<FCrowd_TribalFragment>(EMassFragmentAccess::ReadWrite);
    TribalBehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    TribalBehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

    // Configure movement query
    MovementQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadOnly);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);

    // Configure social interaction query
    SocialInteractionQuery.AddRequirement<FCrowd_TribalFragment>(EMassFragmentAccess::ReadWrite);
    SocialInteractionQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    SocialInteractionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassEntityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Reset frame counter
    ProcessedEntitiesThisFrame = 0;
    
    // Update behavior timer
    if (CurrentTime - LastUpdateTime >= BehaviorUpdateInterval)
    {
        ProcessTribalBehaviors(EntityManager, Context);
        LastUpdateTime = CurrentTime;
    }
    
    // Always process movement and social interactions
    ProcessMovement(EntityManager, Context);
    ProcessSocialInteractions(EntityManager, Context);
}

void UCrowd_MassEntityProcessor::ProcessTribalBehaviors(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    float DeltaTime = Context.GetDeltaTimeSeconds();
    
    TribalBehaviorQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FCrowd_TribalFragment> TribalList = Context.GetMutableFragmentView<FCrowd_TribalFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities && ProcessedEntitiesThisFrame < MaxEntitiesPerFrame; ++EntityIndex)
        {
            FCrowd_TribalFragment& TribalData = TribalList[EntityIndex];
            FCrowd_BehaviorFragment& BehaviorData = BehaviorList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];
            
            UpdateBehaviorState(TribalData, BehaviorData, DeltaTime);
            ProcessedEntitiesThisFrame++;
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    float DeltaTime = Context.GetDeltaTimeSeconds();
    
    MovementQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FCrowd_BehaviorFragment& BehaviorData = BehaviorList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FTransformFragment& Transform = TransformList[EntityIndex];
            
            // Calculate movement direction
            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector Direction = (BehaviorData.TargetLocation - CurrentLocation).GetSafeNormal();
            
            // Apply movement based on behavior
            float SpeedMultiplier = 1.0f;
            switch (BehaviorData.CurrentBehavior)
            {
                case 1: // Work
                    SpeedMultiplier = 0.8f;
                    break;
                case 2: // Hunt
                    SpeedMultiplier = 1.5f;
                    break;
                case 3: // Gather
                    SpeedMultiplier = 0.6f;
                    break;
                case 4: // Social
                    SpeedMultiplier = 0.4f;
                    break;
                case 5: // Rest
                    SpeedMultiplier = 0.1f;
                    break;
                default: // Idle
                    SpeedMultiplier = 0.3f;
                    break;
            }
            
            // Set velocity
            Velocity.Value = Direction * BehaviorData.MovementSpeed * SpeedMultiplier;
            
            // Check if reached target
            float DistanceToTarget = FVector::Dist(CurrentLocation, BehaviorData.TargetLocation);
            if (DistanceToTarget < 100.0f)
            {
                // Reached target - reduce velocity
                Velocity.Value *= 0.1f;
            }
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessSocialInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SocialInteractionQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FCrowd_TribalFragment> TribalList = Context.GetMutableFragmentView<FCrowd_TribalFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        
        // Process social interactions between nearby entities
        for (int32 i = 0; i < NumEntities; ++i)
        {
            FCrowd_TribalFragment& TribalA = TribalList[i];
            FCrowd_BehaviorFragment& BehaviorA = BehaviorList[i];
            const FVector LocationA = TransformList[i].GetTransform().GetLocation();
            
            // Check interactions with other entities
            for (int32 j = i + 1; j < NumEntities; ++j)
            {
                FCrowd_TribalFragment& TribalB = TribalList[j];
                FCrowd_BehaviorFragment& BehaviorB = BehaviorList[j];
                const FVector LocationB = TransformList[j].GetTransform().GetLocation();
                
                float Distance = FVector::Dist(LocationA, LocationB);
                if (Distance <= SocialInteractionRadius)
                {
                    // Same tribe interaction
                    if (TribalA.TribeID == TribalB.TribeID)
                    {
                        // Boost social status and energy
                        TribalA.SocialStatus = FMath::Min(100.0f, TribalA.SocialStatus + 0.5f);
                        TribalB.SocialStatus = FMath::Min(100.0f, TribalB.SocialStatus + 0.5f);
                        TribalA.Energy = FMath::Min(100.0f, TribalA.Energy + 0.2f);
                        TribalB.Energy = FMath::Min(100.0f, TribalB.Energy + 0.2f);
                        
                        // Leader influence
                        if (BehaviorA.bIsGroupLeader)
                        {
                            BehaviorB.TargetLocation = BehaviorA.TargetLocation + FVector(
                                FMath::RandRange(-200.0f, 200.0f),
                                FMath::RandRange(-200.0f, 200.0f),
                                0.0f
                            );
                        }
                    }
                    else
                    {
                        // Different tribe interaction - potential conflict
                        TribalA.SocialStatus = FMath::Max(0.0f, TribalA.SocialStatus - 0.2f);
                        TribalB.SocialStatus = FMath::Max(0.0f, TribalB.SocialStatus - 0.2f);
                    }
                }
            }
        }
    });
}

void UCrowd_MassEntityProcessor::UpdateBehaviorState(FCrowd_TribalFragment& TribalData, FCrowd_BehaviorFragment& BehaviorData, float DeltaTime)
{
    // Update behavior timer
    BehaviorData.BehaviorTimer += DeltaTime;
    
    // Update needs
    TribalData.Hunger = FMath::Max(0.0f, TribalData.Hunger - DeltaTime * 2.0f);
    TribalData.Energy = FMath::Max(0.0f, TribalData.Energy - DeltaTime * 1.5f);
    
    // Behavior state machine
    switch (BehaviorData.CurrentBehavior)
    {
        case 0: // Idle
            if (BehaviorData.BehaviorTimer > 5.0f)
            {
                // Choose next behavior based on needs
                if (TribalData.Hunger < 30.0f)
                {
                    BehaviorData.CurrentBehavior = 3; // Gather
                    BehaviorData.TargetLocation = TribalData.WorkLocation + FVector(
                        FMath::RandRange(-WorkLocationRadius, WorkLocationRadius),
                        FMath::RandRange(-WorkLocationRadius, WorkLocationRadius),
                        0.0f
                    );
                }
                else if (TribalData.Energy < 20.0f)
                {
                    BehaviorData.CurrentBehavior = 5; // Rest
                    BehaviorData.TargetLocation = TribalData.HomeLocation;
                }
                else if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
                {
                    BehaviorData.CurrentBehavior = 2; // Hunt
                    BehaviorData.TargetLocation = TribalData.WorkLocation + FVector(
                        FMath::RandRange(-1000.0f, 1000.0f),
                        FMath::RandRange(-1000.0f, 1000.0f),
                        0.0f
                    );
                }
                else
                {
                    BehaviorData.CurrentBehavior = 1; // Work
                    BehaviorData.TargetLocation = TribalData.WorkLocation;
                }
                BehaviorData.BehaviorTimer = 0.0f;
            }
            break;
            
        case 1: // Work
            if (BehaviorData.BehaviorTimer > 15.0f)
            {
                BehaviorData.CurrentBehavior = 0; // Return to idle
                BehaviorData.BehaviorTimer = 0.0f;
                TribalData.SocialStatus += 2.0f;
            }
            break;
            
        case 2: // Hunt
            if (BehaviorData.BehaviorTimer > 20.0f)
            {
                BehaviorData.CurrentBehavior = 0; // Return to idle
                BehaviorData.BehaviorTimer = 0.0f;
                TribalData.Hunger = FMath::Min(100.0f, TribalData.Hunger + 40.0f);
            }
            break;
            
        case 3: // Gather
            if (BehaviorData.BehaviorTimer > 12.0f)
            {
                BehaviorData.CurrentBehavior = 0; // Return to idle
                BehaviorData.BehaviorTimer = 0.0f;
                TribalData.Hunger = FMath::Min(100.0f, TribalData.Hunger + 25.0f);
            }
            break;
            
        case 4: // Social
            if (BehaviorData.BehaviorTimer > 8.0f)
            {
                BehaviorData.CurrentBehavior = 0; // Return to idle
                BehaviorData.BehaviorTimer = 0.0f;
            }
            break;
            
        case 5: // Rest
            if (BehaviorData.BehaviorTimer > 10.0f || TribalData.Energy > 80.0f)
            {
                BehaviorData.CurrentBehavior = 0; // Return to idle
                BehaviorData.BehaviorTimer = 0.0f;
            }
            TribalData.Energy = FMath::Min(100.0f, TribalData.Energy + DeltaTime * 8.0f);
            break;
    }
}