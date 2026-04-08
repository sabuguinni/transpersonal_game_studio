#include "CombatAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRange;
    SightConfig->LoseSightRadius = SightRange * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize state
    CurrentCombatState = ECombatState::Idle;
    CurrentThreatLevel = EThreatLevel::None;
    CurrentTarget = nullptr;
    LastTargetSightTime = 0.0f;
    TacticalUpdateTimer = 0.0f;
    bHasValidAttackPosition = false;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TacticalUpdateTimer += DeltaTime;
    
    if (TacticalUpdateTimer >= TacticalUpdateInterval)
    {
        UpdateTacticalPosition();
        ExecuteCombatBehavior();
        TacticalUpdateTimer = 0.0f;
    }

    // Handle target loss
    if (CurrentTarget && GetWorld()->GetTimeSeconds() - LastTargetSightTime > 5.0f)
    {
        HandleLostTarget();
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                OnTargetPerceptionUpdated(Actor, Stimulus);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Update blackboard with perceived actor
    if (BlackboardComponent)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // New target acquired or target still visible
            CurrentTarget = Actor;
            LastTargetSightTime = GetWorld()->GetTimeSeconds();
            LastKnownTargetLocation = Actor->GetActorLocation();
            
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Actor);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), true);

            // Update combat state based on distance and threat
            float DistanceToTarget = GetDistanceToTarget(Actor);
            
            if (DistanceToTarget <= AttackRange)
            {
                SetCombatState(ECombatState::Attacking);
            }
            else if (DistanceToTarget <= SightRange * 0.5f)
            {
                SetCombatState(ECombatState::Stalking);
            }
            else
            {
                SetCombatState(ECombatState::Hunting);
            }
        }
        else
        {
            // Target lost
            if (CurrentTarget == Actor)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
                BlackboardComponent->SetValueAsVector(TEXT("LastKnownLocation"), LastKnownTargetLocation);
                BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), false);
            }
        }
    }

    UpdateThreatLevel();
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombatAIController::UpdateThreatLevel()
{
    EThreatLevel NewThreatLevel = EThreatLevel::None;
    
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        if (DistanceToTarget <= AttackRange)
        {
            NewThreatLevel = EThreatLevel::Critical;
        }
        else if (DistanceToTarget <= AttackRange * 2.0f)
        {
            NewThreatLevel = EThreatLevel::High;
        }
        else if (DistanceToTarget <= SightRange * 0.5f)
        {
            NewThreatLevel = EThreatLevel::Medium;
        }
        else
        {
            NewThreatLevel = EThreatLevel::Low;
        }
    }
    
    CurrentThreatLevel = NewThreatLevel;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(NewThreatLevel));
    }
}

AActor* ACombatAIController::GetCurrentTarget() const
{
    return CurrentTarget;
}

bool ACombatAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent) return false;
    
    FAIStimulus Stimulus;
    return AIPerceptionComponent->GetActorsPerception(Target, Stimulus) && Stimulus.WasSuccessfullySensed();
}

float ACombatAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return FLT_MAX;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

FVector ACombatAIController::CalculateFlankingPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    // Position at flanking distance
    FVector FlankPosition = TargetLocation + (FlankDirection * CirclingDistance);
    
    return FlankPosition;
}

FVector ACombatAIController::CalculateAmbushPosition(AActor* Target)
{
    if (!Target) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    // Position behind the target for ambush
    FVector AmbushPosition = TargetLocation - (TargetForward * CirclingDistance);
    
    return AmbushPosition;
}

bool ACombatAIController::ShouldRetreat() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;
    
    // Check health threshold
    // Note: This assumes the pawn has a health component - adjust based on your health system
    return false; // Placeholder - implement based on your health system
}

bool ACombatAIController::ShouldAttack() const
{
    if (!CurrentTarget) return false;
    
    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
    return DistanceToTarget <= AttackRange && CanSeeTarget(CurrentTarget);
}

TArray<ACombatAIController*> ACombatAIController::GetNearbyPackMembers(float Radius) const
{
    TArray<ACombatAIController*> PackMembers;
    
    if (!GetPawn()) return PackMembers;
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    
    for (TActorIterator<ACombatAIController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ACombatAIController* OtherController = *ActorItr;
        if (OtherController && OtherController != this && OtherController->GetPawn())
        {
            float Distance = FVector::Dist(PawnLocation, OtherController->GetPawn()->GetActorLocation());
            if (Distance <= Radius)
            {
                PackMembers.Add(OtherController);
            }
        }
    }
    
    return PackMembers;
}

void ACombatAIController::CoordinatePackAttack(AActor* Target)
{
    if (!Target || !bUsePackTactics) return;
    
    TArray<ACombatAIController*> PackMembers = GetNearbyPackMembers();
    
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        ACombatAIController* Member = PackMembers[i];
        if (Member && Member->BlackboardComponent)
        {
            // Assign different roles to pack members
            if (i % 3 == 0)
            {
                // Frontal attacker
                Member->SetCombatState(ECombatState::Attacking);
            }
            else if (i % 3 == 1)
            {
                // Flanker
                FVector FlankPos = CalculateFlankingPosition(Target);
                Member->BlackboardComponent->SetValueAsVector(TEXT("TacticalPosition"), FlankPos);
                Member->SetCombatState(ECombatState::Circling);
            }
            else
            {
                // Support/ambush
                Member->SetCombatState(ECombatState::Stalking);
            }
        }
    }
}

void ACombatAIController::UpdateTacticalPosition()
{
    if (!CurrentTarget) return;
    
    switch (CurrentCombatState)
    {
        case ECombatState::Circling:
            PreferredAttackPosition = CalculateFlankingPosition(CurrentTarget);
            bHasValidAttackPosition = true;
            break;
            
        case ECombatState::Ambushing:
            PreferredAttackPosition = CalculateAmbushPosition(CurrentTarget);
            bHasValidAttackPosition = true;
            break;
            
        default:
            bHasValidAttackPosition = false;
            break;
    }
    
    if (bHasValidAttackPosition && BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TacticalPosition"), PreferredAttackPosition);
    }
}

void ACombatAIController::HandleLostTarget()
{
    CurrentTarget = nullptr;
    SetCombatState(ECombatState::Idle);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), false);
    }
}

void ACombatAIController::ExecuteCombatBehavior()
{
    if (!CurrentTarget) return;
    
    // Pack coordination
    if (bUsePackTactics)
    {
        CoordinatePackAttack(CurrentTarget);
    }
    
    // Individual tactical decisions
    if (ShouldRetreat())
    {
        SetCombatState(ECombatState::Fleeing);
    }
    else if (ShouldAttack())
    {
        SetCombatState(ECombatState::Attacking);
    }
}