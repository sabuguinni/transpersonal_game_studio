#include "MassCrowdIntelligenceV43.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UMassCrowdIntelligenceProcessorV43::UMassCrowdIntelligenceProcessorV43()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    
    bAutoRegisterWithProcessingPhases = true;
}

void UMassCrowdIntelligenceProcessorV43::ConfigureQueries()
{
    // Main crowd intelligence query
    CrowdIntelligenceQuery.AddRequirement<FMassCrowdIntelligenceFragment>(EMassFragmentAccess::ReadWrite);
    CrowdIntelligenceQuery.AddRequirement<FMassCrowdEnvironmentFragment>(EMassFragmentAccess::ReadWrite);
    CrowdIntelligenceQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    CrowdIntelligenceQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    CrowdIntelligenceQuery.AddOptionalRequirement<FMassNavigationEdgesFragment>(EMassFragmentAccess::ReadOnly);
    
    // Group dynamics query
    GroupDynamicsQuery.AddRequirement<FMassCrowdIntelligenceFragment>(EMassFragmentAccess::ReadWrite);
    GroupDynamicsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    GroupDynamicsQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly);
    
    // Environment awareness query
    EnvironmentAwarenessQuery.AddRequirement<FMassCrowdEnvironmentFragment>(EMassFragmentAccess::ReadWrite);
    EnvironmentAwarenessQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EnvironmentAwarenessQuery.AddRequirement<FMassCrowdIntelligenceFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassCrowdIntelligenceProcessorV43::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_MassCrowdIntelligenceV43_Execute);
    
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Throttle updates for performance
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    LastUpdateTime = CurrentTime;
    
    // Process crowd intelligence in phases
    ProcessCrowdIntelligence(EntityManager, Context);
    UpdateCrowdStates(EntityManager, Context);
    
    if (bEnableAdvancedBehaviors)
    {
        ProcessGroupDynamics(EntityManager, Context);
    }
    
    if (bEnableEmergentBehaviors)
    {
        HandleEmergentBehaviors(EntityManager, Context);
    }
}

