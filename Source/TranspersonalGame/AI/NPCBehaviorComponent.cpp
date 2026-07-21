// NPCBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent
// Full implementation of NPC daily routine, state machine, memory, and personality system

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PrimaryTrait = ENPC_PersonalityTrait::Cautious;
    AlertRadius = 1500.0f;
    FleeRadius = 800.0f;
    PatrolRadius = 2000.0f;
    HomeLocation = FVector::ZeroVector;
    bHomeLocationSet = false;
    StateTimer = 0.0f;
    IdleDuration = 3.0f;
    PatrolWaitDuration = 2.0f;
    MemoryDecayRate = 0.05f;
    bIsDead = false;
    CurrentThreatLevel = 0.0f;
    LastKnownThreatLocation = FVector::ZeroVector;
    bHasThreatMemory = false;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        HomeLocation = Owner->GetActorLocation();
        bHomeLocationSet = true;
    }

    // Build initial daily routine if none set
    if (DailyRoutine.Num() == 0)
    {
        BuildDefaultDailyRoutine();
    }

    // Start first behavior evaluation
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorTickHandle,
        this,
        &UNPCBehaviorComponent::EvaluateBehavior,
        0.5f,
        true
    );
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    StateTimer += DeltaTime;

    // Decay threat memory over time
    if (bHasThreatMemory)
    {
        CurrentThreatLevel = FMath::Max(0.0f, CurrentThreatLevel - MemoryDecayRate * DeltaTime);
        if (CurrentThreatLevel <= 0.0f)
        {
            bHasThreatMemory = false;
            LastKnownThreatLocation = FVector::ZeroVector;
        }
    }

    // Debug visualization
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            DrawDebugSphere(GetWorld(), Owner->GetActorLocation() + FVector(0, 0, 100),
                50.0f, 8, GetStateDebugColor(), false, 0.15f);
        }
    }
#endif
}

