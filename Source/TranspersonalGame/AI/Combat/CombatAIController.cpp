// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CombatAIController.h"
#include "CombatTacticsComponent.h"
#include "ThreatAssessmentComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Create Combat Tactics Component
    CombatTacticsComponent = CreateDefaultSubobject<UCombatTacticsComponent>(TEXT("CombatTacticsComponent"));

    // Create Threat Assessment Component
    ThreatAssessmentComponent = CreateDefaultSubobject<UThreatAssessmentComponent>(TEXT("ThreatAssessmentComponent"));

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);

    // Initialize combat state
    CurrentCombatState = ECombatState::Idle;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Start behavior tree if assigned
    if (CombatBehaviorTree)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Set up timers
    GetWorld()->GetTimerManager().SetTimer(CombatUpdateTimer, this, &ACombatAIController::OnCombatUpdate, 0.1f, true);
    GetWorld()->GetTimerManager().SetTimer(CoordinationTimer, this, &ACombatAIController::OnCoordinationUpdate, 1.0f, true);

    // Find nearby allies
    FindNearbyAllies();

    UE_LOG(LogTemp, Log, TEXT("CombatAIController: BeginPlay completed for %s"), *GetName());
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update combat logic
    UpdateCombatState();
    UpdateTargets();
    UpdateBlackboard();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("CombatAIController: Possessed pawn %s"), *InPawn->GetName());
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(CombatStateKey, static_cast<uint8>(NewState));
        }

        // Broadcast state change to allies
        BroadcastThreatToAllies(PrimaryTarget.TargetActor, PrimaryTarget.ThreatLevel);

        UE_LOG(LogTemp, Log, TEXT("CombatAIController: State changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ACombatAIController::SetPrimaryTarget(AActor* NewTarget)
{
    if (NewTarget != PrimaryTarget.TargetActor)
    {
        PrimaryTarget.TargetActor = NewTarget;
        
        if (NewTarget)
        {
            PrimaryTarget.ThreatLevel = CalculateThreatLevel(NewTarget);
            PrimaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
            PrimaryTarget.LastKnownLocation = NewTarget->GetActorLocation();
            PrimaryTarget.DistanceToTarget = GetDistanceToTarget(NewTarget);
            PrimaryTarget.bIsVisible = IsTargetInSight(NewTarget);
            PrimaryTarget.bIsInAttackRange = CanAttackTarget(NewTarget);

            // Update blackboard
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject(TargetActorKey, NewTarget);
                GetBlackboardComponent()->SetValueAsVector(LastKnownLocationKey, PrimaryTarget.LastKnownLocation);
                GetBlackboardComponent()->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(PrimaryTarget.ThreatLevel));
            }

            UE_LOG(LogTemp, Log, TEXT("CombatAIController: Primary target set to %s"), *NewTarget->GetName());
        }
        else
        {
            // Clear target
            PrimaryTarget = FCombatTarget();
            
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->ClearValue(TargetActorKey);
                GetBlackboardComponent()->ClearValue(LastKnownLocationKey);
                GetBlackboardComponent()->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(EThreatLevel::None));
            }
        }
    }
}

void ACombatAIController::AddSecondaryTarget(AActor* Target)
{
    if (Target && Target != PrimaryTarget.TargetActor)
    {
        // Check if target already exists
        for (FCombatTarget& SecondaryTarget : SecondaryTargets)
        {
            if (SecondaryTarget.TargetActor == Target)
            {
                // Update existing target
                SecondaryTarget.ThreatLevel = CalculateThreatLevel(Target);
                SecondaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
                SecondaryTarget.LastKnownLocation = Target->GetActorLocation();
                SecondaryTarget.DistanceToTarget = GetDistanceToTarget(Target);
                SecondaryTarget.bIsVisible = IsTargetInSight(Target);
                SecondaryTarget.bIsInAttackRange = CanAttackTarget(Target);
                return;
            }
        }

        // Add new secondary target
        FCombatTarget NewTarget;
        NewTarget.TargetActor = Target;
        NewTarget.ThreatLevel = CalculateThreatLevel(Target);
        NewTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewTarget.LastKnownLocation = Target->GetActorLocation();
        NewTarget.DistanceToTarget = GetDistanceToTarget(Target);
        NewTarget.bIsVisible = IsTargetInSight(Target);
        NewTarget.bIsInAttackRange = CanAttackTarget(Target);

        SecondaryTargets.Add(NewTarget);

        UE_LOG(LogTemp, Log, TEXT("CombatAIController: Added secondary target %s"), *Target->GetName());
    }
}

