// BrachiosaurusCharacter.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260624_004
// Passive herbivore: herd behaviour, stampede when threatened, gentle giant stats

#include "BrachiosaurusCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABrachiosaurusCharacter::ABrachiosaurusCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Enormous herbivore — largest land animal in the world
    GetCapsuleComponent()->InitCapsuleSize(120.0f, 350.0f);

    // Slow, deliberate movement — not a predator
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 280.0f;          // Gentle grazing pace
    MoveComp->MaxAcceleration = 150.0f;
    MoveComp->BrakingDecelerationWalking = 200.0f;
    MoveComp->GravityScale = 1.2f;            // Heavy — gravity pulls harder
    MoveComp->JumpZVelocity = 0.0f;           // Cannot jump
    MoveComp->bCanWalkOffLedges = false;       // Stays on safe terrain

    // Survival stats — massive herbivore
    MaxHealth = 2500.0f;
    CurrentHealth = 2500.0f;
    MaxStamina = 600.0f;
    CurrentStamina = 600.0f;
    MoveSpeed = 280.0f;
    StaminaRegenRate = 8.0f;

    // Brachiosaur-specific stats
    HerdRadius = 3500.0f;
    ThreatDetectRadius = 1800.0f;
    StampedeSpeed = 650.0f;
    StampedeDamage = 500.0f;
    StampedeDuration = 12.0f;
    bIsStampeding = false;
    bIsLeadingHerd = false;
    HerdMemberCount = 0;
    GrazingCooldown = 8.0f;
    bIsGrazing = false;

    // Mesh scale hint — actual mesh set in Blueprint
    GetMesh()->SetRelativeScale3D(FVector(2.8f, 2.8f, 2.8f));
}

void ABrachiosaurusCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Elect herd leader — first Brachiosaur in scene
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrachiosaurusCharacter::StaticClass(), AllActors);

    if (AllActors.Num() > 0 && AllActors[0] == this)
    {
        bIsLeadingHerd = true;
        UE_LOG(LogTemp, Log, TEXT("BrachiosaurusCharacter: [%s] elected as HERD LEADER"), *GetActorLabel());
    }

    // Start grazing routine
    GetWorldTimerManager().SetTimer(
        GrazingTimerHandle,
        this,
        &ABrachiosaurusCharacter::UpdateGrazingState,
        GrazingCooldown,
        true
    );

    // Herd cohesion check — every 3 seconds
    GetWorldTimerManager().SetTimer(
        HerdCohesionTimerHandle,
        this,
        &ABrachiosaurusCharacter::MaintainHerdCohesion,
        3.0f,
        true
    );

    // Threat scan — every 2 seconds
    GetWorldTimerManager().SetTimer(
        ThreatScanTimerHandle,
        this,
        &ABrachiosaurusCharacter::ScanForThreats,
        2.0f,
        true
    );
}

void ABrachiosaurusCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina recovery when not stampeding
    if (!bIsStampeding && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }

    // Stamina drain during stampede
    if (bIsStampeding)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - 25.0f * DeltaTime);
        if (CurrentStamina <= 0.0f)
        {
            EndStampede();
        }
    }
}

void ABrachiosaurusCharacter::UpdateGrazingState()
{
    if (bIsStampeding) return;

    bIsGrazing = !bIsGrazing;

    if (bIsGrazing)
    {
        // Slow to grazing speed
        GetCharacterMovement()->MaxWalkSpeed = 80.0f;
        UE_LOG(LogTemp, Verbose, TEXT("BrachiosaurusCharacter: [%s] began grazing"), *GetActorLabel());
    }
    else
    {
        // Resume normal walk speed
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
        UE_LOG(LogTemp, Verbose, TEXT("BrachiosaurusCharacter: [%s] resumed walking"), *GetActorLabel());
    }
}