void UNPCBehaviorComponent::EvaluateBehavior()
{
    if (bIsDead) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Check for nearby threats first — highest priority
    APawn* NearestThreat = DetectNearestThreat();
    if (NearestThreat)
    {
        float DistToThreat = FVector::Dist(Owner->GetActorLocation(), NearestThreat->GetActorLocation());

        if (DistToThreat < FleeRadius)
        {
            TransitionToState(ENPC_BehaviorState::Flee);
            RegisterThreatMemory(NearestThreat->GetActorLocation(), 1.0f);
            return;
        }
        else if (DistToThreat < AlertRadius)
        {
            TransitionToState(ENPC_BehaviorState::Alert);
            RegisterThreatMemory(NearestThreat->GetActorLocation(), 0.6f);
            return;
        }
    }

    // Check residual threat memory
    if (bHasThreatMemory && CurrentThreatLevel > 0.3f)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
        return;
    }

    // Time-of-day routine evaluation
    FNPC_DailyRoutineEntry* CurrentRoutineEntry = GetCurrentRoutineEntry();
    if (CurrentRoutineEntry)
    {
        ENPC_BehaviorState DesiredState = CurrentRoutineEntry->DesiredState;
        if (CurrentBehaviorState != DesiredState)
        {
            TransitionToState(DesiredState);
        }
    }
    else
    {
        // Default fallback — patrol home area
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle && StateTimer > IdleDuration)
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
        else if (CurrentBehaviorState == ENPC_BehaviorState::Patrol && StateTimer > 15.0f)
        {
            TransitionToState(ENPC_BehaviorState::Idle);
        }
    }
}

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateTimer = 0.0f;

    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    // Execute entry logic for new state
    switch (NewState)
    {
        case ENPC_BehaviorState::Patrol:
            ExecutePatrolBehavior();
            break;
        case ENPC_BehaviorState::Flee:
            ExecuteFleeBehavior();
            break;
        case ENPC_BehaviorState::Alert:
            ExecuteAlertBehavior();
            break;
        case ENPC_BehaviorState::Forage:
            ExecuteForageBehavior();
            break;
        case ENPC_BehaviorState::Rest:
            ExecuteRestBehavior();
            break;
        case ENPC_BehaviorState::Dead:
            bIsDead = true;
            OnNPCDied.Broadcast();
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::ExecutePatrolBehavior()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    FNavLocation RandomPoint;
    FVector Origin = bHomeLocationSet ? HomeLocation : Owner->GetActorLocation();

    if (NavSys->GetRandomReachablePointInRadius(Origin, PatrolRadius, RandomPoint))
    {
        PatrolDestination = RandomPoint.Location;

        AAIController* AIC = Cast<AAIController>(Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
        if (AIC)
        {
            AIC->MoveToLocation(PatrolDestination, 50.0f);
        }
    }
}

void UNPCBehaviorComponent::ExecuteFleeBehavior()
{
    AActor* Owner = GetOwner();
    if (!Owner || !bHasThreatMemory) return;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    // Flee away from threat
    FVector AwayDir = (Owner->GetActorLocation() - LastKnownThreatLocation).GetSafeNormal();
    FVector FleeTarget = Owner->GetActorLocation() + AwayDir * 3000.0f;

    FNavLocation FleePoint;
    if (NavSys->GetRandomReachablePointInRadius(FleeTarget, 500.0f, FleePoint))
    {
        AAIController* AIC = Cast<AAIController>(Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
        if (AIC)
        {
            AIC->MoveToLocation(FleePoint.Location, 50.0f);
        }
    }
}

void UNPCBehaviorComponent::ExecuteAlertBehavior()
{
    // Face threat direction, slow movement
    AActor* Owner = GetOwner();
    if (!Owner || !bHasThreatMemory) return;

    FVector LookDir = (LastKnownThreatLocation - Owner->GetActorLocation()).GetSafeNormal();
    FRotator LookRot = LookDir.Rotation();
    Owner->SetActorRotation(FRotator(0, LookRot.Yaw, 0));
}

void UNPCBehaviorComponent::ExecuteForageBehavior()
{
    // Move to a random nearby point to simulate foraging
    ExecutePatrolBehavior();
}

void UNPCBehaviorComponent::ExecuteRestBehavior()
{
    // Stop movement — NPC crouches/sits
    AActor* Owner = GetOwner();
    if (!Owner) return;

    AAIController* AIC = Cast<AAIController>(Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
    if (AIC)
    {
        AIC->StopMovement();
    }
}

APawn* UNPCBehaviorComponent::DetectNearestThreat()
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    APawn* NearestThreat = nullptr;
    float NearestDist = AlertRadius;

    for (AActor* Actor : FoundActors)
    {
        if (Actor == Owner) continue;

        APawn* PawnActor = Cast<APawn>(Actor);
        if (!PawnActor) continue;

        // Check if player-controlled (treat as potential threat based on personality)
        APlayerController* PC = Cast<APlayerController>(PawnActor->GetController());
        if (PC)
        {
            float Dist = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
            if (Dist < NearestDist)
            {
                // Timid NPCs flee from player, Aggressive ones don't
                if (PrimaryTrait == ENPC_PersonalityTrait::Timid ||
                    PrimaryTrait == ENPC_PersonalityTrait::Cautious)
                {
                    NearestDist = Dist;
                    NearestThreat = PawnActor;
                }
            }
        }
    }

    return NearestThreat;
}

void UNPCBehaviorComponent::RegisterThreatMemory(FVector ThreatLocation, float ThreatStrength)
{
    LastKnownThreatLocation = ThreatLocation;
    CurrentThreatLevel = FMath::Clamp(CurrentThreatLevel + ThreatStrength, 0.0f, 1.0f);
    bHasThreatMemory = true;
}

FNPC_DailyRoutineEntry* UNPCBehaviorComponent::GetCurrentRoutineEntry()
{
    if (DailyRoutine.Num() == 0) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Get current game time (0.0 - 24.0)
    float GameHour = FMath::Fmod(World->GetTimeSeconds() / 120.0f, 24.0f);

    for (int32 i = 0; i < DailyRoutine.Num(); i++)
    {
        float StartHour = DailyRoutine[i].StartHour;
        float EndHour = (i + 1 < DailyRoutine.Num()) ? DailyRoutine[i + 1].StartHour : 24.0f;

        if (GameHour >= StartHour && GameHour < EndHour)
        {
            return &DailyRoutine[i];
        }
    }

    return nullptr;
}

void UNPCBehaviorComponent::BuildDefaultDailyRoutine()
{
    // Dawn (6:00) — Forage
    FNPC_DailyRoutineEntry Dawn;
    Dawn.StartHour = 6.0f;
    Dawn.DesiredState = ENPC_BehaviorState::Forage;
    Dawn.RoutineLabel = TEXT("Morning Foraging");
    DailyRoutine.Add(Dawn);

    // Midday (12:00) — Rest (avoid heat/predators)
    FNPC_DailyRoutineEntry Midday;
    Midday.StartHour = 12.0f;
    Midday.DesiredState = ENPC_BehaviorState::Rest;
    Midday.RoutineLabel = TEXT("Midday Rest");
    DailyRoutine.Add(Midday);

    // Afternoon (15:00) — Patrol territory
    FNPC_DailyRoutineEntry Afternoon;
    Afternoon.StartHour = 15.0f;
    Afternoon.DesiredState = ENPC_BehaviorState::Patrol;
    Afternoon.RoutineLabel = TEXT("Afternoon Patrol");
    DailyRoutine.Add(Afternoon);

    // Dusk (18:00) — Forage again
    FNPC_DailyRoutineEntry Dusk;
    Dusk.StartHour = 18.0f;
    Dusk.DesiredState = ENPC_BehaviorState::Forage;
    Dusk.RoutineLabel = TEXT("Evening Foraging");
    DailyRoutine.Add(Dusk);

    // Night (21:00) — Rest/Sleep
    FNPC_DailyRoutineEntry Night;
    Night.StartHour = 21.0f;
    Night.DesiredState = ENPC_BehaviorState::Rest;
    Night.RoutineLabel = TEXT("Night Rest");
    DailyRoutine.Add(Night);
}

FColor UNPCBehaviorComponent::GetStateDebugColor() const
{
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:    return FColor::White;
        case ENPC_BehaviorState::Patrol:  return FColor::Green;
        case ENPC_BehaviorState::Forage:  return FColor::Yellow;
        case ENPC_BehaviorState::Rest:    return FColor::Blue;
        case ENPC_BehaviorState::Flee:    return FColor::Orange;
        case ENPC_BehaviorState::Alert:   return FColor::Red;
        case ENPC_BehaviorState::Interact:return FColor::Cyan;
        case ENPC_BehaviorState::Dead:    return FColor::Black;
        default:                          return FColor::White;
    }
}

void UNPCBehaviorComponent::SetDead()
{
    TransitionToState(ENPC_BehaviorState::Dead);
}

void UNPCBehaviorComponent::ForceState(ENPC_BehaviorState NewState)
{
    TransitionToState(NewState);
}

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentState() const
{
    return CurrentBehaviorState;
}

float UNPCBehaviorComponent::GetThreatLevel() const
{
    return CurrentThreatLevel;
}
