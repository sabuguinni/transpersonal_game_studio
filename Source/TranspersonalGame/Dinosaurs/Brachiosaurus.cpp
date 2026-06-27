#include "Brachiosaurus.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ABrachiosaurus::ABrachiosaurus()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species stats ────────────────────────────────────────────────────────
    MaxHealth       = 800.0f;
    CurrentHealth   = 800.0f;
    MoveSpeed       = 200.0f;
    AttackDamage    = 0.0f;   // Herbivore — no bite attack
    DetectionRadius = 1800.0f;
    bIsCarnivore    = false;

    // ── Movement tuning ──────────────────────────────────────────────────────
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed           = MoveSpeed;
        MoveComp->MaxAcceleration        = 300.0f;
        MoveComp->BrakingDecelerationWalking = 400.0f;
        MoveComp->RotationRate           = FRotator(0.0f, 60.0f, 0.0f); // Slow turns
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale           = 1.0f;
    }

    bUseControllerRotationYaw = false;
}

// ─────────────────────────────────────────────────────────────────────────────

void ABrachiosaurus::BeginPlay()
{
    Super::BeginPlay();
    StartGrazingCycle();
}

void ABrachiosaurus::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic herd spacing check (every 2 seconds)
    TimeSinceLastHerdCheck += DeltaTime;
    if (TimeSinceLastHerdCheck >= 2.0f)
    {
        TimeSinceLastHerdCheck = 0.0f;

        // Gentle separation from herd-mates that are too close
        TArray<ABrachiosaurus*> HerdMembers = GetNearbyHerdMembers();
        for (ABrachiosaurus* Member : HerdMembers)
        {
            if (!Member || Member == this) continue;
            float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
            if (Dist < HerdSpacingRadius)
            {
                FVector AwayDir = (GetActorLocation() - Member->GetActorLocation()).GetSafeNormal();
                AddMovementInput(AwayDir, 0.3f);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AI State — herbivore overrides

void ABrachiosaurus::UpdateAIState()
{
    // Brachiosaurus never enters Hunting or Attacking states
    // It can only be: Idle, Grazing (custom), or Fleeing
    if (CurrentState == EWorld_DinoState::Hunting ||
        CurrentState == EWorld_DinoState::Attacking)
    {
        CurrentState = EWorld_DinoState::Idle;
    }

    // Delegate remaining state logic to base class
    Super::UpdateAIState();
}

// ─────────────────────────────────────────────────────────────────────────────
// Damage handling

float ABrachiosaurus::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.0f)
    {
        // Alert herd immediately
        AlertHerd(DamageCauser);

        // Check flee threshold
        float HealthFraction = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
        if (HealthFraction <= FleeHealthThreshold)
        {
            AttemptEscapeOrDefend(DamageCauser);
        }
    }

    return ActualDamage;
}

// ─────────────────────────────────────────────────────────────────────────────
// Herd alert

void ABrachiosaurus::AlertHerd(AActor* Threat)
{
    TArray<ABrachiosaurus*> HerdMembers = GetNearbyHerdMembers();
    for (ABrachiosaurus* Member : HerdMembers)
    {
        if (Member && Member != this)
        {
            Member->ReceiveHerdFleeSignal(Threat);
        }
    }
}

void ABrachiosaurus::ReceiveHerdFleeSignal(AActor* Threat)
{
    if (!Threat) return;

    // Only react if not already fleeing
    if (CurrentState != EWorld_DinoState::Fleeing)
    {
        CurrentState = EWorld_DinoState::Fleeing;

        // Run away from the threat
        FVector FleeDir = (GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
        AddMovementInput(FleeDir, 1.0f);

        UE_LOG(LogTemp, Log, TEXT("ABrachiosaurus [%s]: Received herd flee signal from threat [%s]"),
            *GetName(), *Threat->GetName());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Escape or defend

void ABrachiosaurus::AttemptEscapeOrDefend(AActor* Threat)
{
    if (!Threat) return;

    // Simple escape check: if threat is within 500 units and directly behind, we're cornered
    FVector ToThreat = (Threat->GetActorLocation() - GetActorLocation());
    float ThreatDist  = ToThreat.Size();
    float DotProduct  = FVector::DotProduct(GetActorForwardVector(), ToThreat.GetSafeNormal());

    bool bCornered = (ThreatDist < 500.0f && DotProduct < -0.5f);

    if (bCornered && bTailSwipeReady)
    {
        PerformTailSwipe();
    }
    else
    {
        // Flee
        CurrentState = EWorld_DinoState::Fleeing;
        FVector FleeDir = -ToThreat.GetSafeNormal();
        AddMovementInput(FleeDir, 1.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tail swipe

void ABrachiosaurus::PerformTailSwipe()
{
    if (!bTailSwipeReady) return;

    bTailSwipeReady = false;

    // Sphere sweep behind the dinosaur
    FVector TailOrigin = GetActorLocation() - GetActorForwardVector() * 300.0f;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        TailOrigin,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(TailSwipeRadius),
        QueryParams
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
                    TailSwipeDamage,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );
                UE_LOG(LogTemp, Log, TEXT("ABrachiosaurus [%s]: Tail swipe hit [%s] for %.1f damage"),
                    *GetName(), *HitActor->GetName(), TailSwipeDamage);
            }
        }
    }

    // Debug visualisation
#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), TailOrigin, TailSwipeRadius, 12,
                    FColor::Orange, false, 1.5f);
#endif

    // Start cooldown
    GetWorldTimerManager().SetTimer(
        TailSwipeCooldownHandle,
        [this]() { bTailSwipeReady = true; },
        TailSwipeCooldown,
        false
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// Grazing

void ABrachiosaurus::StartGrazingCycle()
{
    GetWorldTimerManager().SetTimer(
        GrazeTimerHandle,
        this,
        &ABrachiosaurus::OnGrazeTimerTick,
        GrazeInterval,
        true   // Loop
    );
}

void ABrachiosaurus::OnGrazeTimerTick()
{
    if (CurrentState == EWorld_DinoState::Idle)
    {
        bIsGrazing = true;
        UE_LOG(LogTemp, Verbose, TEXT("ABrachiosaurus [%s]: Grazing started"), *GetName());

        // Stop grazing after GrazeDuration
        FTimerHandle StopGrazeHandle;
        GetWorldTimerManager().SetTimer(
            StopGrazeHandle,
            [this]() { bIsGrazing = false; },
            GrazeDuration,
            false
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Herd query

TArray<ABrachiosaurus*> ABrachiosaurus::GetNearbyHerdMembers() const
{
    TArray<ABrachiosaurus*> Result;
    if (!GetWorld()) return Result;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrachiosaurus::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        ABrachiosaurus* Member = Cast<ABrachiosaurus>(Actor);
        if (Member && Member != this)
        {
            float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
            if (Dist <= HerdAlertRadius)
            {
                Result.Add(Member);
            }
        }
    }

    return Result;
}
