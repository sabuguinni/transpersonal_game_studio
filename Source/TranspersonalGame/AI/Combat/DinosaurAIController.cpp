#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

// -----------------------------------------------------------------------
// Blackboard key name constants
// -----------------------------------------------------------------------
const FName ADinosaurAIController::BB_PlayerActor    = TEXT("PlayerActor");
const FName ADinosaurAIController::BB_PatrolTarget   = TEXT("PatrolTarget");
const FName ADinosaurAIController::BB_bIsChasing     = TEXT("bIsChasing");
const FName ADinosaurAIController::BB_bCanAttack     = TEXT("bCanAttack");
const FName ADinosaurAIController::BB_AttackCooldown = TEXT("AttackCooldown");
const FName ADinosaurAIController::BB_FlankIndex     = TEXT("FlankIndex");

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
ADinosaurAIController::ADinosaurAIController()
    : CurrentState(ECombat_DinoState::Idle)
    , TargetActor(nullptr)
    , FlankIndex(0)
    , AttackCooldownRemaining(0.0f)
    , StateEntryTime(0.0f)
    , DinoSpecies(ECombat_DinoSpecies::Generic)
{
    PrimaryActorTick.bCanEverTick = true;

    // Create perception component
    PerceptionComponent_Combat = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent_Combat")
    );
    SetPerceptionComponent(*PerceptionComponent_Combat);

    // Sight config — defaults (overridden per species in InitPerceptionForSpecies)
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                = 3000.0f;
    SightConfig->LoseSightRadius            = 4000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    PerceptionComponent_Combat->ConfigureSense(*SightConfig);
    PerceptionComponent_Combat->ConfigureSense(*HearingConfig);
    PerceptionComponent_Combat->SetDominantSense(SightConfig->GetSenseImplementation());
}

// -----------------------------------------------------------------------
// OnPossess — start BT and wire perception callback
// -----------------------------------------------------------------------
void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    InitPerceptionForSpecies();

    PerceptionComponent_Combat->OnPerceptionUpdated.AddDynamic(
        this, &ADinosaurAIController::OnPerceptionUpdated
    );

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    // Seed blackboard with initial values
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsBool(BB_bIsChasing, false);
        BB->SetValueAsBool(BB_bCanAttack, false);
        BB->SetValueAsInt(BB_FlankIndex, FlankIndex);
    }

    SetCombatState(ECombat_DinoState::Patrolling);
}

// -----------------------------------------------------------------------
// OnUnPossess
// -----------------------------------------------------------------------
void ADinosaurAIController::OnUnPossess()
{
    Super::OnUnPossess();
    PerceptionComponent_Combat->OnPerceptionUpdated.RemoveDynamic(
        this, &ADinosaurAIController::OnPerceptionUpdated
    );
}

