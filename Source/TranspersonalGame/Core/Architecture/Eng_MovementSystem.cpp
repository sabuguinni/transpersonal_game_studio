#include "Eng_MovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UEng_MovementSystem::UEng_MovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    CurrentStamina = MaxStamina;
    CurrentSpeedModifier = 1.0f;
    bWasMovingLastFrame = false;
}

void UEng_MovementSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to character movement component
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            MovementComponent = Character->GetCharacterMovement();
            LastFrameLocation = Character->GetActorLocation();
            
            if (MovementComponent)
            {
                // Set initial movement speeds
                MovementComponent->MaxWalkSpeed = WalkSpeed;
                UE_LOG(LogTemp, Log, TEXT("Movement System initialized for %s"), *Owner->GetName());
            }
        }
    }
}

void UEng_MovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MovementComponent)
        return;
    
    // Update systems
    UpdateStamina(DeltaTime);
    CheckTerrainConditions();
    UpdateMovementSpeed();
    
    // Update cached location
    if (AActor* Owner = GetOwner())
    {
        LastFrameLocation = Owner->GetActorLocation();
    }
}

void UEng_MovementSystem::StartRunning()
{
    if (!CanRun())
        return;
    
    bIsRunning = true;
    UpdateMovementSpeed();
    
    UE_LOG(LogTemp, Log, TEXT("Started running"));
}

void UEng_MovementSystem::StopRunning()
{
    bIsRunning = false;
    UpdateMovementSpeed();
    
    UE_LOG(LogTemp, Log, TEXT("Stopped running"));
}

void UEng_MovementSystem::StartCrouching()
{
    bIsCrouching = true;
    
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->Crouch();
    }
    
    UpdateMovementSpeed();
    UE_LOG(LogTemp, Log, TEXT("Started crouching"));
}

void UEng_MovementSystem::StopCrouching()
{
    bIsCrouching = false;
    
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->UnCrouch();
    }
    
    UpdateMovementSpeed();
    UE_LOG(LogTemp, Log, TEXT("Stopped crouching"));
}

bool UEng_MovementSystem::CanRun() const
{
    return CurrentStamina > 10.0f && !bIsCrouching && !IsInWater();
}

void UEng_MovementSystem::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
    
    // Stop running if stamina is too low
    if (CurrentStamina < 5.0f && bIsRunning)
    {
        StopRunning();
    }
}

void UEng_MovementSystem::RegenerateStamina(float DeltaTime)
{
    if (!bIsRunning && CurrentStamina < MaxStamina)
    {
        float RegenAmount = StaminaRegenRate * DeltaTime;
        if (!IsMoving())
        {
            RegenAmount *= 2.0f; // Faster regen when stationary
        }
        
        CurrentStamina = FMath::Clamp(CurrentStamina + RegenAmount, 0.0f, MaxStamina);
    }
}

float UEng_MovementSystem::GetTerrainSpeedModifier() const
{
    float Modifier = 1.0f;
    
    if (IsOnSlope())
    {
        Modifier *= SlopeSpeedMultiplier;
    }
    
    if (IsInWater())
    {
        Modifier *= WaterSpeedMultiplier;
    }
    
    return Modifier;
}

bool UEng_MovementSystem::IsOnSlope() const
{
    if (!MovementComponent)
        return false;
    
    // Check if the character is on a slope by examining the floor normal
    FVector FloorNormal = MovementComponent->CurrentFloor.HitResult.Normal;
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FloorNormal.Z));
    
    return SlopeAngle > 15.0f; // Consider slopes steeper than 15 degrees
}

bool UEng_MovementSystem::IsInWater() const
{
    if (!MovementComponent)
        return false;
    
    return MovementComponent->IsSwimming();
}

bool UEng_MovementSystem::IsMoving() const
{
    if (!MovementComponent)
        return false;
    
    return MovementComponent->Velocity.Size() > 10.0f;
}

float UEng_MovementSystem::GetCurrentSpeed() const
{
    if (!MovementComponent)
        return 0.0f;
    
    return MovementComponent->Velocity.Size();
}

FVector UEng_MovementSystem::GetMovementDirection() const
{
    if (!MovementComponent)
        return FVector::ZeroVector;
    
    FVector Velocity = MovementComponent->Velocity;
    Velocity.Z = 0.0f; // Ignore vertical component for direction
    
    return Velocity.GetSafeNormal();
}

void UEng_MovementSystem::UpdateMovementSpeed()
{
    if (!MovementComponent)
        return;
    
    float TargetSpeed = WalkSpeed;
    
    if (bIsCrouching)
    {
        TargetSpeed = CrouchSpeed;
    }
    else if (bIsRunning && CanRun())
    {
        TargetSpeed = RunSpeed;
    }
    
    // Apply terrain modifiers
    TargetSpeed *= GetTerrainSpeedModifier();
    
    // Apply the speed to movement component
    MovementComponent->MaxWalkSpeed = TargetSpeed;
    
    CurrentSpeedModifier = TargetSpeed / WalkSpeed;
}

void UEng_MovementSystem::UpdateStamina(float DeltaTime)
{
    if (bIsRunning && IsMoving())
    {
        // Drain stamina while running
        ConsumeStamina(StaminaDrainRate * DeltaTime);
    }
    else
    {
        // Regenerate stamina
        RegenerateStamina(DeltaTime);
    }
}

void UEng_MovementSystem::CheckTerrainConditions()
{
    // This function can be expanded to detect different terrain types
    // For now, it relies on the movement component's built-in water detection
    // and slope calculation from floor normals
    
    // Future implementation could include:
    // - Raycast downward to detect surface materials
    // - Check for mud, sand, ice, etc.
    // - Apply different movement penalties based on terrain type
}