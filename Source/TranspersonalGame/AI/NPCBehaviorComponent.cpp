#include "NPCBehaviorComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance-friendly
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesDefaults();
    NPC_CurrentState = ENPC_BehaviorState::Idle;
    NPC_StateTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdatePerception(DeltaTime);
    UpdateStateMachine(DeltaTime);
}

void UNPCBehaviorComponent::ApplySpeciesDefaults()
{
    switch (NPC_Species)
    {
    case ENPC_DinoSpecies::TRex:
        NPC_Config.PatrolRadius   = 5000.0f;
        NPC_Config.ChaseRange     = 3000.0f;
        NPC_Config.AttackRange    = 300.0f;
        NPC_Config.MoveSpeed      = 600.0f;
        NPC_Config.bIsPassive     = false;
        NPC_Config.PackSize       = 1;
        NPC_Config.FearRadiusNear = 2000.0f;
        NPC_Config.FearRadiusFar  = 4000.0f;
        break;

    case ENPC_DinoSpecies::Raptor:
        NPC_Config.PatrolRadius   = 3000.0f;
        NPC_Config.ChaseRange     = 2000.0f;
        NPC_Config.AttackRange    = 200.0f;
        NPC_Config.MoveSpeed      = 900.0f;
        NPC_Config.bIsPassive     = false;
        NPC_Config.PackSize       = 3;
        NPC_Config.FearRadiusNear = 1200.0f;
        NPC_Config.FearRadiusFar  = 2500.0f;
        break;

    case ENPC_DinoSpecies::Brachiosaurus:
        NPC_Config.PatrolRadius   = 2000.0f;
        NPC_Config.ChaseRange     = 0.0f;
        NPC_Config.AttackRange    = 0.0f;
        NPC_Config.MoveSpeed      = 300.0f;
        NPC_Config.bIsPassive     = true;
        NPC_Config.PackSize       = 1;
        NPC_Config.FearRadiusNear = 0.0f;
        NPC_Config.FearRadiusFar  = 0.0f;
        break;

    default:
        // Generic defaults already set in USTRUCT
        break;
    }
}

APawn* UNPCBehaviorComponent::FindPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    return PC->GetPawn();
}

void UNPCBehaviorComponent::UpdatePerception(float DeltaTime)
{
    APawn* Player = FindPlayerPawn();
    if (!Player)
    {
        NPC_Perception.bPlayerDetected = false;
        NPC_Perception.DistanceToPlayer = 99999.0f;
        NPC_Perception.TimeSinceLastSeen += DeltaTime;
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
    NPC_Perception.DistanceToPlayer = Dist;

    bool bWasDetected = NPC_Perception.bPlayerDetected;

    if (Dist <= NPC_Config.ChaseRange)
    {
        NPC_Perception.bPlayerDetected = true;
        NPC_Perception.LastKnownPlayerLocation = Player->GetActorLocation();
        NPC_Perception.TimeSinceLastSeen = 0.0f;
    }
    else
    {
        NPC_Perception.bPlayerDetected = false;
        NPC_Perception.TimeSinceLastSeen += DeltaTime;
    }

    // Fear output for player AnimInstance — inverse: closer dino = higher fear
    if (NPC_Config.FearRadiusFar > 0.0f)
    {
        float FearDist = FMath::Clamp(Dist, NPC_Config.FearRadiusNear, NPC_Config.FearRadiusFar);
        NPC_FearNormalized = 1.0f - ((FearDist - NPC_Config.FearRadiusNear) / (NPC_Config.FearRadiusFar - NPC_Config.FearRadiusNear));
        NPC_FearNormalized = FMath::Clamp(NPC_FearNormalized, 0.0f, 1.0f);
    }
    else
    {
        NPC_FearNormalized = 0.0f;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NPC_CurrentState != NewState)
    {
        NPC_CurrentState = NewState;
        NPC_StateTimer = 0.0f;
    }
}

void UNPCBehaviorComponent::UpdateStateMachine(float DeltaTime)
{
    NPC_StateTimer += DeltaTime;

    // Passive species never aggress
    if (NPC_Config.bIsPassive)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }

    float Dist = NPC_Perception.DistanceToPlayer;

    switch (NPC_CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        // After 3 seconds idle, start patrolling
        if (NPC_StateTimer > 3.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        // Immediate alert if player detected
        if (NPC_Perception.bPlayerDetected)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        break;

    case ENPC_BehaviorState::Patrol:
        if (NPC_Perception.bPlayerDetected)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        // Advance waypoint every 5 seconds
        NPC_PatrolTimer += DeltaTime;
        if (NPC_PatrolTimer > 5.0f && NPC_PatrolWaypoints.Num() > 0)
        {
            NPC_PatrolTimer = 0.0f;
            NPC_CurrentWaypointIndex = (NPC_CurrentWaypointIndex + 1) % NPC_PatrolWaypoints.Num();
        }
        break;

    case ENPC_BehaviorState::Alert:
        // Alert for 1.5s then decide: chase or return
        if (NPC_StateTimer > 1.5f)
        {
            if (NPC_Perception.bPlayerDetected && Dist <= NPC_Config.ChaseRange)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
        }
        break;

    case ENPC_BehaviorState::Chase:
        if (!NPC_Perception.bPlayerDetected && NPC_Perception.TimeSinceLastSeen > 5.0f)
        {
            // Lost player — return to patrol
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        else if (Dist <= NPC_Config.AttackRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Attack);
        }
        break;

    case ENPC_BehaviorState::Attack:
        if (Dist > NPC_Config.AttackRange * 1.5f)
        {
            // Player escaped attack range — resume chase
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        break;

    case ENPC_BehaviorState::Flee:
        // Flee for 8 seconds then return to idle
        if (NPC_StateTimer > 8.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;

    case ENPC_BehaviorState::Eat:
    case ENPC_BehaviorState::Rest:
        if (NPC_Perception.bPlayerDetected)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        if (NPC_StateTimer > 10.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;

    default:
        break;
    }
}
