#include "Combat_AdvancedPredatorAI.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

DEFINE_LOG_CATEGORY(LogCombatPredatorAI);

UCombat_AdvancedPredatorAI::UCombat_AdvancedPredatorAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI decisions
    
    // Initialize predator parameters
    TerritoryRadius = 2500.0f;
    DetectionRange = 3000.0f;
    AttackRange = 800.0f;
    StalkingRange = 1500.0f;
    
    AggressionLevel = 0.9f;
    IntelligenceLevel = 0.8f;
    TerritorialBehavior = true;
    
    CurrentBehaviorState = ECombat_PredatorState::Patrolling;
    ThreatLevel = ECombat_ThreatLevel::None;
    
    LastPlayerPosition = FVector::ZeroVector;
    TimeInCurrentState = 0.0f;
    LastAttackTime = 0.0f;
    AttackCooldown = 5.0f;
    
    StalkingSpeed = 0.3f;
    PatrolSpeed = 0.5f;
    AttackSpeed = 1.0f;
    
    bIsInTerritory = true;
    bCanSeePlayer = false;
    bPlayerInAttackRange = false;
}

void UCombat_AdvancedPredatorAI::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogCombatPredatorAI, Error, TEXT("AdvancedPredatorAI component must be attached to a Pawn"));
        return;
    }
    
    AIController = Cast<AAIController>(OwnerPawn->GetController());
    TerritoryCenter = OwnerPawn->GetActorLocation();
    
    // Initialize blackboard keys if AI controller exists
    if (AIController && AIController->GetBlackboardComponent())
    {
        BlackboardComponent = AIController->GetBlackboardComponent();
        InitializeBlackboardKeys();
    }
    
    UE_LOG(LogCombatPredatorAI, Log, TEXT("Advanced Predator AI initialized for %s"), *OwnerPawn->GetName());
}

void UCombat_AdvancedPredatorAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !IsValid(OwnerPawn))
        return;
    
    TimeInCurrentState += DeltaTime;
    
    // Update player detection
    UpdatePlayerDetection();
    
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Execute current behavior
    ExecuteCurrentBehavior(DeltaTime);
    
    // Update blackboard with current state
    UpdateBlackboard();
}

void UCombat_AdvancedPredatorAI::UpdatePlayerDetection()
{
    if (!OwnerPawn)
        return;
    
    // Find player character
    ATranspersonalCharacter* Player = nullptr;
    UWorld* World = OwnerPawn->GetWorld();
    if (World)
    {
        for (TActorIterator<ATranspersonalCharacter> ActorItr(World); ActorItr; ++ActorItr)
        {
            Player = *ActorItr;
            break; // Get first player
        }
    }
    
    if (!Player)
        return;
    
    FVector PlayerLocation = Player->GetActorLocation();
    FVector MyLocation = OwnerPawn->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, MyLocation);
    
    // Update detection flags
    bCanSeePlayer = DistanceToPlayer <= DetectionRange && HasLineOfSightToPlayer(Player);
    bPlayerInAttackRange = DistanceToPlayer <= AttackRange;
    bIsInTerritory = FVector::Dist(MyLocation, TerritoryCenter) <= TerritoryRadius;
    
    if (bCanSeePlayer)
    {
        LastPlayerPosition = PlayerLocation;
        LastPlayerSightTime = GetWorld()->GetTimeSeconds();
    }
}

bool UCombat_AdvancedPredatorAI::HasLineOfSightToPlayer(ATranspersonalCharacter* Player)
{
    if (!Player || !OwnerPawn)
        return false;
    
    FVector StartLocation = OwnerPawn->GetActorLocation() + FVector(0, 0, 100); // Eye level
    FVector EndLocation = Player->GetActorLocation() + FVector(0, 0, 100);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No obstruction means clear line of sight
}

void UCombat_AdvancedPredatorAI::UpdateThreatAssessment()
{
    if (!bCanSeePlayer)
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), LastPlayerPosition);
    
    if (DistanceToPlayer <= AttackRange)
    {
        ThreatLevel = ECombat_ThreatLevel::Extreme;
    }
    else if (DistanceToPlayer <= StalkingRange)
    {
        ThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (DistanceToPlayer <= DetectionRange)
    {
        ThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else
    {
        ThreatLevel = ECombat_ThreatLevel::Low;
    }
}

void UCombat_AdvancedPredatorAI::ExecuteCurrentBehavior(float DeltaTime)
{
    ECombat_PredatorState NewState = DetermineNextBehaviorState();
    
    if (NewState != CurrentBehaviorState)
    {
        OnBehaviorStateChanged(CurrentBehaviorState, NewState);
        CurrentBehaviorState = NewState;
        TimeInCurrentState = 0.0f;
    }
    
    switch (CurrentBehaviorState)
    {
        case ECombat_PredatorState::Patrolling:
            ExecutePatrolBehavior(DeltaTime);
            break;
            
        case ECombat_PredatorState::Investigating:
            ExecuteInvestigateBehavior(DeltaTime);
            break;
            
        case ECombat_PredatorState::Stalking:
            ExecuteStalkingBehavior(DeltaTime);
            break;
            
        case ECombat_PredatorState::Attacking:
            ExecuteAttackBehavior(DeltaTime);
            break;
            
        case ECombat_PredatorState::Retreating:
            ExecuteRetreatBehavior(DeltaTime);
            break;
    }
}

ECombat_PredatorState UCombat_AdvancedPredatorAI::DetermineNextBehaviorState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    bool bCanAttack = (CurrentTime - LastAttackTime) >= AttackCooldown;
    
    // Priority-based state machine
    if (bPlayerInAttackRange && bCanAttack && ThreatLevel == ECombat_ThreatLevel::Extreme)
    {
        return ECombat_PredatorState::Attacking;
    }
    
    if (bCanSeePlayer && ThreatLevel >= ECombat_ThreatLevel::High)
    {
        return ECombat_PredatorState::Stalking;
    }
    
    if (ThreatLevel == ECombat_ThreatLevel::Medium)
    {
        return ECombat_PredatorState::Investigating;
    }
    
    // Return to patrol if no immediate threats
    return ECombat_PredatorState::Patrolling;
}

