#include "Dinosaurs/VelociraptorCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

AVelociraptorCharacter::AVelociraptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    InitVelociraptorStats();
}

void AVelociraptorCharacter::InitVelociraptorStats()
{
    // Species-specific stats — Velociraptor: fast, fragile, deadly in packs
    MaxHealth         = 180.0f;
    CurrentHealth     = 180.0f;
    AttackDamage      = 35.0f;   // Low solo damage — pack multiplies this
    DetectionRadius   = 1800.0f;
    MoveSpeed         = 700.0f;  // Very fast — fastest land predator in game
    bIsCarnivore      = true;

    // Pack hunting parameters
    PackSearchRadius  = 2500.0f;
    MaxPackSize       = 5;
    bIsAlpha          = false;
    PackScanInterval  = 3.0f;

    // Combat
    PounceCooldown    = 6.0f;
    ClawDamage        = 25.0f;
    PounceVelocityScale = 1400.0f;
    bPounceCoolingDown = false;

    // Movement tuning
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed        = MoveSpeed;
        MoveComp->JumpZVelocity       = 600.0f;   // Raptors can leap
        MoveComp->AirControl          = 0.4f;
        MoveComp->GravityScale        = 1.1f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate        = FRotator(0.0f, 540.0f, 0.0f);
    }
}

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic pack scan
    GetWorldTimerManager().SetTimer(
        PackScanTimerHandle,
        this,
        &AVelociraptorCharacter::ScanForPackAllies,
        PackScanInterval,
        true,   // looping
        1.0f    // initial delay
    );
}

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AVelociraptorCharacter::ScanForPackAllies()
{
    PackAllies.Empty();

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorCharacter::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        AVelociraptorCharacter* OtherRaptor = Cast<AVelociraptorCharacter>(Actor);
        if (!OtherRaptor) continue;

        float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
        if (Dist <= PackSearchRadius)
        {
            PackAllies.Add(OtherRaptor);
            if (PackAllies.Num() >= MaxPackSize) break;
        }
    }

    // Determine alpha: the raptor with the most health in the pack is Alpha
    if (PackAllies.Num() > 0)
    {
        bool bShouldBeAlpha = true;
        for (AVelociraptorCharacter* Ally : PackAllies)
        {
            if (Ally->CurrentHealth > CurrentHealth)
            {
                bShouldBeAlpha = false;
                break;
            }
        }
        bIsAlpha = bShouldBeAlpha;
    }
    else
    {
        bIsAlpha = true; // Lone raptor is its own alpha
    }

#if WITH_EDITOR
    // Debug: draw pack radius
    DrawDebugSphere(GetWorld(), GetActorLocation(), PackSearchRadius, 16,
        bIsAlpha ? FColor::Red : FColor::Yellow, false, PackScanInterval * 0.9f);
#endif
}

void AVelociraptorCharacter::InitiatePackAttack(AActor* Target)
{
    if (!Target || !bIsAlpha) return;

    // Alpha attacks from the front
    PerformPounce(Target);

    // Distribute flanking directions to allies
    int32 AllyCount = PackAllies.Num();
    for (int32 i = 0; i < AllyCount; ++i)
    {
        if (!PackAllies[i]) continue;

        // Spread allies around the target: left flank, right flank, rear
        float AngleDeg = (360.0f / (AllyCount + 1)) * (i + 1);
        float AngleRad = FMath::DegreesToRadians(AngleDeg);
        FVector FlankDir = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);

        PackAllies[i]->RespondToPackAttack(Target, FlankDir);
    }
}

void AVelociraptorCharacter::RespondToPackAttack(AActor* Target, FVector FlankDirection)
{
    if (!Target) return;

    // Move to flanking position relative to target, then attack
    FVector TargetLoc = Target->GetActorLocation();
    FVector FlankPos  = TargetLoc + FlankDirection * 300.0f; // 3m offset

    // Move toward flank position — AI controller will handle pathfinding
    // For now, apply a direct launch toward the flank position
    FVector ToFlank = (FlankPos - GetActorLocation()).GetSafeNormal();
    LaunchCharacter(ToFlank * 400.0f, true, false);

    // After reaching flank, perform claw slash
    FTimerHandle FlankAttackTimer;
    GetWorldTimerManager().SetTimer(FlankAttackTimer, this,
        &AVelociraptorCharacter::PerformClawSlash, 0.8f, false);
}

void AVelociraptorCharacter::PerformPounce(AActor* Target)
{
    if (!Target || bPounceCoolingDown) return;

    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector PounceVelocity = ToTarget * PounceVelocityScale + FVector(0, 0, 400.0f);

    LaunchCharacter(PounceVelocity, true, true);

    // Apply damage on landing — simplified: damage target directly
    UGameplayStatics::ApplyDamage(Target, AttackDamage * 1.5f, GetController(), this, nullptr);

    // Start cooldown
    bPounceCoolingDown = true;
    GetWorldTimerManager().SetTimer(
        PounceCooldownTimerHandle,
        [this]() { bPounceCoolingDown = false; },
        PounceCooldown,
        false
    );

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(),
        FColor::Orange, false, 1.5f, 0, 3.0f);
#endif
}

void AVelociraptorCharacter::PerformClawSlash()
{
    // Find actors within melee range (120cm)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        if (Cast<AVelociraptorCharacter>(Actor)) continue; // Don't hit pack allies

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= 120.0f)
        {
            UGameplayStatics::ApplyDamage(Actor, ClawDamage, GetController(), this, nullptr);

#if WITH_EDITOR
            DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 60.0f, 8,
                FColor::Red, false, 0.5f);
#endif
        }
    }
}
