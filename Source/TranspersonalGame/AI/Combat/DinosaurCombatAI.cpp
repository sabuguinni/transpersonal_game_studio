#include "DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    CurrentCombatState = EDinosaurCombatState::Idle;
    CurrentTarget = nullptr;
    LastStateChangeTime = 0.0f;

    // Initialize memory
    DinosaurMemory.LastKnownPlayerLocation = FVector::ZeroVector;
    DinosaurMemory.LastSeenPlayerTime = 0.0f;
    DinosaurMemory.PerceivedThreatLevel = EDinosaurThreatLevel::None;
    DinosaurMemory.bHasBeenAttackedByPlayer = false;
    DinosaurMemory.LastDamageTime = 0.0f;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    InitializePerception();

    // Start behavior tree if assigned
    if (CombatBehaviorTree && BlackboardComponent)
    {
        if (CombatBlackboard)
        {
            UseBlackboard(CombatBlackboard);
        }
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Set territory center to spawn location
    DinosaurMemory.TerritoryCenter = GetPawn()->GetActorLocation();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnTargetPerceptionUpdated);
    }
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBlackboardValues();
    ProcessPerceptionData();
    UpdateCombatState();
}

void ADinosaurCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent && CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
    }
}

void ADinosaurCombatAI::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = CombatProfile.HuntingRange;
        SightConfig->LoseSightRadius = CombatProfile.HuntingRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = CombatProfile.HuntingRange * 0.8f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Configure Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    if (DamageConfig)
    {
        DamageConfig->SetMaxAge(10.0f);
        AIPerceptionComponent->ConfigureSense(*DamageConfig);
    }

    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ADinosaurCombatAI::SetCombatState(EDinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        // State-specific logic
        switch (NewState)
        {
        case EDinosaurCombatState::Hunting:
            if (GetPawn())
            {
                if (UCharacterMovementComponent* MovementComp = GetPawn()->FindComponentByClass<UCharacterMovementComponent>())
                {
                    MovementComp->MaxWalkSpeed *= 1.2f; // Increase speed when hunting
                }
            }
            break;

        case EDinosaurCombatState::Stalking:
            if (GetPawn())
            {
                if (UCharacterMovementComponent* MovementComp = GetPawn()->FindComponentByClass<UCharacterMovementComponent>())
                {
                    MovementComp->MaxWalkSpeed *= 0.6f; // Slow and stealthy
                }
            }
            break;

        case EDinosaurCombatState::Attacking:
            LastEngagementStartTime = GetWorld()->GetTimeSeconds();
            break;

        case EDinosaurCombatState::Retreating:
            if (CurrentTarget)
            {
                PlayerEscapeCount++;
                AdjustDifficultyBasedOnPlayerPerformance();
            }
            break;
        }
    }
}

void ADinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }

    if (NewTarget)
    {
        UpdatePlayerMemory(NewTarget->GetActorLocation());
        SetThreatLevel(EDinosaurThreatLevel::Medium);
    }
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Assume this is the player for now
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    SetTarget(Actor);
                    UpdatePlayerMemory(Actor->GetActorLocation());
                    
                    // Determine appropriate state based on distance and current state
                    float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                    
                    if (DistanceToPlayer <= CombatProfile.AttackRange)
                    {
                        SetCombatState(EDinosaurCombatState::Attacking);
                    }
                    else if (DistanceToPlayer <= CombatProfile.HuntingRange * 0.5f)
                    {
                        SetCombatState(EDinosaurCombatState::Stalking);
                    }
                    else
                    {
                        SetCombatState(EDinosaurCombatState::Hunting);
                    }
                }
                else
                {
                    // Lost sight of target
                    if (CurrentCombatState == EDinosaurCombatState::Attacking || 
                        CurrentCombatState == EDinosaurCombatState::Stalking)
                    {
                        SetCombatState(EDinosaurCombatState::Investigating);
                    }
                }
            }
        }
    }
}

void ADinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->IsA<ACharacter>())
        return;

    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        // Dinosaur was damaged
        DinosaurMemory.bHasBeenAttackedByPlayer = true;
        DinosaurMemory.LastDamageTime = GetWorld()->GetTimeSeconds();
        SetThreatLevel(EDinosaurThreatLevel::High);
        
        if (ShouldRetreat())
        {
            SetCombatState(EDinosaurCombatState::Retreating);
        }
        else
        {
            SetCombatState(EDinosaurCombatState::Attacking);
            SetTarget(Actor);
        }
    }
}

