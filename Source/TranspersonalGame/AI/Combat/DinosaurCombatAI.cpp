#include "DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure AI Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 3000.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Configure AI Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    AIPerceptionComponent->ConfigureSense(*DamageConfig);

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnTargetPerceptionUpdated);
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Initialize memory system
    Memory.TerritoryCenter = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    Memory.TerritoryRadius = FMath::RandRange(1500.0f, 3000.0f);

    // Randomize personality slightly for uniqueness
    Personality.Aggression += FMath::RandRange(-0.1f, 0.1f);
    Personality.Curiosity += FMath::RandRange(-0.1f, 0.1f);
    Personality.Caution += FMath::RandRange(-0.1f, 0.1f);
    Personality.Territoriality += FMath::RandRange(-0.1f, 0.1f);

    // Clamp personality values
    Personality.Aggression = FMath::Clamp(Personality.Aggression, 0.0f, 1.0f);
    Personality.Curiosity = FMath::Clamp(Personality.Curiosity, 0.0f, 1.0f);
    Personality.Caution = FMath::Clamp(Personality.Caution, 0.0f, 1.0f);
    Personality.Territoriality = FMath::Clamp(Personality.Territoriality, 0.0f, 1.0f);
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastStateUpdateTime >= StateUpdateInterval)
    {
        UpdateCombatState();
        UpdateThreatAssessment();
        UpdatePackCoordination();
        ProcessPersonalityFactors();
        
        LastStateUpdateTime = CurrentTime;
    }
}

void ADinosaurCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
    }

    if (CombatBehaviorTree)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ADinosaurCombatAI::OnUnPossess()
{
    Super::OnUnPossess();
    
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ADinosaurCombatAI::SetCombatState(EDinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        EDinosaurCombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        // Handle state transitions
        switch (NewState)
        {
        case EDinosaurCombatState::Hunting:
            if (DinosaurArchetype == EDinosaurArchetype::PackHunter && PackMembers.Num() > 0)
            {
                CoordinatePackAttack(CurrentTarget);
            }
            break;
        case EDinosaurCombatState::Fleeing:
            CurrentTarget = nullptr;
            break;
        case EDinosaurCombatState::Attacking:
            if (CurrentTarget && BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
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
        BlackboardComponent->SetValueAsObject(TEXT("Target"), NewTarget);
    }

    if (NewTarget)
    {
        CurrentThreatLevel = AssessThreatLevel(NewTarget);
        
        // Update memory
        if (ACharacter* PlayerCharacter = Cast<ACharacter>(NewTarget))
        {
            if (PlayerCharacter->IsPlayerControlled())
            {
                UpdatePlayerMemory(NewTarget->GetActorLocation());
            }
        }
    }
}

EDinosaurThreatLevel ADinosaurCombatAI::AssessThreatLevel(AActor* Actor)
{
    if (!Actor)
        return EDinosaurThreatLevel::None;

    float ThreatScore = CalculateThreatScore(Actor);

    if (ThreatScore >= 0.8f)
        return EDinosaurThreatLevel::Extreme;
    else if (ThreatScore >= 0.6f)
        return EDinosaurThreatLevel::High;
    else if (ThreatScore >= 0.4f)
        return EDinosaurThreatLevel::Medium;
    else if (ThreatScore >= 0.2f)
        return EDinosaurThreatLevel::Low;
    
    return EDinosaurThreatLevel::None;
}

float ADinosaurCombatAI::CalculateThreatScore(AActor* Actor)
{
    if (!Actor || !GetPawn())
        return 0.0f;

    float ThreatScore = 0.0f;

    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.3f;

    // Player factor
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (Character->IsPlayerControlled())
        {
            ThreatScore += 0.5f; // Players are inherently threatening
            
            // Check if player is moving (more threatening)
            if (Character->GetVelocity().Size() > 100.0f)
            {
                ThreatScore += 0.1f;
            }
        }
    }

    // Size comparison (larger actors are more threatening)
    if (ACharacter* OtherCharacter = Cast<ACharacter>(Actor))
    {
        if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
        {
            float SizeRatio = OtherCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() / 
                             MyCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
            ThreatScore += FMath::Clamp(SizeRatio - 1.0f, 0.0f, 0.3f);
        }
    }

    // Personality modifiers
    ThreatScore *= (2.0f - Personality.Caution); // More cautious = higher threat perception
    ThreatScore *= (1.0f + Personality.Aggression * 0.5f); // More aggressive = lower threat perception

    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

void ADinosaurCombatAI::UpdatePlayerMemory(FVector PlayerPosition)
{
    Memory.LastKnownPlayerPositions.Add(PlayerPosition);
    Memory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();

    // Limit memory to last 10 positions
    if (Memory.LastKnownPlayerPositions.Num() > 10)
    {
        Memory.LastKnownPlayerPositions.RemoveAt(0);
    }

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), PlayerPosition);
        BlackboardComponent->SetValueAsFloat(TEXT("LastPlayerSightingTime"), Memory.LastPlayerSightingTime);
    }
}

