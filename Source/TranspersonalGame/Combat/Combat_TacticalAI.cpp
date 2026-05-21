#include "Combat_TacticalAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAI::ACombat_TacticalAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize tactical state
    TacticalState.CurrentBehavior = ECombat_AIBehavior::Patrol;
    TacticalState.AggressionLevel = 0.5f;
    TacticalState.FearLevel = 0.0f;
    TacticalState.HealthPercentage = 1.0f;
    TacticalState.bIsInCombat = false;
    TacticalState.bCanSeePlayer = false;
    TacticalState.DistanceToPlayer = 0.0f;
    TacticalState.TimeSinceLastPlayerSighting = 0.0f;
}

void ACombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    SetupAIPerception();
    
    if (BlackboardAsset && BehaviorTreeAsset)
    {
        UseBlackboard(BlackboardAsset);
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ACombat_TacticalAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTacticalState();
    CalculateAggressionAndFear();
    UpdateBlackboard();
    
    // Decay fear and aggression over time
    TacticalState.FearLevel = FMath::Max(0.0f, TacticalState.FearLevel - (FearDecayRate * DeltaTime));
    TacticalState.AggressionLevel = FMath::Max(0.0f, TacticalState.AggressionLevel - (AggressionDecayRate * DeltaTime));
    
    // Update time since last player sighting
    if (!TacticalState.bCanSeePlayer)
    {
        TacticalState.TimeSinceLastPlayerSighting += DeltaTime;
    }
}

void ACombat_TacticalAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        // Get health percentage from possessed pawn
        if (ACharacter* Character = Cast<ACharacter>(InPawn))
        {
            // Assume health component exists - in real implementation would check for health component
            TacticalState.HealthPercentage = 1.0f;
        }
    }
}

void ACombat_TacticalAI::SetupAIPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Setup sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius + 200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Setup hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAI::OnPlayerSighted);
}

void ACombat_TacticalAI::UpdateTacticalState()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetPawn())
        return;

    // Update distance to player
    TacticalState.DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // Check if player is in sight
    TacticalState.bCanSeePlayer = AIPerceptionComponent->HasActiveStimulus(*PlayerPawn, UAISense::GetSenseID<UAISense_Sight>());
    
    if (TacticalState.bCanSeePlayer)
    {
        TacticalState.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        TacticalState.TimeSinceLastPlayerSighting = 0.0f;
    }

    // Determine if in combat based on distance and sight
    TacticalState.bIsInCombat = TacticalState.bCanSeePlayer && TacticalState.DistanceToPlayer < (SightRadius * 0.8f);

    // Update health percentage (would normally get from health component)
    if (GetPawn())
    {
        // Placeholder - in real implementation would get from health component
        TacticalState.HealthPercentage = FMath::Max(0.0f, TacticalState.HealthPercentage);
    }
}

ECombat_AIBehavior ACombat_TacticalAI::DetermineBestBehavior()
{
    // Flee if health is low
    if (ShouldFlee())
    {
        return ECombat_AIBehavior::Flee;
    }

    // Attack if player is in range and we can see them
    if (ShouldAttack())
    {
        return ECombat_AIBehavior::Attack;
    }

    // Hunt if we recently saw the player but lost sight
    if (TacticalState.TimeSinceLastPlayerSighting < 10.0f && TacticalState.TimeSinceLastPlayerSighting > 0.1f)
    {
        return ECombat_AIBehavior::Hunt;
    }

    // Default to patrol
    return ECombat_AIBehavior::Patrol;
}

void ACombat_TacticalAI::ExecuteTacticalBehavior(ECombat_AIBehavior NewBehavior)
{
    if (TacticalState.CurrentBehavior == NewBehavior)
        return;

    TacticalState.CurrentBehavior = NewBehavior;

    switch (NewBehavior)
    {
        case ECombat_AIBehavior::Attack:
            InitiateAttack();
            break;
        case ECombat_AIBehavior::Flee:
            InitiateFlee();
            break;
        case ECombat_AIBehavior::Hunt:
            InitiateHunt();
            break;
        case ECombat_AIBehavior::Patrol:
            InitiatePatrol();
            break;
        default:
            break;
    }
}

void ACombat_TacticalAI::InitiateAttack()
{
    if (!GetPawn())
        return;

    FVector OptimalPosition = GetOptimalAttackPosition();
    
    // Move to optimal attack position
    MoveToLocation(OptimalPosition, AttackRange * 0.8f);
    
    // Increase aggression
    TacticalState.AggressionLevel = FMath::Min(1.0f, TacticalState.AggressionLevel + 0.3f);
    
    UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Initiating attack behavior"));
}

void ACombat_TacticalAI::InitiateFlee()
{
    if (!GetPawn())
        return;

    FVector FleeDirection = GetFleeDirection();
    FVector FleeTarget = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);
    
    MoveToLocation(FleeTarget);
    
    // Increase fear
    TacticalState.FearLevel = FMath::Min(1.0f, TacticalState.FearLevel + 0.5f);
    
    UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Initiating flee behavior"));
}

void ACombat_TacticalAI::InitiateHunt()
{
    if (!GetPawn())
        return;

    // Move towards last known player location
    if (TacticalState.LastKnownPlayerLocation != FVector::ZeroVector)
    {
        MoveToLocation(TacticalState.LastKnownPlayerLocation, 200.0f);
    }
    
    // Moderate aggression increase
    TacticalState.AggressionLevel = FMath::Min(1.0f, TacticalState.AggressionLevel + 0.1f);
    
    UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Initiating hunt behavior"));
}

void ACombat_TacticalAI::InitiatePatrol()
{
    if (!GetPawn())
        return;

    // Find random patrol point around current location
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector PatrolTarget = CurrentLocation + FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );
    
    MoveToLocation(PatrolTarget, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("TacticalAI: Initiating patrol behavior"));
}