void UMassCrowdIntelligenceProcessorV43::ProcessCrowdIntelligence(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    const UWorld* World = Context.GetWorld();
    
    // Get player location for awareness calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (const APlayerController* PlayerController = World->GetFirstPlayerController())
    {
        if (const APawn* PlayerPawn = PlayerController->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    CrowdIntelligenceQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const int32 NumEntities = QueryContext.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = QueryContext.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassCrowdIntelligenceFragment> IntelligenceList = QueryContext.GetMutableFragmentView<FMassCrowdIntelligenceFragment>();
        const TArrayView<FMassCrowdEnvironmentFragment> EnvironmentList = QueryContext.GetMutableFragmentView<FMassCrowdEnvironmentFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = QueryContext.GetMutableFragmentView<FMassVelocityFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassCrowdIntelligenceFragment& Intelligence = IntelligenceList[EntityIndex];
            FMassCrowdEnvironmentFragment& Environment = EnvironmentList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            
            // Update environmental awareness
            Environment.DistanceToPlayer = FVector::Dist(Transform.GetTransform().GetLocation(), PlayerLocation);
            Environment.bCanSeePlayer = Environment.DistanceToPlayer < 1500.0f && Environment.VisibilityLevel > 0.3f;
            Environment.bCanHearPlayer = Environment.DistanceToPlayer < 800.0f && Environment.NoiseLevel > 0.2f;
            
            // Update individual state
            UpdateIndividualState(Intelligence, Environment, Transform, DeltaTime);
            
            // Process state transitions
            ProcessStateTransitions(Intelligence, Environment, DeltaTime);
            
            // Apply behavioral complexity modifiers
            switch (Intelligence.BehaviorComplexity)
            {
                case ECrowdBehaviorComplexity::Simple:
                    Intelligence.ReactionTime = FMath::Max(Intelligence.ReactionTime, 1.0f);
                    break;
                    
                case ECrowdBehaviorComplexity::Moderate:
                    Intelligence.ReactionTime = FMath::Clamp(Intelligence.ReactionTime, 0.5f, 1.5f);
                    break;
                    
                case ECrowdBehaviorComplexity::Advanced:
                    Intelligence.ReactionTime = FMath::Clamp(Intelligence.ReactionTime, 0.2f, 1.0f);
                    Intelligence.MemoryDuration = FMath::Max(Intelligence.MemoryDuration, 60.0f);
                    break;
                    
                case ECrowdBehaviorComplexity::Elite:
                    Intelligence.ReactionTime = FMath::Clamp(Intelligence.ReactionTime, 0.1f, 0.8f);
                    Intelligence.MemoryDuration = FMath::Max(Intelligence.MemoryDuration, 300.0f);
                    break;
            }
            
            // Update velocity based on current state
            float SpeedMultiplier = 1.0f;
            switch (Intelligence.CurrentState)
            {
                case EMassCrowdIntelligenceState::FleeingPredator:
                case EMassCrowdIntelligenceState::FleeingPlayer:
                case EMassCrowdIntelligenceState::Stampeding:
                    SpeedMultiplier = 1.5f;
                    Intelligence.StressLevel = FMath::Min(Intelligence.StressLevel + DeltaTime * 0.5f, 1.0f);
                    break;
                    
                case EMassCrowdIntelligenceState::Grazing:
                case EMassCrowdIntelligenceState::Resting:
                    SpeedMultiplier = 0.3f;
                    Intelligence.StressLevel = FMath::Max(Intelligence.StressLevel - DeltaTime * 0.2f, 0.0f);
                    break;
                    
                case EMassCrowdIntelligenceState::Hunting:
                    SpeedMultiplier = 1.2f;
                    Intelligence.AlertLevel = FMath::Min(Intelligence.AlertLevel + DeltaTime * 0.3f, 1.0f);
                    break;
                    
                case EMassCrowdIntelligenceState::Migration:
                    SpeedMultiplier = 0.8f;
                    break;
                    
                default:
                    SpeedMultiplier = 0.6f;
                    Intelligence.StressLevel = FMath::Max(Intelligence.StressLevel - DeltaTime * 0.1f, 0.0f);
                    break;
            }
            
            // Apply speed modifications
            const float TargetSpeed = Intelligence.PreferredSpeed * SpeedMultiplier;
            const float CurrentSpeed = Velocity.Value.Size();
            
            if (FMath::Abs(CurrentSpeed - TargetSpeed) > 50.0f)
            {
                const FVector Direction = Velocity.Value.GetSafeNormal();
                Velocity.Value = Direction * FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, 2.0f);
            }
        }
    });
}

