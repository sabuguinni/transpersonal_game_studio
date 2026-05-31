#include "Combat_TacticalAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAI::ACombat_TacticalAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    DinosaurType = ECombat_DinosaurType::Predator_Alpha;
    CurrentTacticalState = ECombat_TacticalState::Patrol;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    StateChangeTime = 0.0f;
    LastPerceptionUpdate = 0.0f;
    CombatDecisionCooldown = 1.0f;
    SuccessfulAttacks = 0;
    DamageDealt = 0;
    CombatDuration = 0.0f;

    // Initialize tactical data based on dinosaur type
    TacticalData = FCombat_TacticalData();
}

void ACombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();

    InitializePerception();

    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Set initial blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
        BlackboardComponent->SetValueAsEnum(TEXT("DinosaurType"), static_cast<uint8>(DinosaurType));
    }
}

void ACombat_TacticalAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatBehavior(DeltaTime);
    
    if (CurrentTacticalState == ECombat_TacticalState::Combat)
    {
        CombatDuration += DeltaTime;
    }
}

void ACombat_TacticalAI::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure sight perception
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(10.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        
        // Detect players and other dinosaurs
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing perception
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 2000.0f;
        HearingConfig->SetMaxAge(5.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAI::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAI::OnTargetPerceptionUpdated);
}

void ACombat_TacticalAI::UpdateCombatBehavior(float DeltaTime)
{
    if (GetWorld()->GetTimeSeconds() - LastPerceptionUpdate < CombatDecisionCooldown)
        return;

    ProcessThreatAssessment();
    ExecuteTacticalDecision();
    UpdatePackCoordination();

    LastPerceptionUpdate = GetWorld()->GetTimeSeconds();
}

void ACombat_TacticalAI::ProcessThreatAssessment()
{
    if (!AIPerceptionComponent)
        return;

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISightConfig::StaticClass(), PerceivedActors);

    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;

    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == GetPawn())
            continue;

        float ThreatLevel = CalculateThreatLevel(Actor);
        if (ThreatLevel > HighestThreatLevel)
        {
            HighestThreatLevel = ThreatLevel;
            HighestThreat = Actor;
        }
    }

    // Update current target
    if (HighestThreat != CurrentTarget)
    {
        CurrentTarget = HighestThreat;
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        }
    }
}

void ACombat_TacticalAI::ExecuteTacticalDecision()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Patrol);
        return;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Patrol:
            if (CurrentTarget)
            {
                SetTacticalState(ECombat_TacticalState::Alert);
            }
            break;

        case ECombat_TacticalState::Alert:
            if (DistanceToTarget < TacticalData.AttackRange * 2.0f)
            {
                SetTacticalState(ECombat_TacticalState::Hunting);
            }
            break;

        case ECombat_TacticalState::Hunting:
            if (DistanceToTarget < TacticalData.AttackRange)
            {
                SetTacticalState(ECombat_TacticalState::Combat);
            }
            else if (DinosaurType == ECombat_DinosaurType::Predator_Pack && PackMembers.Num() >= 2)
            {
                SetTacticalState(ECombat_TacticalState::Flanking);
            }
            break;

        case ECombat_TacticalState::Combat:
            if (ShouldRetreat())
            {
                SetTacticalState(ECombat_TacticalState::Retreat);
            }
            else if (DistanceToTarget > TacticalData.AttackRange * 1.5f)
            {
                SetTacticalState(ECombat_TacticalState::Hunting);
            }
            break;

        case ECombat_TacticalState::Flanking:
            if (DistanceToTarget < TacticalData.AttackRange)
            {
                SetTacticalState(ECombat_TacticalState::Combat);
            }
            break;

        case ECombat_TacticalState::Retreat:
            if (DistanceToTarget > TacticalData.AttackRange * 3.0f)
            {
                SetTacticalState(ECombat_TacticalState::Patrol);
            }
            break;

        case ECombat_TacticalState::Ambush:
            if (DistanceToTarget < TacticalData.AttackRange)
            {
                SetTacticalState(ECombat_TacticalState::Combat);
            }
            break;
    }
}

void ACombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState == NewState)
        return;

    CurrentTacticalState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
    }

    // Execute state-specific logic
    switch (NewState)
    {
        case ECombat_TacticalState::Combat:
            if (TacticalData.bCanCallReinforcements && PackMembers.Num() < TacticalData.PackSize)
            {
                CallForReinforcements();
            }
            break;

        case ECombat_TacticalState::Flanking:
            ExecuteFlankingManeuver();
            break;

        case ECombat_TacticalState::Retreat:
            BreakPackFormation();
            break;

        default:
            break;
    }
}

void ACombat_TacticalAI::EngageTarget(AActor* Target)
{
    CurrentTarget = Target;
    SetTacticalState(ECombat_TacticalState::Hunting);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
    }
}