void ABrachiosaurusCharacter::MaintainHerdCohesion()
{
    if (!bIsLeadingHerd || bIsStampeding) return;

    TArray<AActor*> HerdMembers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrachiosaurusCharacter::StaticClass(), HerdMembers);

    HerdMemberCount = 0;
    FVector LeaderPos = GetActorLocation();

    for (AActor* Member : HerdMembers)
    {
        if (!IsValid(Member) || Member == this) continue;

        float Dist = FVector::Dist(LeaderPos, Member->GetActorLocation());
        if (Dist <= HerdRadius)
        {
            HerdMemberCount++;
        }
        else if (Dist > HerdRadius * 1.5f)
        {
            // Straggler — nudge back toward herd
            FVector DirectionToLeader = (LeaderPos - Member->GetActorLocation()).GetSafeNormal();
            ABrachiosaurusCharacter* MemberBrachio = Cast<ABrachiosaurusCharacter>(Member);
            if (IsValid(MemberBrachio) && !MemberBrachio->bIsStampeding)
            {
                MemberBrachio->AddMovementInput(DirectionToLeader, 0.6f);
            }
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("BrachiosaurusCharacter: Herd cohesion — %d members within radius %.0f"),
        HerdMemberCount, HerdRadius);
}

void ABrachiosaurusCharacter::ScanForThreats()
{
    if (bIsStampeding) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    FVector MyPos = GetActorLocation();

    for (AActor* Actor : NearbyActors)
    {
        if (!IsValid(Actor) || Actor == this) continue;

        // Ignore other Brachiosauruses
        if (Cast<ABrachiosaurusCharacter>(Actor)) continue;

        float Dist = FVector::Dist(MyPos, Actor->GetActorLocation());
        if (Dist <= ThreatDetectRadius)
        {
            // Threat detected — trigger stampede
            UE_LOG(LogTemp, Warning, TEXT("BrachiosaurusCharacter: [%s] THREAT DETECTED at dist=%.0f — STAMPEDE!"),
                *GetActorLabel(), Dist);
            TriggerStampede(Actor->GetActorLocation());
            return;
        }
    }
}

void ABrachiosaurusCharacter::TriggerStampede(FVector ThreatLocation)
{
    if (bIsStampeding) return;

    bIsStampeding = true;
    bIsGrazing = false;

    // Direction AWAY from threat
    FVector FleeDirection = (GetActorLocation() - ThreatLocation).GetSafeNormal();
    FleeDirection.Z = 0.0f;

    // Boost movement speed
    GetCharacterMovement()->MaxWalkSpeed = StampedeSpeed;
    GetCharacterMovement()->MaxAcceleration = 800.0f;

    // Apply initial burst velocity
    LaunchCharacter(FleeDirection * StampedeSpeed * 0.8f, true, false);

    // Alert herd members if leader
    if (bIsLeadingHerd)
    {
        AlertHerd(ThreatLocation);
    }

    // End stampede after duration
    GetWorldTimerManager().SetTimer(
        StampedeTimerHandle,
        this,
        &ABrachiosaurusCharacter::EndStampede,
        StampedeDuration,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("BrachiosaurusCharacter: [%s] STAMPEDE STARTED — fleeing from (%.0f,%.0f,%.0f)"),
        *GetActorLabel(), ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
}

void ABrachiosaurusCharacter::EndStampede()
{
    bIsStampeding = false;

    // Restore normal movement
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    GetCharacterMovement()->MaxAcceleration = 150.0f;

    UE_LOG(LogTemp, Log, TEXT("BrachiosaurusCharacter: [%s] stampede ended — resuming normal behaviour"), *GetActorLabel());
}

void ABrachiosaurusCharacter::AlertHerd(FVector ThreatLocation)
{
    TArray<AActor*> HerdMembers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrachiosaurusCharacter::StaticClass(), HerdMembers);

    int32 AlertedCount = 0;
    for (AActor* Member : HerdMembers)
    {
        if (!IsValid(Member) || Member == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
        if (Dist <= HerdRadius * 1.2f)
        {
            ABrachiosaurusCharacter* MemberBrachio = Cast<ABrachiosaurusCharacter>(Member);
            if (IsValid(MemberBrachio) && !MemberBrachio->bIsStampeding)
            {
                MemberBrachio->TriggerStampede(ThreatLocation);
                AlertedCount++;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("BrachiosaurusCharacter: Herd alert sent to %d members"), AlertedCount);
}

float ABrachiosaurusCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // Damage triggers stampede
    if (ActualDamage > 0.0f && !bIsStampeding && IsValid(DamageCauser))
    {
        TriggerStampede(DamageCauser->GetActorLocation());
    }

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("BrachiosaurusCharacter: [%s] has died"), *GetActorLabel());
        // Death handled by Blueprint or GameMode
    }

    return ActualDamage;
}
