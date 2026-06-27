#include "Dinosaurs/BrachiosaurusCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABrachiosaurusCharacter::ABrachiosaurusCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.4f; // Large dino — slower tick than raptor

    // ── Base stats ────────────────────────────────────────────────────────
    MaxHealth = 3000.0f;
    CurrentHealth = 3000.0f;
    BaseDamage = 0.0f;       // Herbivore — no offensive damage
    WalkSpeed = 200.0f;      // Slow walker
    RunSpeed = 360.0f;       // Flee speed = WalkSpeed * FleeSpeedMultiplier (1.8)
    DetectionRadius = 2500.0f;
    AttackRange = 400.0f;    // Stomp range
    BehaviorTickInterval = 0.4f;
    DinosaurSpecies = TEXT("Brachiosaurus");

    // Movement component setup
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 60.0f, 0.0f); // Slow turning — large body
        MoveComp->GravityScale = 1.0f;
        MoveComp->JumpZVelocity = 0.0f; // Cannot jump
        MoveComp->bCanWalkOffLedges = false; // Stays on safe ground
    }

    // Cull distance — visible at long range (large creature)
    SetCullDistance(10000.0f);
}

void ABrachiosaurusCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initial herd setup
    UpdateHerdMembers();
    ElectHerdLeader();

    // Start grazing
    bIsGrazing = true;
    GrazeTimer = GrazeDuration;

    UE_LOG(LogTemp, Log, TEXT("BrachiosaurusCharacter [%s] spawned. HerdLeader=%s, HerdSize=%d"),
        *GetActorLabel(), bIsHerdLeader ? TEXT("YES") : TEXT("NO"), HerdMembers.Num());
}

void ABrachiosaurusCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update herd every 5 seconds
    HerdUpdateTimer += DeltaTime;
    if (HerdUpdateTimer >= 5.0f)
    {
        HerdUpdateTimer = 0.0f;
        UpdateHerdMembers();
        ElectHerdLeader();
    }

    // Check for carnivores every tick
    CheckForCarnivores();

    if (bIsFleeing)
    {
        UpdateFleeState(DeltaTime);
    }
    else
    {
        UpdateGrazingState(DeltaTime);
    }
}

void ABrachiosaurusCharacter::UpdateHerdMembers()
{
    HerdMembers.Empty();

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrachiosaurusCharacter::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        ABrachiosaurusCharacter* Other = Cast<ABrachiosaurusCharacter>(Actor);
        if (!Other || !Other->IsAlive()) continue;

        float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        if (Dist <= HerdRadius && HerdMembers.Num() < MaxHerdSize)
        {
            HerdMembers.Add(Other);
        }
    }
}

void ABrachiosaurusCharacter::ElectHerdLeader()
{
    // Leader = first Brachio in herd with lowest actor name (deterministic)
    bIsHerdLeader = true;
    for (ABrachiosaurusCharacter* Member : HerdMembers)
    {
        if (Member && Member->GetActorLabel() < GetActorLabel())
        {
            bIsHerdLeader = false;
            break;
        }
    }
}

void ABrachiosaurusCharacter::CheckForCarnivores()
{
    if (bIsFleeing) return; // Already fleeing

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
        if (!OtherDino || !OtherDino->IsAlive()) continue;

        // Check if it's a carnivore (non-Brachio dino = potential threat)
        if (OtherDino->IsA(ABrachiosaurusCharacter::StaticClass())) continue;

        float Dist = FVector::Dist(GetActorLocation(), OtherDino->GetActorLocation());
        if (Dist <= CarnivoreDetectionRadius)
        {
            // Flee away from threat
            FleeDirection = (GetActorLocation() - OtherDino->GetActorLocation()).GetSafeNormal();
            bIsFleeing = true;
            FleeTimer = FleeDuration;

            // Alert herd
            AlertHerd(OtherDino->GetActorLocation());

            // Speed up
            UCharacterMovementComponent* MoveComp = GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed = WalkSpeed * FleeSpeedMultiplier;
            }

            UE_LOG(LogTemp, Log, TEXT("Brachio [%s] FLEEING from %s (dist=%.0f)"),
                *GetActorLabel(), *OtherDino->GetActorLabel(), Dist);
            break;
        }
    }

    // Also check player
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Player)
    {
        float PlayerDist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
        if (PlayerDist <= CarnivoreDetectionRadius * 0.6f) // Player triggers flee at 60% range
        {
            FleeDirection = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
            bIsFleeing = true;
            FleeTimer = FleeDuration * 0.7f;
            AlertHerd(Player->GetActorLocation());

            UCharacterMovementComponent* MoveComp = GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed = WalkSpeed * FleeSpeedMultiplier;
            }
        }
    }
}