void ACombat_TacticalAI::OnPlayerSighted(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Actor == PlayerPawn)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            TacticalState.bCanSeePlayer = true;
            TacticalState.LastKnownPlayerLocation = Actor->GetActorLocation();
            TacticalState.AggressionLevel = FMath::Min(1.0f, TacticalState.AggressionLevel + 0.2f);
            
            UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Player sighted!"));
        }
        else
        {
            TacticalState.bCanSeePlayer = false;
            UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Player lost from sight"));
        }
    }
}

void ACombat_TacticalAI::OnPlayerLost(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Actor == PlayerPawn)
    {
        TacticalState.bCanSeePlayer = false;
        UE_LOG(LogTemp, Warning, TEXT("TacticalAI: Player completely lost"));
    }
}

bool ACombat_TacticalAI::IsPlayerInAttackRange() const
{
    return TacticalState.DistanceToPlayer <= AttackRange;
}

bool ACombat_TacticalAI::ShouldFlee() const
{
    return TacticalState.HealthPercentage <= FleeHealthThreshold || TacticalState.FearLevel > 0.8f;
}

bool ACombat_TacticalAI::ShouldAttack() const
{
    return TacticalState.bCanSeePlayer && 
           IsPlayerInAttackRange() && 
           TacticalState.AggressionLevel > 0.3f &&
           !ShouldFlee();
}

FVector ACombat_TacticalAI::GetOptimalAttackPosition() const
{
    if (!GetPawn())
        return FVector::ZeroVector;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return GetPawn()->GetActorLocation();

    // Try to flank the player
    FVector FlankingPos = FindFlankingPosition();
    if (FlankingPos != FVector::ZeroVector)
        return FlankingPos;

    // Fallback to direct approach
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector Direction = (GetPawn()->GetActorLocation() - PlayerLocation).GetSafeNormal();
    return PlayerLocation + (Direction * AttackRange * 0.8f);
}

FVector ACombat_TacticalAI::GetFleeDirection() const
{
    if (!GetPawn())
        return FVector::ZeroVector;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return GetPawn()->GetActorForwardVector();

    // Flee away from player
    FVector FleeDirection = (GetPawn()->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
    
    // Add some randomness to avoid predictable movement
    FleeDirection += FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        0.0f
    );
    
    return FleeDirection.GetSafeNormal();
}

FVector ACombat_TacticalAI::FindFlankingPosition()
{
    // Implementation for finding flanking positions around the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetPawn())
        return FVector::ZeroVector;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    
    // Try positions to the left and right of player
    TArray<FVector> FlankingPositions;
    FlankingPositions.Add(PlayerLocation + (PlayerForward.RotateAngleAxis(90.0f, FVector::UpVector) * AttackRange));
    FlankingPositions.Add(PlayerLocation + (PlayerForward.RotateAngleAxis(-90.0f, FVector::UpVector) * AttackRange));
    FlankingPositions.Add(PlayerLocation + (PlayerForward.RotateAngleAxis(135.0f, FVector::UpVector) * AttackRange));
    FlankingPositions.Add(PlayerLocation + (PlayerForward.RotateAngleAxis(-135.0f, FVector::UpVector) * AttackRange));

    // Find the closest reachable flanking position
    for (const FVector& Position : FlankingPositions)
    {
        if (HasClearLineOfSight(Position))
        {
            return Position;
        }
    }

    return FVector::ZeroVector;
}

FVector ACombat_TacticalAI::FindAmbushPosition()
{
    // Implementation for finding ambush positions
    // This would typically involve finding cover points near player patrol routes
    return FVector::ZeroVector;
}

bool ACombat_TacticalAI::HasClearLineOfSight(const FVector& TargetLocation) const
{
    if (!GetPawn())
        return false;

    FHitResult HitResult;
    FVector StartLocation = GetPawn()->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TargetLocation,
        ECollisionChannel::ECC_Visibility
    );

    return !bHit;
}

void ACombat_TacticalAI::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;

    // Update blackboard with current tactical state
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), TacticalState.AggressionLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("FearLevel"), TacticalState.FearLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("HealthPercentage"), TacticalState.HealthPercentage);
    BlackboardComponent->SetValueAsBool(TEXT("CanSeePlayer"), TacticalState.bCanSeePlayer);
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), TacticalState.bIsInCombat);
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToPlayer"), TacticalState.DistanceToPlayer);
    
    if (TacticalState.LastKnownPlayerLocation != FVector::ZeroVector)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), TacticalState.LastKnownPlayerLocation);
    }
}

void ACombat_TacticalAI::CalculateAggressionAndFear()
{
    // Aggression increases when player is close and we're healthy
    if (TacticalState.bCanSeePlayer && TacticalState.HealthPercentage > 0.5f)
    {
        float DistanceFactor = 1.0f - (TacticalState.DistanceToPlayer / SightRadius);
        TacticalState.AggressionLevel += DistanceFactor * 0.01f;
    }

    // Fear increases when health is low or player is very close
    if (TacticalState.HealthPercentage < 0.5f)
    {
        TacticalState.FearLevel += (0.5f - TacticalState.HealthPercentage) * 0.02f;
    }

    if (TacticalState.DistanceToPlayer < AttackRange * 0.5f && TacticalState.bCanSeePlayer)
    {
        TacticalState.FearLevel += 0.01f;
    }

    // Clamp values
    TacticalState.AggressionLevel = FMath::Clamp(TacticalState.AggressionLevel, 0.0f, 1.0f);
    TacticalState.FearLevel = FMath::Clamp(TacticalState.FearLevel, 0.0f, 1.0f);
}