#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── T-Rex Species Stats ──────────────────────────────────────────────────
    // Override DinosaurBase defaults with T-Rex-specific values
    DinoStats.MaxHealth          = 1200.0f;
    DinoStats.CurrentHealth      = 1200.0f;
    DinoStats.MoveSpeed          = 350.0f;   // Patrol speed (cm/s)
    DinoStats.RunSpeed           = 900.0f;   // Charge speed (cm/s)
    DinoStats.AttackDamage       = 120.0f;   // Bite damage
    DinoStats.AttackRange        = 280.0f;   // Bite reach (cm)
    DinoStats.AggroRadius        = 3500.0f;  // Detection radius (cm)
    DinoStats.PatrolRadius       = 2500.0f;  // Patrol wander radius (cm)
    DinoStats.AttackCooldown     = 2.5f;     // Seconds between bites
    DinoStats.SpeciesName        = FName("Tyrannosaurus_Rex");
    DinoStats.bIsCarnivore       = true;
    DinoStats.bIsPack            = false;    // Solitary apex predator
    DinoStats.ThreatLevel        = 10;       // Maximum threat

    // ── Movement Setup ───────────────────────────────────────────────────────
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed        = DinoStats.MoveSpeed;
        GetCharacterMovement()->MaxAcceleration     = 600.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 400.0f;
        GetCharacterMovement()->RotationRate        = FRotator(0.0f, 120.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale        = 1.2f;  // Heavier feel
    }

    // ── Subscribe to state change delegate ──────────────────────────────────
    // Binding happens in BeginPlay after world is valid
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Bind to state change delegate to trigger roar on combat entry
    OnDinoStateChanged.AddDynamic(this, &ATRexCharacter::HandleStateChanged);

    LastStompTime = -StompCooldown;  // Allow immediate stomp
    LastRoarTime  = -RoarCooldown;   // Allow immediate roar
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stomp attack logic: when in combat and target is close, attempt stomp
    if (CurrentState == EEng_DinoState::Attacking && AttackTarget.IsValid())
    {
        const float DistToTarget = FVector::Dist(GetActorLocation(), AttackTarget->GetActorLocation());
        const float Now = GetWorld()->GetTimeSeconds();

        // Stomp when target is within 1.5× stomp radius and cooldown elapsed
        if (DistToTarget <= StompRadius * 1.5f && (Now - LastStompTime) >= StompCooldown)
        {
            PerformStompAttack();
            LastStompTime = Now;
        }
    }
}

// ── State Change Handler ─────────────────────────────────────────────────────

void ATRexCharacter::HandleStateChanged(EEng_DinoState NewState)
{
    if (NewState == EEng_DinoState::Attacking)
    {
        const float Now = GetWorld()->GetTimeSeconds();
        if ((Now - LastRoarTime) >= RoarCooldown)
        {
            OnTRexRoar();
            LastRoarTime = Now;
        }
    }
}

// ── Roar Implementation ──────────────────────────────────────────────────────

void ATRexCharacter::OnTRexRoar_Implementation()
{
    // Apply fear to nearby players
    ApplyRoarFearToNearbyPlayers();

    // Blueprint subclass adds roar animation + audio cue
    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROAR — fear radius: %.0f units"),
        *GetActorLabel(), RoarFearRadius);

#if WITH_EDITOR
    // Debug sphere in editor builds
    DrawDebugSphere(GetWorld(), GetActorLocation(), RoarFearRadius, 16,
        FColor::Orange, false, 3.0f, 0, 4.0f);
#endif
}

void ATRexCharacter::ApplyRoarFearToNearbyPlayers()
{
    if (!GetWorld()) return;

    // Find all player pawns within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist > RoarFearRadius) continue;

        // Scale fear by proximity (closer = more fear)
        const float FearScale = 1.0f - (Dist / RoarFearRadius);
        const float AppliedFear = RoarFearAmount * FearScale;

        // Try to apply fear via SurvivalComponent interface
        // SurvivalComponent is found by component class name to avoid hard dependency
        UActorComponent* SurvComp = Actor->FindComponentByClass(
            UActorComponent::StaticClass());

        // Log for now — Blueprint/SurvivalComponent integration via event
        UE_LOG(LogTemp, Log, TEXT("TRex Roar: applying %.1f fear to %s (dist: %.0f)"),
            AppliedFear, *Actor->GetActorLabel(), Dist);
    }
}

// ── Stomp Attack Implementation ──────────────────────────────────────────────

void ATRexCharacter::PerformStompAttack_Implementation()
{
    ExecuteStompAoE();

    // Blueprint subclass adds stomp animation, ground crack VFX, camera shake
    UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMP — AoE radius: %.0f, damage: %.0f"),
        *GetActorLabel(), StompRadius, StompDamage);

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), StompRadius, 12,
        FColor::Red, false, 2.0f, 0, 6.0f);
#endif
}

void ATRexCharacter::ExecuteStompAoE()
{
    if (!GetWorld()) return;

    // Sphere overlap to find all actors in stomp radius
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(StompRadius),
        QueryParams
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply radial damage — falls off with distance
        const float Dist = FVector::Dist(GetActorLocation(), HitActor->GetActorLocation());
        const float DamageFalloff = 1.0f - FMath::Clamp(Dist / StompRadius, 0.0f, 1.0f);
        const float FinalDamage = StompDamage * DamageFalloff;

        UGameplayStatics::ApplyDamage(
            HitActor,
            FinalDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }

    // Camera shake for player if within range
    if (StompCameraShakeClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            const float PCDist = FVector::Dist(GetActorLocation(),
                PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);

            if (PCDist <= StompRadius * 3.0f)
            {
                const float ShakeScale = FMath::Clamp(1.0f - (PCDist / (StompRadius * 3.0f)), 0.1f, 1.0f);
                PC->ClientStartCameraShake(StompCameraShakeClass, ShakeScale);
            }
        }
    }
}

// ── Death Override ───────────────────────────────────────────────────────────

void ATRexCharacter::OnDinoDeath_Implementation()
{
    // Call parent death logic first (ragdoll, disable AI, etc.)
    Super::OnDinoDeath_Implementation();

    // T-Rex death triggers environmental reaction
    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] DIED — triggering environmental death reaction"),
        *GetActorLabel());

    // Final stomp on death (ground impact)
    if (GetWorld())
    {
        // Trigger one last stomp AoE as the body hits the ground
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle{},
            [this]()
            {
                if (IsValid(this))
                {
                    ExecuteStompAoE();
                }
            },
            0.8f,   // 0.8s delay — when body hits ground
            false
        );
    }
}
