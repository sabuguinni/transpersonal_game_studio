#include "AI/RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Raptor-specific stats (overrides DinosaurBase defaults) ──
    MaxHealth        = 350.0f;
    AttackDamage     = 55.0f;
    DetectionRadius  = 2000.0f;
    MoveSpeed        = 700.0f;   // Raptors are fast
    bIsPackHunter    = true;

    // Fast, agile movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed         = 700.0f;
        GetCharacterMovement()->MaxAcceleration      = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 1024.0f;
        GetCharacterMovement()->JumpZVelocity        = 600.0f;
        GetCharacterMovement()->AirControl           = 0.4f;
    }
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // First raptor to spawn in an area becomes the alpha
    // Simple heuristic: check if any other raptor is already alpha nearby
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    bool bAlphaExists = false;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        ARaptorCharacter* OtherRaptor = Cast<ARaptorCharacter>(Actor);
        if (OtherRaptor && OtherRaptor->bIsAlpha)
        {
            float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
            if (Dist < PackCoordinationRadius)
            {
                bAlphaExists = true;
                break;
            }
        }
    }

    if (!bAlphaExists)
    {
        BecomePackAlpha();
    }
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Alpha broadcasts its target to nearby pack members every tick
    if (bIsAlpha && PackTarget)
    {
        ShareTargetWithPack(PackTarget);
    }
}

// ── Pack Coordination ─────────────────────────────────────────────

void ARaptorCharacter::BecomePackAlpha()
{
    bIsAlpha = true;
    UE_LOG(LogTemp, Log, TEXT("ARaptorCharacter: %s is now PACK ALPHA"), *GetName());
}

void ARaptorCharacter::ShareTargetWithPack(AActor* NewTarget)
{
    if (!NewTarget) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    int32 PackCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        ARaptorCharacter* PackMember = Cast<ARaptorCharacter>(Actor);
        if (!PackMember) continue;

        float Dist = FVector::Dist(GetActorLocation(), PackMember->GetActorLocation());
        if (Dist <= PackCoordinationRadius && PackCount < MaxPackSize)
        {
            PackMember->PackTarget = NewTarget;
            PackCount++;
        }
    }
}

// ── Abilities ─────────────────────────────────────────────────────

void ARaptorCharacter::PerformLeapAttack()
{
    if (bLeapOnCooldown) return;

    AActor* Target = PackTarget;
    if (!Target) return;

    // Launch raptor toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVelocity = ToTarget * LeapImpulse + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage after brief delay (landing time)
    FTimerHandle LandingTimer;
    FTimerDelegate LandingDelegate;
    LandingDelegate.BindLambda([this, Target]()
    {
        if (!Target || !IsValid(Target)) return;
        float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
        if (Dist < 200.0f)
        {
            UGameplayStatics::ApplyDamage(Target, LeapDamage, GetController(), this, UDamageType::StaticClass());
            UE_LOG(LogTemp, Log, TEXT("ARaptorCharacter: %s LEAP PINNED %s for %.0f dmg"),
                *GetName(), *Target->GetName(), LeapDamage);
        }
    });
    GetWorldTimerManager().SetTimer(LandingTimer, LandingDelegate, 0.5f, false);

    // Start cooldown
    bLeapOnCooldown = true;
    GetWorldTimerManager().SetTimer(LeapCooldownTimer, this,
        &ARaptorCharacter::ResetLeapCooldown, LeapCooldown, false);

    UE_LOG(LogTemp, Log, TEXT("ARaptorCharacter: %s LEAP ATTACK"), *GetName());
}

void ARaptorCharacter::PerformFlankingManeuver()
{
    if (bFlankOnCooldown || !PackTarget) return;

    // Calculate a flanking position 90° to the side of the target
    FVector ToTarget = (PackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector RightFlank = FVector(-ToTarget.Y, ToTarget.X, 0.0f) * 600.0f;
    FVector FlankPos = PackTarget->GetActorLocation() + RightFlank;

    // Move toward flank position — AI controller handles actual pathfinding
    // We just set the destination hint via a property
    // (DinosaurAIController reads FlankDestination when bIsFlanking=true)
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), FlankPos, GetWorld()->GetDeltaSeconds(), 3.0f));

    bFlankOnCooldown = true;
    GetWorldTimerManager().SetTimer(FlankCooldownTimer, this,
        &ARaptorCharacter::ResetFlankCooldown, FlankCooldown, false);

    UE_LOG(LogTemp, Log, TEXT("ARaptorCharacter: %s FLANKING to (%.0f, %.0f, %.0f)"),
        *GetName(), FlankPos.X, FlankPos.Y, FlankPos.Z);
}

void ARaptorCharacter::ResetLeapCooldown()
{
    bLeapOnCooldown = false;
}

void ARaptorCharacter::ResetFlankCooldown()
{
    bFlankOnCooldown = false;
}
