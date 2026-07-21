#include "Combat_DinosaurCombatController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

ACombat_DinosaurCombatController::ACombat_DinosaurCombatController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Set default combat state
    CurrentCombatState = ECombat_DinosaurCombatState::Idle;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    PatrolRadius = 1000.0f;

    // Initialize combat stats with default values
    CombatStats.AttackDamage = 50.0f;
    CombatStats.AttackRange = 300.0f;
    CombatStats.DetectionRadius = 2000.0f;
    CombatStats.MovementSpeed = 400.0f;
    CombatStats.AttackCooldown = 2.0f;
    CombatStats.FleeHealthThreshold = 0.3f;
}

void ACombat_DinosaurCombatController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    // Set patrol center to current location
    if (GetPawn())
    {
        PatrolCenter = GetPawn()->GetActorLocation();
    }
}

void ACombat_DinosaurCombatController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        PatrolCenter = InPawn->GetActorLocation();
        InitializeBehaviorTree();
        SetCombatState(ECombat_DinosaurCombatState::Patrolling);
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatController possessed pawn: %s"), *InPawn->GetName());
    }
}

void ACombat_DinosaurCombatController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ACombat_DinosaurCombatController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update AI behavior based on current state
    switch (CurrentCombatState)
    {
        case ECombat_DinosaurCombatState::Patrolling:
            if (DetectPlayer())
            {
                SetCombatState(ECombat_DinosaurCombatState::Hunting);
            }
            break;
            
        case ECombat_DinosaurCombatState::Hunting:
            if (CurrentTarget)
            {
                if (IsInAttackRange())
                {
                    SetCombatState(ECombat_DinosaurCombatState::Attacking);
                }
                else
                {
                    MoveToTarget();
                }
            }
            else
            {
                SetCombatState(ECombat_DinosaurCombatState::Patrolling);
            }
            break;
            
        case ECombat_DinosaurCombatState::Attacking:
            if (CurrentTarget && IsInAttackRange())
            {
                float CurrentTime = GetWorld()->GetTimeSeconds();
                if (CurrentTime - LastAttackTime >= CombatStats.AttackCooldown)
                {
                    ExecuteAttack();
                    LastAttackTime = CurrentTime;
                }
            }
            else
            {
                SetCombatState(ECombat_DinosaurCombatState::Hunting);
            }
            break;
            
        case ECombat_DinosaurCombatState::Fleeing:
            FleeFromTarget();
            break;
    }
    
    UpdateBlackboard();
}

void ACombat_DinosaurCombatController::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_DinosaurCombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
        
        // Handle state transitions
        switch (NewState)
        {
            case ECombat_DinosaurCombatState::Patrolling:
                StartPatrol();
                break;
                
            case ECombat_DinosaurCombatState::Hunting:
                if (CurrentTarget)
                {
                    MoveToTarget();
                }
                break;
                
            case ECombat_DinosaurCombatState::Attacking:
                StopMovement();
                break;
                
            case ECombat_DinosaurCombatState::Fleeing:
                FleeFromTarget();
                break;
        }
    }
}

bool ACombat_DinosaurCombatController::DetectPlayer()
{
    if (!GetPawn())
    {
        return false;
    }
    
    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        return false;
    }
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float Distance = FVector::Dist(MyLocation, PlayerLocation);
    
    // Check if player is within detection radius
    if (Distance <= CombatStats.DetectionRadius)
    {
        // Perform line of sight check
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        QueryParams.AddIgnoredActor(PlayerCharacter);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            MyLocation,
            PlayerLocation,
            ECollisionChannel::ECC_Visibility,
            QueryParams
        );
        
        if (!bHit)
        {
            CurrentTarget = PlayerCharacter;
            UE_LOG(LogTemp, Warning, TEXT("Player detected at distance: %f"), Distance);
            return true;
        }
    }
    
    return false;
}

void ACombat_DinosaurCombatController::StartAttack()
{
    if (CurrentTarget && IsInAttackRange())
    {
        SetCombatState(ECombat_DinosaurCombatState::Attacking);
    }
}

void ACombat_DinosaurCombatController::ExecuteAttack()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking target: %s"), *CurrentTarget->GetName());
    
    // Apply damage to target (simplified implementation)
    if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
    {
        // Here you would apply damage using the damage system
        // For now, just log the attack
        UE_LOG(LogTemp, Warning, TEXT("Applied %f damage to %s"), 
               CombatStats.AttackDamage, *TargetCharacter->GetName());
    }
}

bool ACombat_DinosaurCombatController::IsInAttackRange() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Distance <= CombatStats.AttackRange;
}

void ACombat_DinosaurCombatController::MoveToTarget()
{
    if (CurrentTarget)
    {
        MoveToActor(CurrentTarget, CombatStats.AttackRange * 0.8f);
    }
}

void ACombat_DinosaurCombatController::StartPatrol()
{
    FVector PatrolPoint = GetRandomPatrolPoint();
    MoveToLocation(PatrolPoint);
}

void ACombat_DinosaurCombatController::FleeFromTarget()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector FleeDirection = (MyLocation - TargetLocation).GetSafeNormal();
    FVector FleeLocation = MyLocation + (FleeDirection * 2000.0f);
    
    MoveToLocation(FleeLocation);
}

void ACombat_DinosaurCombatController::InitializePerception()
{
    if (AIPerceptionComponent)
    {
        // Configure sight sense
        SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = CombatStats.DetectionRadius;
            SightConfig->LoseSightRadius = CombatStats.DetectionRadius * 1.2f;
            SightConfig->PeripheralVisionAngleDegrees = 90.0f;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
            
            AIPerceptionComponent->ConfigureSense(*SightConfig);
            AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
            
            // Bind perception events
            AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatController::OnPerceptionUpdated);
        }
    }
}

void ACombat_DinosaurCombatController::InitializeBehaviorTree()
{
    if (BehaviorTreeComponent && BlackboardComponent)
    {
        // Start behavior tree if available
        if (BehaviorTree)
        {
            RunBehaviorTree(BehaviorTree);
        }
    }
}

FVector ACombat_DinosaurCombatController::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = PatrolCenter + (RandomDirection * RandomDistance);
    
    return PatrolPoint;
}

bool ACombat_DinosaurCombatController::LineOfSightToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.AddIgnoredActor(CurrentTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetPawn()->GetActorLocation(),
        CurrentTarget->GetActorLocation(),
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}

float ACombat_DinosaurCombatController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return -1.0f;
    }
    
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ACombat_DinosaurCombatController::UpdateBlackboard()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), (uint8)CurrentCombatState);
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
            BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
        }
    }
}

void ACombat_DinosaurCombatController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Check if this is a player character
            if (Character->IsPlayerControlled())
            {
                CurrentTarget = Character;
                if (CurrentCombatState == ECombat_DinosaurCombatState::Patrolling)
                {
                    SetCombatState(ECombat_DinosaurCombatState::Hunting);
                }
                break;
            }
        }
    }
}