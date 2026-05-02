#include "Combat_TacticalBehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Combat_TacticalAIController.h"

UCombat_TacticalBehaviorTree::UCombat_TacticalBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update rate
    
    // Initialize behavior state
    CurrentBehaviorState = ECombat_BehaviorState::Patrol;
    LastKnownPlayerLocation = FVector::ZeroVector;
    AlertLevel = 0.0f;
    PackCoordinationRadius = 800.0f;
    AttackRange = 200.0f;
    FlankingDistance = 400.0f;
    
    // Initialize timers
    LastPlayerSightingTime = 0.0f;
    StateChangeTime = 0.0f;
    NextDecisionTime = 0.0f;
    
    bIsPackLeader = false;
    bIsInCombat = false;
    bCanSeePlayer = false;
    
    // Pack behavior settings
    PackMembers.Empty();
    PreferredPackSize = 3;
    MaxPackSize = 5;
}

void UCombat_TacticalBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<ACombat_TacticalAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Connected to AI Controller"));
        }
    }
    
    // Initialize behavior tree
    InitializeBehaviorTree();
    
    // Start with patrol behavior
    SetBehaviorState(ECombat_BehaviorState::Patrol);
}

void UCombat_TacticalBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!AIController || !GetOwner())
    {
        return;
    }
    
    // Update behavior tree logic
    UpdateBehaviorLogic(DeltaTime);
    
    // Update pack coordination if this is a pack member
    if (PackMembers.Num() > 0)
    {
        UpdatePackCoordination(DeltaTime);
    }
    
    // Update alert level based on player proximity
    UpdateAlertLevel(DeltaTime);
}

void UCombat_TacticalBehaviorTree::InitializeBehaviorTree()
{
    if (!AIController)
    {
        return;
    }
    
    // Set up blackboard keys
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        BlackboardComp->SetValueAsVector(TEXT("LastKnownPlayerLocation"), LastKnownPlayerLocation);
        BlackboardComp->SetValueAsFloat(TEXT("AlertLevel"), AlertLevel);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
        BlackboardComp->SetValueAsBool(TEXT("InCombat"), bIsInCombat);
        
        UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Blackboard initialized"));
    }
}

void UCombat_TacticalBehaviorTree::UpdateBehaviorLogic(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time for next decision
    if (CurrentTime < NextDecisionTime)
    {
        return;
    }
    
    // Update player detection
    UpdatePlayerDetection();
    
    // Execute current behavior state
    switch (CurrentBehaviorState)
    {
        case ECombat_BehaviorState::Patrol:
            ExecutePatrolBehavior();
            break;
            
        case ECombat_BehaviorState::Alert:
            ExecuteAlertBehavior();
            break;
            
        case ECombat_BehaviorState::Hunt:
            ExecuteHuntBehavior();
            break;
            
        case ECombat_BehaviorState::Attack:
            ExecuteAttackBehavior();
            break;
            
        case ECombat_BehaviorState::Flank:
            ExecuteFlankBehavior();
            break;
            
        case ECombat_BehaviorState::Retreat:
            ExecuteRetreatBehavior();
            break;
    }
    
    // Schedule next decision
    NextDecisionTime = CurrentTime + FMath::RandRange(0.2f, 0.5f);
}

void UCombat_TacticalBehaviorTree::UpdatePlayerDetection()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        bCanSeePlayer = false;
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(OwnerLocation, PlayerLocation);
    
    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(PlayerPawn);
    
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLocation + FVector(0, 0, 50), // Offset for eye level
        PlayerLocation + FVector(0, 0, 50),
        ECC_Visibility,
        QueryParams
    );
    
    // Update sight status
    bCanSeePlayer = bHasLineOfSight && DistanceToPlayer <= GetSightRange();
    
    if (bCanSeePlayer)
    {
        LastKnownPlayerLocation = PlayerLocation;
        LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
        
        // Increase alert level when player is spotted
        AlertLevel = FMath::Min(AlertLevel + 0.1f, 1.0f);
    }
}

void UCombat_TacticalBehaviorTree::ExecutePatrolBehavior()
{
    if (bCanSeePlayer)
    {
        SetBehaviorState(ECombat_BehaviorState::Alert);
        return;
    }
    
    // Simple patrol logic - move to random nearby location
    if (!AIController)
    {
        return;
    }
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector PatrolTarget = CurrentLocation + FVector(
        FMath::RandRange(-500.0f, 500.0f),
        FMath::RandRange(-500.0f, 500.0f),
        0.0f
    );
    
    // Move to patrol point
    AIController->MoveToLocation(PatrolTarget, 100.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("TacticalBehaviorTree: Executing patrol behavior"));
}

