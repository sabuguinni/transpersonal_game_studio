#include "Combat_DinosaurCombatController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_DinosaurCombatController::ACombat_DinosaurCombatController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize combat state
    CombatState = FCombat_CombatState();
    
    // Setup default attack patterns
    FCombat_AttackPattern BasicBite;
    BasicBite.AttackName = TEXT("Bite");
    BasicBite.Damage = 35.0f;
    BasicBite.Range = 150.0f;
    BasicBite.Cooldown = 2.5f;
    BasicBite.WindupTime = 0.8f;
    AttackPatterns.Add(BasicBite);

    FCombat_AttackPattern TailSwipe;
    TailSwipe.AttackName = TEXT("Tail Swipe");
    TailSwipe.Damage = 25.0f;
    TailSwipe.Range = 200.0f;
    TailSwipe.Cooldown = 3.0f;
    TailSwipe.WindupTime = 1.2f;
    AttackPatterns.Add(TailSwipe);

    FCombat_AttackPattern Charge;
    Charge.AttackName = TEXT("Charge");
    Charge.Damage = 50.0f;
    Charge.Range = 400.0f;
    Charge.Cooldown = 5.0f;
    Charge.WindupTime = 1.5f;
    AttackPatterns.Add(Charge);

    // Initialize perception settings
    SightRadius = 1500.0f;
    LoseSightRadius = 2000.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 1200.0f;

    SetupPerception();
}

void ACombat_DinosaurCombatController::BeginPlay()
{
    Super::BeginPlay();

    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Initialize combat state
    CombatState.bIsInCombat = false;
    CombatState.bCanAttack = true;
    LastAttackTime = 0.0f;
    CurrentTarget = nullptr;
}

void ACombat_DinosaurCombatController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatState(DeltaTime);
    HandleCombatLogic();
    UpdateCombatBlackboard();

    // Update distance to target
    if (CurrentTarget && GetPawn())
    {
        DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    }
}

void ACombat_DinosaurCombatController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Setup perception for the possessed pawn
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatController::OnTargetPerceptionUpdated);
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatController::OnPerceptionUpdated);
    }
}

void ACombat_DinosaurCombatController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Setup sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Setup hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ACombat_DinosaurCombatController::SetCombatMode(ECombat_CombatMode NewMode)
{
    CombatState.CombatMode = NewMode;

    // Adjust aggression based on mode
    switch (NewMode)
    {
    case ECombat_CombatMode::Passive:
        CombatState.AggressionLevel = 0.2f;
        break;
    case ECombat_CombatMode::Defensive:
        CombatState.AggressionLevel = 0.5f;
        break;
    case ECombat_CombatMode::Aggressive:
        CombatState.AggressionLevel = 0.8f;
        break;
    case ECombat_CombatMode::Berserk:
        CombatState.AggressionLevel = 1.0f;
        break;
    }
}

void ACombat_DinosaurCombatController::UpdateThreatLevel(float NewThreatLevel)
{
    CombatState.ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 1.0f);

    // Auto-adjust combat mode based on threat level
    if (CombatState.ThreatLevel > 0.8f)
    {
        SetCombatMode(ECombat_CombatMode::Berserk);
    }
    else if (CombatState.ThreatLevel > 0.6f)
    {
        SetCombatMode(ECombat_CombatMode::Aggressive);
    }
    else if (CombatState.ThreatLevel > 0.3f)
    {
        SetCombatMode(ECombat_CombatMode::Defensive);
    }
    else
    {
        SetCombatMode(ECombat_CombatMode::Passive);
    }
}

void ACombat_DinosaurCombatController::EnterCombat(AActor* Target)
{
    if (!Target)
        return;

    CurrentTarget = Target;
    CombatState.bIsInCombat = true;
    
    // Increase threat level when entering combat
    UpdateThreatLevel(FMath::Min(CombatState.ThreatLevel + 0.3f, 1.0f));

    // Set focus on target
    SetFocus(Target);

    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombat: %s entering combat with %s"), 
           *GetPawn()->GetName(), *Target->GetName());
}

void ACombat_DinosaurCombatController::ExitCombat()
{
    CombatState.bIsInCombat = false;
    CurrentTarget = nullptr;
    
    // Gradually reduce threat level
    UpdateThreatLevel(FMath::Max(CombatState.ThreatLevel - 0.2f, 0.0f));

    // Clear focus
    ClearFocus(EAIFocusPriority::Gameplay);

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombat: %s exiting combat"), *GetPawn()->GetName());
}

bool ACombat_DinosaurCombatController::CanAttackTarget() const
{
    if (!CurrentTarget || !CombatState.bCanAttack)
        return false;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < CombatState.AttackCooldown)
        return false;

    return DistanceToTarget <= CombatState.CombatRange;
}

void ACombat_DinosaurCombatController::ExecuteAttack(int32 AttackIndex)
{
    if (!CanAttackTarget() || AttackIndex >= AttackPatterns.Num())
        return;

    FCombat_AttackPattern& Attack = AttackPatterns[AttackIndex];
    
    // Check if target is in range for this specific attack
    if (DistanceToTarget > Attack.Range)
        return;

    // Execute attack logic here
    LastAttackTime = GetWorld()->GetTimeSeconds();
    CombatState.bCanAttack = false;

    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombat: %s executing %s attack on %s"), 
           *GetPawn()->GetName(), *Attack.AttackName, *CurrentTarget->GetName());

    // Reset attack cooldown after attack duration
    FTimerHandle AttackCooldownTimer;
    GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimer, [this]()
    {
        CombatState.bCanAttack = true;
    }, Attack.Cooldown, false);
}

