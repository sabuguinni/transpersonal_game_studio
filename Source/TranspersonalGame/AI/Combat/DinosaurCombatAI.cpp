#include "DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Perception/AIDamageConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Configure AI Sight
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfView;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Hearing
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Damage Perception
    UAIDamageConfig* DamageConfig = CreateDefaultSubobject<UAIDamageConfig>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add senses to perception component
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize default values
    CurrentCombatState = EDinosaurCombatState::Idle;
    CurrentAggressionLevel = EDinosaurAggressionLevel::Defensive;
    
    CombatMetrics.CurrentHealth = 100.0f;
    CombatMetrics.MaxHealth = 100.0f;
    CombatMetrics.CurrentStamina = 100.0f;
    CombatMetrics.MaxStamina = 100.0f;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
        UpdateBlackboardValues();
    }

    CombatStartTime = GetWorld()->GetTimeSeconds();
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatMetrics(DeltaTime);
    EvaluateTacticalSituation();
    ProcessCombatStateTransitions();
    UpdateBlackboardValues();
}

void ADinosaurCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent)
    {
        UseBlackboard(CombatBehaviorTree->BlackboardAsset);
    }
}

void ADinosaurCombatAI::SetCombatState(EDinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        EDinosaurCombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s: Combat state changed from %d to %d"), 
               *GetPawn()->GetName(), (int32)PreviousState, (int32)NewState);

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(CombatStateKey, (uint8)NewState);
        }
    }
}

void ADinosaurCombatAI::SetAggressionLevel(EDinosaurAggressionLevel NewLevel)
{
    if (CurrentAggressionLevel != NewLevel)
    {
        CurrentAggressionLevel = NewLevel;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(AggressionLevelKey, (uint8)NewLevel);
        }
    }
}

void ADinosaurCombatAI::OnTakeDamage(float DamageAmount, AActor* DamageSource)
{
    CombatMetrics.CurrentHealth = FMath::Max(0.0f, CombatMetrics.CurrentHealth - DamageAmount);
    CombatMetrics.LastDamageTime = GetWorld()->GetTimeSeconds();
    CombatMetrics.bIsWounded = CombatMetrics.CurrentHealth < (CombatMetrics.MaxHealth * 0.5f);

    // Escalate aggression based on damage and personality
    float HealthPercentage = CombatMetrics.CurrentHealth / CombatMetrics.MaxHealth;
    float AggressionIncrease = (DamageAmount / CombatMetrics.MaxHealth) * CombatPersonality.EscalationRate;

    if (HealthPercentage < 0.3f && CombatPersonality.SelfPreservation > 0.7f)
    {
        // High self-preservation dinosaurs flee when critically wounded
        SetCombatState(EDinosaurCombatState::Fleeing);
    }
    else if (DamageSource && DamageSource != CurrentTarget)
    {
        // Switch target to damage source if it's a new threat
        CurrentTarget = DamageSource;
        SetCombatState(EDinosaurCombatState::Hunting);
        
        // Escalate aggression
        if (CurrentAggressionLevel < EDinosaurAggressionLevel::Enraged)
        {
            SetAggressionLevel(static_cast<EDinosaurAggressionLevel>(
                FMath::Min((int32)EDinosaurAggressionLevel::Enraged, 
                          (int32)CurrentAggressionLevel + 1)));
        }
    }

    // Update threat level
    CombatMetrics.ThreatLevel = FMath::Min(1.0f, CombatMetrics.ThreatLevel + AggressionIncrease);
}

void ADinosaurCombatAI::OnAttackHit(AActor* HitTarget)
{
    SuccessfulHits++;
    TotalAttacksMade++;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Reduce stamina
    CombatMetrics.CurrentStamina = FMath::Max(0.0f, CombatMetrics.CurrentStamina - 15.0f);
    CombatMetrics.bIsExhausted = CombatMetrics.CurrentStamina < 20.0f;

    // Successful hits can escalate aggression for aggressive personalities
    if (CombatPersonality.AggressionMultiplier > 1.5f)
    {
        CombatMetrics.ThreatLevel = FMath::Min(1.0f, CombatMetrics.ThreatLevel + 0.1f);
    }
}

void ADinosaurCombatAI::OnAttackMissed()
{
    TotalAttacksMade++;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Reduce stamina less for missed attacks
    CombatMetrics.CurrentStamina = FMath::Max(0.0f, CombatMetrics.CurrentStamina - 8.0f);
    CombatMetrics.bIsExhausted = CombatMetrics.CurrentStamina < 20.0f;

    // Missing attacks can cause tactical repositioning
    if (CurrentTarget)
    {
        FVector OptimalPosition = CalculateOptimalAttackPosition(CurrentTarget);
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(OptimalAttackPositionKey, OptimalPosition);
        }
    }
}