void UCombat_TacticalBehaviorTree::ExecuteAlertBehavior()
{
    if (!bCanSeePlayer)
    {
        // Lost sight of player, investigate last known location
        if (FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation) > 100.0f)
        {
            AIController->MoveToLocation(LastKnownPlayerLocation, 50.0f);
        }
        else
        {
            // Searched area, return to patrol
            SetBehaviorState(ECombat_BehaviorState::Patrol);
        }
        return;
    }
    
    // Player still visible, escalate to hunt
    SetBehaviorState(ECombat_BehaviorState::Hunt);
}

void UCombat_TacticalBehaviorTree::ExecuteHuntBehavior()
{
    if (!bCanSeePlayer)
    {
        SetBehaviorState(ECombat_BehaviorState::Alert);
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
    
    if (DistanceToPlayer <= AttackRange)
    {
        SetBehaviorState(ECombat_BehaviorState::Attack);
        return;
    }
    
    // Coordinate with pack if available
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        CoordinatePackAttack();
    }
    else if (!bIsPackLeader && PackMembers.Num() > 0)
    {
        // Follow pack leader's commands
        FollowPackLeader();
    }
    else
    {
        // Solo hunt - move towards player
        AIController->MoveToLocation(LastKnownPlayerLocation, AttackRange * 0.8f);
    }
}

void UCombat_TacticalBehaviorTree::ExecuteAttackBehavior()
{
    if (!bCanSeePlayer)
    {
        SetBehaviorState(ECombat_BehaviorState::Hunt);
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
    
    if (DistanceToPlayer > AttackRange * 1.5f)
    {
        SetBehaviorState(ECombat_BehaviorState::Hunt);
        return;
    }
    
    // Execute attack
    PerformAttack();
    
    UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Executing attack behavior"));
}

void UCombat_TacticalBehaviorTree::ExecuteFlankBehavior()
{
    if (!bCanSeePlayer)
    {
        SetBehaviorState(ECombat_BehaviorState::Alert);
        return;
    }
    
    // Calculate flanking position
    FVector PlayerLocation = LastKnownPlayerLocation;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToPlayer = (PlayerLocation - OwnerLocation).GetSafeNormal();
    
    // Move to flanking position (90 degrees from direct approach)
    FVector FlankDirection = FVector(-DirectionToPlayer.Y, DirectionToPlayer.X, 0.0f);
    FVector FlankPosition = PlayerLocation + (FlankDirection * FlankingDistance);
    
    AIController->MoveToLocation(FlankPosition, 100.0f);
    
    // Check if close enough to attack
    if (FVector::Dist(OwnerLocation, PlayerLocation) <= AttackRange)
    {
        SetBehaviorState(ECombat_BehaviorState::Attack);
    }
}

void UCombat_TacticalBehaviorTree::ExecuteRetreatBehavior()
{
    // Move away from player
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionAway = (OwnerLocation - LastKnownPlayerLocation).GetSafeNormal();
    FVector RetreatTarget = OwnerLocation + (DirectionAway * 800.0f);
    
    AIController->MoveToLocation(RetreatTarget, 100.0f);
    
    // Reduce alert level over time
    AlertLevel = FMath::Max(AlertLevel - 0.05f, 0.0f);
    
    // Return to patrol when far enough away
    if (FVector::Dist(OwnerLocation, LastKnownPlayerLocation) > 1000.0f)
    {
        SetBehaviorState(ECombat_BehaviorState::Patrol);
    }
}

void UCombat_TacticalBehaviorTree::SetBehaviorState(ECombat_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
    {
        return;
    }
    
    ECombat_BehaviorState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Update blackboard
    if (AIController && AIController->GetBlackboardComponent())
    {
        AIController->GetBlackboardComponent()->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
    }
    
    UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: State changed from %d to %d"), 
           static_cast<int32>(PreviousState), static_cast<int32>(NewState));
}

void UCombat_TacticalBehaviorTree::UpdatePackCoordination(float DeltaTime)
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Update pack member positions and states
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(PackMembers[i]))
        {
            PackMembers.RemoveAt(i);
            continue;
        }
        
        // Share information with pack members
        if (bCanSeePlayer && bIsPackLeader)
        {
            // Leader shares player location with pack
            PackMembers[i]->ReceivePackCommand(ECombat_PackCommand::AttackTarget, LastKnownPlayerLocation);
        }
    }
}

void UCombat_TacticalBehaviorTree::UpdateAlertLevel(float DeltaTime)
{
    float TimeSinceLastSighting = GetWorld()->GetTimeSeconds() - LastPlayerSightingTime;
    
    if (!bCanSeePlayer && TimeSinceLastSighting > 5.0f)
    {
        // Gradually reduce alert level when player not seen
        AlertLevel = FMath::Max(AlertLevel - DeltaTime * 0.1f, 0.0f);
    }
    
    // Update blackboard
    if (AIController && AIController->GetBlackboardComponent())
    {
        AIController->GetBlackboardComponent()->SetValueAsFloat(TEXT("AlertLevel"), AlertLevel);
    }
}