bool ADinosaurCombatAI::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
        return false;

    FAIStimulus Stimulus;
    return AIPerceptionComponent->GetActorsPerception(Target, Stimulus) && Stimulus.WasSuccessfullySensed();
}

float ADinosaurCombatAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
        return -1.0f;

    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ADinosaurCombatAI::IsInAttackRange() const
{
    return GetDistanceToTarget() <= CombatProfile.AttackRange && GetDistanceToTarget() > 0.0f;
}

bool ADinosaurCombatAI::ShouldRetreat() const
{
    if (!GetPawn())
        return false;

    // Check health threshold
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Assuming health component exists - adapt as needed
        float HealthPercentage = 1.0f; // Placeholder - implement actual health check
        if (HealthPercentage <= CombatProfile.RetreatHealthThreshold)
        {
            return true;
        }
    }

    // Check if overwhelmed (multiple threats, etc.)
    return false;
}

FVector ADinosaurCombatAI::GetFlankingPosition() const
{
    if (!CurrentTarget || !GetPawn() || !CombatProfile.bCanFlankPlayer)
        return FVector::ZeroVector;

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the right or left
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    bool bFlankRight = FMath::RandBool();
    FVector FlankDirection = bFlankRight ? RightVector : -RightVector;
    
    FVector FlankPosition = TargetLocation + FlankDirection * CombatProfile.AttackRange * 1.5f;
    
    // Ensure position is navigable
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return MyLocation; // Fallback to current position
}

void ADinosaurCombatAI::ExecuteAttack()
{
    // This would trigger attack animations and damage dealing
    // Implementation depends on your combat system
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("ShouldAttack"), true);
    }
}

void ADinosaurCombatAI::CallForHelp()
{
    if (!CombatProfile.bCanCallForHelp)
        return;

    // Find nearby allies and alert them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCombatAI::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (ADinosaurCombatAI* OtherAI = Cast<ADinosaurCombatAI>(Actor))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= CombatProfile.TerritorialRadius && OtherAI != this)
            {
                OtherAI->SetTarget(CurrentTarget);
                OtherAI->SetCombatState(EDinosaurCombatState::Hunting);
            }
        }
    }
}

void ADinosaurCombatAI::UpdatePlayerMemory(FVector PlayerLocation)
{
    DinosaurMemory.LastKnownPlayerLocation = PlayerLocation;
    DinosaurMemory.LastSeenPlayerTime = GetWorld()->GetTimeSeconds();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), PlayerLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("LastSeenPlayerTime"), DinosaurMemory.LastSeenPlayerTime);
    }
}

FVector ADinosaurCombatAI::GetLastKnownPlayerLocation() const
{
    return DinosaurMemory.LastKnownPlayerLocation;
}

void ADinosaurCombatAI::SetThreatLevel(EDinosaurThreatLevel ThreatLevel)
{
    DinosaurMemory.PerceivedThreatLevel = ThreatLevel;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(ThreatLevel));
    }
}

void ADinosaurCombatAI::AdjustDifficultyBasedOnPlayerPerformance()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Calculate average engagement time
    if (LastEngagementStartTime > 0.0f)
    {
        float EngagementDuration = CurrentTime - LastEngagementStartTime;
        AverageEngagementTime = (AverageEngagementTime + EngagementDuration) * 0.5f;
    }
    
    // Adjust AI parameters based on player performance
    float PlayerSuccessRate = static_cast<float>(PlayerEscapeCount) / FMath::Max(1.0f, static_cast<float>(PlayerDeathCount + PlayerEscapeCount));
    
    if (PlayerSuccessRate > 0.7f) // Player is doing well, increase difficulty
    {
        CombatProfile.AggressionLevel = FMath::Min(1.0f, CombatProfile.AggressionLevel + 0.1f);
        CombatProfile.HuntingRange *= 1.1f;
    }
    else if (PlayerSuccessRate < 0.3f) // Player struggling, decrease difficulty
    {
        CombatProfile.AggressionLevel = FMath::Max(0.1f, CombatProfile.AggressionLevel - 0.1f);
        CombatProfile.HuntingRange *= 0.9f;
    }
}

void ADinosaurCombatAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    // Update basic values
    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(DinosaurMemory.PerceivedThreatLevel));
    
    // Update distances and ranges
    if (CurrentTarget)
    {
        float Distance = GetDistanceToTarget();
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), Distance);
        BlackboardComponent->SetValueAsBool(TEXT("IsInAttackRange"), IsInAttackRange());
        BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), CanSeeTarget(CurrentTarget));
    }
    
    // Update territory information
    BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), DinosaurMemory.TerritoryCenter);
    BlackboardComponent->SetValueAsFloat(TEXT("TerritorialRadius"), CombatProfile.TerritorialRadius);
    BlackboardComponent->SetValueAsBool(TEXT("IsPlayerInTerritory"), IsPlayerInTerritory());
}

void ADinosaurCombatAI::ProcessPerceptionData()
{
    // Process any additional perception logic here
    // This could include analyzing sound patterns, tracking multiple targets, etc.
}

float ADinosaurCombatAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target)
        return 0.0f;

    float ThreatLevel = 0.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    ThreatLevel += FMath::Clamp(1.0f - (Distance / CombatProfile.HuntingRange), 0.0f, 1.0f) * 0.4f;
    
    // Recent damage factor
    if (DinosaurMemory.bHasBeenAttackedByPlayer)
    {
        float TimeSinceDamage = GetWorld()->GetTimeSeconds() - DinosaurMemory.LastDamageTime;
        ThreatLevel += FMath::Clamp(1.0f - (TimeSinceDamage / 10.0f), 0.0f, 1.0f) * 0.6f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool ADinosaurCombatAI::IsPlayerInTerritory() const
{
    if (!CurrentTarget)
        return false;

    float DistanceFromTerritory = FVector::Dist(CurrentTarget->GetActorLocation(), DinosaurMemory.TerritoryCenter);
    return DistanceFromTerritory <= CombatProfile.TerritorialRadius;
}

void ADinosaurCombatAI::UpdateCombatState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceStateChange = CurrentTime - LastStateChangeTime;
    
    // State transition logic based on current conditions
    switch (CurrentCombatState)
    {
    case EDinosaurCombatState::Stalking:
        if (TimeSinceStateChange >= CombatProfile.StalkingDuration)
        {
            if (IsInAttackRange())
            {
                SetCombatState(EDinosaurCombatState::Attacking);
            }
            else
            {
                SetCombatState(EDinosaurCombatState::Hunting);
            }
        }
        break;
        
    case EDinosaurCombatState::Investigating:
        if (TimeSinceStateChange >= 10.0f) // Investigate for 10 seconds
        {
            SetCombatState(EDinosaurCombatState::Idle);
        }
        break;
    }
}

FVector ADinosaurCombatAI::CalculateOptimalPosition() const
{
    // Calculate the best position for this dinosaur based on its archetype and current situation
    if (!CurrentTarget || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    switch (CombatProfile.Archetype)
    {
    case EDinosaurArchetype::ApexPredator:
        // Direct approach, maintain optimal attack distance
        return TargetLocation + (MyLocation - TargetLocation).GetSafeNormal() * CombatProfile.AttackRange * 0.8f;
        
    case EDinosaurArchetype::PackHunter:
        // Flanking position
        return GetFlankingPosition();
        
    case EDinosaurArchetype::AmbushPredator:
        // Find cover near target
        // This would require additional environmental analysis
        return MyLocation; // Placeholder
        
    default:
        return MyLocation;
    }
}

bool ADinosaurCombatAI::ShouldUseSpecialAbility() const
{
    // Determine if conditions are right for special abilities
    if (CombatProfile.CombatAbilities.Num() == 0)
        return false;

    // Example conditions: low health, multiple enemies, etc.
    return IsInAttackRange() && DinosaurMemory.PerceivedThreatLevel >= EDinosaurThreatLevel::High;
}

void ADinosaurCombatAI::PlanNextMove()
{
    // Advanced tactical planning would go here
    // This could include pathfinding optimization, team coordination, etc.
}