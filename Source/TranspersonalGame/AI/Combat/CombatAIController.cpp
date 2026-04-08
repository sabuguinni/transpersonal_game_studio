#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize Gameplay Tags
    IdleState = FGameplayTag::RequestGameplayTag(FName("Combat.State.Idle"));
    HuntingState = FGameplayTag::RequestGameplayTag(FName("Combat.State.Hunting"));
    AttackingState = FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking"));
    FleeingState = FGameplayTag::RequestGameplayTag(FName("Combat.State.Fleeing"));
    PatrollingState = FGameplayTag::RequestGameplayTag(FName("Combat.State.Patrolling"));

    CombatState = IdleState;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    SetupPerception();
    SetupBlackboard();

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatLogic(DeltaTime);
    UpdateThreatAssessment();
}

void ACombatAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = DetectionRange * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(3.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = DetectionRange * 0.8f;
    HearingConfig->SetMaxAge(2.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(5.0f);

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::SetupBlackboard()
{
    if (BlackboardAsset && BlackboardComponent)
    {
        UseBlackboard(BlackboardAsset);
        
        // Initialize key values
        BlackboardComponent->SetValueAsEnum(FName("CombatState"), static_cast<uint8>(0)); // Idle
        BlackboardComponent->SetValueAsFloat(FName("ThreatLevel"), 0.0f);
        BlackboardComponent->SetValueAsFloat(FName("AggressionLevel"), AggressionLevel);
        BlackboardComponent->SetValueAsFloat(FName("AttackRange"), AttackRange);
        BlackboardComponent->SetValueAsFloat(FName("DetectionRange"), DetectionRange);
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            float ThreatValue = CalculateThreatLevel(Actor);
            ThreatMap.Add(Actor, ThreatValue);

            // Update current target if this is a higher threat
            if (!CurrentTarget || ThreatValue > ThreatLevel)
            {
                CurrentTarget = Actor;
                ThreatLevel = ThreatValue;
                BlackboardComponent->SetValueAsObject(FName("TargetActor"), CurrentTarget);
                BlackboardComponent->SetValueAsFloat(FName("ThreatLevel"), ThreatLevel);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target acquired or updated
        if (Actor->IsA<ACharacter>())
        {
            float ThreatValue = CalculateThreatLevel(Actor);
            
            if (ThreatValue > 0.5f) // Significant threat threshold
            {
                StartHunting(Actor);
            }
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            CurrentTarget = nullptr;
            ThreatLevel = 0.0f;
            SetCombatState(PatrollingState);
            BlackboardComponent->ClearValue(FName("TargetActor"));
            BlackboardComponent->SetValueAsFloat(FName("ThreatLevel"), 0.0f);
        }
    }
}

void ACombatAIController::SetCombatState(FGameplayTag NewState)
{
    if (CombatState != NewState)
    {
        CombatState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            if (NewState == IdleState)
                BlackboardComponent->SetValueAsEnum(FName("CombatState"), 0);
            else if (NewState == HuntingState)
                BlackboardComponent->SetValueAsEnum(FName("CombatState"), 1);
            else if (NewState == AttackingState)
                BlackboardComponent->SetValueAsEnum(FName("CombatState"), 2);
            else if (NewState == FleeingState)
                BlackboardComponent->SetValueAsEnum(FName("CombatState"), 3);
            else if (NewState == PatrollingState)
                BlackboardComponent->SetValueAsEnum(FName("CombatState"), 4);
        }
    }
}

float ACombatAIController::CalculateThreatLevel(AActor* Target)
{
    if (!Target) return 0.0f;

    float Threat = 0.0f;
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Distance factor (closer = more threatening)
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / DetectionRange), 0.0f, 1.0f);
    Threat += DistanceFactor * 0.4f;

    // Player character is always high threat
    if (Target->IsA<ACharacter>() && Target->ActorHasTag(FName("Player")))
    {
        Threat += 0.6f;
    }

    // Health factor - if we're injured, everything is more threatening
    if (APawn* MyPawn = GetPawn())
    {
        // Assuming health component exists
        float HealthPercentage = 1.0f; // TODO: Get actual health percentage
        if (HealthPercentage < FleeHealthThreshold)
        {
            Threat *= 1.5f; // Increase threat when low health
        }
    }

    return FMath::Clamp(Threat, 0.0f, 1.0f);
}