void ABrachiosaurusCharacter::UpdateFleeState(float DeltaTime)
{
    FleeTimer -= DeltaTime;
    if (FleeTimer <= 0.0f)
    {
        // Stop fleeing
        bIsFleeing = false;
        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = WalkSpeed;
        }
        bIsGrazing = true;
        GrazeTimer = GrazeDuration;
        return;
    }

    // Move in flee direction
    if (!FleeDirection.IsNearlyZero())
    {
        AddMovementInput(FleeDirection, 1.0f);
    }
}

void ABrachiosaurusCharacter::UpdateGrazingState(float DeltaTime)
{
    GrazeTimer -= DeltaTime;
    if (GrazeTimer <= 0.0f)
    {
        bIsGrazing = !bIsGrazing;
        GrazeTimer = bIsGrazing ? GrazeDuration : 4.0f; // 4s walking between graze spots

        if (!bIsGrazing)
        {
            // Pick a random wander direction near current position
            FVector WanderDir = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal();
            AddMovementInput(WanderDir, 0.5f);
        }
    }

    if (!bIsGrazing)
    {
        // Slowly wander
        FVector Forward = GetActorForwardVector();
        AddMovementInput(Forward, 0.3f);
    }
}

void ABrachiosaurusCharacter::PerformStomp()
{
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastStompTime < StompCooldown) return;
    LastStompTime = Now;

    FVector StompOrigin = GetActorLocation();

    // Damage all actors within StompRadius
    TArray<AActor*> HitActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), HitActors);

    for (AActor* Actor : HitActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(StompOrigin, Actor->GetActorLocation());
        if (Dist <= StompRadius)
        {
            FDamageEvent DmgEvent;
            Actor->TakeDamage(StompDamage, DmgEvent, GetController(), this);
            UE_LOG(LogTemp, Log, TEXT("Brachio STOMP hit %s for %.0f dmg"), *Actor->GetActorLabel(), StompDamage);
        }
    }

    // Debug sphere
    DrawDebugSphere(GetWorld(), StompOrigin, StompRadius, 16, FColor::Orange, false, 1.5f);
}

void ABrachiosaurusCharacter::AlertHerd(FVector ThreatLocation)
{
    for (ABrachiosaurusCharacter* Member : HerdMembers)
    {
        if (!Member || !Member->IsAlive()) continue;

        // Propagate flee state to herd members
        Member->FleeDirection = (Member->GetActorLocation() - ThreatLocation).GetSafeNormal();
        Member->bIsFleeing = true;
        Member->FleeTimer = Member->FleeDuration * 0.8f;

        UCharacterMovementComponent* MoveComp = Member->GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = Member->WalkSpeed * Member->FleeSpeedMultiplier;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Brachio [%s] alerted %d herd members to flee"),
        *GetActorLabel(), HerdMembers.Num());
}

void ABrachiosaurusCharacter::OnDeath()
{
    Super::OnDeath();

    // Notify herd leader is gone — trigger re-election
    for (ABrachiosaurusCharacter* Member : HerdMembers)
    {
        if (Member)
        {
            Member->UpdateHerdMembers();
            Member->ElectHerdLeader();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Brachio [%s] died. Herd re-electing leader."), *GetActorLabel());
}