FVector ADinosaurCombatAI::CalculateOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn())
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();

    // Calculate position based on preferred combat range and personality
    float DesiredDistance = CombatPersonality.PreferredCombatRange;
    
    // Adjust distance based on current health and aggression
    float HealthFactor = CombatMetrics.CurrentHealth / CombatMetrics.MaxHealth;
    if (HealthFactor < 0.5f && CombatPersonality.SelfPreservation > 0.5f)
    {
        DesiredDistance *= 1.5f; // Stay further when wounded and cautious
    }
    else if (CurrentAggressionLevel == EDinosaurAggressionLevel::Enraged)
    {
        DesiredDistance *= 0.7f; // Get closer when enraged
    }

    // Calculate optimal position
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * DesiredDistance);

    // Use navigation system to find valid position
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }

    return OptimalPosition;
}

bool ADinosaurCombatAI::ShouldRetreat()
{
    float HealthPercentage = CombatMetrics.CurrentHealth / CombatMetrics.MaxHealth;
    float StaminaPercentage = CombatMetrics.CurrentStamina / CombatMetrics.MaxStamina;
    
    // Base retreat threshold influenced by self-preservation
    float RetreatThreshold = 0.2f + (CombatPersonality.SelfPreservation * 0.3f);
    
    // Consider multiple factors
    bool LowHealth = HealthPercentage < RetreatThreshold;
    bool Exhausted = StaminaPercentage < 0.1f;
    bool OutnumberedOrOverwhelmed = CombatMetrics.ThreatLevel > 0.8f;
    
    return LowHealth || Exhausted || OutnumberedOrOverwhelmed;
}

bool ADinosaurCombatAI::ShouldUseSpecialAttack()
{
    float TimeSinceLastSpecial = GetWorld()->GetTimeSeconds() - LastSpecialAttackTime;
    
    if (TimeSinceLastSpecial < SpecialAttackCooldown)
    {
        return false;
    }

    // Chance based on personality and current situation
    float BaseChance = CombatPersonality.SpecialAttackChance;
    
    // Increase chance when enraged or wounded
    if (CurrentAggressionLevel == EDinosaurAggressionLevel::Enraged)
    {
        BaseChance *= 1.5f;
    }
    
    if (CombatMetrics.bIsWounded)
    {
        BaseChance *= 1.3f; // Desperate measures
    }

    return FMath::RandRange(0.0f, 1.0f) < BaseChance;
}

float ADinosaurCombatAI::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
    {
        return 0.0f;
    }

    float ThreatLevel = 0.0f;
    
    // Distance factor - closer targets are more threatening
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / SightRadius), 0.0f, 1.0f);
    ThreatLevel += DistanceFactor * 0.4f;
    
    // Recent damage factor
    float TimeSinceDamage = GetWorld()->GetTimeSeconds() - CombatMetrics.LastDamageTime;
    if (TimeSinceDamage < 5.0f)
    {
        ThreatLevel += (5.0f - TimeSinceDamage) / 5.0f * 0.3f;
    }
    
    // Line of sight factor
    if (PerceptionComponent && PerceptionComponent->GetActorsPerception(Target).Num() > 0)
    {
        ThreatLevel += 0.3f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }

        // Check if this is a potential target (player or other threat)
        if (Actor->IsA<APawn>() && Actor != GetPawn())
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            
            // Update current target if this is a higher threat
            if (!CurrentTarget || ThreatLevel > CombatMetrics.ThreatLevel)
            {
                CurrentTarget = Actor;
                CombatMetrics.ThreatLevel = ThreatLevel;
                LastKnownTargetLocation = Actor->GetActorLocation();
                LastTargetSightTime = GetWorld()->GetTimeSeconds();
                
                // Transition to appropriate combat state
                if (CurrentCombatState == EDinosaurCombatState::Idle)
                {
                    SetCombatState(EDinosaurCombatState::Stalking);
                }
            }
        }
    }
}

void ADinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor != CurrentTarget)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        LastKnownTargetLocation = Stimulus.StimulusLocation;
        LastTargetSightTime = GetWorld()->GetTimeSeconds();
        
        // Escalate based on stimulus type and distance
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Stimulus.StimulusLocation);
        
        if (Distance < CombatPersonality.PreferredCombatRange * 0.5f)
        {
            // Very close - immediate attack
            SetCombatState(EDinosaurCombatState::Attacking);
        }
        else if (Distance < CombatPersonality.PreferredCombatRange)
        {
            // In range - hunt or attack
            SetCombatState(EDinosaurCombatState::Hunting);
        }
        else
        {
            // Far - stalk
            SetCombatState(EDinosaurCombatState::Stalking);
        }
    }
    else
    {
        // Lost sight - investigate last known location
        float TimeSinceLastSight = GetWorld()->GetTimeSeconds() - LastTargetSightTime;
        if (TimeSinceLastSight > CombatPersonality.ThreatMemoryDuration)
        {
            CurrentTarget = nullptr;
            SetCombatState(EDinosaurCombatState::Idle);
        }
    }
}

void ADinosaurCombatAI::UpdateCombatMetrics(float DeltaTime)
{
    // Regenerate stamina over time
    if (CombatMetrics.CurrentStamina < CombatMetrics.MaxStamina)
    {
        float StaminaRegen = 20.0f * DeltaTime; // 20 stamina per second
        if (CurrentCombatState == EDinosaurCombatState::Idle)
        {
            StaminaRegen *= 2.0f; // Faster regen when idle
        }
        
        CombatMetrics.CurrentStamina = FMath::Min(CombatMetrics.MaxStamina, 
                                                  CombatMetrics.CurrentStamina + StaminaRegen);
    }
    
    CombatMetrics.bIsExhausted = CombatMetrics.CurrentStamina < 20.0f;
    
    // Decay threat level over time
    if (CombatMetrics.ThreatLevel > 0.0f)
    {
        CombatMetrics.ThreatLevel = FMath::Max(0.0f, CombatMetrics.ThreatLevel - (0.1f * DeltaTime));
    }
}

void ADinosaurCombatAI::EvaluateTacticalSituation()
{
    if (!CurrentTarget)
    {
        return;
    }

    // Check if we should retreat
    if (ShouldRetreat() && CurrentCombatState != EDinosaurCombatState::Retreating)
    {
        SetCombatState(EDinosaurCombatState::Retreating);
        return;
    }

    // Evaluate attack opportunities
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    bool CanAttack = DistanceToTarget <= CombatPersonality.PreferredCombatRange && 
                     CombatMetrics.CurrentStamina > 30.0f &&
                     (GetWorld()->GetTimeSeconds() - LastAttackTime) > AttackCooldown;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(CanAttackKey, CanAttack);
    }
}

void ADinosaurCombatAI::ProcessCombatStateTransitions()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceStateChange = CurrentTime - LastStateChangeTime;
    
    // Prevent rapid state changes
    if (TimeSinceStateChange < StateChangeDelay)
    {
        return;
    }

    // State-specific logic
    switch (CurrentCombatState)
    {
        case EDinosaurCombatState::Attacking:
            // Return to hunting after attack
            if (TimeSinceStateChange > 3.0f)
            {
                SetCombatState(EDinosaurCombatState::Hunting);
            }
            break;
            
        case EDinosaurCombatState::Retreating:
            // Return to idle if safe
            if (CombatMetrics.ThreatLevel < 0.2f && TimeSinceStateChange > 5.0f)
            {
                SetCombatState(EDinosaurCombatState::Idle);
            }
            break;
            
        case EDinosaurCombatState::Stalking:
            // Escalate to hunting if target gets closer
            if (CurrentTarget)
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
                if (Distance < CombatPersonality.PreferredCombatRange * 1.2f)
                {
                    SetCombatState(EDinosaurCombatState::Hunting);
                }
            }
            break;
    }
}

void ADinosaurCombatAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update target information
    BlackboardComponent->SetValueAsObject(TargetActorKey, CurrentTarget);
    BlackboardComponent->SetValueAsVector(LastKnownLocationKey, LastKnownTargetLocation);
    
    // Update combat state
    BlackboardComponent->SetValueAsEnum(CombatStateKey, (uint8)CurrentCombatState);
    BlackboardComponent->SetValueAsEnum(AggressionLevelKey, (uint8)CurrentAggressionLevel);
    
    // Update status flags
    BlackboardComponent->SetValueAsBool(IsWoundedKey, CombatMetrics.bIsWounded);
    
    // Calculate and update optimal attack position
    if (CurrentTarget)
    {
        FVector OptimalPosition = CalculateOptimalAttackPosition(CurrentTarget);
        BlackboardComponent->SetValueAsVector(OptimalAttackPositionKey, OptimalPosition);
    }
}