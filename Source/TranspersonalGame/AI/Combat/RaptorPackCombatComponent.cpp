#include "RaptorPackCombatComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

URaptorPackCombatComponent::URaptorPackCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for AI
}

void URaptorPackCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // Alpha self-registers and finds pack members nearby
    if (MyRole == ECombat_RaptorRole::Alpha)
    {
        PackInfo.AlphaRaptor = GetOwner();
        PackInfo.PackMembers.Add(GetOwner());
    }
}

void URaptorPackCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastLunge += DeltaTime;
    StateTimer         += DeltaTime;

    // Decay alert level over time
    if (PackInfo.PackAlertLevel > 0.0f)
    {
        PackInfo.PackAlertLevel = FMath::Max(0.0f, PackInfo.PackAlertLevel - AlertDecayRate * DeltaTime);
    }

    switch (CurrentState)
    {
        case ECombat_RaptorState::Idle:     TickIdle(DeltaTime);     break;
        case ECombat_RaptorState::Stalk:    TickStalk(DeltaTime);    break;
        case ECombat_RaptorState::Flank:    TickFlank(DeltaTime);    break;
        case ECombat_RaptorState::Lunge:    TickLunge(DeltaTime);    break;
        case ECombat_RaptorState::Retreat:  TickRetreat(DeltaTime);  break;
        case ECombat_RaptorState::Distract: TickDistract(DeltaTime); break;
    }
}

// ── State Machine ──────────────────────────────────────────────────────────────

void URaptorPackCombatComponent::TickIdle(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    if (!Player) return;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    if (Dist <= DetectionRange && CanSeePlayer(Player))
    {
        OnPlayerDetected(Player);
    }
}

void URaptorPackCombatComponent::TickStalk(float DeltaTime)
{
    if (!TrackedPlayer) { SetState(ECombat_RaptorState::Idle); return; }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TrackedPlayer->GetActorLocation());

    // Alpha: broadcast location to pack, then distract
    if (MyRole == ECombat_RaptorRole::Alpha)
    {
        BroadcastPlayerLocation(TrackedPlayer->GetActorLocation());

        if (StateTimer > 2.5f) // After 2.5s of stalking, alpha distracts
        {
            SetState(ECombat_RaptorState::Distract);
        }
        else
        {
            MoveTowardLocation(TrackedPlayer->GetActorLocation(), StalkSpeed, DeltaTime);
            FaceLocation(TrackedPlayer->GetActorLocation(), DeltaTime);
        }
    }
    else
    {
        // Flankers: move to assigned flank position
        FVector FlankPos = ComputeFlankPosition(
            PackInfo.LastKnownPlayerLocation,
            TrackedPlayer->GetActorForwardVector()
        );
        AssignedFlankPosition.WorldPosition = FlankPos;

        float DistToFlank = FVector::Dist(GetOwner()->GetActorLocation(), FlankPos);
        if (DistToFlank < 100.0f)
        {
            bFlankPositionReached = true;
            SetState(ECombat_RaptorState::Flank);
        }
        else
        {
            MoveTowardLocation(FlankPos, StalkSpeed, DeltaTime);
            FaceLocation(TrackedPlayer->GetActorLocation(), DeltaTime);
        }
    }
}

void URaptorPackCombatComponent::TickFlank(float DeltaTime)
{
    if (!TrackedPlayer) { SetState(ECombat_RaptorState::Idle); return; }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TrackedPlayer->GetActorLocation());

    // Flankers wait for alpha to distract, then lunge from the side
    if (Dist <= LungeRange && TimeSinceLastLunge >= LungeCooldown)
    {
        // Check if alpha is distracting (pack engaged)
        if (PackInfo.bPackEngaged || MyRole == ECombat_RaptorRole::Alpha)
        {
            SetState(ECombat_RaptorState::Lunge);
        }
    }
    else
    {
        // Update flank position as player moves
        FVector NewFlank = ComputeFlankPosition(
            TrackedPlayer->GetActorLocation(),
            TrackedPlayer->GetActorForwardVector()
        );
        MoveTowardLocation(NewFlank, StalkSpeed, DeltaTime);
        FaceLocation(TrackedPlayer->GetActorLocation(), DeltaTime);
    }
}

