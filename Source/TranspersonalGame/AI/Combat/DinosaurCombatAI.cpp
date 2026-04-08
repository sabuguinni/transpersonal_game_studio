#include "DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardAsset.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Configure Sight Perception
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Perception
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Perception
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

    // Setup Perception Component
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Combat State
    CurrentAggression = 0.0f;
    CurrentFear = 0.0f;
    bIsInCombat = false;
    CombatContext.CurrentState = ECombatBehaviorState::Patrol;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind Perception Events
    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnTargetPerceptionUpdated);
    }

    // Start Behavior Tree
    if (CombatBehaviorTree && BlackboardComponent)
    {
        UseBlackboard(CombatBlackboard);
        RunBehaviorTree(CombatBehaviorTree);
        
        // Initialize Blackboard values
        UpdateBlackboardValues();
    }

    // Apply Combat Profile to Perception
    if (SightConfig)
    {
        SightConfig->SightRadius = CombatProfile.DetectionRange;
        SightConfig->LoseSightRadius = CombatProfile.DetectionRange * 1.1f;
        PerceptionComponent->ConfigureSense(*SightConfig);
    }
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatBehavior(DeltaTime);
    HandlePackCoordination();
    UpdateBlackboardValues();
}

void ADinosaurCombatAI::SetCombatState(ECombatBehaviorState NewState)
{
    if (CombatContext.CurrentState != NewState)
    {
        ECombatBehaviorState PreviousState = CombatContext.CurrentState;
        CombatContext.CurrentState = NewState;
        
        OnCombatStateChanged.Broadcast(NewState);

        // Update behavior based on state change
        switch (NewState)
        {
            case ECombatBehaviorState::Hunt:
                bIsInCombat = true;
                UpdateAggression(0.3f);
                break;
            case ECombatBehaviorState::Attack:
                bIsInCombat = true;
                UpdateAggression(0.5f);
                break;
            case ECombatBehaviorState::Retreat:
                UpdateFear(0.4f);
                break;
            case ECombatBehaviorState::Patrol:
                if (PreviousState != ECombatBehaviorState::Feeding)
                {
                    bIsInCombat = false;
                    CurrentAggression = FMath::Max(0.0f, CurrentAggression - 0.2f);
                }
                break;
        }
    }
}

void ADinosaurCombatAI::InitiateCombat(AActor* Target)
{
    if (!Target || !ValidateTarget(Target))
        return;

    CombatContext.Target = Target;
    CombatContext.LastKnownTargetLocation = Target->GetActorLocation();
    CombatContext.TimeSinceLastSeen = 0.0f;
    CombatContext.CurrentThreatLevel = CalculateThreatLevel(Target);

    // Determine initial combat behavior
    if (ShouldAmbush(Target))
    {
        SetCombatState(ECombatBehaviorState::Stalk);
    }
    else if (CombatProfile.ThreatLevel == EDinosaurThreatLevel::Apex)
    {
        SetCombatState(ECombatBehaviorState::Hunt);
    }
    else
    {
        SetCombatState(ECombatBehaviorState::Investigate);
    }

    OnThreatDetected.Broadcast(Target, CombatContext.CurrentThreatLevel);
}

void ADinosaurCombatAI::EndCombat()
{
    CombatContext.Target = nullptr;
    CombatContext.TimeSinceLastSeen = 0.0f;
    CombatContext.CurrentThreatLevel = 0.0f;
    bIsInCombat = false;
    
    SetCombatState(ECombatBehaviorState::Patrol);
}

void ADinosaurCombatAI::FormPack(const TArray<AActor*>& PackMembers, ECombatFormation Formation)
{
    if (!CombatProfile.bCanFormPacks)
        return;

    CombatContext.PackMembers = PackMembers;
    CombatContext.ActiveFormation = Formation;
    
    // Determine pack leader (usually this AI if initiating)
    if (!CombatContext.PackLeader)
    {
        CombatContext.PackLeader = GetPawn();
    }

    ExecuteFormation(Formation);
    OnPackFormation.Broadcast(Formation, PackMembers, CombatContext.Target);
}

bool ADinosaurCombatAI::CanSeeTarget(AActor* Target) const
{
    if (!Target || !PerceptionComponent)
        return false;

    FActorPerceptionBlueprintInfo PerceptionInfo;
    return PerceptionComponent->GetActorsPerception(Target, PerceptionInfo) && 
           PerceptionInfo.LastSensedStimuli.Num() > 0 &&
           PerceptionInfo.LastSensedStimuli[0].WasSuccessfullySensed();
}

float ADinosaurCombatAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target)
        return 0.0f;

    float ThreatLevel = 0.5f; // Base threat
    
    // Distance factor (closer = more threatening)
    float Distance = GetDistanceToTarget(Target);
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / CombatProfile.DetectionRange), 0.0f, 1.0f);
    
    // Visibility factor
    float VisibilityFactor = CanSeeTarget(Target) ? 1.0f : 0.3f;
    
    // Health factor (lower health = higher threat perception)
    float HealthFactor = 1.0f;
    if (GetPawn())
    {
        // Assuming health component exists
        HealthFactor = (100.0f - 50.0f) / 100.0f; // Placeholder
    }

    ThreatLevel = (ThreatLevel + DistanceFactor + VisibilityFactor + HealthFactor) / 4.0f;
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

FVector ADinosaurCombatAI::GetOptimalAttackPosition(AActor* Target) const
{
    if (!Target)
        return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate position based on formation and threat level
    FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
    float OptimalDistance = CombatProfile.AttackRange * 0.8f; // Slightly within attack range
    
    // Add some randomization for unpredictability
    FVector RandomOffset = FVector(
        FMath::RandRange(-200.0f, 200.0f),
        FMath::RandRange(-200.0f, 200.0f),
        0.0f
    );
    
    return TargetLocation - (Direction * OptimalDistance) + RandomOffset;
}

