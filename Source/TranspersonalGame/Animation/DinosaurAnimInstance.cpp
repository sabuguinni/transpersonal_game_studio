// DinosaurAnimInstance.cpp
// Agent #10 — Animation Agent
// Prehistoric survival game — UAnimInstance for dinosaur AI pawns

#include "DinosaurAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    DinoSpeed = 0.f;
    SmoothedDinoSpeed = 0.f;
    DinoDirection = 0.f;
    bDinoIsInAir = false;
    bDinoIsAttacking = false;
    bDinoIsEating = false;
    bDinoIsRoaring = false;
    bDinoIsDead = false;
    bDinoIsAlert = false;
    bDinoIsSleeping = false;
    DinoHealthAlpha = 1.f;
    AttackTypeIndex = 0;
    TailSwingAlpha = 0.f;
    HeadTrackingAlpha = 0.f;
    CachedDinoCharacter = nullptr;
    CachedDinoMovement = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn) return;

    CachedDinoCharacter = Cast<ACharacter>(OwnerPawn);
    if (CachedDinoCharacter)
    {
        CachedDinoMovement = CachedDinoCharacter->GetCharacterMovement();
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedDinoCharacter || !CachedDinoMovement) return;

    // --- Locomotion ---
    FVector Velocity = CachedDinoMovement->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
    DinoSpeed = HorizontalVelocity.Size();

    SmoothedDinoSpeed = FMath::FInterpTo(SmoothedDinoSpeed, DinoSpeed, DeltaSeconds, 6.f);

    FRotator ActorRotation = CachedDinoCharacter->GetActorRotation();
    DinoDirection = CalculateDirection(Velocity, ActorRotation);

    bDinoIsInAir = CachedDinoMovement->IsFalling();

    // --- Tail swing scales with speed ---
    TailSwingAlpha = FMath::FInterpTo(
        TailSwingAlpha,
        FMath::GetMappedRangeValueClamped(FVector2D(0.f, 600.f), FVector2D(0.f, 1.f), DinoSpeed),
        DeltaSeconds,
        4.f
    );

    // --- Head tracking: alert dinosaurs track the player ---
    float TargetHeadTracking = (bDinoIsAlert && !bDinoIsAttacking && !bDinoIsDead) ? 1.f : 0.f;
    HeadTrackingAlpha = FMath::FInterpTo(HeadTrackingAlpha, TargetHeadTracking, DeltaSeconds, 3.f);

    // --- Dead state: freeze all motion ---
    if (bDinoIsDead)
    {
        DinoSpeed = 0.f;
        SmoothedDinoSpeed = 0.f;
        TailSwingAlpha = 0.f;
        HeadTrackingAlpha = 0.f;
    }
}

void UDinosaurAnimInstance::SetDinoAIState(bool bAttacking, bool bEating, bool bRoaring, bool bAlert, bool bSleeping)
{
    bDinoIsAttacking = bAttacking;
    bDinoIsEating    = bEating;
    bDinoIsRoaring   = bRoaring;
    bDinoIsAlert     = bAlert;
    bDinoIsSleeping  = bSleeping;
}

void UDinosaurAnimInstance::SetDinoHealth(float HealthNormalized)
{
    DinoHealthAlpha = FMath::Clamp(HealthNormalized, 0.f, 1.f);
    bDinoIsDead = (DinoHealthAlpha <= 0.f);
}

void UDinosaurAnimInstance::TriggerAttackMontage(int32 AttackType)
{
    AttackTypeIndex = AttackType;
    bDinoIsAttacking = true;
}
