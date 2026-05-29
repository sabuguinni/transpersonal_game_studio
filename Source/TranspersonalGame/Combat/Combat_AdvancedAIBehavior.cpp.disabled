#include "Combat_AdvancedAIBehavior.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCombat_AdvancedAIBehavior::UCombat_AdvancedAIBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    ThreatLevel = ECombat_ThreatLevel::Low;
    CombatState = ECombat_AIState::Patrol;
    DetectionRadius = 1500.0f;
    AttackRange = 300.0f;
    FleeHealthThreshold = 0.25f;
    GroupCoordinationRadius = 800.0f;
    
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    TimeSinceLastTargetSeen = 0.0f;
    
    // Pack behavior settings
    bIsPackLeader = false;
    PackSize = 1;
    PreferredFormation = ECombat_FormationType::Circle;
}

void UCombat_AdvancedAIBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        OwnerController = Cast<AAIController>(OwnerPawn->GetController());
    }
    
    // Initialize behavior state
    InitializeBehaviorState();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_AdvancedAIBehavior initialized for %s"), 
           OwnerPawn ? *OwnerPawn->GetName() : TEXT("Unknown"));
}

void UCombat_AdvancedAIBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !OwnerController)
    {
        return;
    }
    
    // Update timers
    TimeSinceLastTargetSeen += DeltaTime;
    
    // Main AI behavior update
    UpdateBehaviorState(DeltaTime);
    UpdateThreatAssessment();
    UpdatePackCoordination();
    ExecuteCurrentBehavior(DeltaTime);
}

void UCombat_AdvancedAIBehavior::InitializeBehaviorState()
{
    // Set initial patrol behavior
    CombatState = ECombat_AIState::Patrol;
    ThreatLevel = ECombat_ThreatLevel::Low;
    
    // Find nearby pack members
    DiscoverPackMembers();
    
    // Determine pack hierarchy
    if (PackMembers.Num() > 0)
    {
        bIsPackLeader = DeterminePackLeadership();
        PackSize = PackMembers.Num() + 1; // +1 for self
    }
}

void UCombat_AdvancedAIBehavior::UpdateBehaviorState(float DeltaTime)
{
    // Detect threats in environment
    AActor* NewTarget = DetectNearestThreat();
    
    if (NewTarget && NewTarget != CurrentTarget)
    {
        SetNewTarget(NewTarget);
    }
    
    // Update state based on current situation
    switch (CombatState)
    {
        case ECombat_AIState::Patrol:
            HandlePatrolState(DeltaTime);
            break;
            
        case ECombat_AIState::Investigating:
            HandleInvestigatingState(DeltaTime);
            break;
            
        case ECombat_AIState::Pursuing:
            HandlePursuingState(DeltaTime);
            break;
            
        case ECombat_AIState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
            
        case ECombat_AIState::Flanking:
            HandleFlankingState(DeltaTime);
            break;
            
        case ECombat_AIState::Retreating:
            HandleRetreatingState(DeltaTime);
            break;
    }
}

AActor* UCombat_AdvancedAIBehavior::DetectNearestThreat()
{
    if (!OwnerPawn)
    {
        return nullptr;
    }
    
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    AActor* NearestThreat = nullptr;
    float NearestDistance = DetectionRadius;
    
    // Find all pawns in detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == OwnerPawn)
        {
            continue;
        }
        
        APawn* TargetPawn = Cast<APawn>(Actor);
        if (!TargetPawn)
        {
            continue;
        }
        
        // Check if this is a player character
        bool bIsPlayerCharacter = TargetPawn->IsPlayerControlled();
        if (!bIsPlayerCharacter)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, TargetPawn->GetActorLocation());
        if (Distance < NearestDistance)
        {
            // Check line of sight
            if (HasLineOfSight(TargetPawn))
            {
                NearestThreat = TargetPawn;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestThreat;
}

bool UCombat_AdvancedAIBehavior::HasLineOfSight(AActor* Target)
{
    if (!OwnerPawn || !Target)
    {
        return false;
    }
    
    FVector Start = OwnerPawn->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No obstruction means clear line of sight
}

void UCombat_AdvancedAIBehavior::SetNewTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        TimeSinceLastTargetSeen = 0.0f;
        
        // Transition to appropriate combat state
        TransitionToCombatState();
        
        // Notify pack members
        NotifyPackOfTarget(CurrentTarget);
        
        UE_LOG(LogTemp, Log, TEXT("%s acquired new target: %s"), 
               *OwnerPawn->GetName(), *CurrentTarget->GetName());
    }
}