void ACombatAIController::RemoveTarget(AActor* Target)
{
    if (Target == PrimaryTarget.TargetActor)
    {
        SetPrimaryTarget(nullptr);
    }

    SecondaryTargets.RemoveAll([Target](const FCombatTarget& SecondaryTarget)
    {
        return SecondaryTarget.TargetActor == Target;
    });
}

bool ACombatAIController::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    float Distance = GetDistanceToTarget(Target);
    return Distance <= AttackRange && IsTargetInSight(Target);
}

bool ACombatAIController::IsTargetInSight(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    // Use AI perception to check if target is visible
    if (AIPerceptionComponent)
    {
        FActorPerceptionBlueprintInfo PerceptionInfo;
        AIPerceptionComponent->GetActorsPerception(Target, PerceptionInfo);
        
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
            {
                return true;
            }
        }
    }

    return false;
}

float ACombatAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return FLT_MAX;
    }

    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

void ACombatAIController::RegisterAlliedController(ACombatAIController* AllyController)
{
    if (AllyController && !AlliedControllers.Contains(AllyController))
    {
        AlliedControllers.Add(AllyController);
        UE_LOG(LogTemp, Log, TEXT("CombatAIController: Registered ally %s"), *AllyController->GetName());
    }
}

void ACombatAIController::UnregisterAlliedController(ACombatAIController* AllyController)
{
    if (AllyController)
    {
        AlliedControllers.Remove(AllyController);
        UE_LOG(LogTemp, Log, TEXT("CombatAIController: Unregistered ally %s"), *AllyController->GetName());
    }
}

void ACombatAIController::BroadcastThreatToAllies(AActor* ThreatActor, EThreatLevel Level)
{
    if (!ThreatActor)
    {
        return;
    }

    for (ACombatAIController* Ally : AlliedControllers)
    {
        if (Ally && IsValid(Ally))
        {
            // Inform ally about threat
            Ally->AddSecondaryTarget(ThreatActor);
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Check if this is a potential target
            if (ShouldEngageTarget(Actor))
            {
                if (!PrimaryTarget.TargetActor)
                {
                    SetPrimaryTarget(Actor);
                }
                else
                {
                    AddSecondaryTarget(Actor);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == GetPawn())
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (ShouldEngageTarget(Actor))
        {
            if (!PrimaryTarget.TargetActor)
            {
                SetPrimaryTarget(Actor);
            }
            else
            {
                AddSecondaryTarget(Actor);
            }
        }
    }
    else
    {
        // Target lost
        if (Actor == PrimaryTarget.TargetActor)
        {
            PrimaryTarget.bIsVisible = false;
            PrimaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void ACombatAIController::UpdateCombatState()
{
    ECombatState NewState = CurrentCombatState;

    if (!PrimaryTarget.TargetActor)
    {
        NewState = ECombatState::Idle;
    }
    else
    {
        float DistanceToTarget = GetDistanceToTarget(PrimaryTarget.TargetActor);
        bool bCanSeeTarget = IsTargetInSight(PrimaryTarget.TargetActor);

        if (ShouldRetreat())
        {
            NewState = ECombatState::Retreating;
        }
        else if (CanAttackTarget(PrimaryTarget.TargetActor))
        {
            NewState = ECombatState::Engaging;
        }
        else if (bCanSeeTarget && DistanceToTarget > AttackRange)
        {
            NewState = ECombatState::Stalking;
        }
        else if (!bCanSeeTarget)
        {
            NewState = ECombatState::Hunting;
        }
    }

    SetCombatState(NewState);
}

void ACombatAIController::UpdateTargets()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update primary target
    if (PrimaryTarget.TargetActor)
    {
        PrimaryTarget.DistanceToTarget = GetDistanceToTarget(PrimaryTarget.TargetActor);
        PrimaryTarget.bIsVisible = IsTargetInSight(PrimaryTarget.TargetActor);
        PrimaryTarget.bIsInAttackRange = CanAttackTarget(PrimaryTarget.TargetActor);

        if (PrimaryTarget.bIsVisible)
        {
            PrimaryTarget.LastSeenTime = CurrentTime;
            PrimaryTarget.LastKnownLocation = PrimaryTarget.TargetActor->GetActorLocation();
        }

        // Remove target if too old or invalid
        if (!IsValid(PrimaryTarget.TargetActor) || 
            (CurrentTime - PrimaryTarget.LastSeenTime > 30.0f))
        {
            SetPrimaryTarget(nullptr);
        }
    }

    // Update secondary targets
    SecondaryTargets.RemoveAll([this, CurrentTime](FCombatTarget& Target)
    {
        if (!IsValid(Target.TargetActor))
        {
            return true;
        }

        Target.DistanceToTarget = GetDistanceToTarget(Target.TargetActor);
        Target.bIsVisible = IsTargetInSight(Target.TargetActor);
        Target.bIsInAttackRange = CanAttackTarget(Target.TargetActor);

        if (Target.bIsVisible)
        {
            Target.LastSeenTime = CurrentTime;
            Target.LastKnownLocation = Target.TargetActor->GetActorLocation();
        }

        // Remove if too old
        return (CurrentTime - Target.LastSeenTime > 20.0f);
    });
}

void ACombatAIController::UpdateBlackboard()
{
    if (!GetBlackboardComponent())
    {
        return;
    }

    // Update combat state
    GetBlackboardComponent()->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));

    // Update aggression level
    GetBlackboardComponent()->SetValueAsFloat(AggressionKey, AggressionLevel);

    // Update attack range
    GetBlackboardComponent()->SetValueAsFloat(AttackRangeKey, AttackRange);

    // Update primary target info
    if (PrimaryTarget.TargetActor)
    {
        GetBlackboardComponent()->SetValueAsObject(TargetActorKey, PrimaryTarget.TargetActor);
        GetBlackboardComponent()->SetValueAsVector(LastKnownLocationKey, PrimaryTarget.LastKnownLocation);
        GetBlackboardComponent()->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(PrimaryTarget.ThreatLevel));
    }
}

void ACombatAIController::FindNearbyAllies()
{
    if (!GetPawn())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetPawn()->GetActorLocation(),
        CoordinationRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        ACombatAIController::StaticClass(),
        TArray<AActor*>{GetPawn()},
        FoundActors
    );

    for (AActor* Actor : FoundActors)
    {
        if (ACombatAIController* AllyController = Cast<ACombatAIController>(Actor))
        {
            RegisterAlliedController(AllyController);
            AllyController->RegisterAlliedController(this);
        }
    }
}