void UMassCrowdIntelligenceProcessorV43::UpdateCrowdStates(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    CrowdIntelligenceQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const TArrayView<FMassCrowdIntelligenceFragment> IntelligenceList = QueryContext.GetMutableFragmentView<FMassCrowdIntelligenceFragment>();
        const TConstArrayView<FMassCrowdEnvironmentFragment> EnvironmentList = QueryContext.GetFragmentView<FMassCrowdEnvironmentFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
        {
            FMassCrowdIntelligenceFragment& Intelligence = IntelligenceList[EntityIndex];
            const FMassCrowdEnvironmentFragment& Environment = EnvironmentList[EntityIndex];
            
            // Update biological needs
            Intelligence.EnergyLevel = FMath::Max(Intelligence.EnergyLevel - DeltaTime * 0.01f, 0.0f);
            Intelligence.HungerLevel = FMath::Min(Intelligence.HungerLevel + DeltaTime * 0.02f, 1.0f);
            Intelligence.ThirstLevel = FMath::Min(Intelligence.ThirstLevel + DeltaTime * 0.015f, 1.0f);
            
            // Reduce thirst when near water
            if (Environment.bIsInWater || Environment.DistanceToWater < 100.0f)
            {
                Intelligence.ThirstLevel = FMath::Max(Intelligence.ThirstLevel - DeltaTime * 0.5f, 0.0f);
            }
            
            // Reduce hunger when in feeding areas
            if (Environment.VegetationDensity > 0.7f && Intelligence.CurrentState == EMassCrowdIntelligenceState::Grazing)
            {
                Intelligence.HungerLevel = FMath::Max(Intelligence.HungerLevel - DeltaTime * 0.3f, 0.0f);
            }
            
            // Update state timer
            Intelligence.StateTimer += DeltaTime;
            
            // Memory decay
            if (Intelligence.LastThreatTime > 0.0f)
            {
                const float TimeSinceThreat = Context.GetWorld()->GetTimeSeconds() - Intelligence.LastThreatTime;
                if (TimeSinceThreat > Intelligence.MemoryDuration)
                {
                    Intelligence.LastThreatLocation = FVector::ZeroVector;
                    Intelligence.LastThreatTime = 0.0f;
                    Intelligence.bHasMemoryOfPlayer = false;
                }
            }
            
            // Alert level decay
            Intelligence.AlertLevel = FMath::Max(Intelligence.AlertLevel - DeltaTime * 0.1f, 0.0f);
            
            // Social influence updates
            if (Environment.NearbyEntitiesCount > 0)
            {
                Intelligence.SocialInfluence = FMath::Min(Intelligence.SocialInfluence + DeltaTime * 0.2f, 1.0f);
            }
            else
            {
                Intelligence.SocialInfluence = FMath::Max(Intelligence.SocialInfluence - DeltaTime * 0.1f, 0.0f);
            }
        }
    });
}

void UMassCrowdIntelligenceProcessorV43::ProcessGroupDynamics(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Clear previous group data
    GroupEntities.Reset();
    
    // Collect entities by group
    GroupDynamicsQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const TConstArrayView<FMassCrowdIntelligenceFragment> IntelligenceList = QueryContext.GetFragmentView<FMassCrowdIntelligenceFragment>();
        const TConstArrayView<FMassEntityHandle> EntityHandles = QueryContext.GetEntities();
        
        for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
        {
            const FMassCrowdIntelligenceFragment& Intelligence = IntelligenceList[EntityIndex];
            const FMassEntityHandle EntityHandle = EntityHandles[EntityIndex];
            
            if (Intelligence.GroupID != INDEX_NONE)
            {
                GroupEntities.FindOrAdd(Intelligence.GroupID).Add(EntityHandle);
            }
        }
    });
    
    // Process each group
    for (auto& GroupPair : GroupEntities)
    {
        const int32 GroupID = GroupPair.Key;
        TArray<FMassEntityHandle>& GroupMembers = GroupPair.Value;
        
        if (GroupMembers.Num() < 2)
        {
            continue;
        }
        
        // Find or elect group leader
        FMassEntityHandle LeaderHandle;
        bool bHasLeader = false;
        
        for (const FMassEntityHandle& MemberHandle : GroupMembers)
        {
            if (EntityManager.IsEntityValid(MemberHandle))
            {
                FMassCrowdIntelligenceFragment& Intelligence = EntityManager.GetFragmentDataChecked<FMassCrowdIntelligenceFragment>(MemberHandle);
                
                if (Intelligence.bIsGroupLeader)
                {
                    LeaderHandle = MemberHandle;
                    bHasLeader = true;
                    break;
                }
            }
        }
        
        // Elect new leader if needed
        if (!bHasLeader && GroupMembers.Num() > 0)
        {
            // Find most suitable leader (highest energy + lowest stress)
            float BestLeadershipScore = -1.0f;
            
            for (const FMassEntityHandle& MemberHandle : GroupMembers)
            {
                if (EntityManager.IsEntityValid(MemberHandle))
                {
                    FMassCrowdIntelligenceFragment& Intelligence = EntityManager.GetFragmentDataChecked<FMassCrowdIntelligenceFragment>(MemberHandle);
                    
                    if (Intelligence.bCanBecomeLeader)
                    {
                        const float LeadershipScore = Intelligence.EnergyLevel + (1.0f - Intelligence.StressLevel) + Intelligence.IndividualWill;
                        
                        if (LeadershipScore > BestLeadershipScore)
                        {
                            BestLeadershipScore = LeadershipScore;
                            LeaderHandle = MemberHandle;
                        }
                    }
                }
            }
            
            // Assign leadership
            if (EntityManager.IsEntityValid(LeaderHandle))
            {
                FMassCrowdIntelligenceFragment& LeaderIntelligence = EntityManager.GetFragmentDataChecked<FMassCrowdIntelligenceFragment>(LeaderHandle);
                LeaderIntelligence.bIsGroupLeader = true;
                bHasLeader = true;
            }
        }
        
        // Update group cohesion and follower behavior
        if (bHasLeader && EntityManager.IsEntityValid(LeaderHandle))
        {
            const FTransformFragment& LeaderTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(LeaderHandle);
            const FVector LeaderLocation = LeaderTransform.GetTransform().GetLocation();
            
            for (const FMassEntityHandle& MemberHandle : GroupMembers)
            {
                if (EntityManager.IsEntityValid(MemberHandle) && MemberHandle != LeaderHandle)
                {
                    FMassCrowdIntelligenceFragment& Intelligence = EntityManager.GetFragmentDataChecked<FMassCrowdIntelligenceFragment>(MemberHandle);
                    const FTransformFragment& MemberTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(MemberHandle);
                    
                    const float DistanceToLeader = FVector::Dist(MemberTransform.GetTransform().GetLocation(), LeaderLocation);
                    
                    // Update group cohesion based on distance to leader
                    if (DistanceToLeader < Intelligence.FollowDistance)
                    {
                        Intelligence.GroupCohesion = FMath::Min(Intelligence.GroupCohesion + Context.GetDeltaTimeSeconds() * 0.3f, 1.0f);
                    }
                    else
                    {
                        Intelligence.GroupCohesion = FMath::Max(Intelligence.GroupCohesion - Context.GetDeltaTimeSeconds() * 0.2f, 0.0f);
                    }
                    
                    // Set leader reference
                    Intelligence.LeaderEntityHandle = LeaderHandle;
                    Intelligence.bIsGroupLeader = false;
                }
            }
        }
    }
}