void ADinosaurCombatAI::UpdateNoiseMemory(FVector NoiseLocation)
{
    Memory.LastNoiseLocation = NoiseLocation;
    Memory.LastNoiseTime = GetWorld()->GetTimeSeconds();

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastNoiseLocation"), NoiseLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("LastNoiseTime"), Memory.LastNoiseTime);
    }
}

FVector ADinosaurCombatAI::GetLastKnownPlayerPosition()
{
    if (Memory.LastKnownPlayerPositions.Num() > 0)
    {
        return Memory.LastKnownPlayerPositions.Last();
    }
    return FVector::ZeroVector;
}

bool ADinosaurCombatAI::HasRecentPlayerMemory(float TimeThreshold)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - Memory.LastPlayerSightingTime) <= TimeThreshold;
}

bool ADinosaurCombatAI::ShouldAttack(AActor* Target)
{
    if (!Target)
        return false;

    float AttackThreshold = 0.5f;
    
    // Personality factors
    AttackThreshold -= Personality.Aggression * 0.3f;
    AttackThreshold += Personality.Caution * 0.2f;

    // Pack factors
    if (PackMembers.Num() > 0)
    {
        AttackThreshold -= 0.2f; // More likely to attack in pack
    }

    // Archetype factors
    switch (DinosaurArchetype)
    {
    case EDinosaurArchetype::ApexPredator:
        AttackThreshold -= 0.3f;
        break;
    case EDinosaurArchetype::PackHunter:
        AttackThreshold -= PackMembers.Num() * 0.1f;
        break;
    case EDinosaurArchetype::AmbushPredator:
        // Only attack if target is unaware or isolated
        AttackThreshold += 0.2f;
        break;
    }

    float ThreatScore = CalculateThreatScore(Target);
    return ThreatScore >= AttackThreshold;
}

bool ADinosaurCombatAI::ShouldFlee(AActor* Threat)
{
    if (!Threat)
        return false;

    float FleeThreshold = 0.7f;
    
    // Personality factors
    FleeThreshold -= Personality.Aggression * 0.2f;
    FleeThreshold -= Personality.Caution * 0.3f;

    // Pack factors
    if (PackMembers.Num() > 0)
    {
        FleeThreshold += 0.2f; // Less likely to flee with pack support
    }

    float ThreatScore = CalculateThreatScore(Threat);
    return ThreatScore >= FleeThreshold;
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;

        FAIStimulus Stimulus;
        if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                // Update memory based on what we sensed
                if (ACharacter* Character = Cast<ACharacter>(Actor))
                {
                    if (Character->IsPlayerControlled())
                    {
                        UpdatePlayerMemory(Actor->GetActorLocation());
                        
                        // Decide if this should become our target
                        if (!CurrentTarget || CalculateThreatScore(Actor) > CalculateThreatScore(CurrentTarget))
                        {
                            SetTarget(Actor);
                        }
                    }
                }
            }
        }
    }
}

void ADinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        UpdateNoiseMemory(Stimulus.StimulusLocation);
        
        // Investigate noise if curious enough
        if (Personality.Curiosity > 0.5f && CurrentCombatState == EDinosaurCombatState::Idle)
        {
            SetCombatState(EDinosaurCombatState::Investigating);
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("InvestigateLocation"), Stimulus.StimulusLocation);
            }
        }
    }
}

void ADinosaurCombatAI::UpdateCombatState()
{
    if (!GetPawn())
        return;

    // State machine logic based on current conditions
    switch (CurrentCombatState)
    {
    case EDinosaurCombatState::Idle:
        if (CurrentTarget)
        {
            if (ShouldAttack(CurrentTarget))
            {
                SetCombatState(EDinosaurCombatState::Hunting);
            }
            else if (ShouldFlee(CurrentTarget))
            {
                SetCombatState(EDinosaurCombatState::Fleeing);
            }
            else
            {
                SetCombatState(EDinosaurCombatState::Stalking);
            }
        }
        break;

    case EDinosaurCombatState::Stalking:
        if (CurrentTarget)
        {
            if (ShouldAttack(CurrentTarget))
            {
                SetCombatState(EDinosaurCombatState::Attacking);
            }
            else if (ShouldFlee(CurrentTarget))
            {
                SetCombatState(EDinosaurCombatState::Fleeing);
            }
        }
        else
        {
            SetCombatState(EDinosaurCombatState::Idle);
        }
        break;

    case EDinosaurCombatState::Hunting:
        if (CurrentTarget)
        {
            float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistanceToTarget <= 500.0f) // Attack range
            {
                SetCombatState(EDinosaurCombatState::Attacking);
            }
            else if (ShouldFlee(CurrentTarget))
            {
                SetCombatState(EDinosaurCombatState::Fleeing);
            }
        }
        else
        {
            SetCombatState(EDinosaurCombatState::Idle);
        }
        break;

    case EDinosaurCombatState::Attacking:
        if (!CurrentTarget || ShouldFlee(CurrentTarget))
        {
            SetCombatState(EDinosaurCombatState::Fleeing);
        }
        else
        {
            float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistanceToTarget > 800.0f) // Lost target
            {
                SetCombatState(EDinosaurCombatState::Hunting);
            }
        }
        break;

    case EDinosaurCombatState::Fleeing:
        if (!CurrentTarget)
        {
            SetCombatState(EDinosaurCombatState::Idle);
        }
        else
        {
            float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistanceToTarget > 1500.0f) // Safe distance
            {
                SetCombatState(EDinosaurCombatState::Idle);
            }
        }
        break;
    }
}

void ADinosaurCombatAI::UpdateThreatAssessment()
{
    if (CurrentTarget)
    {
        CurrentThreatLevel = AssessThreatLevel(CurrentTarget);
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
        }
    }
}

void ADinosaurCombatAI::UpdatePackCoordination()
{
    // Pack behavior logic will be implemented here
    // This includes coordinated attacks, formations, and communication
}

void ADinosaurCombatAI::ProcessPersonalityFactors()
{
    // Apply personality-based modifications to behavior
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Personality.Aggression);
        BlackboardComponent->SetValueAsFloat(TEXT("Caution"), Personality.Caution);
        BlackboardComponent->SetValueAsFloat(TEXT("Curiosity"), Personality.Curiosity);
        BlackboardComponent->SetValueAsFloat(TEXT("Territoriality"), Personality.Territoriality);
    }
}

void ADinosaurCombatAI::CoordinatePackAttack(AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
        return;

    // Implement pack coordination logic
    for (ADinosaurCombatAI* PackMember : PackMembers)
    {
        if (PackMember && PackMember != this)
        {
            PackMember->SetTarget(Target);
            PackMember->SetCombatState(EDinosaurCombatState::Hunting);
        }
    }
}