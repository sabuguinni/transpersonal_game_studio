#include "Combat_DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_DinosaurCombatAI::ACombat_DinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Setup Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1600.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Setup Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1200.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Behavior Tree Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize Combat Properties
    AggressionLevel = ECombat_AggressionLevel::Defensive;
    TacticalState = ECombat_TacticalState::Patrol;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    bIsInCombat = false;
    bIsPackHunter = false;
    PackLeader = nullptr;

    // Initialize Combat Stats with default values
    CombatStats.AttackDamage = 50.0f;
    CombatStats.AttackRange = 300.0f;
    CombatStats.DetectionRadius = 1500.0f;
    CombatStats.MovementSpeed = 400.0f;
    CombatStats.Health = 100.0f;
    CombatStats.Stamina = 100.0f;
}

void ACombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
        
        // Initialize blackboard values
        UpdateBlackboardValues();
    }

    // Set initial tactical state
    SetTacticalState(ECombat_TacticalState::Patrol);
}

void ACombat_DinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatBehavior(DeltaTime);
    UpdatePackBehavior(DeltaTime);
    UpdateBlackboardValues();
}

void ACombat_DinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    if (NewTarget != CurrentTarget)
    {
        CurrentTarget = NewTarget;
        
        if (CurrentTarget)
        {
            EnterCombatMode();
            UE_LOG(LogTemp, Warning, TEXT("Combat AI: New target acquired - %s"), *CurrentTarget->GetName());
        }
        else
        {
            ExitCombatMode();
        }
        
        UpdateBlackboardValues();
    }
}

void ACombat_DinosaurCombatAI::AttackTarget()
{
    if (!CanAttack() || !IsTargetValid())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < CombatCooldown)
    {
        return;
    }

    // Perform attack logic
    if (IsTargetInRange())
    {
        LastAttackTime = CurrentTime;
        
        // Apply damage to target if it's a character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
        {
            // TODO: Implement damage system
            UE_LOG(LogTemp, Warning, TEXT("Combat AI: Attacking %s for %f damage"), 
                   *CurrentTarget->GetName(), CombatStats.AttackDamage);
        }

        // Coordinate pack attack if this is a pack hunter
        if (bIsPackHunter && PackLeader == this)
        {
            CoordinatePackAttack(CurrentTarget);
        }
    }
}

void ACombat_DinosaurCombatAI::EnterCombatMode()
{
    if (!bIsInCombat)
    {
        bIsInCombat = true;
        SetTacticalState(ECombat_TacticalState::Engage);
        
        // Increase aggression level
        if (AggressionLevel == ECombat_AggressionLevel::Passive)
        {
            SetAggressionLevel(ECombat_AggressionLevel::Defensive);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI: Entering combat mode"));
    }
}

void ACombat_DinosaurCombatAI::ExitCombatMode()
{
    if (bIsInCombat)
    {
        bIsInCombat = false;
        CurrentTarget = nullptr;
        SetTacticalState(ECombat_TacticalState::Patrol);
        
        // Reset aggression level
        SetAggressionLevel(ECombat_AggressionLevel::Defensive);
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI: Exiting combat mode"));
    }
}

bool ACombat_DinosaurCombatAI::CanAttack() const
{
    return bIsInCombat && 
           IsTargetValid() && 
           IsTargetInRange() && 
           CombatStats.Stamina > 20.0f;
}

float ACombat_DinosaurCombatAI::GetDistanceToTarget() const
{
    if (!IsTargetValid())
    {
        return -1.0f;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return -1.0f;
    }

    return FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ACombat_DinosaurCombatAI::SetAggressionLevel(ECombat_AggressionLevel NewLevel)
{
    if (AggressionLevel != NewLevel)
    {
        AggressionLevel = NewLevel;
        
        // Adjust combat stats based on aggression level
        switch (AggressionLevel)
        {
            case ECombat_AggressionLevel::Passive:
                CombatStats.AttackDamage *= 0.7f;
                CombatStats.DetectionRadius *= 0.8f;
                break;
            case ECombat_AggressionLevel::Defensive:
                // Default stats
                break;
            case ECombat_AggressionLevel::Aggressive:
                CombatStats.AttackDamage *= 1.3f;
                CombatStats.DetectionRadius *= 1.2f;
                break;
            case ECombat_AggressionLevel::Enraged:
                CombatStats.AttackDamage *= 1.6f;
                CombatStats.DetectionRadius *= 1.5f;
                CombatCooldown *= 0.7f;
                break;
        }
        
        UpdateBlackboardValues();
    }
}

void ACombat_DinosaurCombatAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (TacticalState != NewState)
    {
        TacticalState = NewState;
        ExecuteTacticalManeuver();
        UpdateBlackboardValues();
    }
}

void ACombat_DinosaurCombatAI::JoinPack(ACombat_DinosaurCombatAI* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        bIsPackHunter = true;
        
        if (Leader->PackMembers.Find(this) == INDEX_NONE)
        {
            Leader->PackMembers.Add(this);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI: Joined pack under leader %s"), *Leader->GetName());
    }
}

void ACombat_DinosaurCombatAI::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    bIsPackHunter = false;
    PackMembers.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI: Left pack"));
}