void UMassCrowdIntelligenceProcessorV43::HandleEmergentBehaviors(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    CrowdIntelligenceQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const TArrayView<FMassCrowdIntelligenceFragment> IntelligenceList = QueryContext.GetMutableFragmentView<FMassCrowdIntelligenceFragment>();
        const TConstArrayView<FMassCrowdEnvironmentFragment> EnvironmentList = QueryContext.GetFragmentView<FMassCrowdEnvironmentFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
        {
            FMassCrowdIntelligenceFragment& Intelligence = IntelligenceList[EntityIndex];
            const FMassCrowdEnvironmentFragment& Environment = EnvironmentList[EntityIndex];
            
            // Detect stampede conditions
            if (DetectStampedeConditions(Intelligence, Environment))
            {
                Intelligence.CurrentState = EMassCrowdIntelligenceState::Stampeding;
                Intelligence.bIsInPanic = true;
                Intelligence.StressLevel = 1.0f;
                Intelligence.AlertLevel = 1.0f;
                
                // Spread panic to nearby entities
                // This would be implemented with spatial queries in a full system
            }
            
            // Detect flocking behavior
            if (DetectFlockingBehavior(Intelligence, Environment))
            {
                Intelligence.CurrentState = EMassCrowdIntelligenceState::Flocking;
                Intelligence.GroupCohesion = FMath::Min(Intelligence.GroupCohesion + 0.5f, 1.0f);
            }
            
            // Detect territorial behavior
            if (Intelligence.CurrentState == EMassCrowdIntelligenceState::Territorial)
            {
                Intelligence.IndividualWill = FMath::Min(Intelligence.IndividualWill + Context.GetDeltaTimeSeconds() * 0.2f, 1.0f);
                Intelligence.SocialInfluence = FMath::Max(Intelligence.SocialInfluence - Context.GetDeltaTimeSeconds() * 0.1f, 0.0f);
            }
        }
    });
}