void UCombat_AdvancedAIBehavior::TransitionToCombatState()
{
    if (!CurrentTarget)
    {
        CombatState = ECombat_AIState::Patrol;
        return;
    }
    
    float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget <= AttackRange)
    {
        CombatState = ECombat_AIState::Attacking;
    }
    else if (bIsPackLeader && PackMembers.Num() > 0)
    {
        // Pack leader coordinates attack
        CombatState = ECombat_AIState::Pursuing;
        InitiatePackTactics();
    }
    else
    {
        CombatState = ECombat_AIState::Pursuing;
    }
    
    ThreatLevel = ECombat_ThreatLevel::High;
}

void UCombat_AdvancedAIBehavior::HandlePatrolState(float DeltaTime)
{
    // Simple patrol behavior - move to random points
    if (!OwnerController)
    {
        return;
    }
    
    // Check if we've reached our destination
    if (FVector::Dist(OwnerPawn->GetActorLocation(), PatrolTarget) < 200.0f)
    {
        // Choose new patrol point
        ChooseNewPatrolPoint();
    }
    
    // Move towards patrol target
    OwnerController->MoveToLocation(PatrolTarget);
}

void UCombat_AdvancedAIBehavior::HandlePursuingState(float DeltaTime)
{
    if (!CurrentTarget || !OwnerController)
    {
        CombatState = ECombat_AIState::Patrol;
        return;
    }
    
    LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownTargetLocation);
    
    if (DistanceToTarget <= AttackRange)
    {
        CombatState = ECombat_AIState::Attacking;
    }
    else
    {
        // Move towards target
        OwnerController->MoveToActor(CurrentTarget);
    }
}

void UCombat_AdvancedAIBehavior::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        CombatState = ECombat_AIState::Patrol;
        return;
    }
    
    float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget > AttackRange * 1.5f)
    {
        CombatState = ECombat_AIState::Pursuing;
        return;
    }
    
    // Execute attack
    ExecuteAttack();
}

void UCombat_AdvancedAIBehavior::HandleFlankingState(float DeltaTime)
{
    if (!CurrentTarget || !OwnerController)
    {
        CombatState = ECombat_AIState::Pursuing;
        return;
    }
    
    // Calculate flanking position
    FVector FlankPosition = CalculateFlankingPosition();
    
    // Move to flanking position
    OwnerController->MoveToLocation(FlankPosition);
    
    // Check if we've reached flanking position
    if (FVector::Dist(OwnerPawn->GetActorLocation(), FlankPosition) < 150.0f)
    {
        CombatState = ECombat_AIState::Attacking;
    }
}

void UCombat_AdvancedAIBehavior::HandleRetreatingState(float DeltaTime)
{
    if (!OwnerController)
    {
        return;
    }
    
    // Calculate retreat position (away from target)
    FVector RetreatPosition = CalculateRetreatPosition();
    
    // Move to retreat position
    OwnerController->MoveToLocation(RetreatPosition);
    
    // Check if we're safe to return to patrol
    if (!CurrentTarget || FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation()) > DetectionRadius * 2.0f)
    {
        CombatState = ECombat_AIState::Patrol;
        CurrentTarget = nullptr;
        ThreatLevel = ECombat_ThreatLevel::Low;
    }
}

void UCombat_AdvancedAIBehavior::HandleInvestigatingState(float DeltaTime)
{
    if (!OwnerController)
    {
        return;
    }
    
    // Move to last known target location
    OwnerController->MoveToLocation(LastKnownTargetLocation);
    
    // Check if we've reached the location
    if (FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownTargetLocation) < 200.0f)
    {
        // Look around for the target
        if (TimeSinceLastTargetSeen > 5.0f)
        {
            // Give up and return to patrol
            CombatState = ECombat_AIState::Patrol;
            CurrentTarget = nullptr;
            ThreatLevel = ECombat_ThreatLevel::Low;
        }
    }
}

void UCombat_AdvancedAIBehavior::ChooseNewPatrolPoint()
{
    if (!OwnerPawn)
    {
        return;
    }
    
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    
    // Generate random patrol point within reasonable distance
    float PatrolRadius = 1000.0f;
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();
    
    PatrolTarget = CurrentLocation + (RandomDirection * FMath::RandRange(300.0f, PatrolRadius));
    
    // Ensure the point is navigable
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(PatrolTarget, NavLocation))
        {
            PatrolTarget = NavLocation.Location;
        }
    }
}

