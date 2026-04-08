#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CombatBehaviorComponent.h"
#include "DinosaurPersonalityComponent.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize core components
    CombatBehavior = CreateDefaultSubobject<UCombatBehaviorComponent>(TEXT("CombatBehavior"));
    Personality = CreateDefaultSubobject<UDinosaurPersonalityComponent>(TEXT("Personality"));
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);
    
    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
    SightConfig->SetMaxAge(LoseTargetTime);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Damage Sense
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
    PrimaryTarget = nullptr;
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
    
    // Start behavior tree
    if (IdleBehaviorTree)
    {
        RunBehaviorTree(IdleBehaviorTree);
    }
    
    // Setup timers
    GetWorld()->GetTimerManager().SetTimer(ThreatAssessmentTimer, this, &ACombatAIController::UpdateThreatAssessment, 0.5f, true);
    GetWorld()->GetTimerManager().SetTimer(StateUpdateTimer, [this]() { SelectBehaviorTree(); }, 1.0f, true);
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update blackboard with current state
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        Blackboard->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
        Blackboard->SetValueAsObject(TEXT("PrimaryTarget"), PrimaryTarget);
        
        if (PrimaryTarget)
        {
            Blackboard->SetValueAsVector(TEXT("TargetLocation"), PrimaryTarget->GetActorLocation());
            Blackboard->SetValueAsFloat(TEXT("DistanceToTarget"), 
                FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation()));
        }
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Initialize personality component with pawn reference
    if (Personality && InPawn)
    {
        Personality->InitializePersonality(InPawn);
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        HandleCombatStateTransition(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("%s: Combat state changed from %d to %d"), 
            *GetPawn()->GetName(), static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ACombatAIController::SetThreatLevel(EThreatLevel NewLevel)
{
    if (CurrentThreatLevel != NewLevel)
    {
        CurrentThreatLevel = NewLevel;
        
        // Adjust perception based on threat level
        if (AIPerceptionComponent)
        {
            float ThreatMultiplier = 1.0f;
            switch (NewLevel)
            {
                case EThreatLevel::High:
                    ThreatMultiplier = 1.5f;
                    break;
                case EThreatLevel::Critical:
                    ThreatMultiplier = 2.0f;
                    break;
                default:
                    break;
            }
            
            // Increase perception radius when threatened
            AIPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
        }
    }
}

void ACombatAIController::SetPrimaryTarget(AActor* NewTarget)
{
    if (PrimaryTarget != NewTarget)
    {
        PrimaryTarget = NewTarget;
        
        if (NewTarget)
        {
            AddKnownThreat(NewTarget);
            
            // Determine appropriate combat state based on target
            if (ShouldFlee())
            {
                SetCombatState(ECombatState::Fleeing);
            }
            else if (ShouldAttack())
            {
                SetCombatState(ECombatState::Attacking);
            }
            else
            {
                SetCombatState(ECombatState::Stalking);
            }
        }
        else
        {
            SetCombatState(ECombatState::Idle);
        }
    }
}

void ACombatAIController::AddKnownThreat(AActor* Threat)
{
    if (Threat && !KnownThreats.Contains(Threat))
    {
        KnownThreats.Add(Threat);
        UpdateThreatAssessment();
    }
}

void ACombatAIController::RemoveKnownThreat(AActor* Threat)
{
    if (KnownThreats.Remove(Threat) > 0)
    {
        if (PrimaryTarget == Threat)
        {
            PrimaryTarget = GetNearestThreat();
        }
        UpdateThreatAssessment();
    }
}

AActor* ACombatAIController::GetNearestThreat() const
{
    if (KnownThreats.Num() == 0 || !GetPawn())
        return nullptr;
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = FLT_MAX;
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(MyLocation, Threat->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestThreat = Threat;
            }
        }
    }
    
    return NearestThreat;
}

bool ACombatAIController::ShouldAttack() const
{
    if (!PrimaryTarget || !GetPawn())
        return false;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation());
    
    // Consider personality, health, and tactical situation
    float HealthPercentage = 1.0f; // TODO: Get actual health from pawn
    float AggressionLevel = Personality ? Personality->GetAggressionLevel() : 0.5f;
    
    bool InAttackRange = DistanceToTarget <= AttackRange;
    bool HealthyEnough = HealthPercentage > FleeHealthThreshold;
    bool AggressiveEnough = AggressionLevel > 0.6f;
    
    return InAttackRange && HealthyEnough && (AggressiveEnough || CurrentThreatLevel >= EThreatLevel::High);
}

bool ACombatAIController::ShouldFlee() const
{
    if (!GetPawn())
        return false;
    
    float HealthPercentage = 1.0f; // TODO: Get actual health from pawn
    float FearLevel = Personality ? Personality->GetFearLevel() : 0.5f;
    
    bool LowHealth = HealthPercentage <= FleeHealthThreshold;
    bool Outnumbered = KnownThreats.Num() > 2;
    bool VeryFearful = FearLevel > 0.8f;
    bool CriticalThreat = CurrentThreatLevel == EThreatLevel::Critical;
    
    return LowHealth || (Outnumbered && VeryFearful) || CriticalThreat;
}

bool ACombatAIController::ShouldStalk() const
{
    if (!PrimaryTarget || !GetPawn())
        return false;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation());
    float CunningLevel = Personality ? Personality->GetCunningLevel() : 0.5f;
    
    bool TooFarToAttack = DistanceToTarget > AttackRange;
    bool NotFleeingOrAttacking = CurrentCombatState != ECombatState::Fleeing && CurrentCombatState != ECombatState::Attacking;
    bool CleverEnough = CunningLevel > 0.4f;
    
    return TooFarToAttack && NotFleeingOrAttacking && CleverEnough;
}