void URaptorPackCombatComponent::TickLunge(float DeltaTime)
{
    if (!TrackedPlayer) { SetState(ECombat_RaptorState::Idle); return; }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TrackedPlayer->GetActorLocation());

    MoveTowardLocation(TrackedPlayer->GetActorLocation(), LungeSpeed, DeltaTime);
    FaceLocation(TrackedPlayer->GetActorLocation(), DeltaTime);

    if (Dist <= LungeRange * 0.5f)
    {
        ExecuteLunge(TrackedPlayer);
        SetState(ECombat_RaptorState::Retreat);
    }
}

void URaptorPackCombatComponent::TickRetreat(float DeltaTime)
{
    if (!TrackedPlayer) { SetState(ECombat_RaptorState::Idle); return; }

    RetreatTimer += DeltaTime;

    // Retreat away from player
    FVector AwayDir = (GetOwner()->GetActorLocation() - TrackedPlayer->GetActorLocation()).GetSafeNormal();
    FVector RetreatTarget = GetOwner()->GetActorLocation() + AwayDir * 500.0f;
    MoveTowardLocation(RetreatTarget, RetreatSpeed, DeltaTime);

    if (RetreatTimer >= 1.5f)
    {
        RetreatTimer = 0.0f;
        // Re-engage from flank after retreat
        SetState(ECombat_RaptorState::Flank);
    }
}

void URaptorPackCombatComponent::TickDistract(float DeltaTime)
{
    if (!TrackedPlayer) { SetState(ECombat_RaptorState::Idle); return; }

    DistractTimer += DeltaTime;
    PackInfo.bPackEngaged = true;

    // Alpha circles and feints to draw player attention
    float Angle = DistractTimer * 1.5f;
    FVector CircleOffset = FVector(FMath::Cos(Angle) * 300.0f, FMath::Sin(Angle) * 300.0f, 0.0f);
    FVector CircleTarget = TrackedPlayer->GetActorLocation() + CircleOffset;
    MoveTowardLocation(CircleTarget, StalkSpeed * 1.2f, DeltaTime);
    FaceLocation(TrackedPlayer->GetActorLocation(), DeltaTime);

    // After 4 seconds of distraction, alpha also lunges
    if (DistractTimer >= 4.0f)
    {
        DistractTimer = 0.0f;
        PackInfo.bPackEngaged = false;
        SetState(ECombat_RaptorState::Lunge);
    }
}

// ── Pack Coordination ──────────────────────────────────────────────────────────

void URaptorPackCombatComponent::RegisterWithPack(AActor* AlphaActor)
{
    if (!AlphaActor) return;

    URaptorPackCombatComponent* AlphaComp = AlphaActor->FindComponentByClass<URaptorPackCombatComponent>();
    if (AlphaComp)
    {
        AlphaComp->PackInfo.PackMembers.AddUnique(GetOwner());
        PackInfo = AlphaComp->PackInfo;
    }
}

void URaptorPackCombatComponent::BroadcastPlayerLocation(FVector PlayerLocation)
{
    PackInfo.LastKnownPlayerLocation = PlayerLocation;

    for (AActor* Member : PackInfo.PackMembers)
    {
        if (!Member || Member == GetOwner()) continue;
        URaptorPackCombatComponent* MemberComp = Member->FindComponentByClass<URaptorPackCombatComponent>();
        if (MemberComp)
        {
            MemberComp->PackInfo.LastKnownPlayerLocation = PlayerLocation;
            MemberComp->TrackedPlayer = TrackedPlayer;

            // Wake up idle flankers
            if (MemberComp->CurrentState == ECombat_RaptorState::Idle)
            {
                MemberComp->SetState(ECombat_RaptorState::Stalk);
                MemberComp->OnPackEngaged(PlayerLocation);
            }
        }
    }
}