void ACombat_TacticalAI::ExecuteFlankingManeuver()
{
    if (!CurrentTarget || !GetPawn())
        return;

    FVector FlankingPosition = CalculateFlankingPosition(CurrentTarget);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("FlankingPosition"), FlankingPosition);
    }

    // Coordinate with pack members
    CoordinatePackAttack();
}

void ACombat_TacticalAI::CallForReinforcements()
{
    if (!GetWorld())
        return;

    // Find nearby AI controllers of the same type
    TArray<AActor*> NearbyAIs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombat_TacticalAI::StaticClass(), NearbyAIs);

    for (AActor* Actor : NearbyAIs)
    {
        ACombat_TacticalAI* OtherAI = Cast<ACombat_TacticalAI>(Actor);
        if (!OtherAI || OtherAI == this || OtherAI->DinosaurType != DinosaurType)
            continue;

        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherAI->GetPawn()->GetActorLocation());
        if (Distance < 2000.0f && PackMembers.Num() < TacticalData.PackSize)
        {
            PackMembers.AddUnique(OtherAI->GetPawn());
            OtherAI->PackMembers.AddUnique(GetPawn());
            
            if (CurrentTarget)
            {
                OtherAI->EngageTarget(CurrentTarget);
            }
        }
    }
}

bool ACombat_TacticalAI::ShouldRetreat() const
{
    if (!GetPawn())
        return false;

    // Check health percentage (assuming health component exists)
    ACharacter* Character = Cast<ACharacter>(GetPawn());
    if (Character)
    {
        // Simple health check - would need actual health component in real implementation
        return false; // Placeholder
    }

    // Check if outnumbered significantly
    if (PackMembers.Num() < 2 && DinosaurType == ECombat_DinosaurType::Predator_Pack)
    {
        return true;
    }

    return false;
}

FVector ACombat_TacticalAI::GetOptimalAttackPosition() const
{
    if (!CurrentTarget || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Position slightly to the side for better attack angle
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * TacticalData.AttackRange * 0.8f);
    OptimalPosition += RightVector * 200.0f; // Offset to the side

    return OptimalPosition;
}

void ACombat_TacticalAI::UpdatePackCoordination()
{
    if (PackMembers.Num() == 0)
        return;

    // Remove invalid pack members
    PackMembers.RemoveAll([](AActor* Member) {
        return !IsValid(Member);
    });

    // Update pack formation based on current state
    if (CurrentTacticalState == ECombat_TacticalState::Combat || 
        CurrentTacticalState == ECombat_TacticalState::Flanking)
    {
        FormPackFormation();
    }
}

void ACombat_TacticalAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Process newly perceived actors
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor || Actor == GetPawn())
            continue;

        // Check if this is a potential threat
        float ThreatLevel = CalculateThreatLevel(Actor);
        if (ThreatLevel > 0.5f && CurrentTacticalState == ECombat_TacticalState::Patrol)
        {
            SetTacticalState(ECombat_TacticalState::Alert);
        }
    }
}

void ACombat_TacticalAI::OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target acquired or updated
        if (Actor == CurrentTarget)
        {
            // Update target location in blackboard
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Actor->GetActorLocation());
            }
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            CurrentTarget = nullptr;
            if (BlackboardComponent)
            {
                BlackboardComponent->ClearValue(TEXT("TargetActor"));
            }
        }
    }
}

float ACombat_TacticalAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetPawn())
        return 0.0f;

    float ThreatLevel = 0.0f;

    // Base threat for players
    if (Target->IsA<ACharacter>())
    {
        ThreatLevel += 0.8f;
    }

    // Distance factor (closer = higher threat)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 3000.0f), 0.0f, 1.0f);
    ThreatLevel += DistanceFactor * 0.3f;

    // Line of sight bonus
    if (HasLineOfSight(Target))
    {
        ThreatLevel += 0.2f;
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool ACombat_TacticalAI::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= TacticalData.AttackRange;
}

bool ACombat_TacticalAI::HasLineOfSight(AActor* Target) const
{
    if (!Target || !GetPawn() || !GetWorld())
        return false;

    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Target->GetActorLocation();

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.AddIgnoredActor(Target);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    return !bHit;
}

FVector ACombat_TacticalAI::CalculateFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate position to the side of the target
    FVector DirectionToMe = (MyLocation - TargetLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(DirectionToMe, FVector::UpVector);
    
    // Choose left or right flank randomly
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankingPosition = TargetLocation + (RightVector * FlankDirection * TacticalData.FlankingRadius);
    
    return FlankingPosition;
}

void ACombat_TacticalAI::FormPackFormation()
{
    // Implement pack formation logic
    // This would coordinate positions with pack members
}

void ACombat_TacticalAI::CoordinatePackAttack()
{
    // Implement coordinated attack patterns
    // This would synchronize attacks with pack members
}

void ACombat_TacticalAI::BreakPackFormation()
{
    // Clear pack coordination when retreating
    PackMembers.Empty();
}