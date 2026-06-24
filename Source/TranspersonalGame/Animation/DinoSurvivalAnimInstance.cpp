#include "DinoSurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivalAnimInstance::UDinoSurvivalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsWalking = false;
    bIsIdle = true;

    // Survival defaults
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    bIsExhausted = false;
    bIsInjured = false;

    // Foot IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    PelvisOffset = 0.0f;

    // Thresholds (cm/s)
    WalkThreshold = 10.0f;
    RunThreshold = 200.0f;
    SprintThreshold = 450.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UDinoSurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UDinoSurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache if not set
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ACharacter>(OwnerPawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateLocomotionState();
    UpdateSurvivalState();
    UpdateFootIK();
}

void UDinoSurvivalAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Get velocity magnitude (horizontal only)
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction angle relative to character facing
    FRotator CharRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = CharRotation.UnrotateVector(Velocity);
    Direction = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X) * (180.0f / PI);

    // Air state
    bIsInAir = MovementComponent->IsFalling();

    // Crouch state
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Speed-based locomotion states
    bIsIdle = (Speed < WalkThreshold);
    bIsWalking = (Speed >= WalkThreshold && Speed < RunThreshold);
    bIsSprinting = (Speed >= SprintThreshold);
    // Running = walking but not sprinting
    // bIsRunning = (Speed >= RunThreshold && Speed < SprintThreshold);
}

void UDinoSurvivalAnimInstance::UpdateSurvivalState()
{
    if (!OwnerCharacter) return;

    // Try to get survival stats from TranspersonalCharacter
    // Using generic approach — access properties via reflection if available
    // Default values if character doesn't expose these
    
    // Exhaustion check based on movement speed reduction
    if (MovementComponent)
    {
        float MaxSpeed = MovementComponent->MaxWalkSpeed;
        float CurrentSpeed = Speed;
        
        // Infer stamina from speed ratio (placeholder until full survival system)
        if (MaxSpeed > 0.0f)
        {
            StaminaLevel = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
        }
    }

    bIsExhausted = (StaminaLevel < 0.1f);
    bIsInjured = false; // Will be driven by health system when integrated
}

void UDinoSurvivalAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Only apply foot IK when grounded
    if (bIsInAir)
    {
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        PelvisOffset = 0.0f;
        return;
    }

    float LeftAlpha = 0.0f;
    float RightAlpha = 0.0f;

    LeftFootIKLocation = TraceFootIK(FName("foot_l"), LeftAlpha);
    RightFootIKLocation = TraceFootIK(FName("foot_r"), RightAlpha);

    // Smooth alpha transitions
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, LeftAlpha, GetWorld()->GetDeltaSeconds(), 10.0f);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, RightAlpha, GetWorld()->GetDeltaSeconds(), 10.0f);

    // Pelvis offset — lower pelvis to accommodate the lower foot
    float LeftZ = LeftFootIKLocation.Z;
    float RightZ = RightFootIKLocation.Z;
    float LowestFoot = FMath::Min(LeftZ, RightZ);
    float CharZ = OwnerCharacter->GetActorLocation().Z;
    
    float TargetPelvisOffset = FMath::Clamp(LowestFoot - CharZ, -30.0f, 0.0f);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, GetWorld()->GetDeltaSeconds(), 15.0f);
}

FVector UDinoSurvivalAnimInstance::TraceFootIK(FName SocketName, float& OutAlpha)
{
    OutAlpha = 0.0f;

    if (!OwnerCharacter) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    // Get socket world location
    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);

    // Trace downward from above the foot
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, 50.0f);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, 75.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        OutAlpha = 1.0f;
        return HitResult.Location;
    }

    return SocketLocation;
}
