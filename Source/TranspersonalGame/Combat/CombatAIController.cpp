#include "CombatAIController.h"
#include "TacticalBehaviorComponent.h"
#include "CombatStateComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Create combat components
    TacticalBehavior = CreateDefaultSubobject<UCombat_TacticalBehaviorComponent>(TEXT("TacticalBehavior"));
    CombatState = CreateDefaultSubobject<UCombat_CombatStateComponent>(TEXT("CombatState"));

    // Initialize default values
    AIPersonality = ECombat_AIPersonality::Aggressive;
    Aggressiveness = 0.7f;
    TacticalIntelligence = 0.6f;
    DetectionRange = 1500.0f;
    AttackRange = 300.0f;
    FleeHealthThreshold = 0.2f;

    // Internal state
    CurrentTarget = nullptr;
    bInCombatMode = false;
    LastAttackTime = 0.0f;
    LastFlankTime = 0.0f;
    LastKnownTargetLocation = FVector::ZeroVector;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    // Start tactical update timer
    GetWorldTimerManager().SetTimer(TacticalUpdateTimer, this, &ACombat_AIController::UpdateTacticalState, 0.5f, true);
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBlackboardValues();
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a potential target
            if (!CurrentTarget && Actor != GetPawn())
            {
                HandleTargetFound(Actor);
            }
        }
    }
}

void ACombat_AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (Actor != GetPawn())
        {
            HandleTargetFound(Actor);
            LastKnownTargetLocation = Actor->GetActorLocation();
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            HandleTargetLost();
        }
    }
}

void ACombat_AIController::SetCombatTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (CurrentTarget)
    {
        EnterCombatMode();
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    }
    else
    {
        ExitCombatMode();
    }
}

AActor* ACombat_AIController::GetCombatTarget() const
{
    return CurrentTarget;
}

void ACombat_AIController::EnterCombatMode()
{
    if (!bInCombatMode)
    {
        bInCombatMode = true;
        
        // Notify tactical behavior component
        if (TacticalBehavior)
        {
            TacticalBehavior->OnEnterCombat();
        }

        // Notify combat state component
        if (CombatState)
        {
            CombatState->SetCombatState(ECombat_CombatState::Engaging);
        }
    }
}

void ACombat_AIController::ExitCombatMode()
{
    if (bInCombatMode)
    {
        bInCombatMode = false;
        CurrentTarget = nullptr;
        
        // Notify tactical behavior component
        if (TacticalBehavior)
        {
            TacticalBehavior->OnExitCombat();
        }

        // Notify combat state component
        if (CombatState)
        {
            CombatState->SetCombatState(ECombat_CombatState::Idle);
        }
    }
}

bool ACombat_AIController::IsInCombat() const
{
    return bInCombatMode;
}

FVector ACombat_AIController::GetOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Calculate attack position based on attack range
    FVector AttackPosition = TargetLocation - (DirectionToTarget * AttackRange);

    // Add some tactical variation based on AI personality
    if (AIPersonality == ECombat_AIPersonality::Flanker)
    {
        // Flankers prefer side approaches
        FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
        float FlankOffset = FMath::RandBool() ? 200.0f : -200.0f;
        AttackPosition += RightVector * FlankOffset;
    }

    return AttackPosition;
}

FVector ACombat_AIController::GetOptimalFlankPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Calculate flank position 90 degrees to the side
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    
    // Choose left or right flank based on tactical intelligence
    bool bFlankRight = FMath::RandRange(0.0f, 1.0f) < TacticalIntelligence;
    FVector FlankDirection = bFlankRight ? RightVector : -RightVector;

    FVector FlankPosition = TargetLocation + (FlankDirection * AttackRange * 1.5f);

    return FlankPosition;
}

bool ACombat_AIController::ShouldRetreat() const
{
    if (!GetPawn()) return false;

    // Check health threshold
    // Note: This would need to be connected to a health component
    // For now, use a simple distance-based retreat logic
    
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget();
        
        // Retreat if too close and low aggression
        if (DistanceToTarget < AttackRange * 0.5f && Aggressiveness < 0.3f)
        {
            return true;
        }
    }

    return false;
}

bool ACombat_AIController::ShouldCallForHelp() const
{
    if (!CurrentTarget) return false;

    // Call for help based on tactical intelligence and situation
    float DistanceToTarget = GetDistanceToTarget();
    
    // More intelligent AI calls for help more strategically
    if (TacticalIntelligence > 0.7f && DistanceToTarget < AttackRange * 2.0f)
    {
        return true;
    }

    return false;
}

void ACombat_AIController::RequestPackAssistance()
{
    TArray<ACombat_AIController*> PackMembers = GetNearbyPackMembers();
    
    for (ACombat_AIController* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->RespondToPackCall(this);
        }
    }
}

void ACombat_AIController::RespondToPackCall(ACombat_AIController* Caller)
{
    if (!Caller || IsInCombat()) return;

    // Respond to pack call by setting the same target
    if (Caller->GetCombatTarget())
    {
        SetCombatTarget(Caller->GetCombatTarget());
    }
}

TArray<ACombat_AIController*> ACombat_AIController::GetNearbyPackMembers(float SearchRadius)
{
    TArray<ACombat_AIController*> PackMembers;
    
    if (!GetWorld() || !GetPawn()) return PackMembers;

    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find all AI controllers in range
    for (TActorIterator<ACombat_AIController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ACombat_AIController* OtherAI = *ActorItr;
        if (OtherAI && OtherAI != this && OtherAI->GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, OtherAI->GetPawn()->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                PackMembers.Add(OtherAI);
            }
        }
    }

    return PackMembers;
}

void ACombat_AIController::UpdateTacticalState()
{
    if (!GetPawn()) return;

    // Update target validity
    if (CurrentTarget)
    {
        if (!CanSeeTarget(CurrentTarget))
        {
            // Target lost, but keep last known position for a while
            float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - LastAttackTime;
            if (TimeSinceLastSeen > 5.0f)
            {
                HandleTargetLost();
            }
        }
        else
        {
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        }
    }

    // Update combat state based on situation
    if (IsInCombat())
    {
        if (ShouldRetreat())
        {
            if (CombatState)
            {
                CombatState->SetCombatState(ECombat_CombatState::Retreating);
            }
        }
        else if (ShouldCallForHelp())
        {
            RequestPackAssistance();
        }
    }
}

void ACombat_AIController::UpdateBlackboardValues()
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    // Update common blackboard keys
    BlackboardComp->SetValueAsObject(TEXT("Target"), CurrentTarget);
    BlackboardComp->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
    BlackboardComp->SetValueAsBool(TEXT("InCombat"), bInCombatMode);
    BlackboardComp->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    BlackboardComp->SetValueAsBool(TEXT("CanSeeTarget"), CanSeeTarget(CurrentTarget));
}

bool ACombat_AIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent) return false;

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(Target, Info);
    
    return Info.LastSensedStimuli.Num() > 0 && Info.LastSensedStimuli[0].WasSuccessfullySensed();
}

float ACombat_AIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return -1.0f;

    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ACombat_AIController::HandleTargetLost()
{
    CurrentTarget = nullptr;
    ExitCombatMode();
}

void ACombat_AIController::HandleTargetFound(AActor* NewTarget)
{
    SetCombatTarget(NewTarget);
}