EThreatLevel ACombatAIController::CalculateThreatLevel(AActor* Target) const
{
    if (!Target)
    {
        return EThreatLevel::None;
    }

    float Distance = GetDistanceToTarget(Target);
    bool bIsPlayer = Target->IsA<ACharacter>() && Target->GetClass()->GetName().Contains(TEXT("Player"));

    if (bIsPlayer)
    {
        if (Distance < AttackRange * 0.5f)
        {
            return EThreatLevel::Critical;
        }
        else if (Distance < AttackRange)
        {
            return EThreatLevel::High;
        }
        else if (Distance < SightRadius * 0.5f)
        {
            return EThreatLevel::Medium;
        }
        else
        {
            return EThreatLevel::Low;
        }
    }

    return EThreatLevel::Low;
}

bool ACombatAIController::ShouldEngageTarget(AActor* Target) const
{
    if (!Target || Target == GetPawn())
    {
        return false;
    }

    // Check if target is a player or other threatening entity
    bool bIsPlayer = Target->IsA<ACharacter>() && Target->GetClass()->GetName().Contains(TEXT("Player"));
    
    if (bIsPlayer)
    {
        float Distance = GetDistanceToTarget(Target);
        return Distance <= SightRadius && AggressionLevel > 0.3f;
    }

    return false;
}

bool ACombatAIController::ShouldRetreat() const
{
    if (!PrimaryTarget.TargetActor)
    {
        return false;
    }

    // Retreat if health is low or outnumbered
    if (APawn* ControlledPawn = GetPawn())
    {
        // Simple health check (would need actual health component)
        float HealthRatio = 1.0f; // Placeholder
        
        if (HealthRatio < FearThreshold)
        {
            return true;
        }
    }

    // Check if outnumbered
    int32 NearbyEnemies = SecondaryTargets.Num() + (PrimaryTarget.TargetActor ? 1 : 0);
    int32 NearbyAllies = AlliedControllers.Num();

    return NearbyEnemies > NearbyAllies * 2;
}

void ACombatAIController::OnCombatUpdate()
{
    // High-frequency combat updates
    if (CombatTacticsComponent)
    {
        CombatTacticsComponent->UpdateTactics(PrimaryTarget.TargetActor);
    }

    if (ThreatAssessmentComponent)
    {
        ThreatAssessmentComponent->AssessThreats();
    }
}

void ACombatAIController::OnCoordinationUpdate()
{
    // Lower frequency coordination updates
    FindNearbyAllies();
    
    // Clean up invalid allies
    AlliedControllers.RemoveAll([](ACombatAIController* Ally)
    {
        return !IsValid(Ally);
    });
}