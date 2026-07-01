#include "TriceratopsDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATriceratopsDinosaur::ATriceratopsDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── BASE STATS (heavy herbivore) ─────────────────────────────────────────
    MaxHealth           = 800.0f;
    CurrentHealth       = 800.0f;
    BaseDamage          = 80.0f;
    BaseWalkSpeed       = 400.0f;
    DetectionRadius     = 1200.0f;

    // ── HERD ─────────────────────────────────────────────────────────────────
    HerdID                  = 0;
    HerdProtectionRadius    = 1800.0f;
    MaxHerdSize             = 6;
    bIsHerdLeader           = false;
    bIsProtectingJuvenile   = false;

    // ── HORN CHARGE ──────────────────────────────────────────────────────────
    HornChargeRange         = 800.0f;
    HornChargeDamage        = 120.0f;
    HornChargeCooldown      = 8.0f;
    HornChargeSpeed         = 900.0f;
    HornChargeKnockbackForce = 2500.0f;
    bIsCharging             = false;
    CurrentChargeCooldown   = 0.0f;

    // ── DEFENSIVE STANCE ─────────────────────────────────────────────────────
    FrillArmorMultiplier            = 0.6f;   // 40% damage reduction from front
    bIsInDefensiveStance            = false;
    DefensiveStanceDamageReduction  = 0.35f;

    // ── CAPSULE (large herbivore) ─────────────────────────────────────────────
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleRadius(70.0f);
        GetCapsuleComponent()->SetCapsuleHalfHeight(100.0f);
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        GetCharacterMovement()->Mass         = 6000.0f;
        GetCharacterMovement()->JumpZVelocity = 0.0f; // cannot jump
    }
}

void ATriceratopsDinosaur::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TriceratopsDinosaur BeginPlay — HerdID=%d, Leader=%s"),
        HerdID, bIsHerdLeader ? TEXT("YES") : TEXT("NO"));
}

void ATriceratopsDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateChargeCooldown(DeltaTime);
}

// ── HORN CHARGE ──────────────────────────────────────────────────────────────
void ATriceratopsDinosaur::ExecuteHornCharge(AActor* Target)
{
    if (!Target || !CanCharge()) return;

    bIsCharging = true;
    CurrentChargeCooldown = HornChargeCooldown;

    // Launch toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVelocity = ToTarget * HornChargeSpeed;
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage + knockback
    UGameplayStatics::ApplyDamage(Target, HornChargeDamage, GetController(), this, nullptr);

    // Knockback target
    if (UPrimitiveComponent* TargetPrim = Cast<UPrimitiveComponent>(
            Target->GetComponentByClass(UPrimitiveComponent::StaticClass())))
    {
        TargetPrim->AddImpulse(ToTarget * HornChargeKnockbackForce, NAME_None, true);
    }

    UE_LOG(LogTemp, Log, TEXT("Triceratops HornCharge → %s | Damage=%.0f | Knockback=%.0f"),
        *Target->GetName(), HornChargeDamage, HornChargeKnockbackForce);

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(),
        FColor::Orange, false, 2.0f, 0, 8.0f);
#endif

    // Reset charging state after brief delay
    bIsCharging = false;
}

// ── HERD DEFENSE SIGNAL ──────────────────────────────────────────────────────
void ATriceratopsDinosaur::SignalHerdDefense(AActor* Threat)
{
    if (!Threat) return;

    // Find all herd members within protection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATriceratopsDinosaur::StaticClass(), NearbyActors);

    int32 HerdMembersAlerted = 0;
    for (AActor* Member : NearbyActors)
    {
        if (Member == this) continue;
        ATriceratopsDinosaur* HerdMember = Cast<ATriceratopsDinosaur>(Member);
        if (HerdMember && HerdMember->HerdID == HerdID)
        {
            float Dist = FVector::Dist(GetActorLocation(), HerdMember->GetActorLocation());
            if (Dist <= HerdProtectionRadius)
            {
                HerdMember->bIsProtectingJuvenile = true;
                HerdMember->EnterDefensiveStance();
                HerdMembersAlerted++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Triceratops SignalHerdDefense — %d members alerted, Threat=%s"),
        HerdMembersAlerted, *Threat->GetName());

    FormHerdCircle(Threat);
}

// ── DEFENSIVE STANCE ─────────────────────────────────────────────────────────
void ATriceratopsDinosaur::EnterDefensiveStance()
{
    bIsInDefensiveStance = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * 0.5f; // slow while defending
    }
    UE_LOG(LogTemp, Log, TEXT("Triceratops %s entered defensive stance"), *GetName());
}

void ATriceratopsDinosaur::ExitDefensiveStance()
{
    bIsInDefensiveStance = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
    UE_LOG(LogTemp, Log, TEXT("Triceratops %s exited defensive stance"), *GetName());
}

// ── PURE FUNCTIONS ────────────────────────────────────────────────────────────
bool ATriceratopsDinosaur::CanCharge() const
{
    return !bIsCharging && CurrentChargeCooldown <= 0.0f;
}

bool ATriceratopsDinosaur::IsHerdLeader() const
{
    return bIsHerdLeader;
}

// ── PRIVATE HELPERS ───────────────────────────────────────────────────────────
void ATriceratopsDinosaur::UpdateChargeCooldown(float DeltaTime)
{
    if (CurrentChargeCooldown > 0.0f)
    {
        CurrentChargeCooldown -= DeltaTime;
        if (CurrentChargeCooldown <= 0.0f)
        {
            CurrentChargeCooldown = 0.0f;
            bIsCharging = false;
        }
    }
}

void ATriceratopsDinosaur::FormHerdCircle(AActor* Threat)
{
    if (!Threat) return;

    // Herd leader faces threat; others form protective arc
    FVector ThreatLoc = Threat->GetActorLocation();
    FVector ToThreat  = (ThreatLoc - GetActorLocation()).GetSafeNormal();
    FRotator FaceRot  = ToThreat.Rotation();
    SetActorRotation(FaceRot);

    UE_LOG(LogTemp, Log, TEXT("Triceratops %s forming herd circle facing %s"),
        *GetName(), *Threat->GetName());
}
