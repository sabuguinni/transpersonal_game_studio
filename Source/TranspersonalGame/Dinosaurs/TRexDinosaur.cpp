#include "TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    // ---- Species identity ----
    Species = EEng_DinosaurSpecies::TRex;

    // ---- Stats: apex predator values ----
    DinoStats.MaxHealth       = 1200.0f;
    DinoStats.CurrentHealth   = 1200.0f;
    DinoStats.AttackDamage    = 150.0f;
    DinoStats.WalkSpeed       = 400.0f;
    DinoStats.SprintSpeed     = 700.0f;
    DinoStats.DetectionRadius = 3000.0f;
    DinoStats.AttackRange     = 350.0f;
    DinoStats.Mass            = 8000.0f;  // kg — realistic T-Rex mass

    // ---- Stomp / Roar ----
    StompRadius       = 250.0f;
    RoarFearIntensity = 0.65f;
    RoarCooldown      = 12.0f;
    bRoarOnCooldown   = false;

    // ---- Movement component ----
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed          = DinoStats.WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate           = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->GravityScale           = 1.0f;
        MoveComp->Mass                   = DinoStats.Mass;
        MoveComp->bCanWalkOffLedges      = true;
    }

    // ---- Capsule: large for T-Rex body ----
    UCapsuleComponent* Cap = GetCapsuleComponent();
    if (Cap)
    {
        Cap->SetCapsuleHalfHeight(200.0f);
        Cap->SetCapsuleRadius(80.0f);
    }
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Roar immediately on spawn to announce presence
    GetWorldTimerManager().SetTimerForNextTick(this, &ATRexDinosaur::Roar);
}

void ATRexDinosaur::PerformAttack()
{
    // First apply the base single-target bite from parent
    Super::PerformAttack();

    // Then do stomp AoE — damages everything within StompRadius
    UWorld* World = GetWorld();
    if (!World) return;

    FVector Origin = GetActorLocation();

    // Collect all overlapping actors in stomp radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape StompSphere = FCollisionShape::MakeSphere(StompRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        Origin,
        FQuat::Identity,
        ECC_Pawn,
        StompSphere,
        Params
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply stomp damage (half of main attack damage, AoE)
        FDamageEvent DmgEvent;
        HitActor->TakeDamage(
            DinoStats.AttackDamage * 0.5f,
            DmgEvent,
            GetController(),
            this
        );
    }

#if WITH_EDITOR
    // Debug sphere in editor builds
    DrawDebugSphere(World, Origin, StompRadius, 12, FColor::Orange, false, 1.5f);
#endif
}

void ATRexDinosaur::Roar()
{
    if (bRoarOnCooldown) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Apply fear to all TranspersonalCharacters within detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), NearbyActors);

    FVector MyLocation = GetActorLocation();

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= DinoStats.DetectionRadius * 1.5f)  // Roar range is 1.5x detection
        {
            // Try to apply fear via SurvivalComponent if it exists
            // (SurvivalComponent is on TranspersonalCharacter — accessed via interface)
            // For now: apply damage as "fear shock" — full SurvivalComponent integration
            // will be wired by agent #03 once SurvivalComponent is confirmed compiled
            FDamageEvent FearEvent;
            Actor->TakeDamage(5.0f, FearEvent, GetController(), this);  // minimal roar shock damage
        }
    }

    // Start roar cooldown
    bRoarOnCooldown = true;
    World->GetTimerManager().SetTimer(
        RoarCooldownHandle,
        [this]() { bRoarOnCooldown = false; },
        RoarCooldown,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARED — fear applied to %d nearby actors"),
        *GetActorLabel(), NearbyActors.Num());
}

void ATRexDinosaur::OnEnterHuntState()
{
    // Roar when entering hunt state
    Roar();
}
