#include "Core_CharacterMovement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCore_CharacterMovement::UCore_CharacterMovement()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize movement parameters
    WalkSpeed = 600.0f;
    RunSpeed = 1200.0f;
    JumpVelocity = 420.0f;
    StaminaDrainRate = 20.0f;
    StaminaRegenRate = 15.0f;
    CurrentStamina = 100.0f;
    MaxStamina = 100.0f;
    
    // Terrain adaptation settings
    TerrainAdaptationSpeed = 5.0f;
    SlopeSpeedModifier = 0.7f;
    MaxClimbAngle = 45.0f;
    
    // Physics integration settings
    bUsePhysicsIntegration = true;
    GroundFriction = 8.0f;
    AirControl = 0.2f;
    
    // Initialize state
    CurrentMovementState = ECore_MovementState::Idle;
    bIsRunning = false;
    bIsJumping = false;
    bIsOnGround = true;
    TimeSinceGroundContact = 0.0f;
    LastGroundNormal = FVector::UpVector;
}

void UCore_CharacterMovement::BeginPlay()
{
    Super::BeginPlay();
    InitializeMovementComponent();
}

void UCore_CharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CheckGroundContact();
    HandleStaminaSystem(DeltaTime);
    ApplyTerrainAdaptation();
    UpdateMovementSpeed();
    
    if (!bIsOnGround)
    {
        HandleAirMovement(DeltaTime);
    }
}

void UCore_CharacterMovement::InitializeMovementComponent()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        CharacterMovementComponent = Character->GetCharacterMovement();
        CapsuleComponent = Character->GetCapsuleComponent();
        
        if (CharacterMovementComponent)
        {
            // Configure base movement settings
            CharacterMovementComponent->MaxWalkSpeed = WalkSpeed;
            CharacterMovementComponent->JumpZVelocity = JumpVelocity;
            CharacterMovementComponent->AirControl = AirControl;
            CharacterMovementComponent->GroundFriction = GroundFriction;
            CharacterMovementComponent->MaxAcceleration = 2048.0f;
            CharacterMovementComponent->BrakingDecelerationWalking = 2048.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Core_CharacterMovement: Initialized with Character Movement Component"));
        }
    }
}

void UCore_CharacterMovement::SetMovementState(ECore_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        CurrentMovementState = NewState;
        UpdateMovementSpeed();
        
        UE_LOG(LogTemp, Log, TEXT("Core_CharacterMovement: State changed to %d"), (int32)NewState);
    }
}

void UCore_CharacterMovement::ProcessMovementInput(FVector InputVector)
{
    if (!CharacterMovementComponent || InputVector.IsZero())
    {
        SetMovementState(ECore_MovementState::Idle);
        return;
    }
    
    // Determine movement state based on input and running status
    if (bIsRunning && CurrentStamina > 0.0f)
    {
        SetMovementState(ECore_MovementState::Running);
    }
    else
    {
        SetMovementState(ECore_MovementState::Walking);
    }
    
    // Calculate movement direction considering terrain
    FVector MovementDirection = CalculateMovementDirection(InputVector);
    
    // Apply terrain slope modifier
    float TerrainSlope = GetTerrainSlope();
    float SpeedModifier = FMath::Lerp(1.0f, SlopeSpeedModifier, TerrainSlope / MaxClimbAngle);
    
    // Apply movement
    if (bUsePhysicsIntegration)
    {
        ApplyPhysicsMovement(MovementDirection * SpeedModifier, GetWorld()->GetDeltaSeconds());
    }
    else
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            Character->AddMovementInput(MovementDirection, SpeedModifier);
        }
    }
}

void UCore_CharacterMovement::StartRunning()
{
    if (CurrentStamina > 10.0f && bIsOnGround)
    {
        bIsRunning = true;
        UE_LOG(LogTemp, Log, TEXT("Core_CharacterMovement: Started running"));
    }
}

void UCore_CharacterMovement::StopRunning()
{
    bIsRunning = false;
    UE_LOG(LogTemp, Log, TEXT("Core_CharacterMovement: Stopped running"));
}

void UCore_CharacterMovement::Jump()
{
    if (bIsOnGround && CharacterMovementComponent && CurrentStamina > 20.0f)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            Character->Jump();
            bIsJumping = true;
            CurrentStamina = FMath::Max(0.0f, CurrentStamina - 20.0f);
            SetMovementState(ECore_MovementState::Jumping);
            
            UE_LOG(LogTemp, Log, TEXT("Core_CharacterMovement: Jump executed"));
        }
    }
}

void UCore_CharacterMovement::UpdateStamina(float DeltaTime)
{
    HandleStaminaSystem(DeltaTime);
}

void UCore_CharacterMovement::HandleStaminaSystem(float DeltaTime)
{
    if (bIsRunning && CurrentMovementState == ECore_MovementState::Running)
    {
        // Drain stamina while running
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * DeltaTime));
        
        // Stop running if stamina depleted
        if (CurrentStamina <= 0.0f)
        {
            StopRunning();
        }
    }
    else
    {
        // Regenerate stamina when not running
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
    }
}