bool ACombatAIController::ShouldFlee()
{
    // Check health threshold
    if (APawn* MyPawn = GetPawn())
    {
        // TODO: Implement actual health check
        float HealthPercentage = 1.0f;
        if (HealthPercentage < FleeHealthThreshold)
        {
            return true;
        }
    }

    // Check if outnumbered
    int32 NearbyThreats = 0;
    for (auto& ThreatPair : ThreatMap)
    {
        if (ThreatPair.Value > 0.3f)
        {
            NearbyThreats++;
        }
    }

    return NearbyThreats > 2; // Flee if facing multiple threats
}

bool ACombatAIController::CanAttackTarget(AActor* Target)
{
    if (!Target || !GetPawn()) return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    bool bInRange = Distance <= AttackRange;
    bool bCooldownReady = (GetWorld()->GetTimeSeconds() - LastAttackTime) >= AttackCooldown;

    return bInRange && bCooldownReady;
}

void ACombatAIController::ExecuteAttack()
{
    if (!CurrentTarget || !CanAttackTarget(CurrentTarget)) return;

    SetCombatState(AttackingState);
    LastAttackTime = GetWorld()->GetTimeSeconds();

    // TODO: Trigger attack animation and damage
    // This would integrate with the animation system
}

void ACombatAIController::StartHunting(AActor* Target)
{
    CurrentTarget = Target;
    SetCombatState(HuntingState);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetActor"), Target);
        BlackboardComponent->SetValueAsVector(FName("TargetLocation"), Target->GetActorLocation());
    }
}

void ACombatAIController::StartFleeing()
{
    SetCombatState(FleeingState);
    
    FVector FleePosition = FindFleePosition();
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(FName("FleeLocation"), FleePosition);
    }
}

FVector ACombatAIController::FindOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find position at optimal attack range
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * AttackRange * 0.8f);

    // Ensure position is on navmesh
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(200.0f)))
        {
            return NavLocation.Location;
        }
    }

    return OptimalPosition;
}

FVector ACombatAIController::FindFleePosition()
{
    if (!GetPawn()) return FVector::ZeroVector;

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector FleeDirection = FVector::ZeroVector;

    // Calculate flee direction away from all threats
    for (auto& ThreatPair : ThreatMap)
    {
        if (ThreatPair.Key && ThreatPair.Value > 0.0f)
        {
            FVector ThreatLocation = ThreatPair.Key->GetActorLocation();
            FVector AwayFromThreat = (MyLocation - ThreatLocation).GetSafeNormal();
            FleeDirection += AwayFromThreat * ThreatPair.Value;
        }
    }

    FleeDirection.Normalize();
    FVector FleePosition = MyLocation + (FleeDirection * DetectionRange * 1.5f);

    // Project to navmesh
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(FleePosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }

    return FleePosition;
}

void ACombatAIController::CallForBackup()
{
    // TODO: Implement pack behavior
    // Send signal to nearby allies of same species
}

void ACombatAIController::RespondToBackupCall(AActor* Caller)
{
    // TODO: Implement response to pack calls
    // Move to assist ally in combat
}

void ACombatAIController::UpdateCombatLogic(float DeltaTime)
{
    if (!GetPawn()) return;

    // State-specific logic
    if (CombatState == HuntingState && CurrentTarget)
    {
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (CanAttackTarget(CurrentTarget))
        {
            ExecuteAttack();
        }
        else if (DistanceToTarget > DetectionRange * 1.5f)
        {
            // Lost target, return to patrol
            SetCombatState(PatrollingState);
            CurrentTarget = nullptr;
        }
    }
    else if (CombatState == AttackingState)
    {
        // Check if attack animation is complete
        // TODO: Integrate with animation system
        if ((GetWorld()->GetTimeSeconds() - LastAttackTime) > 1.0f)
        {
            if (CurrentTarget && ShouldFlee())
            {
                StartFleeing();
            }
            else if (CurrentTarget)
            {
                SetCombatState(HuntingState);
            }
            else
            {
                SetCombatState(IdleState);
            }
        }
    }
}

void ACombatAIController::UpdateThreatAssessment()
{
    // Clean up old threats
    TArray<AActor*> ActorsToRemove;
    for (auto& ThreatPair : ThreatMap)
    {
        if (!ThreatPair.Key || !IsValid(ThreatPair.Key))
        {
            ActorsToRemove.Add(ThreatPair.Key);
        }
    }

    for (AActor* Actor : ActorsToRemove)
    {
        ThreatMap.Remove(Actor);
    }

    // Update current target if it's no longer valid
    if (CurrentTarget && (!IsValid(CurrentTarget) || !ThreatMap.Contains(CurrentTarget)))
    {
        CurrentTarget = nullptr;
        ThreatLevel = 0.0f;
        SetCombatState(IdleState);
    }
}