void UCombat_TacticalBehaviorTree::CoordinatePackAttack()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (!IsValid(PackMembers[i]))
        {
            continue;
        }
        
        if (i == 0)
        {
            // First member flanks left
            PackMembers[i]->ReceivePackCommand(ECombat_PackCommand::FlankLeft, LastKnownPlayerLocation);
        }
        else if (i == 1)
        {
            // Second member flanks right
            PackMembers[i]->ReceivePackCommand(ECombat_PackCommand::FlankRight, LastKnownPlayerLocation);
        }
        else
        {
            // Additional members provide support
            PackMembers[i]->ReceivePackCommand(ECombat_PackCommand::Support, LastKnownPlayerLocation);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Coordinating pack attack with %d members"), PackMembers.Num());
}

void UCombat_TacticalBehaviorTree::FollowPackLeader()
{
    // Find pack leader
    UCombat_TacticalBehaviorTree* PackLeader = nullptr;
    
    for (UCombat_TacticalBehaviorTree* Member : PackMembers)
    {
        if (IsValid(Member) && Member->bIsPackLeader)
        {
            PackLeader = Member;
            break;
        }
    }
    
    if (!PackLeader)
    {
        // No leader found, become independent
        bIsPackLeader = true;
        return;
    }
    
    // Follow leader's target
    LastKnownPlayerLocation = PackLeader->LastKnownPlayerLocation;
    
    // Maintain formation relative to leader
    FVector LeaderLocation = PackLeader->GetOwner()->GetActorLocation();
    FVector FormationOffset = FVector(200.0f, 200.0f, 0.0f); // Simple offset
    FVector TargetLocation = LeaderLocation + FormationOffset;
    
    AIController->MoveToLocation(TargetLocation, 100.0f);
}

void UCombat_TacticalBehaviorTree::PerformAttack()
{
    // Basic attack implementation
    bIsInCombat = true;
    
    // Update blackboard
    if (AIController && AIController->GetBlackboardComponent())
    {
        AIController->GetBlackboardComponent()->SetValueAsBool(TEXT("InCombat"), bIsInCombat);
    }
    
    // Trigger attack animation/behavior
    UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Performing attack on player"));
    
    // Reset attack cooldown
    NextDecisionTime = GetWorld()->GetTimeSeconds() + 1.0f;
}

void UCombat_TacticalBehaviorTree::ReceivePackCommand(ECombat_PackCommand Command, const FVector& TargetLocation)
{
    switch (Command)
    {
        case ECombat_PackCommand::AttackTarget:
            LastKnownPlayerLocation = TargetLocation;
            SetBehaviorState(ECombat_BehaviorState::Hunt);
            break;
            
        case ECombat_PackCommand::FlankLeft:
            LastKnownPlayerLocation = TargetLocation;
            SetBehaviorState(ECombat_BehaviorState::Flank);
            break;
            
        case ECombat_PackCommand::FlankRight:
            LastKnownPlayerLocation = TargetLocation;
            SetBehaviorState(ECombat_BehaviorState::Flank);
            break;
            
        case ECombat_PackCommand::Support:
            LastKnownPlayerLocation = TargetLocation;
            SetBehaviorState(ECombat_BehaviorState::Hunt);
            break;
            
        case ECombat_PackCommand::Retreat:
            SetBehaviorState(ECombat_BehaviorState::Retreat);
            break;
    }
}

void UCombat_TacticalBehaviorTree::AddPackMember(UCombat_TacticalBehaviorTree* NewMember)
{
    if (!IsValid(NewMember) || PackMembers.Contains(NewMember))
    {
        return;
    }
    
    if (PackMembers.Num() < MaxPackSize)
    {
        PackMembers.Add(NewMember);
        NewMember->PackMembers = PackMembers; // Share pack list
        
        // First member becomes leader if no leader exists
        if (!bIsPackLeader && PackMembers.Num() == 1)
        {
            bIsPackLeader = true;
        }
        
        UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Added pack member. Pack size: %d"), PackMembers.Num());
    }
}

void UCombat_TacticalBehaviorTree::RemovePackMember(UCombat_TacticalBehaviorTree* Member)
{
    if (PackMembers.Contains(Member))
    {
        PackMembers.Remove(Member);
        
        // If leader left, assign new leader
        if (Member->bIsPackLeader && PackMembers.Num() > 0)
        {
            PackMembers[0]->bIsPackLeader = true;
        }
        
        UE_LOG(LogTemp, Log, TEXT("TacticalBehaviorTree: Removed pack member. Pack size: %d"), PackMembers.Num());
    }
}

float UCombat_TacticalBehaviorTree::GetSightRange() const
{
    // Base sight range modified by alert level
    float BaseSightRange = 600.0f;
    return BaseSightRange * (1.0f + AlertLevel * 0.5f);
}

bool UCombat_TacticalBehaviorTree::IsInAttackRange() const
{
    if (LastKnownPlayerLocation == FVector::ZeroVector)
    {
        return false;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
    return DistanceToPlayer <= AttackRange;
}

ECombat_BehaviorState UCombat_TacticalBehaviorTree::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}