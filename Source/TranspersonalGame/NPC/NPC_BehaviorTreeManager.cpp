#include "NPC_BehaviorTreeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    SetupDefaultBehaviorRules();
    InitializeSpeciesBehaviorTrees();
}

void UNPC_BehaviorTreeManager::InitializeBehaviorTree(AAIController* AIController, ENPC_DinosaurSpecies Species)
{
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_BehaviorTreeManager: Invalid AIController"));
        return;
    }

    // Initialize behavior context
    FNPC_BehaviorContext NewContext;
    NewContext.Species = Species;
    NewContext.CurrentState = ENPC_DinosaurState::Idle;
    
    // Set species-specific defaults
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            NewContext.AggressionLevel = 0.8f;
            NewContext.TerritoryRadius = 8000.0f;
            NewContext.bIsPackLeader = true;
            break;
        case ENPC_DinosaurSpecies::Raptor:
            NewContext.AggressionLevel = 0.7f;
            NewContext.TerritoryRadius = 4000.0f;
            NewContext.bIsPackLeader = false;
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            NewContext.AggressionLevel = 0.2f;
            NewContext.TerritoryRadius = 6000.0f;
            NewContext.bIsPackLeader = false;
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            NewContext.AggressionLevel = 0.5f;
            NewContext.TerritoryRadius = 5000.0f;
            NewContext.bIsPackLeader = false;
            break;
        default:
            NewContext.AggressionLevel = 0.5f;
            NewContext.TerritoryRadius = 5000.0f;
            NewContext.bIsPackLeader = false;
            break;
    }

    // Set territory center to current location
    if (AIController->GetPawn())
    {
        NewContext.TerritoryCenter = AIController->GetPawn()->GetActorLocation();
    }

    BehaviorContexts.Add(AIController, NewContext);

    // Start behavior tree if available
    if (UBehaviorTree** FoundTree = SpeciesBehaviorTrees.Find(Species))
    {
        if (*FoundTree)
        {
            AIController->RunBehaviorTree(*FoundTree);
            UE_LOG(LogTemp, Log, TEXT("Started behavior tree for species %d"), (int32)Species);
        }
    }

    // Initialize blackboard values
    if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)NewContext.CurrentState);
        BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), NewContext.AggressionLevel);
        BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), NewContext.TerritoryCenter);
        BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), NewContext.TerritoryRadius);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), NewContext.bIsPackLeader);
    }
}

void UNPC_BehaviorTreeManager::UpdateBehaviorContext(AAIController* AIController, const FNPC_BehaviorContext& NewContext)
{
    if (!AIController)
    {
        return;
    }

    if (FNPC_BehaviorContext* ExistingContext = BehaviorContexts.Find(AIController))
    {
        *ExistingContext = NewContext;

        // Update blackboard
        if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)NewContext.CurrentState);
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), NewContext.AggressionLevel);
            BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), NewContext.FearLevel);
            BlackboardComp->SetValueAsFloat(TEXT("HungerLevel"), NewContext.HungerLevel);
            BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), NewContext.TerritoryCenter);
            BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), NewContext.TerritoryRadius);
            BlackboardComp->SetValueAsObject(TEXT("CurrentTarget"), NewContext.CurrentTarget);
            BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), NewContext.bIsPackLeader);
        }
    }
}

ENPC_DinosaurState UNPC_BehaviorTreeManager::EvaluateStateTransition(const FNPC_BehaviorContext& Context)
{
    // Find the best matching behavior rule
    FNPC_BehaviorRule BestRule = GetBestMatchingRule(Context);
    
    // Check if we should transition
    if (BestRule.TriggerState == Context.CurrentState)
    {
        // Additional checks based on context
        if (Context.CurrentTarget && Context.AggressionLevel > 0.6f)
        {
            return ENPC_DinosaurState::Chasing;
        }
        
        if (Context.HungerLevel > 0.7f)
        {
            return ENPC_DinosaurState::Hunting;
        }
        
        if (Context.FearLevel > 0.5f)
        {
            return ENPC_DinosaurState::Fleeing;
        }
        
        return BestRule.TargetState;
    }
    
    return Context.CurrentState;
}