void UCombat_AdvancedPredatorAI::ExecutePatrolBehavior(float DeltaTime)
{
    if (!AIController)
        return;
    
    // Simple patrol around territory
    if (TimeInCurrentState > 5.0f) // Change patrol point every 5 seconds
    {
        FVector RandomPoint = TerritoryCenter + FMath::VRand() * TerritoryRadius * 0.7f;
        RandomPoint.Z = TerritoryCenter.Z; // Keep same height
        
        AIController->MoveToLocation(RandomPoint, 100.0f);
        TimeInCurrentState = 0.0f;
    }
}

void UCombat_AdvancedPredatorAI::ExecuteInvestigateBehavior(float DeltaTime)
{
    if (!AIController)
        return;
    
    // Move towards last known player position
    if (!LastPlayerPosition.IsZero())
    {
        AIController->MoveToLocation(LastPlayerPosition, 200.0f);
    }
}

void UCombat_AdvancedPredatorAI::ExecuteStalkingBehavior(float DeltaTime)
{
    if (!AIController || LastPlayerPosition.IsZero())
        return;
    
    // Calculate stalking position - approach from behind or side
    FVector ToPlayer = (LastPlayerPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
    FVector StalkPosition = LastPlayerPosition - ToPlayer * (StalkingRange * 0.8f);
    
    // Add some randomness to stalking pattern
    FVector RandomOffset = FMath::VRand() * 300.0f;
    RandomOffset.Z = 0; // Keep on ground level
    StalkPosition += RandomOffset;
    
    AIController->MoveToLocation(StalkPosition, 150.0f);
    
    UE_LOG(LogCombatPredatorAI, Log, TEXT("%s is stalking player"), *OwnerPawn->GetName());
}

void UCombat_AdvancedPredatorAI::ExecuteAttackBehavior(float DeltaTime)
{
    if (!AIController || LastPlayerPosition.IsZero())
        return;
    
    // Direct attack move to player position
    AIController->MoveToLocation(LastPlayerPosition, 50.0f);
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogCombatPredatorAI, Warning, TEXT("%s is ATTACKING!"), *OwnerPawn->GetName());
}

void UCombat_AdvancedPredatorAI::ExecuteRetreatBehavior(float DeltaTime)
{
    if (!AIController)
        return;
    
    // Move back towards territory center
    AIController->MoveToLocation(TerritoryCenter, 200.0f);
}

void UCombat_AdvancedPredatorAI::OnBehaviorStateChanged(ECombat_PredatorState OldState, ECombat_PredatorState NewState)
{
    UE_LOG(LogCombatPredatorAI, Log, TEXT("%s behavior changed: %s -> %s"), 
           *OwnerPawn->GetName(),
           *UEnum::GetValueAsString(OldState),
           *UEnum::GetValueAsString(NewState));
    
    // Broadcast state change event
    OnPredatorStateChanged.Broadcast(OldState, NewState);
}

void UCombat_AdvancedPredatorAI::InitializeBlackboardKeys()
{
    if (!BlackboardComponent)
        return;
    
    // Set up blackboard key names
    PlayerLocationKey = FName("PlayerLocation");
    BehaviorStateKey = FName("BehaviorState");
    ThreatLevelKey = FName("ThreatLevel");
    CanSeePlayerKey = FName("CanSeePlayer");
}

void UCombat_AdvancedPredatorAI::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;
    
    BlackboardComponent->SetValueAsVector(PlayerLocationKey, LastPlayerPosition);
    BlackboardComponent->SetValueAsEnum(BehaviorStateKey, static_cast<uint8>(CurrentBehaviorState));
    BlackboardComponent->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(ThreatLevel));
    BlackboardComponent->SetValueAsBool(CanSeePlayerKey, bCanSeePlayer);
}

void UCombat_AdvancedPredatorAI::SetAggressionLevel(float NewAggression)
{
    AggressionLevel = FMath::Clamp(NewAggression, 0.0f, 1.0f);
    
    // Adjust ranges based on aggression
    AttackRange = 800.0f * (1.0f + AggressionLevel * 0.5f);
    StalkingRange = 1500.0f * (1.0f + AggressionLevel * 0.3f);
}

void UCombat_AdvancedPredatorAI::SetTerritoryCenter(const FVector& NewCenter)
{
    TerritoryCenter = NewCenter;
}

bool UCombat_AdvancedPredatorAI::IsPlayerInTerritory() const
{
    if (LastPlayerPosition.IsZero())
        return false;
    
    return FVector::Dist(LastPlayerPosition, TerritoryCenter) <= TerritoryRadius;
}