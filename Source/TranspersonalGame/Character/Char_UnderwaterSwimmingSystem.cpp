#include "Char_UnderwaterSwimmingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UChar_UnderwaterSwimmingSystem::UChar_UnderwaterSwimmingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentSwimmingState = EChar_SwimmingState::OnSurface;
    WaterSurfaceLevel = 0.0f;
    ShallowWaterDepth = 100.0f;
    DeepWaterDepth = 300.0f;
    bWasUnderwater = false;
    LastWaterCheckTime = 0.0f;
    WaterCheckInterval = 0.1f;
}

void UChar_UnderwaterSwimmingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            // Store original movement settings
            ConfigureLandMovement();
        }
    }
    
    // Initialize breathing data
    BreathingData.CurrentBreath = BreathingData.MaxBreathTime;
}

void UChar_UnderwaterSwimmingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update water level checking
    LastWaterCheckTime += DeltaTime;
    if (LastWaterCheckTime >= WaterCheckInterval)
    {
        CheckWaterLevel();
        LastWaterCheckTime = 0.0f;
    }
    
    // Update swimming state and breathing
    UpdateSwimmingState(DeltaTime);
    UpdateBreathing(DeltaTime);
    
    // Apply buoyancy when underwater
    if (IsUnderwater())
    {
        ApplyBuoyancy(DeltaTime);
    }
}

void UChar_UnderwaterSwimmingSystem::EnterWater(float WaterLevel)
{
    WaterSurfaceLevel = WaterLevel;
    
    if (MovementComponent && !MovementComponent->IsSwimming())
    {
        ConfigureSwimmingMovement();
        CurrentSwimmingState = EChar_SwimmingState::Shallow;
        
        UE_LOG(LogTemp, Log, TEXT("Character entered water at level: %f"), WaterLevel);
    }
}

void UChar_UnderwaterSwimmingSystem::ExitWater()
{
    if (MovementComponent && MovementComponent->IsSwimming())
    {
        ConfigureLandMovement();
        CurrentSwimmingState = EChar_SwimmingState::OnSurface;
        StopHoldingBreath();
        
        UE_LOG(LogTemp, Log, TEXT("Character exited water"));
    }
}

void UChar_UnderwaterSwimmingSystem::StartDiving()
{
    if (CurrentSwimmingState != EChar_SwimmingState::OnSurface)
    {
        CurrentSwimmingState = EChar_SwimmingState::Diving;
        StartHoldingBreath();
        
        if (MovementComponent)
        {
            MovementComponent->MaxSwimSpeed = SwimmingSettings.DivingSpeed;
        }
    }
}

void UChar_UnderwaterSwimmingSystem::StartSurfacing()
{
    if (IsUnderwater())
    {
        CurrentSwimmingState = EChar_SwimmingState::Surfacing;
        
        if (MovementComponent)
        {
            MovementComponent->MaxSwimSpeed = SwimmingSettings.UnderwaterSwimSpeed;
        }
    }
}