void UNPC_BehaviorTreeManager::ExecuteStateChange(AAIController* AIController, ENPC_DinosaurState NewState)
{
    if (!AIController)
    {
        return;
    }

    FNPC_BehaviorContext* Context = BehaviorContexts.Find(AIController);
    if (!Context)
    {
        return;
    }

    ENPC_DinosaurState OldState = Context->CurrentState;
    Context->CurrentState = NewState;

    // Update blackboard
    if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)NewState);
    }

    // State-specific logic
    switch (NewState)
    {
        case ENPC_DinosaurState::Patrolling:
            {
                FVector PatrolPoint = GetRandomPatrolPoint(Context->TerritoryCenter, Context->TerritoryRadius);
                if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
                {
                    BlackboardComp->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoint);
                }
            }
            break;
        case ENPC_DinosaurState::Chasing:
            if (Context->CurrentTarget)
            {
                if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
                {
                    BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Context->CurrentTarget);
                }
            }
            break;
        case ENPC_DinosaurState::Hunting:
            // Look for prey in the area
            break;
        case ENPC_DinosaurState::Fleeing:
            // Find escape route
            break;
        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("State transition: %s -> %s"), 
           *GetBehaviorStateString(OldState), 
           *GetBehaviorStateString(NewState));
}

void UNPC_BehaviorTreeManager::RegisterPackMember(AActor* PackLeader, AActor* PackMember)
{
    if (!PackLeader || !PackMember)
    {
        return;
    }

    TArray<AActor*>& Pack = PackMemberships.FindOrAdd(PackLeader);
    Pack.AddUnique(PackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Registered pack member %s to leader %s"), 
           *PackMember->GetName(), *PackLeader->GetName());
}

void UNPC_BehaviorTreeManager::RemovePackMember(AActor* PackLeader, AActor* PackMember)
{
    if (!PackLeader || !PackMember)
    {
        return;
    }

    if (TArray<AActor*>* Pack = PackMemberships.Find(PackLeader))
    {
        Pack->Remove(PackMember);
        UE_LOG(LogTemp, Log, TEXT("Removed pack member %s from leader %s"), 
               *PackMember->GetName(), *PackLeader->GetName());
    }
}

TArray<AActor*> UNPC_BehaviorTreeManager::GetPackMembers(AActor* PackLeader)
{
    if (TArray<AActor*>* Pack = PackMemberships.Find(PackLeader))
    {
        return *Pack;
    }
    return TArray<AActor*>();
}

void UNPC_BehaviorTreeManager::CoordinatePackBehavior(AActor* PackLeader, ENPC_DinosaurState TargetState)
{
    TArray<AActor*> PackMembers = GetPackMembers(PackLeader);
    
    for (AActor* Member : PackMembers)
    {
        if (APawn* MemberPawn = Cast<APawn>(Member))
        {
            if (AAIController* MemberAI = Cast<AAIController>(MemberPawn->GetController()))
            {
                ExecuteStateChange(MemberAI, TargetState);
            }
        }
    }
}

bool UNPC_BehaviorTreeManager::IsWithinTerritory(const FVector& Position, const FVector& TerritoryCenter, float TerritoryRadius)
{
    float Distance = FVector::Dist(Position, TerritoryCenter);
    return Distance <= TerritoryRadius;
}

FVector UNPC_BehaviorTreeManager::GetRandomPatrolPoint(const FVector& TerritoryCenter, float TerritoryRadius)
{
    // Generate random point within territory
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return TerritoryCenter + RandomOffset;
}

void UNPC_BehaviorTreeManager::DefendTerritory(AAIController* AIController, AActor* Intruder)
{
    if (!AIController || !Intruder)
    {
        return;
    }

    FNPC_BehaviorContext* Context = BehaviorContexts.Find(AIController);
    if (!Context)
    {
        return;
    }

    // Set intruder as target and switch to aggressive state
    Context->CurrentTarget = Intruder;
    Context->AggressionLevel = FMath::Min(Context->AggressionLevel + 0.3f, 1.0f);
    
    ExecuteStateChange(AIController, ENPC_DinosaurState::Chasing);
    
    // Coordinate pack if this is a pack leader
    if (Context->bIsPackLeader)
    {
        CoordinatePackBehavior(AIController->GetPawn(), ENPC_DinosaurState::Chasing);
    }
}