FCombat_AttackPattern ACombat_DinosaurCombatController::GetBestAttackForRange(float Range) const
{
    FCombat_AttackPattern BestAttack;
    float BestScore = -1.0f;

    for (const FCombat_AttackPattern& Attack : AttackPatterns)
    {
        if (Range <= Attack.Range)
        {
            // Score based on damage and range efficiency
            float Score = Attack.Damage / Attack.Cooldown;
            if (Score > BestScore)
            {
                BestScore = Score;
                BestAttack = Attack;
            }
        }
    }

    return BestAttack;
}

FVector ACombat_DinosaurCombatController::CalculateFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();

    // Calculate position to the side of the target
    FVector RightVector = TargetForward.RotateAngleAxis(90.0f, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (RightVector * 300.0f);

    return FlankPosition;
}

bool ACombat_DinosaurCombatController::ShouldRetreat() const
{
    // Retreat if health is low or heavily outnumbered
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
        return false;

    // Simple retreat logic - can be expanded
    return CombatState.ThreatLevel > 0.9f && CombatState.AggressionLevel < 0.6f;
}

void ACombat_DinosaurCombatController::UpdateCombatBlackboard()
{
    if (!BlackboardComponent)
        return;

    // Update blackboard with current combat state
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), CombatState.bIsInCombat);
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), CombatState.AggressionLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), CombatState.ThreatLevel);
    BlackboardComponent->SetValueAsBool(TEXT("CanAttack"), CanAttackTarget());
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), DistanceToTarget);
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
        BlackboardComponent->ClearValue(TEXT("TargetLocation"));
    }
}

void ACombat_DinosaurCombatController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Process all perceived actors for threat assessment
    ProcessThreatAssessment();
}

void ACombat_DinosaurCombatController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor was detected
        float ThreatScore = CalculateThreatScore(Actor);
        
        if (ThreatScore > 0.5f && !CombatState.bIsInCombat)
        {
            EnterCombat(Actor);
        }
    }
    else
    {
        // Actor was lost
        if (Actor == CurrentTarget)
        {
            ExitCombat();
        }
    }
}

void ACombat_DinosaurCombatController::UpdateCombatState(float DeltaTime)
{
    // Gradually decay threat level over time when not in active combat
    if (!CombatState.bIsInCombat)
    {
        CombatState.ThreatLevel = FMath::Max(0.0f, CombatState.ThreatLevel - (DeltaTime * 0.1f));
    }

    // Update attack cooldown
    if (!CombatState.bCanAttack)
    {
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (TimeSinceLastAttack >= CombatState.AttackCooldown)
        {
            CombatState.bCanAttack = true;
        }
    }
}

void ACombat_DinosaurCombatController::ProcessThreatAssessment()
{
    if (!AIPerceptionComponent)
        return;

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    AActor* HighestThreatTarget = nullptr;
    float HighestThreatScore = 0.0f;

    for (AActor* Actor : PerceivedActors)
    {
        float ThreatScore = CalculateThreatScore(Actor);
        if (ThreatScore > HighestThreatScore)
        {
            HighestThreatScore = ThreatScore;
            HighestThreatTarget = Actor;
        }
    }

    // Switch targets if a higher threat is detected
    if (HighestThreatTarget && HighestThreatTarget != CurrentTarget && HighestThreatScore > 0.6f)
    {
        EnterCombat(HighestThreatTarget);
    }
}

void ACombat_DinosaurCombatController::HandleCombatLogic()
{
    if (!CombatState.bIsInCombat || !CurrentTarget)
        return;

    // Check if target is still valid and in range
    if (!IsValid(CurrentTarget))
    {
        ExitCombat();
        return;
    }

    // Update distance and check if we should attack
    if (CanAttackTarget())
    {
        // Choose best attack for current range
        FCombat_AttackPattern BestAttack = GetBestAttackForRange(DistanceToTarget);
        if (!BestAttack.AttackName.IsEmpty())
        {
            ExecuteAttack(0); // Use first available attack for now
        }
    }

    // Check if we should retreat
    if (ShouldRetreat())
    {
        ExitCombat();
        // TODO: Implement retreat behavior
    }
}

AActor* ACombat_DinosaurCombatController::FindBestTarget() const
{
    if (!AIPerceptionComponent)
        return nullptr;

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    AActor* BestTarget = nullptr;
    float BestScore = 0.0f;

    for (AActor* Actor : PerceivedActors)
    {
        float Score = CalculateThreatScore(Actor);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}

float ACombat_DinosaurCombatController::CalculateThreatScore(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return 0.0f;

    float ThreatScore = 0.0f;

    // Check if it's a player character
    if (Actor->IsA<ACharacter>())
    {
        ThreatScore += 0.7f;
    }

    // Distance factor - closer targets are more threatening
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.3f;

    // Aggression modifier
    ThreatScore *= CombatState.AggressionLevel;

    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}