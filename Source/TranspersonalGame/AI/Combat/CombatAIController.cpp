#include "CombatAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize perception
    InitializePerception();
    
    // Set as dominant sense for location updates
    SetPerceptionComponent(*AIPerceptionComponent);
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
    if (CombatBehaviorTree)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
    
    // Initialize blackboard values
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsEnum(CombatStateKey, static_cast<uint8>(ECombatState::Idle));
        BlackboardComp->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(EThreatLevel::None));
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTacticalState();
    
    // Debug visualization in development builds
#if WITH_EDITOR
    if (GetTarget())
    {
        DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetTarget()->GetActorLocation(), 
                     FColor::Red, false, 0.1f, 0, 2.0f);
        
        // Draw combat range
        DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation(), AttackRange, 12, 
                       FColor::Orange, false, 0.1f, 0, 1.0f);
    }
#endif
}

void ACombatAIController::InitializePerception()
{
    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f; // 20% larger lose sight radius
    SightConfig->PeripheralVisionAngleDegrees = 120.0f; // Wide field of view for predators
    SightConfig->SetMaxAge(5.0f); // Remember seen targets for 5 seconds
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f); // Remember damage for 10 seconds
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    
    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
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
                if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
                {
                    ProcessSightStimulus(Actor, Stimulus);
                }
                else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
                {
                    ProcessHearingStimulus(Actor, Stimulus);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    
    // Update last known location
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsVector(LastKnownLocationKey, Stimulus.StimulusLocation);
    }
    
    // If we lost sight of our current target
    if (Actor == GetTarget() && !Stimulus.WasSuccessfullySensed())
    {
        SetCombatState(ECombatState::Hunting);
        SetThreatLevel(EThreatLevel::Medium);
    }
}

void ACombatAIController::ProcessSightStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (!Actor || !Stimulus.WasSuccessfullySensed()) return;
    
    // Check if this is a player or hostile target
    if (APawn* SeenPawn = Cast<APawn>(Actor))
    {
        if (SeenPawn->IsPlayerControlled() || SeenPawn->GetActorTag().Contains(TEXT("Hostile")))
        {
            SetTarget(Actor);
            SetThreatLevel(EThreatLevel::High);
            
            float Distance = GetDistanceToTarget();
            if (Distance <= AttackRange)
            {
                SetCombatState(ECombatState::Engaging);
            }
            else if (Distance <= OptimalCombatDistance)
            {
                SetCombatState(ECombatState::Stalking);
            }
            else
            {
                SetCombatState(ECombatState::Hunting);
            }
            
            // Alert nearby allies
            AlertNearbyAllies(Actor->GetActorLocation());
        }
    }
}

void ACombatAIController::ProcessHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (!Actor || !Stimulus.WasSuccessfullySensed()) return;
    
    // If we don't have a target, investigate the sound
    if (!GetTarget())
    {
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(LastKnownLocationKey, Stimulus.StimulusLocation);
        }
        SetCombatState(ECombatState::Hunting);
        SetThreatLevel(EThreatLevel::Low);
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(CombatStateKey, static_cast<uint8>(NewState));
        }
    }
}

ECombatState ACombatAIController::GetCombatState() const
{
    return CurrentCombatState;
}

void ACombatAIController::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(NewThreatLevel));
        }
    }
}

EThreatLevel ACombatAIController::GetThreatLevel() const
{
    return CurrentThreatLevel;
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject(TargetActorKey, NewTarget);
    }
}

AActor* ACombatAIController::GetTarget() const
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        return Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey));
    }
    return nullptr;
}

void ACombatAIController::ClearTarget()
{
    SetTarget(nullptr);
    SetCombatState(ECombatState::Idle);
    SetThreatLevel(EThreatLevel::None);
}

bool ACombatAIController::ShouldEngageTarget() const
{
    if (!GetTarget()) return false;
    
    float Distance = GetDistanceToTarget();
    float HealthPercent = GetHealthPercentage();
    
    // Don't engage if health is too low and we can retreat
    if (bCanRetreat && HealthPercent < RetreatThreshold)
    {
        return false;
    }
    
    // Engage if target is in attack range
    if (Distance <= AttackRange)
    {
        return true;
    }
    
    // Consider aggression level for longer range engagements
    return (Distance <= OptimalCombatDistance) && (AggressionLevel > 0.5f);
}

bool ACombatAIController::ShouldRetreat() const
{
    if (!bCanRetreat || !GetTarget()) return false;
    
    float HealthPercent = GetHealthPercentage();
    float Distance = GetDistanceToTarget();
    
    // Retreat if health is below threshold
    if (HealthPercent < RetreatThreshold)
    {
        return true;
    }
    
    // Retreat if target is too close and we're not aggressive
    if (Distance < AttackRange * 0.5f && AggressionLevel < 0.3f)
    {
        return true;
    }
    
    return false;
}

bool ACombatAIController::ShouldFlank() const
{
    if (!bCanFlank || !GetTarget()) return false;
    
    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFlankingTime < FlankingCooldown)
    {
        return false;
    }
    
    float Distance = GetDistanceToTarget();
    
    // Flank if we're at optimal distance and have line of sight
    return (Distance > AttackRange && Distance <= FlankingDistance) && HasLineOfSightToTarget();
}