// -----------------------------------------------------------------------
// Tick — drive FSM per species
// -----------------------------------------------------------------------
void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
        if (AttackCooldownRemaining <= 0.0f)
        {
            if (UBlackboardComponent* BB = GetBlackboardComponent())
            {
                BB->SetValueAsBool(BB_bCanAttack, true);
            }
        }
    }

    switch (DinoSpecies)
    {
        case ECombat_DinoSpecies::TRex:
            HandleTRexCombat(DeltaTime);
            break;
        case ECombat_DinoSpecies::Velociraptor:
            HandleRaptorCombat(DeltaTime);
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------
// OnPerceptionUpdated — detect player entering sight/hearing range
// -----------------------------------------------------------------------
void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    for (AActor* Actor : UpdatedActors)
    {
        if (Actor == PlayerPawn)
        {
            FActorPerceptionBlueprintInfo Info;
            PerceptionComponent_Combat->GetActorsPerception(Actor, Info);

            bool bCurrentlySensed = false;
            for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    bCurrentlySensed = true;
                    break;
                }
            }

            if (bCurrentlySensed)
            {
                SetTargetActor(PlayerPawn);
                SetCombatState(ECombat_DinoState::Chasing);
            }
            else if (CurrentState == ECombat_DinoState::Chasing && TargetActor == PlayerPawn)
            {
                SetCombatState(ECombat_DinoState::Investigating);
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------
// SetCombatState
// -----------------------------------------------------------------------
void ADinosaurAIController::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsBool(BB_bIsChasing,
            NewState == ECombat_DinoState::Chasing ||
            NewState == ECombat_DinoState::Attacking);
    }

    // Adjust movement speed based on state
    if (APawn* Pawn = GetPawn())
    {
        if (ACharacter* Char = Cast<ACharacter>(Pawn))
        {
            UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
            if (MoveComp)
            {
                switch (NewState)
                {
                    case ECombat_DinoState::Chasing:
                    case ECombat_DinoState::Attacking:
                        MoveComp->MaxWalkSpeed = PerceptionConfig.ChaseSpeed;
                        break;
                    case ECombat_DinoState::Patrolling:
                    case ECombat_DinoState::Idle:
                        MoveComp->MaxWalkSpeed = PerceptionConfig.PatrolSpeed;
                        break;
                    default:
                        MoveComp->MaxWalkSpeed = PerceptionConfig.PatrolSpeed * 1.2f;
                        break;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------
// SetTargetActor
// -----------------------------------------------------------------------
void ADinosaurAIController::SetTargetActor(AActor* Target)
{
    TargetActor = Target;
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(BB_PlayerActor, Target);
    }
}

// -----------------------------------------------------------------------
// GetDistanceToTarget
// -----------------------------------------------------------------------
float ADinosaurAIController::GetDistanceToTarget() const
{
    if (!TargetActor || !GetPawn()) return MAX_FLT;
    return FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
}

// -----------------------------------------------------------------------
// TriggerRaptorFlank — signal pack members to execute flanking manoeuvre
// -----------------------------------------------------------------------
void ADinosaurAIController::TriggerRaptorFlank()
{
    if (DinoSpecies != ECombat_DinoSpecies::Velociraptor) return;
    if (!TargetActor) return;

    FVector PlayerLoc = TargetActor->GetActorLocation();
    FVector Forward   = (PlayerLoc - GetPawn()->GetActorLocation()).GetSafeNormal();
    FVector Right     = FVector::CrossProduct(Forward, FVector::UpVector);

    // Assign flank positions to pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (!PackMembers[i]) continue;

        FVector FlankTarget;
        switch (i % 3)
        {
            case 0: // Frontal distraction — charge straight at player
                FlankTarget = PlayerLoc + Forward * 200.0f;
                break;
            case 1: // Left flank
                FlankTarget = PlayerLoc - Right * 600.0f + Forward * 100.0f;
                break;
            case 2: // Right flank
                FlankTarget = PlayerLoc + Right * 600.0f + Forward * 100.0f;
                break;
            default:
                FlankTarget = PlayerLoc;
                break;
        }

        PackMembers[i]->SetTargetActor(TargetActor);
        PackMembers[i]->MoveToLocation(FlankTarget, 150.0f);

        if (UBlackboardComponent* BB = PackMembers[i]->GetBlackboardComponent())
        {
            BB->SetValueAsInt(BB_FlankIndex, i % 3);
            BB->SetValueAsBool(BB_bIsChasing, true);
        }
    }
}

// -----------------------------------------------------------------------
// InitPerceptionForSpecies — tune sight/hearing per species
// -----------------------------------------------------------------------
void ADinosaurAIController::InitPerceptionForSpecies()
{
    switch (DinoSpecies)
    {
        case ECombat_DinoSpecies::TRex:
            PerceptionConfig.SightRadius             = 3500.0f;
            PerceptionConfig.LoseSightRadius         = 5000.0f;
            PerceptionConfig.PeripheralVisionAngle   = 70.0f;   // Narrow — T-Rex has poor peripheral
            PerceptionConfig.HearingRange            = 2500.0f; // Excellent hearing
            PerceptionConfig.AttackRange             = 450.0f;
            PerceptionConfig.ChaseSpeed              = 1200.0f;
            PerceptionConfig.PatrolSpeed             = 350.0f;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            PerceptionConfig.SightRadius             = 2500.0f;
            PerceptionConfig.LoseSightRadius         = 3500.0f;
            PerceptionConfig.PeripheralVisionAngle   = 120.0f;  // Wide — raptors had excellent vision
            PerceptionConfig.HearingRange            = 1200.0f;
            PerceptionConfig.AttackRange             = 200.0f;
            PerceptionConfig.ChaseSpeed              = 1600.0f; // Fastest dinosaur
            PerceptionConfig.PatrolSpeed             = 500.0f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            PerceptionConfig.SightRadius             = 2000.0f;
            PerceptionConfig.LoseSightRadius         = 3000.0f;
            PerceptionConfig.PeripheralVisionAngle   = 100.0f;
            PerceptionConfig.HearingRange            = 1000.0f;
            PerceptionConfig.AttackRange             = 350.0f;
            PerceptionConfig.ChaseSpeed              = 900.0f;
            PerceptionConfig.PatrolSpeed             = 300.0f;
            break;

        default:
            break;
    }

    // Apply updated config to perception component
    if (SightConfig)
    {
        SightConfig->SightRadius                  = PerceptionConfig.SightRadius;
        SightConfig->LoseSightRadius              = PerceptionConfig.LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PerceptionConfig.PeripheralVisionAngle;
        PerceptionComponent_Combat->ConfigureSense(*SightConfig);
    }
    if (HearingConfig)
    {
        HearingConfig->HearingRange = PerceptionConfig.HearingRange;
        PerceptionComponent_Combat->ConfigureSense(*HearingConfig);
    }
}

// -----------------------------------------------------------------------
// HandleTRexCombat — T-Rex specific combat logic (called from Tick)
// -----------------------------------------------------------------------
void ADinosaurAIController::HandleTRexCombat(float DeltaTime)
{
    if (CurrentState != ECombat_DinoState::Chasing &&
        CurrentState != ECombat_DinoState::Attacking) return;

    float DistToTarget = GetDistanceToTarget();

    if (DistToTarget <= PerceptionConfig.AttackRange)
    {
        if (AttackCooldownRemaining <= 0.0f)
        {
            SetCombatState(ECombat_DinoState::Attacking);
            if (UBlackboardComponent* BB = GetBlackboardComponent())
            {
                BB->SetValueAsBool(BB_bCanAttack, true);
            }
            AttackCooldownRemaining = 3.0f; // T-Rex attacks every 3 seconds
        }
    }
    else if (DistToTarget > PerceptionConfig.LoseSightRadius)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        SetTargetActor(nullptr);
    }
}

// -----------------------------------------------------------------------
// HandleRaptorCombat — Raptor pack logic (called from Tick)
// -----------------------------------------------------------------------
void ADinosaurAIController::HandleRaptorCombat(float DeltaTime)
{
    if (CurrentState != ECombat_DinoState::Chasing &&
        CurrentState != ECombat_DinoState::Attacking) return;

    float DistToTarget = GetDistanceToTarget();

    // Trigger flanking when pack leader (FlankIndex==0) gets within 1000 units
    if (FlankIndex == 0 && DistToTarget < 1000.0f && PackMembers.Num() > 0)
    {
        TriggerRaptorFlank();
    }

    if (DistToTarget <= PerceptionConfig.AttackRange)
    {
        if (AttackCooldownRemaining <= 0.0f)
        {
            SetCombatState(ECombat_DinoState::Attacking);
            if (UBlackboardComponent* BB = GetBlackboardComponent())
            {
                BB->SetValueAsBool(BB_bCanAttack, true);
            }
            AttackCooldownRemaining = 1.5f; // Raptors attack faster
        }
    }
    else if (DistToTarget > PerceptionConfig.LoseSightRadius)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        SetTargetActor(nullptr);
    }
}

// -----------------------------------------------------------------------
// UpdateBlackboard — sync all state to blackboard
// -----------------------------------------------------------------------
void ADinosaurAIController::UpdateBlackboard()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    BB->SetValueAsObject(BB_PlayerActor, TargetActor);
    BB->SetValueAsBool(BB_bIsChasing,
        CurrentState == ECombat_DinoState::Chasing ||
        CurrentState == ECombat_DinoState::Attacking);
    BB->SetValueAsBool(BB_bCanAttack, AttackCooldownRemaining <= 0.0f);
    BB->SetValueAsFloat(BB_AttackCooldown, AttackCooldownRemaining);
    BB->SetValueAsInt(BB_FlankIndex, FlankIndex);
}