FVector URaptorPackCombatComponent::ComputeFlankPosition(FVector PlayerLocation, FVector PlayerForward) const
{
    FVector Right = FVector::CrossProduct(PlayerForward, FVector::UpVector).GetSafeNormal();

    switch (MyRole)
    {
        case ECombat_RaptorRole::LeftFlank:
            return PlayerLocation + Right * FlankRadius + PlayerForward * (-100.0f);

        case ECombat_RaptorRole::RightFlank:
            return PlayerLocation - Right * FlankRadius + PlayerForward * (-100.0f);

        case ECombat_RaptorRole::Alpha:
        default:
            return PlayerLocation + PlayerForward * 350.0f; // Alpha approaches from front
    }
}

// ── Combat Interface ───────────────────────────────────────────────────────────

void URaptorPackCombatComponent::OnPlayerDetected(AActor* Player)
{
    if (!Player) return;

    TrackedPlayer = Player;
    PackInfo.PackAlertLevel = 100.0f;
    PackInfo.LastKnownPlayerLocation = Player->GetActorLocation();

    SetState(ECombat_RaptorState::Stalk);

    // Alpha broadcasts to pack immediately
    if (MyRole == ECombat_RaptorRole::Alpha)
    {
        BroadcastPlayerLocation(Player->GetActorLocation());
    }
}

void URaptorPackCombatComponent::ExecuteLunge(AActor* Target)
{
    if (!Target) return;

    TimeSinceLastLunge = 0.0f;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(Target, LungeDamage, nullptr, GetOwner(), nullptr);

    // Fire Blueprint event for animation/VFX/audio
    OnRaptorLunge(Target, LungeDamage);
}

void URaptorPackCombatComponent::SetState(ECombat_RaptorState NewState)
{
    if (NewState == CurrentState) return;

    ECombat_RaptorState OldState = CurrentState;
    CurrentState = NewState;
    StateTimer   = 0.0f;

    if (NewState == ECombat_RaptorState::Retreat)
    {
        OnRaptorRetreat();
    }

    OnRaptorStateChanged(OldState, NewState);
}

// ── Private Helpers ────────────────────────────────────────────────────────────

bool URaptorPackCombatComponent::CanSeePlayer(AActor* Player) const
{
    if (!Player || !GetWorld()) return false;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50.0f);
    FVector End   = Player->GetActorLocation()     + FVector(0, 0, 50.0f);

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    return !bBlocked || Hit.GetActor() == Player;
}

AActor* URaptorPackCombatComponent::FindNearestPlayer() const
{
    if (!GetWorld()) return nullptr;

    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Players);

    AActor* Nearest = nullptr;
    float   MinDist = DetectionRange;

    for (AActor* P : Players)
    {
        if (!P || P == GetOwner()) continue;
        APawn* Pawn = Cast<APawn>(P);
        if (!Pawn || !Pawn->IsPlayerControlled()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), P->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Nearest = P;
        }
    }
    return Nearest;
}

void URaptorPackCombatComponent::MoveTowardLocation(FVector TargetLocation, float Speed, float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLoc = Owner->GetActorLocation();
    FVector Direction   = (TargetLocation - CurrentLoc).GetSafeNormal();
    FVector NewLocation = CurrentLoc + Direction * Speed * DeltaTime;

    Owner->SetActorLocation(NewLocation, true);
}

void URaptorPackCombatComponent::FaceLocation(FVector TargetLocation, float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Direction = (TargetLocation - Owner->GetActorLocation()).GetSafeNormal();
    if (Direction.IsNearlyZero()) return;

    FRotator TargetRot   = Direction.Rotation();
    FRotator CurrentRot  = Owner->GetActorRotation();
    FRotator NewRot      = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.0f);
    Owner->SetActorRotation(NewRot);
}
