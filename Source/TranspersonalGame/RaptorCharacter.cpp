// RaptorCharacter.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260620_007
// Velociraptor implementation — pack hunter, ambush predator, leap attacker

#include "RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Species identity ---
    SpeciesName = FName("Velociraptor");
    DinoSize    = EDinoSize::Small;
    bIsPack     = true;

    // --- Species stats ---
    MaxHealth    = 300.0f;
    CurrentHealth = MaxHealth;
    AttackDamage = 65.0f;
    AttackRange  = 150.0f;
    SightRange   = 1800.0f;
    HearingRange = 1200.0f;

    // --- Movement ---
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (Move)
    {
        Move->MaxWalkSpeed          = 450.0f;
        Move->MaxWalkSpeedCrouched  = 180.0f;
        Move->JumpZVelocity         = 600.0f;
        Move->AirControl            = 0.4f;
        Move->bCanWalkOffLedges     = true;
        Move->NavAgentProps.bCanCrouch = true;
    }

    // --- Capsule (smaller than TRex) ---
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 75.0f);

    // --- Pack defaults ---
    PackLeader              = nullptr;
    MaxPackSize             = 5;
    bIsPackLeader           = false;
    PackCoordinationRadius  = 2500.0f;

    // --- Ambush defaults ---
    bIsInAmbush       = false;
    AmbushBreakDistance = 400.0f;
    AmbushChargeMult    = 1.6f;

    // --- Leap attack defaults ---
    bCanLeapAttack     = true;
    LeapAttackCooldown = 8.0f;
    LeapDamageMult     = 1.5f;
    LeapKnockbackForce = 800.0f;

    // --- Audio ---
    PackCallSound    = nullptr;
    LeapAttackSound  = nullptr;
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // If no leader assigned, this raptor becomes the leader
    if (!PackLeader)
    {
        bIsPackLeader = true;
    }
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // AI controller drives state transitions; Tick reserved for cosmetic updates
}

// ─── Ambush System ─────────────────────────────────────────────────────────────

void ARaptorCharacter::EnterAmbush()
{
    if (bIsInAmbush || !IsAlive()) return;

    bIsInAmbush = true;
    SetDinoState(EDinoState::Idle); // Visually still

    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (Move)
    {
        Move->MaxWalkSpeed = 0.0f; // Freeze movement
        Move->Crouch(true);
    }
}

void ARaptorCharacter::BreakAmbushAndCharge(AActor* Target)
{
    if (!Target || !IsAlive()) return;

    bIsInAmbush = false;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (Move)
    {
        Move->UnCrouch(true);
        Move->MaxWalkSpeed = 700.0f * AmbushChargeMult; // Burst speed
    }

    SetDinoState(EDinoState::Attacking);

    // Restore normal sprint speed after 3 seconds
    FTimerHandle RestoreTimer;
    GetWorldTimerManager().SetTimer(RestoreTimer, [this]()
    {
        UCharacterMovementComponent* M = GetCharacterMovement();
        if (M) M->MaxWalkSpeed = 700.0f;
    }, 3.0f, false);
}

// ─── Leap Attack ───────────────────────────────────────────────────────────────

void ARaptorCharacter::LeapAttack(AActor* Target)
{
    if (!Target || !bCanLeapAttack || !IsAlive()) return;

    bCanLeapAttack = false;

    // Play leap sound
    if (LeapAttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LeapAttackSound, GetActorLocation());
    }

    // Calculate leap direction
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LeapVelocity = ToTarget * 900.0f + FVector(0.0f, 0.0f, 500.0f);

    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (Move)
    {
        Move->Launch(LeapVelocity);
    }

    // Apply damage on landing (simplified: deal damage immediately on leap initiation)
    float FinalDamage = AttackDamage * LeapDamageMult;
    UGameplayStatics::ApplyDamage(Target, FinalDamage, GetController(), this, nullptr);

    // Apply knockback to target if it has a movement component
    ACharacter* TargetChar = Cast<ACharacter>(Target);
    if (TargetChar && TargetChar->GetCharacterMovement())
    {
        TargetChar->GetCharacterMovement()->AddImpulse(ToTarget * LeapKnockbackForce, true);
    }

    SetDinoState(EDinoState::Attacking);

    // Start cooldown
    GetWorldTimerManager().SetTimer(LeapCooldownTimer, this, &ARaptorCharacter::ResetLeapAttack, LeapAttackCooldown, false);
}

void ARaptorCharacter::ResetLeapAttack()
{
    bCanLeapAttack = true;
}

// ─── Pack System ───────────────────────────────────────────────────────────────

void ARaptorCharacter::CallPack(AActor* ThreatTarget)
{
    if (!IsAlive()) return;

    // Play pack call sound
    if (PackCallSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PackCallSound, GetActorLocation());
    }

    // Find all raptors within coordination radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        ARaptorCharacter* OtherRaptor = Cast<ARaptorCharacter>(Actor);
        if (!OtherRaptor || OtherRaptor == this || !OtherRaptor->IsAlive()) continue;

        float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
        if (Dist <= PackCoordinationRadius)
        {
            // Add to pack members list if not already present
            if (!PackMembers.Contains(OtherRaptor))
            {
                PackMembers.Add(OtherRaptor);
            }
            // Set their leader to this raptor if they have none
            if (!OtherRaptor->PackLeader)
            {
                OtherRaptor->PackLeader = this;
            }
            // Alert them to the threat — AI controller picks this up via perception
            OtherRaptor->SetDinoState(EDinoState::Aggressive);
        }
    }
}

void ARaptorCharacter::ElectNewPackLeader()
{
    if (PackMembers.Num() == 0) return;

    // Elect the pack member with the highest health percentage
    ARaptorCharacter* NewLeader = nullptr;
    float BestHealth = 0.0f;

    for (ARaptorCharacter* Member : PackMembers)
    {
        if (!Member || !Member->IsAlive()) continue;
        float HP = Member->GetHealthPercent();
        if (HP > BestHealth)
        {
            BestHealth = HP;
            NewLeader = Member;
        }
    }

    if (NewLeader)
    {
        NewLeader->bIsPackLeader = true;
        NewLeader->PackLeader    = nullptr;

        // Update all members to point to new leader
        for (ARaptorCharacter* Member : PackMembers)
        {
            if (Member && Member != NewLeader)
            {
                Member->PackLeader    = NewLeader;
                Member->bIsPackLeader = false;
            }
        }
    }
}

bool ARaptorCharacter::CanFlank() const
{
    // Need at least 2 alive pack members to execute a flank
    int32 AliveCount = 0;
    for (const ARaptorCharacter* Member : PackMembers)
    {
        if (Member && Member->IsAlive()) AliveCount++;
    }
    return AliveCount >= 2;
}

// ─── Death Override ────────────────────────────────────────────────────────────

void ARaptorCharacter::OnDeath_Implementation()
{
    // If this raptor was the pack leader, elect a new one
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        ElectNewPackLeader();
    }

    // Remove self from all pack members' lists
    for (ARaptorCharacter* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackMembers.Remove(this);
        }
    }
    PackMembers.Empty();

    // Call parent death (ragdoll, delegate broadcast, etc.)
    Super::OnDeath_Implementation();
}