void ACombat_DinosaurCombatAI::CoordinatePackAttack(AActor* Target)
{
    if (!bIsPackHunter || PackMembers.Num() == 0)
    {
        return;
    }

    // Coordinate attack with pack members
    for (ACombat_DinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->SetTarget(Target);
            
            // Stagger attacks for tactical advantage
            float Delay = FMath::RandRange(0.5f, 2.0f);
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, 
                [Member]() { Member->AttackTarget(); }, Delay, false);
        }
    }
}

void ACombat_DinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Check if this is a potential target (player character)
            if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                if (!CurrentTarget || GetDistanceToTarget() > FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation()))
                {
                    SetTarget(Actor);
                }
            }
        }
    }
}

void ACombat_DinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == CurrentTarget && !Stimulus.WasSuccessfullySensed())
    {
        // Lost sight of target
        SetTarget(nullptr);
    }
}

void ACombat_DinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    if (!bIsInCombat)
    {
        return;
    }

    // Update stamina
    if (CombatStats.Stamina < 100.0f)
    {
        CombatStats.Stamina = FMath::Min(100.0f, CombatStats.Stamina + (DeltaTime * 10.0f));
    }

    // Check if target is still valid
    if (!IsTargetValid())
    {
        SelectBestTarget();
    }

    // Execute combat behavior based on tactical state
    switch (TacticalState)
    {
        case ECombat_TacticalState::Engage:
            if (CanAttack())
            {
                AttackTarget();
            }
            break;
        case ECombat_TacticalState::Retreat:
            // TODO: Implement retreat behavior
            break;
        case ECombat_TacticalState::Ambush:
            // TODO: Implement ambush behavior
            break;
        default:
            break;
    }
}

void ACombat_DinosaurCombatAI::UpdatePackBehavior(float DeltaTime)
{
    if (!bIsPackHunter)
    {
        return;
    }

    // Clean up invalid pack members
    PackMembers.RemoveAll([](ACombat_DinosaurCombatAI* Member) {
        return !IsValid(Member);
    });

    // If we're the leader, coordinate pack behavior
    if (PackLeader == this && CurrentTarget)
    {
        for (ACombat_DinosaurCombatAI* Member : PackMembers)
        {
            if (Member && !Member->CurrentTarget)
            {
                Member->SetTarget(CurrentTarget);
            }
        }
    }
}

void ACombat_DinosaurCombatAI::SelectBestTarget()
{
    TArray<AActor*> PerceivedActors;
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
    }

    AActor* BestTarget = nullptr;
    float BestDistance = FLT_MAX;

    for (AActor* Actor : PerceivedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < BestDistance && Distance <= CombatStats.DetectionRadius)
            {
                BestTarget = Actor;
                BestDistance = Distance;
            }
        }
    }

    SetTarget(BestTarget);
}

void ACombat_DinosaurCombatAI::ExecuteTacticalManeuver()
{
    // TODO: Implement tactical maneuvers based on TacticalState
    switch (TacticalState)
    {
        case ECombat_TacticalState::Patrol:
            // Set patrol behavior
            break;
        case ECombat_TacticalState::Investigate:
            // Set investigation behavior
            break;
        case ECombat_TacticalState::Engage:
            // Set engagement behavior
            break;
        case ECombat_TacticalState::Retreat:
            // Set retreat behavior
            break;
        case ECombat_TacticalState::Ambush:
            // Set ambush behavior
            break;
    }
}

bool ACombat_DinosaurCombatAI::IsTargetValid() const
{
    return IsValid(CurrentTarget) && !CurrentTarget->IsPendingKill();
}

bool ACombat_DinosaurCombatAI::IsTargetInRange() const
{
    if (!IsTargetValid())
    {
        return false;
    }

    float Distance = GetDistanceToTarget();
    return Distance >= 0.0f && Distance <= CombatStats.AttackRange;
}

bool ACombat_DinosaurCombatAI::HasLineOfSight() const
{
    if (!IsTargetValid())
    {
        return false;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return false;
    }

    FVector Start = ControlledPawn->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ControlledPawn);
    QueryParams.AddIgnoredActor(CurrentTarget);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    return !bHit;
}

void ACombat_DinosaurCombatAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update blackboard with current combat state
    BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
    BlackboardComponent->SetValueAsEnum(TEXT("AggressionLevel"), static_cast<uint8>(AggressionLevel));
    BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(TacticalState));
    BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), CombatStats.AttackRange);
    BlackboardComponent->SetValueAsFloat(TEXT("DetectionRadius"), CombatStats.DetectionRadius);
    BlackboardComponent->SetValueAsBool(TEXT("CanAttack"), CanAttack());
    
    if (IsTargetValid())
    {
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    }
}