void UNPC_BehaviorTreeManager::OnPlayerDetected(AAIController* AIController, AActor* Player, float Distance)
{
    if (!AIController || !Player)
    {
        return;
    }

    FNPC_BehaviorContext* Context = BehaviorContexts.Find(AIController);
    if (!Context)
    {
        return;
    }

    Context->LastPlayerInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Determine reaction based on species and distance
    if (ShouldAttackPlayer(*Context, Distance))
    {
        Context->CurrentTarget = Player;
        ExecuteStateChange(AIController, ENPC_DinosaurState::Chasing);
    }
    else if (Context->Species == ENPC_DinosaurSpecies::Brachiosaurus)
    {
        // Herbivores might flee
        if (Distance < 2000.0f)
        {
            Context->FearLevel += 0.2f;
            ExecuteStateChange(AIController, ENPC_DinosaurState::Fleeing);
        }
    }
}

void UNPC_BehaviorTreeManager::OnPlayerLost(AAIController* AIController, AActor* Player)
{
    if (!AIController)
    {
        return;
    }

    FNPC_BehaviorContext* Context = BehaviorContexts.Find(AIController);
    if (!Context)
    {
        return;
    }

    if (Context->CurrentTarget == Player)
    {
        Context->CurrentTarget = nullptr;
        ExecuteStateChange(AIController, ENPC_DinosaurState::Patrolling);
    }
}

bool UNPC_BehaviorTreeManager::ShouldAttackPlayer(const FNPC_BehaviorContext& Context, float PlayerDistance)
{
    // Carnivores are more likely to attack
    bool bIsCarnivore = (Context.Species == ENPC_DinosaurSpecies::TRex || 
                        Context.Species == ENPC_DinosaurSpecies::Raptor);
    
    if (!bIsCarnivore)
    {
        return false;
    }
    
    // Attack if player is close and aggression is high
    float AttackThreshold = 3000.0f;
    if (Context.Species == ENPC_DinosaurSpecies::TRex)
    {
        AttackThreshold = 4000.0f;
    }
    
    return (PlayerDistance < AttackThreshold && Context.AggressionLevel > 0.5f);
}

void UNPC_BehaviorTreeManager::AddBehaviorRule(const FNPC_BehaviorRule& Rule)
{
    BehaviorRules.Add(Rule);
}

void UNPC_BehaviorTreeManager::RemoveBehaviorRule(ENPC_DinosaurState TriggerState, ENPC_DinosaurState TargetState)
{
    BehaviorRules.RemoveAll([TriggerState, TargetState](const FNPC_BehaviorRule& Rule)
    {
        return Rule.TriggerState == TriggerState && Rule.TargetState == TargetState;
    });
}

FNPC_BehaviorRule UNPC_BehaviorTreeManager::GetBestMatchingRule(const FNPC_BehaviorContext& Context)
{
    FNPC_BehaviorRule BestRule;
    float BestPriority = -1.0f;
    
    for (const FNPC_BehaviorRule& Rule : BehaviorRules)
    {
        if (Rule.TriggerState == Context.CurrentState &&
            Context.AggressionLevel >= Rule.MinAggressionLevel &&
            Context.AggressionLevel <= Rule.MaxAggressionLevel &&
            Rule.Priority > BestPriority)
        {
            BestRule = Rule;
            BestPriority = Rule.Priority;
        }
    }
    
    return BestRule;
}

void UNPC_BehaviorTreeManager::DebugDrawBehaviorInfo(AActor* DinosaurActor, const FNPC_BehaviorContext& Context)
{
    if (!DinosaurActor || !GetWorld())
    {
        return;
    }

    FVector ActorLocation = DinosaurActor->GetActorLocation();
    
    // Draw territory circle
    DrawDebugCircle(GetWorld(), Context.TerritoryCenter, Context.TerritoryRadius, 
                   32, FColor::Blue, false, 1.0f, 0, 50.0f, FVector(0, 1, 0), FVector(1, 0, 0));
    
    // Draw state text
    FString StateText = FString::Printf(TEXT("State: %s\nAggression: %.2f\nFear: %.2f"), 
                                       *GetBehaviorStateString(Context.CurrentState),
                                       Context.AggressionLevel,
                                       Context.FearLevel);
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), StateText, 
                   nullptr, FColor::White, 1.0f);
}