bool ADinosaurCombatAI::ShouldAmbush(AActor* Target) const
{
    if (!Target || CombatProfile.AmbushProbability <= 0.0f)
        return false;

    // Check if conditions are right for ambush
    bool bHasLineOfSight = CanSeeTarget(Target);
    bool bTargetUnaware = true; // Would need to check if target is looking this way
    float RandomChance = FMath::RandRange(0.0f, 1.0f);
    
    return !bHasLineOfSight && bTargetUnaware && 
           RandomChance <= CombatProfile.AmbushProbability;
}

void ADinosaurCombatAI::UpdateAggression(float DeltaAggression)
{
    CurrentAggression = FMath::Clamp(CurrentAggression + DeltaAggression, 0.0f, 1.0f);
}

void ADinosaurCombatAI::UpdateFear(float DeltaFear)
{
    CurrentFear = FMath::Clamp(CurrentFear + DeltaFear, 0.0f, 1.0f);
    
    // Fear reduces aggression
    if (DeltaFear > 0.0f)
    {
        CurrentAggression = FMath::Max(0.0f, CurrentAggression - DeltaFear * 0.5f);
    }
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        ProcessThreatDetection(Actor);
    }
}

void ADinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (!bIsInCombat && ValidateTarget(Actor))
        {
            InitiateCombat(Actor);
        }
        else if (CombatContext.Target == Actor)
        {
            CombatContext.LastKnownTargetLocation = Actor->GetActorLocation();
            CombatContext.TimeSinceLastSeen = 0.0f;
        }
    }
    else
    {
        // Target lost
        if (CombatContext.Target == Actor)
        {
            CombatContext.TimeSinceLastSeen = 0.0f;
            SetCombatState(ECombatBehaviorState::Investigate);
        }
    }
}

void ADinosaurCombatAI::ProcessThreatDetection(AActor* Actor)
{
    if (!ValidateTarget(Actor))
        return;

    float ThreatLevel = CalculateThreatLevel(Actor);
    
    // Only react to significant threats
    if (ThreatLevel > 0.3f && !bIsInCombat)
    {
        InitiateCombat(Actor);
    }
}

void ADinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    if (!bIsInCombat)
        return;

    // Update time since last seen target
    CombatContext.TimeSinceLastSeen += DeltaTime;

    // Check if we should give up combat
    if (CombatContext.TimeSinceLastSeen > 30.0f) // 30 seconds without seeing target
    {
        EndCombat();
        return;
    }

    // Update aggression and fear over time
    CurrentAggression = FMath::Max(0.0f, CurrentAggression - DeltaTime * 0.1f);
    CurrentFear = FMath::Max(0.0f, CurrentFear - DeltaTime * 0.05f);

    // Check retreat conditions
    if (CurrentFear > 0.7f || (GetPawn() && false)) // Health check placeholder
    {
        SetCombatState(ECombatBehaviorState::Retreat);
    }
}

void ADinosaurCombatAI::HandlePackCoordination()
{
    if (!CombatProfile.bCanFormPacks || CombatContext.PackMembers.Num() == 0)
        return;

    // Pack coordination logic would go here
    // This is a simplified version
    if (CombatContext.PackLeader == GetPawn() && bIsInCombat)
    {
        ExecuteFormation(CombatContext.ActiveFormation);
    }
}

void ADinosaurCombatAI::ExecuteFormation(ECombatFormation Formation)
{
    // Formation execution logic
    switch (Formation)
    {
        case ECombatFormation::Pack:
            // Coordinate pack attack
            break;
        case ECombatFormation::Ambush:
            // Setup ambush positions
            break;
        case ECombatFormation::Pincer:
            // Coordinate pincer movement
            break;
        case ECombatFormation::Distraction:
            // One distracts, others flank
            break;
        default:
            break;
    }
}

bool ADinosaurCombatAI::ValidateTarget(AActor* Target) const
{
    if (!Target)
        return false;

    // Check if target is the player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    return Target == PlayerPawn;
}

void ADinosaurCombatAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    // Update key blackboard values
    BlackboardComponent->SetValueAsObject("Target", CombatContext.Target);
    BlackboardComponent->SetValueAsVector("LastKnownTargetLocation", CombatContext.LastKnownTargetLocation);
    BlackboardComponent->SetValueAsFloat("Aggression", CurrentAggression);
    BlackboardComponent->SetValueAsFloat("Fear", CurrentFear);
    BlackboardComponent->SetValueAsBool("IsInCombat", bIsInCombat);
    BlackboardComponent->SetValueAsEnum("CombatState", static_cast<uint8>(CombatContext.CurrentState));
}

float ADinosaurCombatAI::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return FLT_MAX;

    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

bool ADinosaurCombatAI::IsTargetInAttackRange(AActor* Target) const
{
    return GetDistanceToTarget(Target) <= CombatProfile.AttackRange;
}

bool ADinosaurCombatAI::HasLineOfSight(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;

    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());
    Params.AddIgnoredActor(Target);
    
    return !GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
}

FVector ADinosaurCombatAI::PredictTargetLocation(AActor* Target, float PredictionTime) const
{
    if (!Target)
        return FVector::ZeroVector;

    FVector CurrentLocation = Target->GetActorLocation();
    FVector Velocity = Target->GetVelocity();
    
    return CurrentLocation + (Velocity * PredictionTime);
}