void UMassCrowdIntelligenceProcessorV43::UpdateIndividualState(FMassCrowdIntelligenceFragment& Intelligence, 
                                                              const FMassCrowdEnvironmentFragment& Environment,
                                                              const FTransformFragment& Transform,
                                                              float DeltaTime)
{
    // Update safety level based on environment
    float SafetyScore = 1.0f;
    
    if (Environment.NearbyPredatorsCount > 0)
    {
        SafetyScore -= 0.5f * Environment.NearbyPredatorsCount;
    }
    
    if (Environment.bCanSeePlayer || Environment.bCanHearPlayer)
    {
        SafetyScore -= 0.3f;
    }
    
    if (Environment.bIsInShelter)
    {
        SafetyScore += 0.4f;
    }
    
    if (Environment.VegetationDensity > 0.7f)
    {
        SafetyScore += 0.2f;
    }
    
    Intelligence.AlertLevel = FMath::Clamp(1.0f - SafetyScore, 0.0f, 1.0f);
}

void UMassCrowdIntelligenceProcessorV43::ProcessStateTransitions(FMassCrowdIntelligenceFragment& Intelligence,
                                                                const FMassCrowdEnvironmentFragment& Environment,
                                                                float DeltaTime)
{
    const EMassCrowdIntelligenceState PreviousState = Intelligence.CurrentState;
    
    // High priority state transitions (threats)
    if (Environment.NearbyPredatorsCount > 0 && Intelligence.AlertLevel > 0.7f)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::FleeingPredator;
        Intelligence.StateTimer = 0.0f;
        return;
    }
    
    if ((Environment.bCanSeePlayer || Environment.bCanHearPlayer) && Intelligence.AlertLevel > 0.5f)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::FleeingPlayer;
        Intelligence.StateTimer = 0.0f;
        Intelligence.bHasMemoryOfPlayer = true;
        return;
    }
    
    // Medium priority transitions (needs)
    if (Intelligence.ThirstLevel > 0.8f && Environment.DistanceToWater < 500.0f)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::Migration; // Moving to water
        Intelligence.StateTimer = 0.0f;
        return;
    }
    
    if (Intelligence.HungerLevel > 0.7f && Environment.VegetationDensity > 0.5f)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::Grazing;
        Intelligence.StateTimer = 0.0f;
        return;
    }
    
    // Low priority transitions (comfort states)
    if (Intelligence.EnergyLevel < 0.3f && Environment.bIsInShelter)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::Resting;
        Intelligence.StateTimer = 0.0f;
        return;
    }
    
    // Social state transitions
    if (Environment.NearbyEntitiesCount > 3 && Intelligence.SocialInfluence > 0.6f)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::Socializing;
        Intelligence.StateTimer = 0.0f;
        return;
    }
    
    // Default to idle if no specific conditions are met
    if (Intelligence.StateTimer > 10.0f && Intelligence.CurrentState != EMassCrowdIntelligenceState::Idle)
    {
        Intelligence.PreviousState = Intelligence.CurrentState;
        Intelligence.CurrentState = EMassCrowdIntelligenceState::Idle;
        Intelligence.StateTimer = 0.0f;
    }
}

void UMassCrowdIntelligenceProcessorV43::UpdateGroupCohesion(FMassCrowdIntelligenceFragment& Intelligence,
                                                            const FMassCrowdEnvironmentFragment& Environment,
                                                            const TArrayView<FMassEntityHandle>& NearbyEntities)
{
    // Implementation would use spatial queries to find nearby entities
    // For now, use the environment data
    
    if (Environment.NearbyEntitiesCount > 0)
    {
        Intelligence.GroupCohesion = FMath::Min(Intelligence.GroupCohesion + 0.1f, 1.0f);
    }
    else
    {
        Intelligence.GroupCohesion = FMath::Max(Intelligence.GroupCohesion - 0.05f, 0.0f);
    }
}