FString UNPC_BehaviorTreeManager::GetBehaviorStateString(ENPC_DinosaurState State)
{
    switch (State)
    {
        case ENPC_DinosaurState::Idle: return TEXT("Idle");
        case ENPC_DinosaurState::Patrolling: return TEXT("Patrolling");
        case ENPC_DinosaurState::Hunting: return TEXT("Hunting");
        case ENPC_DinosaurState::Chasing: return TEXT("Chasing");
        case ENPC_DinosaurState::Fleeing: return TEXT("Fleeing");
        case ENPC_DinosaurState::Feeding: return TEXT("Feeding");
        case ENPC_DinosaurState::Resting: return TEXT("Resting");
        case ENPC_DinosaurState::Territorial: return TEXT("Territorial");
        default: return TEXT("Unknown");
    }
}

void UNPC_BehaviorTreeManager::SetupDefaultBehaviorRules()
{
    BehaviorRules.Empty();
    
    // Idle to Patrolling
    FNPC_BehaviorRule IdleToPatrol;
    IdleToPatrol.TriggerState = ENPC_DinosaurState::Idle;
    IdleToPatrol.TargetState = ENPC_DinosaurState::Patrolling;
    IdleToPatrol.Priority = 1.0f;
    BehaviorRules.Add(IdleToPatrol);
    
    // Patrolling to Hunting (when hungry)
    FNPC_BehaviorRule PatrolToHunt;
    PatrolToHunt.TriggerState = ENPC_DinosaurState::Patrolling;
    PatrolToHunt.TargetState = ENPC_DinosaurState::Hunting;
    PatrolToHunt.MinAggressionLevel = 0.4f;
    PatrolToHunt.Priority = 2.0f;
    BehaviorRules.Add(PatrolToHunt);
    
    // Any state to Chasing (when target detected)
    FNPC_BehaviorRule AnyToChase;
    AnyToChase.TriggerState = ENPC_DinosaurState::Patrolling;
    AnyToChase.TargetState = ENPC_DinosaurState::Chasing;
    AnyToChase.MinAggressionLevel = 0.6f;
    AnyToChase.Priority = 3.0f;
    BehaviorRules.Add(AnyToChase);
}

void UNPC_BehaviorTreeManager::InitializeSpeciesBehaviorTrees()
{
    // Behavior trees would be loaded from assets in a real implementation
    // For now, we'll leave this empty and rely on the AI controller setup
}

bool UNPC_BehaviorTreeManager::CheckLineOfSight(AActor* Observer, AActor* Target)
{
    if (!Observer || !Target || !GetWorld())
    {
        return false;
    }

    FVector Start = Observer->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Observer);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, 
                                                    ECC_Visibility, QueryParams);
    
    return !bHit;
}

float UNPC_BehaviorTreeManager::CalculateAggressionModifier(ENPC_DinosaurSpecies Species, const FNPC_BehaviorContext& Context)
{
    float Modifier = 1.0f;
    
    // Species-based modifiers
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Modifier = 1.5f;
            break;
        case ENPC_DinosaurSpecies::Raptor:
            Modifier = 1.2f;
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Modifier = 0.3f;
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            Modifier = 0.8f;
            break;
    }
    
    // Hunger increases aggression for carnivores
    if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Raptor)
    {
        Modifier += Context.HungerLevel * 0.5f;
    }
    
    return Modifier;
}

void UNPC_BehaviorTreeManager::UpdatePackCoordination(AActor* PackLeader)
{
    if (!PackLeader)
    {
        return;
    }

    TArray<AActor*> PackMembers = GetPackMembers(PackLeader);
    
    // Simple pack coordination - members follow leader's behavior
    if (APawn* LeaderPawn = Cast<APawn>(PackLeader))
    {
        if (AAIController* LeaderAI = Cast<AAIController>(LeaderPawn->GetController()))
        {
            if (FNPC_BehaviorContext* LeaderContext = BehaviorContexts.Find(LeaderAI))
            {
                for (AActor* Member : PackMembers)
                {
                    if (APawn* MemberPawn = Cast<APawn>(Member))
                    {
                        if (AAIController* MemberAI = Cast<AAIController>(MemberPawn->GetController()))
                        {
                            if (FNPC_BehaviorContext* MemberContext = BehaviorContexts.Find(MemberAI))
                            {
                                // Sync some behaviors
                                if (LeaderContext->CurrentState == ENPC_DinosaurState::Chasing)
                                {
                                    MemberContext->CurrentTarget = LeaderContext->CurrentTarget;
                                    ExecuteStateChange(MemberAI, ENPC_DinosaurState::Chasing);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}