bool ACombatAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
        return false;
    
    FActorPerceptionBlueprintInfo Info;
    return AIPerceptionComponent->GetActorsPerception(Target, Info) && 
           Info.LastSensedStimuli.Num() > 0 && 
           Info.LastSensedStimuli[0].WasSuccessfullySensed();
}

FVector ACombatAIController::GetOptimalAttackPosition() const
{
    if (!PrimaryTarget || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Position at optimal combat distance
    return TargetLocation - (DirectionToTarget * OptimalCombatDistance);
}

FVector ACombatAIController::GetFleeDirection() const
{
    if (!GetPawn())
        return FVector::ZeroVector;
    
    FVector FleeDirection = FVector::ZeroVector;
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate direction away from all threats
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat))
        {
            FVector ThreatDirection = (MyLocation - Threat->GetActorLocation()).GetSafeNormal();
            float ThreatWeight = CalculateThreatScore(Threat);
            FleeDirection += ThreatDirection * ThreatWeight;
        }
    }
    
    return FleeDirection.GetSafeNormal();
}

FVector ACombatAIController::GetStalkingPosition() const
{
    if (!PrimaryTarget || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    // TODO: Use EQS to find optimal stalking position
    // For now, return a position that maintains distance but keeps line of sight
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Maintain stalking distance
    float StalkingDistance = OptimalCombatDistance * 1.5f;
    return TargetLocation - (DirectionToTarget * StalkingDistance);
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            FActorPerceptionBlueprintInfo Info;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Info))
            {
                bool bCanSense = false;
                for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
                {
                    if (Stimulus.WasSuccessfullySensed())
                    {
                        bCanSense = true;
                        break;
                    }
                }
                
                if (bCanSense)
                {
                    AddKnownThreat(Actor);
                    if (!PrimaryTarget)
                    {
                        SetPrimaryTarget(Actor);
                    }
                }
                else
                {
                    RemoveKnownThreat(Actor);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // React to specific stimulus types
        if (Stimulus.Type == UAISense_Damage::StaticClass()->GetDefaultObject<UAISense_Damage>()->GetSenseID())
        {
            // Immediate threat response to damage
            SetThreatLevel(EThreatLevel::High);
            SetPrimaryTarget(Actor);
        }
        else if (Stimulus.Type == UAISense_Hearing::StaticClass()->GetDefaultObject<UAISense_Hearing>()->GetSenseID())
        {
            // Investigate sound
            if (CurrentCombatState == ECombatState::Idle)
            {
                SetCombatState(ECombatState::Investigating);
            }
        }
    }
}

void ACombatAIController::UpdateThreatAssessment()
{
    if (KnownThreats.Num() == 0)
    {
        SetThreatLevel(EThreatLevel::None);
        return;
    }
    
    float TotalThreatScore = 0.0f;
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat))
        {
            TotalThreatScore += CalculateThreatScore(Threat);
        }
    }
    
    // Determine threat level based on total score
    if (TotalThreatScore > 8.0f)
        SetThreatLevel(EThreatLevel::Critical);
    else if (TotalThreatScore > 5.0f)
        SetThreatLevel(EThreatLevel::High);
    else if (TotalThreatScore > 2.0f)
        SetThreatLevel(EThreatLevel::Medium);
    else
        SetThreatLevel(EThreatLevel::Low);
}

void ACombatAIController::SelectBehaviorTree()
{
    UBehaviorTree* TargetBehaviorTree = nullptr;
    
    switch (CurrentCombatState)
    {
        case ECombatState::Attacking:
        case ECombatState::Stalking:
        case ECombatState::Hunting:
            TargetBehaviorTree = CombatBehaviorTree;
            break;
        case ECombatState::Fleeing:
            TargetBehaviorTree = FleeingBehaviorTree;
            break;
        default:
            TargetBehaviorTree = IdleBehaviorTree;
            break;
    }
    
    if (TargetBehaviorTree && GetBehaviorTreeComponent() && 
        GetBehaviorTreeComponent()->GetCurrentTree() != TargetBehaviorTree)
    {
        RunBehaviorTree(TargetBehaviorTree);
    }
}

float ACombatAIController::CalculateThreatScore(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return 0.0f;
    
    float ThreatScore = 1.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1000.0f / FMath::Max(Distance, 100.0f), 0.1f, 3.0f);
    
    // Size factor (larger = more threatening)
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        float SizeFactor = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() / 50.0f;
        ThreatScore *= SizeFactor;
    }
    
    // Line of sight factor
    if (CanSeeTarget(Actor))
    {
        ThreatScore *= 1.5f;
    }
    
    return ThreatScore * DistanceFactor;
}

void ACombatAIController::HandleCombatStateTransition(ECombatState NewState)
{
    // Adjust movement speed based on state
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
        if (Movement)
        {
            switch (NewState)
            {
                case ECombatState::Attacking:
                    Movement->MaxWalkSpeed = 600.0f;
                    break;
                case ECombatState::Fleeing:
                    Movement->MaxWalkSpeed = 800.0f;
                    break;
                case ECombatState::Stalking:
                    Movement->MaxWalkSpeed = 200.0f;
                    break;
                default:
                    Movement->MaxWalkSpeed = 400.0f;
                    break;
            }
        }
    }
    
    // Update behavior tree selection
    SelectBehaviorTree();
}