// Velociraptor.cpp
// Performance Optimizer Agent #4 — Cycle AUTO_20260701_010
// Pack hunter implementation — high speed, flanking, coordinated attacks

#include "Dinosaurs/Velociraptor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

AVelociraptor::AVelociraptor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Pack coordination defaults
    PackTargetActor = nullptr;
    PackCoordinationRadius = 1500.0f;
    MaxPackSize = 5;

    // Combat defaults
    FlankAngle = 90.0f;
    JumpAttackRange = 300.0f;
    JumpAttackCooldown = 4.0f;
    bIsJumpAttacking = false;
    LastJumpAttackTime = -999.0f;

    // Apply species-specific stats
    ApplySpeciesDefaults();
}

void AVelociraptor::ApplySpeciesDefaults()
{
    // Velociraptor — fast, fragile, pack hunter
    // Capsule: 60cm height, 30cm radius (lean predator)
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
        GetCapsuleComponent()->SetCapsuleRadius(30.0f);
    }

    MaxHealth = 400.0f;
    CurrentHealth = 400.0f;
    AttackDamage = 45.0f;
    AggressionLevel = 0.85f;
    DetectionRadius = 1800.0f;
    AttackRange = 120.0f;

    // High speed — pack hunters rely on speed and coordination
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 1200.0f;
        GetCharacterMovement()->MaxAcceleration = 3000.0f;
        GetCharacterMovement()->JumpZVelocity = 600.0f;
        GetCharacterMovement()->AirControl = 0.4f;
    }
}

void AVelociraptor::BeginPlay()
{
    Super::BeginPlay();
}

void AVelociraptor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If jump attacking, apply forward momentum
    if (bIsJumpAttacking)
    {
        FVector Forward = GetActorForwardVector();
        FVector Velocity = GetVelocity();
        // Maintain jump attack trajectory
        if (Velocity.Z < -100.0f)
        {
            // Landing — end jump attack
            bIsJumpAttacking = false;
        }
    }
}

float AVelociraptor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // Raptors are fragile — no damage reduction
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Alert pack members when hit
    if (ActualDamage > 0.0f && DamageCauser)
    {
        TArray<AVelociraptor*> PackMembers = FindNearbyPackMembers();
        for (AVelociraptor* Member : PackMembers)
        {
            if (Member && Member != this)
            {
                Member->SetPackTarget(DamageCauser);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Velociraptor hit for %.1f — alerting %d pack members"),
            ActualDamage, PackMembers.Num());
    }

    return ActualDamage;
}

void AVelociraptor::CoordinatePackAttack(AActor* Target)
{
    if (!Target) return;

    PackTargetActor = Target;
    TArray<AVelociraptor*> PackMembers = FindNearbyPackMembers();

    // Assign flanking positions to each pack member
    int32 MemberIndex = 0;
    for (AVelociraptor* Member : PackMembers)
    {
        if (!Member || Member == this) continue;

        Member->PackTargetActor = Target;
        FVector FlankPos = Member->CalculateFlankPosition(Target);

        UE_LOG(LogTemp, Log, TEXT("Raptor pack member %d assigned flank position (%.0f, %.0f, %.0f)"),
            MemberIndex, FlankPos.X, FlankPos.Y, FlankPos.Z);
        MemberIndex++;
    }

    // Debug visualization in editor
#if WITH_EDITOR
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), Target->GetActorLocation(), 200.0f, 12,
            FColor::Red, false, 3.0f);
        for (AVelociraptor* Member : PackMembers)
        {
            if (Member)
            {
                DrawDebugLine(GetWorld(), Member->GetActorLocation(),
                    Target->GetActorLocation(), FColor::Orange, false, 3.0f, 0, 2.0f);
            }
        }
    }
#endif
}

TArray<AVelociraptor*> AVelociraptor::FindNearbyPackMembers() const
{
    TArray<AVelociraptor*> PackMembers;
    if (!GetWorld()) return PackMembers;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AVelociraptor* Raptor = Cast<AVelociraptor>(Actor);
        if (Raptor && Raptor != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Raptor->GetActorLocation());
            if (Distance <= PackCoordinationRadius)
            {
                PackMembers.Add(Raptor);
                if (PackMembers.Num() >= MaxPackSize - 1) break;
            }
        }
    }

    return PackMembers;
}

void AVelociraptor::SetPackTarget(AActor* NewTarget)
{
    PackTargetActor = NewTarget;
    UE_LOG(LogTemp, Log, TEXT("Velociraptor pack target updated: %s"),
        NewTarget ? *NewTarget->GetName() : TEXT("None"));
}

void AVelociraptor::PerformJumpAttack()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastJumpAttackTime < JumpAttackCooldown) return;

    bIsJumpAttacking = true;
    LastJumpAttackTime = CurrentTime;

    // Launch forward and upward
    FVector LaunchVelocity = GetActorForwardVector() * 800.0f + FVector(0, 0, 500.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Start cooldown timer
    GetWorldTimerManager().SetTimer(JumpAttackCooldownTimer, this,
        &AVelociraptor::ResetJumpAttackCooldown, JumpAttackCooldown, false);

    UE_LOG(LogTemp, Log, TEXT("Velociraptor jump attack launched!"));

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation() + GetActorForwardVector() * 200.0f,
        80.0f, 8, FColor::Yellow, false, 2.0f);
#endif
}

FVector AVelociraptor::CalculateFlankPosition(AActor* Target) const
{
    if (!Target) return GetActorLocation();

    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = (TargetLoc - GetActorLocation()).GetSafeNormal();

    // Calculate perpendicular flank direction
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);
    float FlankRadians = FMath::DegreesToRadians(FlankAngle);

    // Alternate left/right flanking based on actor index
    int32 ActorIndex = GetUniqueID() % 2;
    FVector FlankDir = ActorIndex == 0 ? Right : -Right;

    return TargetLoc + FlankDir * 400.0f;
}

void AVelociraptor::ResetJumpAttackCooldown()
{
    bIsJumpAttacking = false;
    UE_LOG(LogTemp, Log, TEXT("Velociraptor jump attack cooldown reset"));
}
