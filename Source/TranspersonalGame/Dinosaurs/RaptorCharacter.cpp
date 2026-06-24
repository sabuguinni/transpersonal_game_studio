// RaptorCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Raptor species: pack hunter, flanking AI, leap attack, coordinated assault

#include "RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ARaptorCharacter::ARaptorCharacter()
{
    // Species identity
    SpeciesName = FName("Velociraptor");

    // Pack hunter — fast, agile, lower individual damage
    MaxHealth       = 350.0f;
    CurrentHealth   = 350.0f;
    AttackDamage    = 45.0f;
    AttackRange     = 150.0f;
    AttackCooldown  = 0.8f;

    // Raptors are fast
    PatrolSpeed     = 300.0f;
    ChaseSpeed      = 700.0f;
    TerritoryRadius = 2500.0f;
    ThreatDetectRadius = 1800.0f;

    // Pack behaviour
    PackRadius          = 1200.0f;
    bIsPackLeader       = false;
    PackCoordDelay      = 1.5f;   // seconds between coordinated attacks
    FlankAngleOffset    = 90.0f;  // degrees — flankers approach from the side

    // Leap attack
    bLeapReady          = true;
    LeapDamage          = 80.0f;
    LeapRange           = 500.0f;
    LeapCooldown        = 6.0f;
    bIsLeaping          = false;

    // Capsule — Raptor is small and agile
    GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
    GetCapsuleComponent()->SetCapsuleRadius(30.0f);

    // Movement — very agile
    GetCharacterMovement()->MaxWalkSpeed    = PatrolSpeed;
    GetCharacterMovement()->MaxAcceleration = 2000.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 800.0f;
    GetCharacterMovement()->RotationRate    = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->JumpZVelocity   = 600.0f;
    GetCharacterMovement()->AirControl      = 0.6f;
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Elect pack leader: first raptor in proximity becomes leader
    TArray<AActor*> NearbyRaptors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyRaptors);

    bool bFoundLeader = false;
    for (AActor* Actor : NearbyRaptors)
    {
        ARaptorCharacter* Other = Cast<ARaptorCharacter>(Actor);
        if (Other && Other != this && Other->bIsPackLeader)
        {
            bFoundLeader = true;
            PackLeader = Other;
            break;
        }
    }

    if (!bFoundLeader)
    {
        // This raptor becomes the pack leader
        bIsPackLeader = true;
        UE_LOG(LogTemp, Log, TEXT("Raptor [%s] elected as PACK LEADER"), *GetName());
    }

    // Start pack coordination timer (leaders only)
    if (bIsPackLeader)
    {
        GetWorldTimerManager().SetTimer(
            PackCoordHandle,
            this,
            &ARaptorCharacter::CoordinatePackAttack,
            PackCoordDelay,
            true   // looping
        );
    }
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // During leap, apply upward + forward impulse
    if (bIsLeaping)
    {
        // Leap handled by Jump() — just track state
        if (GetCharacterMovement()->IsMovingOnGround())
        {
            bIsLeaping = false;
            GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
        }
    }
}

void ARaptorCharacter::PerformAttack()
{
    // Raptors prefer leap attack when in range
    if (bLeapReady)
    {
        APawn* Target = GetNearestThreat();
        if (Target)
        {
            float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
            if (Dist <= LeapRange && Dist > AttackRange)
            {
                PerformLeapAttack(Target);
                return;
            }
        }
    }

    // Normal claw/bite attack
    Super::PerformAttack();
}

void ARaptorCharacter::PerformLeapAttack(APawn* Target)
{
    if (!Target || bIsLeaping || !bLeapReady) return;

    bIsLeaping  = true;
    bLeapReady  = false;

    // Face target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    SetActorRotation(ToTarget.Rotation());

    // Launch toward target with upward arc
    FVector LaunchVelocity = ToTarget * 900.0f + FVector(0.0f, 0.0f, 500.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Deal damage on landing (0.6s)
    FTimerHandle LeapDmgHandle;
    GetWorldTimerManager().SetTimer(
        LeapDmgHandle,
        [this, Target]()
        {
            if (Target && IsValid(Target))
            {
                float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
                if (Dist <= AttackRange * 2.0f)
                {
                    UGameplayStatics::ApplyDamage(
                        Target,
                        LeapDamage,
                        GetController(),
                        this,
                        UDamageType::StaticClass()
                    );
                    UE_LOG(LogTemp, Log, TEXT("Raptor LEAP HIT: %s for %.0f dmg"), *Target->GetName(), LeapDamage);
                }
            }
        },
        0.6f,
        false
    );

    // Leap cooldown
    GetWorldTimerManager().SetTimer(
        LeapCooldownHandle,
        this,
        &ARaptorCharacter::OnLeapCooldownExpired,
        LeapCooldown,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("Raptor [%s] LEAPING at %s"), *GetName(), *Target->GetName());
}

void ARaptorCharacter::CoordinatePackAttack()
{
    if (!bIsPackLeader) return;

    // Find current threat
    APawn* Target = GetNearestThreat();
    if (!Target) return;

    // Find all pack members
    TArray<AActor*> PackMembers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), PackMembers);

    int32 FlankIndex = 0;
    for (AActor* Actor : PackMembers)
    {
        ARaptorCharacter* Member = Cast<ARaptorCharacter>(Actor);
        if (!Member || Member == this) continue;

        float DistToLeader = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
        if (DistToLeader > PackRadius) continue; // Out of pack range

        // Assign flanking positions
        float AngleOffset = FlankAngleOffset * FlankIndex;
        Member->SetFlankTarget(Target, AngleOffset);
        FlankIndex++;
    }

    // Leader attacks from the front
    if (Target)
    {
        FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        AddMovementInput(ToTarget, 1.0f);
    }
}

void ARaptorCharacter::SetFlankTarget(APawn* Target, float AngleDegrees)
{
    if (!Target) return;

    FlankTarget      = Target;
    FlankAngle       = AngleDegrees;
    bIsFlanking      = true;

    // Calculate flank position
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector FlankDir = ToTarget.RotateAngleAxis(AngleDegrees, FVector::UpVector);
    FVector FlankPos = Target->GetActorLocation() + FlankDir * (-200.0f); // approach from flank side

    // Move toward flank position
    AddMovementInput((FlankPos - GetActorLocation()).GetSafeNormal(), 1.0f);
}

void ARaptorCharacter::OnLeapCooldownExpired()
{
    bLeapReady = true;
}

APawn* ARaptorCharacter::GetNearestThreat() const
{
    TArray<AActor*> NearbyPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyPawns);

    APawn* Closest = nullptr;
    float  MinDist = ThreatDetectRadius;

    for (AActor* Actor : NearbyPawns)
    {
        if (Actor == this) continue;
        if (Actor->IsA(ADinosaurBase::StaticClass())) continue; // Don't attack other dinos

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Closest = Cast<APawn>(Actor);
        }
    }

    return Closest;
}