FVector UCombat_AdvancedAIBehavior::CalculateFlankingPosition()
{
    if (!CurrentTarget || !OwnerPawn)
    {
        return OwnerPawn->GetActorLocation();
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Calculate perpendicular direction for flanking
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    // Position at optimal flanking distance
    FVector FlankPosition = TargetLocation + (FlankDirection * AttackRange * 0.8f);
    
    return FlankPosition;
}

FVector UCombat_AdvancedAIBehavior::CalculateRetreatPosition()
{
    if (!OwnerPawn)
    {
        return FVector::ZeroVector;
    }
    
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    if (CurrentTarget)
    {
        // Retreat away from target
        FVector AwayFromTarget = (OwnerLocation - CurrentTarget->GetActorLocation()).GetSafeNormal();
        return OwnerLocation + (AwayFromTarget * 800.0f);
    }
    else
    {
        // Retreat to random safe position
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0;
        RandomDirection.Normalize();
        return OwnerLocation + (RandomDirection * 600.0f);
    }
}

void UCombat_AdvancedAIBehavior::ExecuteAttack()
{
    if (!CurrentTarget || !OwnerPawn)
    {
        return;
    }
    
    // Basic attack implementation
    UE_LOG(LogTemp, Log, TEXT("%s attacking %s"), 
           *OwnerPawn->GetName(), *CurrentTarget->GetName());
    
    // Face the target
    FVector ToTarget = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
    FRotator TargetRotation = ToTarget.Rotation();
    OwnerPawn->SetActorRotation(TargetRotation);
    
    // TODO: Implement actual damage dealing
    // This would integrate with the damage system
}

void UCombat_AdvancedAIBehavior::UpdateThreatAssessment()
{
    if (!CurrentTarget)
    {
        ThreatLevel = ECombat_ThreatLevel::Low;
        return;
    }
    
    // Assess threat based on various factors
    float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget < AttackRange)
    {
        ThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else if (DistanceToTarget < DetectionRadius * 0.5f)
    {
        ThreatLevel = ECombat_ThreatLevel::High;
    }
    else
    {
        ThreatLevel = ECombat_ThreatLevel::Medium;
    }
    
    // Check health status for retreat decision
    if (OwnerPawn)
    {
        // TODO: Get actual health from health component
        float HealthPercentage = 1.0f; // Placeholder
        
        if (HealthPercentage < FleeHealthThreshold && ThreatLevel >= ECombat_ThreatLevel::High)
        {
            CombatState = ECombat_AIState::Retreating;
        }
    }
}

void UCombat_AdvancedAIBehavior::UpdatePackCoordination()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Pack leader coordinates group behavior
    if (CurrentTarget)
    {
        AssignPackRoles();
    }
}

void UCombat_AdvancedAIBehavior::DiscoverPackMembers()
{
    if (!OwnerPawn)
    {
        return;
    }
    
    PackMembers.Empty();
    
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Find nearby AI with same class
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), OwnerPawn->GetClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == OwnerPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= GroupCoordinationRadius)
        {
            PackMembers.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s found %d pack members"), 
           *OwnerPawn->GetName(), PackMembers.Num());
}

bool UCombat_AdvancedAIBehavior::DeterminePackLeadership()
{
    // Simple leadership determination - could be based on health, experience, etc.
    return FMath::RandBool();
}

void UCombat_AdvancedAIBehavior::NotifyPackOfTarget(AActor* Target)
{
    // Notify pack members of new target
    for (AActor* PackMember : PackMembers)
    {
        if (PackMember)
        {
            UCombat_AdvancedAIBehavior* MemberBehavior = PackMember->FindComponentByClass<UCombat_AdvancedAIBehavior>();
            if (MemberBehavior)
            {
                MemberBehavior->ReceivePackTarget(Target);
            }
        }
    }
}

void UCombat_AdvancedAIBehavior::ReceivePackTarget(AActor* Target)
{
    if (!CurrentTarget && Target)
    {
        SetNewTarget(Target);
    }
}

void UCombat_AdvancedAIBehavior::InitiatePackTactics()
{
    if (!bIsPackLeader || PackMembers.Num() == 0 || !CurrentTarget)
    {
        return;
    }
    
    // Assign different roles to pack members
    AssignPackRoles();
}

void UCombat_AdvancedAIBehavior::AssignPackRoles()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Simple role assignment
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        AActor* PackMember = PackMembers[i];
        if (PackMember)
        {
            UCombat_AdvancedAIBehavior* MemberBehavior = PackMember->FindComponentByClass<UCombat_AdvancedAIBehavior>();
            if (MemberBehavior)
            {
                if (i % 2 == 0)
                {
                    // Assign flanking role
                    MemberBehavior->CombatState = ECombat_AIState::Flanking;
                }
                else
                {
                    // Assign direct pursuit
                    MemberBehavior->CombatState = ECombat_AIState::Pursuing;
                }
            }
        }
    }
}