// TRexCharacter.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_001
// Tyrannosaurus Rex — full implementation

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Movement — T-Rex is massive, slow turn rate, high mass
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->Mass = 8000.0f;
        MoveComp->JumpZVelocity = 0.0f;    // T-Rex cannot jump
        MoveComp->AirControl = 0.0f;
        MoveComp->GravityScale = 1.5f;     // Heavy — sticks to ground
        MoveComp->MaxStepHeight = 120.0f;  // Can step over large obstacles
        MoveComp->bCanWalkOffLedges = false;
    }

    CurrentState = EWorld_TRexState::Idle;
    bUseControllerRotationYaw = false;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial walk speed from stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Alpha T-Rex gets a 20% stat bonus
    if (bIsAlpha)
    {
        Stats.MaxHealth *= 1.2f;
        Stats.Health = Stats.MaxHealth;
        Stats.AttackDamage *= 1.2f;
        Stats.DetectionRadius *= 1.3f;
    }

    SetBehaviourState(EWorld_TRexState::Patrolling);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    UpdateHunger(DeltaTime);
    UpdateBehaviour(DeltaTime);
    StateTimer += DeltaTime;
}

// ── Hunger ───────────────────────────────────────────────────────────────────

void ATRexCharacter::UpdateHunger(float DeltaTime)
{
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - HungerDecayRate * DeltaTime);

    // Hungry T-Rex actively hunts
    if (Stats.Hunger < 30.0f && CurrentState == EWorld_TRexState::Patrolling)
    {
        TransitionToHunting();
    }
}

// ── Behaviour State Machine ───────────────────────────────────────────────────

void ATRexCharacter::UpdateBehaviour(float DeltaTime)
{
    switch (CurrentState)
    {
    case EWorld_TRexState::Idle:
        // After 5s idle, start patrolling
        if (StateTimer > 5.0f)
        {
            SetBehaviourState(EWorld_TRexState::Patrolling);
        }
        break;

    case EWorld_TRexState::Patrolling:
        // Check for prey every 2 seconds
        if (StateTimer > 2.0f)
        {
            StateTimer = 0.0f;
            AActor* Prey = FindNearestPrey();
            if (Prey)
            {
                TransitionToHunting();
            }
        }
        break;

    case EWorld_TRexState::Hunting:
    {
        AActor* Prey = FindNearestPrey();
        if (!Prey)
        {
            SetBehaviourState(EWorld_TRexState::Patrolling);
            break;
        }

        float DistToPrey = FVector::Dist(GetActorLocation(), Prey->GetActorLocation());
        if (DistToPrey <= Stats.AttackRadius)
        {
            SetBehaviourState(EWorld_TRexState::Attacking);
        }
        break;
    }

    case EWorld_TRexState::Attacking:
        if (StateTimer > 1.5f)
        {
            PerformBite();
            StateTimer = 0.0f;
        }
        break;

    case EWorld_TRexState::Roaring:
        if (StateTimer > 3.0f)
        {
            SetBehaviourState(EWorld_TRexState::Hunting);
        }
        break;

    case EWorld_TRexState::Sleeping:
        // Wake up when hungry
        if (Stats.Hunger < 20.0f)
        {
            SetBehaviourState(EWorld_TRexState::Idle);
        }
        break;

    default:
        break;
    }
}

void ATRexCharacter::SetBehaviourState(EWorld_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EWorld_TRexState::Hunting:
        case EWorld_TRexState::Attacking:
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
            break;
        case EWorld_TRexState::Sleeping:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }
}

void ATRexCharacter::TransitionToHunting()
{
    // Roar before hunting if cooldown allows
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastRoarTime > RoarCooldown)
    {
        Roar();
    }
    else
    {
        SetBehaviourState(EWorld_TRexState::Hunting);
    }
}

// ── Combat ────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformBite()
{
    if (!GetWorld()) return;

    // Sphere overlap in front of T-Rex for bite
    FVector BiteOrigin = GetActorLocation() + GetActorForwardVector() * Stats.AttackRadius;
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        BiteOrigin,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(Stats.AttackRadius * 0.5f),
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                UGameplayStatics::ApplyDamage(
                    HitActor,
                    Stats.AttackDamage,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
    }

    // Feed on successful bite
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + 15.0f);
}

void ATRexCharacter::PerformStomp()
{
    if (!GetWorld()) return;

    // Radial damage from stomp — affects area around feet
    FVector StompOrigin = GetActorLocation();
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        Stats.StompDamage,
        StompOrigin,
        Stats.StompDamageRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetController(),
        true,
        ECC_Visibility
    );
}

void ATRexCharacter::Roar()
{
    if (!GetWorld()) return;

    LastRoarTime = GetWorld()->GetTimeSeconds();
    SetBehaviourState(EWorld_TRexState::Roaring);

    // Roar scares nearby prey — apply fear impulse (logged for AI to pick up)
    UE_LOG(LogTemp, Warning, TEXT("TRex ROAR at location %s"), *GetActorLocation().ToString());
}

float ATRexCharacter::ApplyDamage(float DamageAmount)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);

    if (Stats.Health <= 0.0f)
    {
        // Death — ragdoll
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->DisableMovement();
        }
        SetActorEnableCollision(false);
        UE_LOG(LogTemp, Warning, TEXT("TRex %s has died!"), *GetActorLabel());
    }
    else if (Stats.Health < Stats.MaxHealth * 0.3f && CurrentState != EWorld_TRexState::Fleeing)
    {
        // Flee when critically wounded
        SetBehaviourState(EWorld_TRexState::Fleeing);
    }

    return Stats.Health;
}

// ── Detection ─────────────────────────────────────────────────────────────────

AActor* ATRexCharacter::FindNearestPrey() const
{
    if (!GetWorld()) return nullptr;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllActors);

    AActor* NearestPrey = nullptr;
    float NearestDist = Stats.DetectionRadius;

    for (AActor* Actor : AllActors)
    {
        if (Actor == this) continue;

        // Skip other T-Rex instances
        if (Actor->IsA(ATRexCharacter::StaticClass())) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < NearestDist && CanSeeActor(Actor))
        {
            NearestDist = Dist;
            NearestPrey = Actor;
        }
    }

    return NearestPrey;
}

bool ATRexCharacter::CanSeeActor(AActor* Target) const
{
    if (!Target || !GetWorld()) return false;

    // Line-of-sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(Target);

    FVector EyeLocation = GetActorLocation() + FVector(0, 0, 200.0f); // Eye height
    FVector TargetLocation = Target->GetActorLocation();

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        EyeLocation,
        TargetLocation,
        ECC_Visibility,
        Params
    );

    return !bBlocked;
}