void UMassCrowdIntelligenceProcessorV43::ProcessLeadershipDynamics(FMassCrowdIntelligenceFragment& Intelligence,
                                                                  const FMassCrowdEnvironmentFragment& Environment)
{
    if (Intelligence.bIsGroupLeader)
    {
        // Leaders have higher stress but more influence
        Intelligence.StressLevel = FMath::Min(Intelligence.StressLevel + 0.02f, 1.0f);
        Intelligence.SocialInfluence = FMath::Min(Intelligence.SocialInfluence + 0.1f, 1.0f);
    }
}

void UMassCrowdIntelligenceProcessorV43::UpdateEnvironmentalAwareness(FMassCrowdEnvironmentFragment& Environment,
                                                                     const FTransformFragment& Transform,
                                                                     const FMassEntityHandle& EntityHandle)
{
    // This would be implemented with spatial queries and environmental sampling
    // For now, provide basic updates
    
    const FVector Location = Transform.GetTransform().GetLocation();
    
    // Simulate environmental factors
    Environment.TerrainSlope = FMath::RandRange(0.0f, 45.0f);
    Environment.VegetationDensity = FMath::RandRange(0.0f, 1.0f);
    Environment.NoiseLevel = FMath::RandRange(0.0f, 0.5f);
    Environment.VisibilityLevel = FMath::RandRange(0.3f, 1.0f);
    
    // Update safety level based on multiple factors
    Environment.SafetyLevel = (Environment.VegetationDensity + (1.0f - Environment.NoiseLevel)) * 0.5f;
    
    if (Environment.bIsInShelter)
    {
        Environment.SafetyLevel = FMath::Min(Environment.SafetyLevel + 0.3f, 1.0f);
    }
    
    if (Environment.NearbyPredatorsCount > 0)
    {
        Environment.SafetyLevel = FMath::Max(Environment.SafetyLevel - 0.5f, 0.0f);
    }
}

bool UMassCrowdIntelligenceProcessorV43::DetectStampedeConditions(const FMassCrowdIntelligenceFragment& Intelligence,
                                                                 const FMassCrowdEnvironmentFragment& Environment)
{
    return (Intelligence.StressLevel > 0.8f && 
            Intelligence.AlertLevel > 0.9f && 
            Environment.NearbyEntitiesCount > 5 &&
            (Environment.NearbyPredatorsCount > 0 || Environment.bCanSeePlayer));
}

bool UMassCrowdIntelligenceProcessorV43::DetectFlockingBehavior(const FMassCrowdIntelligenceFragment& Intelligence,
                                                               const FMassCrowdEnvironmentFragment& Environment)
{
    return (Intelligence.GroupCohesion > 0.7f && 
            Environment.NearbyEntitiesCount >= 3 && 
            Intelligence.SocialInfluence > 0.6f &&
            Intelligence.StressLevel < 0.4f);
}

// Observer Processor Implementation
UMassCrowdIntelligenceObserverV43::UMassCrowdIntelligenceObserverV43()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ObservedType = FMassCrowdIntelligenceFragment::StaticStruct();
    Operation = EMassObservedOperation::Add;
}

void UMassCrowdIntelligenceObserverV43::ConfigureQueries()
{
    StateChangeQuery.AddRequirement<FMassCrowdIntelligenceFragment>(EMassFragmentAccess::ReadOnly);
    StateChangeQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassCrowdIntelligenceObserverV43::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    StateChangeQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const TConstArrayView<FMassCrowdIntelligenceFragment> IntelligenceList = QueryContext.GetFragmentView<FMassCrowdIntelligenceFragment>();
        
        for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
        {
            const FMassCrowdIntelligenceFragment& Intelligence = IntelligenceList[EntityIndex];
            
            // Log state changes for debugging
            if (Intelligence.CurrentState != Intelligence.PreviousState)
            {
                UE_LOG(LogTemp, Log, TEXT("Crowd entity state changed from %d to %d"), 
                       (int32)Intelligence.PreviousState, (int32)Intelligence.CurrentState);
            }
        }
    });
}