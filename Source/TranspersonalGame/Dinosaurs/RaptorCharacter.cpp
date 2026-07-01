// RaptorCharacter.cpp
// Transpersonal Game Studio — Agent #4 Performance Optimizer
// Cycle: AUTO_20260701_001

#include "Dinosaurs/RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Raptor stats — fast, fragile, pack hunter
    ClawDamage = 35.0f;
    LeapDamage = 55.0f;
    PackCallRadius = 2000.0f;
    PackRole = 0;
    FlankingOffset = 300.0f;
    bIsPackLeader = false;

    ClawCooldown = 0.0f;
    LeapCooldown = 0.0f;
    PackCallCooldown = 0.0f;

    // Movement — fast and agile
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 700.0f;        // Fast sprint
        MoveComp->JumpZVelocity = 600.0f;       // Can leap
        MoveComp->Mass = 80.0f;                 // Light — 80kg
        MoveComp->BrakingDecelerationWalking = 2000.0f;
        MoveComp->GravityScale = 1.2f;
    }

    // DinosaurBase stats (set via parent properties)
    // Health: 150, Species: Raptor — set in DinosaurBase or Blueprint
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Determine pack role based on PackRole index
    bIsPackLeader = (PackRole == 0);

    if (bIsPackLeader)
    {
        // Leader calls pack to coordinate after 2 seconds
        FTimerHandle InitPackTimer;
        GetWorldTimerManager().SetTimer(InitPackTimer, this, &ARaptorCharacter::CallPackMembers, 2.0f, false);
    }
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick-based cooldown tracking (complement to timer system)
    if (ClawCooldown > 0.0f) ClawCooldown -= DeltaTime;
    if (LeapCooldown > 0.0f) LeapCooldown -= DeltaTime;
    if (PackCallCooldown > 0.0f) PackCallCooldown -= DeltaTime;
}

void ARaptorCharacter::PerformClawAttack()
{
    if (ClawCooldown > 0.0f) return;

    // Range check — raptor claw reach is 150 units
    AActor* Target = nullptr;
    float ClosestDist = 150.0f;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Target = Actor;
        }
    }

    if (Target)
    {
        UGameplayStatics::ApplyDamage(Target, ClawDamage, GetController(), this, nullptr);
    }

    // Set cooldown — raptors attack fast
    ClawCooldown = 1.2f;
    GetWorldTimerManager().SetTimer(ClawCooldownTimer, this, &ARaptorCharacter::ResetClawCooldown, 1.2f, false);
}

void ARaptorCharacter::PerformLeapAttack()
{
    if (LeapCooldown > 0.0f) return;

    // Leap toward target — apply damage on landing
    AActor* Target = nullptr;
    float ClosestDist = 500.0f; // Leap range

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            Target = Actor;
        }
    }

    if (Target)
    {
        // Launch toward target
        FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FVector LaunchVelocity = ToTarget * 800.0f + FVector(0.0f, 0.0f, 400.0f);
        LaunchCharacter(LaunchVelocity, true, true);

        // Apply leap damage
        UGameplayStatics::ApplyDamage(Target, LeapDamage, GetController(), this, nullptr);
    }

    // Leap cooldown — 5 seconds
    LeapCooldown = 5.0f;
    GetWorldTimerManager().SetTimer(LeapCooldownTimer, this, &ARaptorCharacter::ResetLeapCooldown, 5.0f, false);
}

void ARaptorCharacter::CallPackMembers()
{
    if (PackCallCooldown > 0.0f) return;

    // Find other raptors within call radius and alert them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    int32 PackMembersAlerted = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= PackCallRadius)
        {
            // Signal pack member to converge on current target
            ARaptorCharacter* PackMember = Cast<ARaptorCharacter>(Actor);
            if (PackMember)
            {
                PackMembersAlerted++;
            }
        }
    }

    // Pack call cooldown — 8 seconds
    PackCallCooldown = 8.0f;
    GetWorldTimerManager().SetTimer(PackCallTimer, this, &ARaptorCharacter::ResetPackCallCooldown, 8.0f, false);
}

void ARaptorCharacter::SetPackRole(int32 RoleIndex)
{
    PackRole = FMath::Clamp(RoleIndex, 0, 2);
    bIsPackLeader = (PackRole == 0);
}

void ARaptorCharacter::CoordinateFlankingManeuver(AActor* Target)
{
    if (!Target) return;

    // Calculate flanking position based on role
    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = (TargetLoc - GetActorLocation()).GetSafeNormal();
    FVector RightVec = FVector::CrossProduct(ToTarget, FVector::UpVector);

    FVector FlankPosition;
    switch (PackRole)
    {
        case 0: // Leader — direct frontal approach
            FlankPosition = TargetLoc - ToTarget * 200.0f;
            break;
        case 1: // Left flanker
            FlankPosition = TargetLoc + RightVec * FlankingOffset;
            break;
        case 2: // Right flanker
            FlankPosition = TargetLoc - RightVec * FlankingOffset;
            break;
        default:
            FlankPosition = TargetLoc;
            break;
    }

    // Move to flanking position (AI controller handles actual movement)
    // This sets the destination for the BehaviorTree to use
    SetActorLocation(FlankPosition, true);
}

void ARaptorCharacter::ResetClawCooldown() { ClawCooldown = 0.0f; }
void ARaptorCharacter::ResetLeapCooldown() { LeapCooldown = 0.0f; }
void ARaptorCharacter::ResetPackCallCooldown() { PackCallCooldown = 0.0f; }