bool UChar_UnderwaterSwimmingSystem::IsUnderwater() const
{
    if (!OwnerCharacter)
    {
        return false;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    return CharacterLocation.Z < (WaterSurfaceLevel - 50.0f); // 50cm buffer
}

float UChar_UnderwaterSwimmingSystem::GetWaterDepth() const
{
    if (!OwnerCharacter || !IsUnderwater())
    {
        return 0.0f;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    return WaterSurfaceLevel - CharacterLocation.Z;
}

float UChar_UnderwaterSwimmingSystem::GetBreathPercentage() const
{
    return (BreathingData.CurrentBreath / BreathingData.MaxBreathTime) * 100.0f;
}

void UChar_UnderwaterSwimmingSystem::StartHoldingBreath()
{
    BreathingData.bIsHoldingBreath = true;
    UE_LOG(LogTemp, Log, TEXT("Character started holding breath"));
}

void UChar_UnderwaterSwimmingSystem::StopHoldingBreath()
{
    BreathingData.bIsHoldingBreath = false;
    UE_LOG(LogTemp, Log, TEXT("Character stopped holding breath"));
}

bool UChar_UnderwaterSwimmingSystem::IsBreathCritical() const
{
    return BreathingData.CurrentBreath <= (BreathingData.MaxBreathTime * 0.2f);
}

void UChar_UnderwaterSwimmingSystem::RecoverBreath(float DeltaTime)
{
    if (!BreathingData.bIsHoldingBreath && !IsUnderwater())
    {
        BreathingData.CurrentBreath = FMath::Min(
            BreathingData.MaxBreathTime,
            BreathingData.CurrentBreath + (BreathingData.BreathRecoveryRate * DeltaTime)
        );
    }
}

void UChar_UnderwaterSwimmingSystem::ConfigureSwimmingMovement()
{
    if (!MovementComponent)
    {
        return;
    }
    
    MovementComponent->SetMovementMode(MOVE_Swimming);
    MovementComponent->MaxSwimSpeed = SwimmingSettings.SurfaceSwimSpeed;
    MovementComponent->Buoyancy = SwimmingSettings.WaterDensity;
    MovementComponent->SwimmingCapsuleRadius = 50.0f;
    MovementComponent->SwimmingCapsuleHalfHeight = 60.0f;
}

void UChar_UnderwaterSwimmingSystem::ConfigureLandMovement()
{
    if (!MovementComponent)
    {
        return;
    }
    
    MovementComponent->SetMovementMode(MOVE_Walking);
    MovementComponent->MaxWalkSpeed = 400.0f;
    MovementComponent->Buoyancy = 0.0f;
}

void UChar_UnderwaterSwimmingSystem::UpdateSwimmingState(float DeltaTime)
{
    EChar_SwimmingState NewState = DetermineSwimmingState();
    
    if (NewState != CurrentSwimmingState)
    {
        CurrentSwimmingState = NewState;
        
        // Update movement settings based on new state
        if (MovementComponent)
        {
            switch (CurrentSwimmingState)
            {
                case EChar_SwimmingState::OnSurface:
                    ConfigureLandMovement();
                    break;
                case EChar_SwimmingState::Shallow:
                    MovementComponent->MaxSwimSpeed = SwimmingSettings.SurfaceSwimSpeed;
                    break;
                case EChar_SwimmingState::Deep:
                case EChar_SwimmingState::Diving:
                    MovementComponent->MaxSwimSpeed = SwimmingSettings.UnderwaterSwimSpeed;
                    StartHoldingBreath();
                    break;
                case EChar_SwimmingState::Surfacing:
                    MovementComponent->MaxSwimSpeed = SwimmingSettings.UnderwaterSwimSpeed;
                    break;
            }
        }
    }
}

void UChar_UnderwaterSwimmingSystem::UpdateBreathing(float DeltaTime)
{
    if (BreathingData.bIsHoldingBreath && IsUnderwater())
    {
        // Deplete breath while underwater
        BreathingData.CurrentBreath = FMath::Max(
            0.0f,
            BreathingData.CurrentBreath - (BreathingData.BreathDepletionRate * DeltaTime)
        );
        
        // Check for critical breath level
        if (IsBreathCritical())
        {
            UE_LOG(LogTemp, Warning, TEXT("Breath critical! %f seconds remaining"), BreathingData.CurrentBreath);
        }
        
        // Handle breath depletion
        if (BreathingData.CurrentBreath <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Character out of breath! Taking damage"));
            // TODO: Apply drowning damage
        }
    }
    else
    {
        // Recover breath when not underwater or not holding breath
        RecoverBreath(DeltaTime);
    }
}

void UChar_UnderwaterSwimmingSystem::CheckWaterLevel()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Simple water level check - in a real implementation, this would
    // trace against water volumes or use a water system
    bool bCurrentlyUnderwater = CharacterLocation.Z < WaterSurfaceLevel;
    
    if (bCurrentlyUnderwater && !bWasUnderwater)
    {
        EnterWater(WaterSurfaceLevel);
    }
    else if (!bCurrentlyUnderwater && bWasUnderwater)
    {
        ExitWater();
    }
    
    bWasUnderwater = bCurrentlyUnderwater;
}

void UChar_UnderwaterSwimmingSystem::ApplyBuoyancy(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    float WaterDepth = GetWaterDepth();
    if (WaterDepth > 0.0f)
    {
        // Calculate buoyancy force based on submersion depth
        float SubmersionRatio = FMath::Clamp(WaterDepth / 200.0f, 0.0f, 1.0f);
        FVector BuoyancyForce = FVector(0, 0, SwimmingSettings.BuoyancyForce * SubmersionRatio);
        
        // Apply buoyancy force
        MovementComponent->AddForce(BuoyancyForce);
    }
}

EChar_SwimmingState UChar_UnderwaterSwimmingSystem::DetermineSwimmingState() const
{
    if (!OwnerCharacter)
    {
        return EChar_SwimmingState::OnSurface;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    float DepthBelowSurface = WaterSurfaceLevel - CharacterLocation.Z;
    
    if (DepthBelowSurface <= 0.0f)
    {
        return EChar_SwimmingState::OnSurface;
    }
    else if (DepthBelowSurface <= ShallowWaterDepth)
    {
        return EChar_SwimmingState::Shallow;
    }
    else if (DepthBelowSurface <= DeepWaterDepth)
    {
        return EChar_SwimmingState::Deep;
    }
    else
    {
        return EChar_SwimmingState::Diving;
    }
}