void UCore_CharacterMovement::UpdateMovementSpeed()
{
    if (!CharacterMovementComponent) return;
    
    float TargetSpeed = WalkSpeed;
    
    switch (CurrentMovementState)
    {
        case ECore_MovementState::Walking:
            TargetSpeed = WalkSpeed;
            break;
        case ECore_MovementState::Running:
            TargetSpeed = RunSpeed;
            break;
        case ECore_MovementState::Jumping:
        case ECore_MovementState::Falling:
            // Don't change speed during air movement
            return;
        case ECore_MovementState::Idle:
        default:
            TargetSpeed = 0.0f;
            break;
    }
    
    CharacterMovementComponent->MaxWalkSpeed = TargetSpeed;
}

void UCore_CharacterMovement::CheckGroundContact()
{
    if (!CapsuleComponent) return;
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Start - FVector(0, 0, CapsuleComponent->GetScaledCapsuleHalfHeight() + 50.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    bool bWasOnGround = bIsOnGround;
    bIsOnGround = bHit && HitResult.Distance < 100.0f;
    
    if (bIsOnGround)
    {
        TimeSinceGroundContact = 0.0f;
        LastGroundNormal = HitResult.Normal;
        
        if (bIsJumping)
        {
            bIsJumping = false;
            SetMovementState(ECore_MovementState::Idle);
        }
        
        HandleGroundContact(HitResult);
    }
    else
    {
        TimeSinceGroundContact += GetWorld()->GetDeltaSeconds();
        
        if (bWasOnGround && !bIsJumping)
        {
            SetMovementState(ECore_MovementState::Falling);
        }
    }
}

float UCore_CharacterMovement::GetTerrainSlope()
{
    if (!bIsOnGround) return 0.0f;
    
    float SlopeAngle = FMath::Acos(FVector::DotProduct(LastGroundNormal, FVector::UpVector));
    return FMath::RadiansToDegrees(SlopeAngle);
}

bool UCore_CharacterMovement::IsOnValidTerrain()
{
    float SlopeAngle = GetTerrainSlope();
    return SlopeAngle <= MaxClimbAngle;
}

FVector UCore_CharacterMovement::GetTerrainNormal()
{
    return LastGroundNormal;
}

void UCore_CharacterMovement::ApplyTerrainAdaptation()
{
    if (!bIsOnGround || !CharacterMovementComponent) return;
    
    // Adjust movement based on terrain slope
    float SlopeAngle = GetTerrainSlope();
    
    if (SlopeAngle > MaxClimbAngle)
    {
        // Prevent movement up steep slopes
        FVector CurrentVelocity = CharacterMovementComponent->Velocity;
        FVector ProjectedVelocity = FVector::VectorPlaneProject(CurrentVelocity, LastGroundNormal);
        CharacterMovementComponent->Velocity = ProjectedVelocity;
    }
}

FVector UCore_CharacterMovement::CalculateMovementDirection(FVector InputVector)
{
    if (!GetOwner()) return FVector::ZeroVector;
    
    // Get actor's forward and right vectors
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = GetOwner()->GetActorRightVector();
    
    // Calculate movement direction
    FVector MovementDirection = (ForwardVector * InputVector.X) + (RightVector * InputVector.Y);
    MovementDirection.Z = 0.0f; // Remove vertical component
    MovementDirection.Normalize();
    
    return MovementDirection;
}

void UCore_CharacterMovement::ApplyPhysicsMovement(FVector MovementVector, float DeltaTime)
{
    if (!CharacterMovementComponent) return;
    
    // Apply movement with physics integration
    FVector CurrentVelocity = CharacterMovementComponent->Velocity;
    FVector TargetVelocity = MovementVector * CharacterMovementComponent->MaxWalkSpeed;
    
    // Smooth velocity transition
    FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, TerrainAdaptationSpeed);
    
    // Preserve vertical velocity for jumping/falling
    NewVelocity.Z = CurrentVelocity.Z;
    
    CharacterMovementComponent->Velocity = NewVelocity;
}

void UCore_CharacterMovement::HandleGroundContact(FHitResult HitResult)
{
    // Handle special ground contact behaviors
    if (HitResult.GetActor())
    {
        // Could add special behaviors for different surface types
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CharacterMovement: Ground contact with %s"), 
               *HitResult.GetActor()->GetName());
    }
}

void UCore_CharacterMovement::HandleAirMovement(float DeltaTime)
{
    if (!CharacterMovementComponent) return;
    
    // Reduce air control over time to simulate realistic physics
    float AirControlReduction = FMath::Clamp(TimeSinceGroundContact / 2.0f, 0.0f, 0.8f);
    float EffectiveAirControl = AirControl * (1.0f - AirControlReduction);
    
    CharacterMovementComponent->AirControl = EffectiveAirControl;
}