FVector ACombatAIController::GetFlankingPosition() const
{
    if (!GetTarget() || !GetPawn()) return FVector::ZeroVector;
    
    FVector TargetLocation = GetTarget()->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flanking
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankingDistance);
    
    // Try to find a valid navigation point near the flanking position
    FNavLocation NavLocation;
    if (GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(FlankPosition, NavLocation, FVector(500.0f)))
    {
        return NavLocation.Location;
    }
    
    return FlankPosition;
}

FVector ACombatAIController::GetRetreatPosition() const
{
    if (!GetTarget() || !GetPawn()) return FVector::ZeroVector;
    
    FVector TargetLocation = GetTarget()->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector RetreatDirection = (MyLocation - TargetLocation).GetSafeNormal();
    
    FVector RetreatPosition = MyLocation + (RetreatDirection * FlankingDistance * 1.5f);
    
    // Try to find a valid navigation point
    FNavLocation NavLocation;
    if (GetWorld()->GetNavigationSystem()->ProjectPointToNavigation(RetreatPosition, NavLocation, FVector(500.0f)))
    {
        return NavLocation.Location;
    }
    
    return RetreatPosition;
}

float ACombatAIController::GetDistanceToTarget() const
{
    if (!GetTarget() || !GetPawn()) return 0.0f;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), GetTarget()->GetActorLocation());
}

bool ACombatAIController::HasLineOfSightToTarget() const
{
    if (!GetTarget() || !GetPawn()) return false;
    
    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = GetTarget()->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit || HitResult.GetActor() == GetTarget();
}

float ACombatAIController::GetHealthPercentage() const
{
    if (!GetPawn()) return 1.0f;
    
    if (UHealthComponent* HealthComp = GetPawn()->FindComponentByClass<UHealthComponent>())
    {
        return HealthComp->GetHealthPercentage();
    }
    
    // Fallback to character health if available
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Assuming a basic health system - adapt as needed
        return 1.0f; // Placeholder
    }
    
    return 1.0f;
}

void ACombatAIController::AlertNearbyAllies(const FVector& ThreatLocation, float AlertRadius)
{
    if (!bCanCallForHelp) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAlertTime < AlertCooldown)
    {
        return;
    }
    
    LastAlertTime = CurrentTime;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetPawn()->GetClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetPawn())
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= AlertRadius)
            {
                if (ACombatAIController* AllyController = Cast<ACombatAIController>(Cast<APawn>(Actor)->GetController()))
                {
                    // Alert the ally about the threat
                    if (UBlackboardComponent* AllyBlackboard = AllyController->GetBlackboardComponent())
                    {
                        AllyBlackboard->SetValueAsVector(AllyController->LastKnownLocationKey, ThreatLocation);
                    }
                    AllyController->SetCombatState(ECombatState::Hunting);
                    AllyController->SetThreatLevel(EThreatLevel::Medium);
                }
            }
        }
    }
}

void ACombatAIController::UpdateTacticalState()
{
    if (!GetTarget()) return;
    
    // Update threat level based on current situation
    EThreatLevel NewThreatLevel = CalculateThreatLevel();
    SetThreatLevel(NewThreatLevel);
    
    // Update flanking position if needed
    if (ShouldFlank())
    {
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(FlankingPositionKey, GetFlankingPosition());
        }
        LastFlankingTime = GetWorld()->GetTimeSeconds();
    }
    
    // Update retreat position if needed
    if (ShouldRetreat())
    {
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(RetreatPositionKey, GetRetreatPosition());
        }
        SetCombatState(ECombatState::Retreating);
        LastRetreatTime = GetWorld()->GetTimeSeconds();
    }
}

EThreatLevel ACombatAIController::CalculateThreatLevel() const
{
    if (!GetTarget()) return EThreatLevel::None;
    
    float Distance = GetDistanceToTarget();
    float HealthPercent = GetHealthPercentage();
    bool bHasLineOfSight = HasLineOfSightToTarget();
    
    // Critical threat: Very close, low health
    if (Distance < AttackRange * 0.5f && HealthPercent < 0.3f)
    {
        return EThreatLevel::Critical;
    }
    
    // High threat: In combat range with line of sight
    if (Distance <= AttackRange && bHasLineOfSight)
    {
        return EThreatLevel::High;
    }
    
    // Medium threat: Close but not in immediate danger
    if (Distance <= OptimalCombatDistance)
    {
        return EThreatLevel::Medium;
    }
    
    // Low threat: Distant or no line of sight
    if (Distance > OptimalCombatDistance || !bHasLineOfSight)
    {
        return EThreatLevel::Low;
    }
    
    return EThreatLevel::None;
}

bool ACombatAIController::IsInCombatRange() const
{
    return GetDistanceToTarget() <= AttackRange;
}

bool ACombatAIController::CanSeeTarget() const
{
    return HasLineOfSightToTarget();
}