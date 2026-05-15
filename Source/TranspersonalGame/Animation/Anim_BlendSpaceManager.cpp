#include "Anim_BlendSpaceManager.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_BlendSpaceManager::UAnim_BlendSpaceManager()
{
    // Initialize default values
    LocomotionData = FAnim_LocomotionBlendSpace();
}

UBlendSpace* UAnim_BlendSpaceManager::CreateLocomotionBlendSpace(const FString& BlendSpaceName, 
                                                               const TArray<UAnimSequence*>& Animations)
{
    if (Animations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create blend space: No animations provided"));
        return nullptr;
    }

    // Create new blend space
    UBlendSpace* NewBlendSpace = NewObject<UBlendSpace>(this, FName(*BlendSpaceName));
    if (!NewBlendSpace)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create blend space: %s"), *BlendSpaceName);
        return nullptr;
    }

    // Configure axes
    ConfigureBlendSpaceAxes(NewBlendSpace, TEXT("Speed"), TEXT("Direction"));

    // Add animations to blend space in standard locomotion positions
    if (Animations.Num() >= 8)
    {
        // Standard 8-directional locomotion setup
        AddAnimationToBlendSpace(NewBlendSpace, Animations[0], FVector(1.0f, 0.0f, 0.0f));    // Forward
        AddAnimationToBlendSpace(NewBlendSpace, Animations[1], FVector(-1.0f, 0.0f, 0.0f));   // Backward
        AddAnimationToBlendSpace(NewBlendSpace, Animations[2], FVector(0.0f, -1.0f, 0.0f));   // Left
        AddAnimationToBlendSpace(NewBlendSpace, Animations[3], FVector(0.0f, 1.0f, 0.0f));    // Right
        AddAnimationToBlendSpace(NewBlendSpace, Animations[4], FVector(0.7f, -0.7f, 0.0f));   // Forward-Left
        AddAnimationToBlendSpace(NewBlendSpace, Animations[5], FVector(0.7f, 0.7f, 0.0f));    // Forward-Right
        AddAnimationToBlendSpace(NewBlendSpace, Animations[6], FVector(-0.7f, -0.7f, 0.0f));  // Backward-Left
        AddAnimationToBlendSpace(NewBlendSpace, Animations[7], FVector(-0.7f, 0.7f, 0.0f));   // Backward-Right
    }
    else
    {
        // Simplified setup with available animations
        for (int32 i = 0; i < Animations.Num(); ++i)
        {
            float Angle = (i * 360.0f / Animations.Num()) * PI / 180.0f;
            FVector Position(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
            AddAnimationToBlendSpace(NewBlendSpace, Animations[i], Position);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Created locomotion blend space: %s with %d animations"), 
           *BlendSpaceName, Animations.Num());

    return NewBlendSpace;
}

UBlendSpace1D* UAnim_BlendSpaceManager::CreateSpeedBlendSpace(const FString& BlendSpaceName,
                                                            UAnimSequence* SlowAnim,
                                                            UAnimSequence* FastAnim)
{
    if (!SlowAnim || !FastAnim)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create speed blend space: Missing animations"));
        return nullptr;
    }

    UBlendSpace1D* NewBlendSpace1D = NewObject<UBlendSpace1D>(this, FName(*BlendSpaceName));
    if (!NewBlendSpace1D)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create 1D blend space: %s"), *BlendSpaceName);
        return nullptr;
    }

    // Add animations at different speed positions
    AddAnimationToBlendSpace1D(NewBlendSpace1D, SlowAnim, 0.0f);
    AddAnimationToBlendSpace1D(NewBlendSpace1D, FastAnim, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Created speed blend space: %s"), *BlendSpaceName);

    return NewBlendSpace1D;
}

void UAnim_BlendSpaceManager::SetupWalkBlendSpace()
{
    if (!LocomotionData.WalkBlendSpace)
    {
        TArray<UAnimSequence*> WalkAnimations;
        WalkAnimations.Add(LocomotionData.WalkForward);
        WalkAnimations.Add(LocomotionData.WalkBackward);
        WalkAnimations.Add(LocomotionData.WalkLeft);
        WalkAnimations.Add(LocomotionData.WalkRight);
        WalkAnimations.Add(LocomotionData.WalkForwardLeft);
        WalkAnimations.Add(LocomotionData.WalkForwardRight);
        WalkAnimations.Add(LocomotionData.WalkBackwardLeft);
        WalkAnimations.Add(LocomotionData.WalkBackwardRight);

        // Filter out null animations
        WalkAnimations.RemoveAll([](UAnimSequence* Anim) { return Anim == nullptr; });

        if (WalkAnimations.Num() > 0)
        {
            LocomotionData.WalkBlendSpace = CreateLocomotionBlendSpace(TEXT("WalkBlendSpace"), WalkAnimations);
        }
    }
}

void UAnim_BlendSpaceManager::SetupRunBlendSpace()
{
    if (!LocomotionData.RunBlendSpace)
    {
        TArray<UAnimSequence*> RunAnimations;
        RunAnimations.Add(LocomotionData.RunForward);
        RunAnimations.Add(LocomotionData.RunBackward);
        RunAnimations.Add(LocomotionData.RunLeft);
        RunAnimations.Add(LocomotionData.RunRight);

        // Filter out null animations
        RunAnimations.RemoveAll([](UAnimSequence* Anim) { return Anim == nullptr; });

        if (RunAnimations.Num() > 0)
        {
            LocomotionData.RunBlendSpace = CreateLocomotionBlendSpace(TEXT("RunBlendSpace"), RunAnimations);
        }
    }
}

void UAnim_BlendSpaceManager::SetupCrouchBlendSpace()
{
    // Placeholder for crouch blend space setup
    // Would use crouch-specific animations when available
    UE_LOG(LogTemp, Log, TEXT("Crouch blend space setup - placeholder implementation"));
}

FVector2D UAnim_BlendSpaceManager::CalculateLocomotionInput(float Speed, float Direction) const
{
    // Normalize speed (0-1 range)
    float NormalizedSpeed = FMath::Clamp(Speed / MAX_LOCOMOTION_SPEED, 0.0f, 1.0f);
    
    // Convert direction from degrees to normalized range (-1 to 1)
    float NormalizedDirection = FMath::Clamp(Direction / MAX_DIRECTION_ANGLE, -1.0f, 1.0f);
    
    return FVector2D(NormalizedSpeed, NormalizedDirection);
}

float UAnim_BlendSpaceManager::CalculateSpeedInput(float CurrentSpeed, float MaxSpeed) const
{
    return FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
}

FVector2D UAnim_BlendSpaceManager::CalculateDirectionalInput(const FVector& Velocity, const FVector& ForwardVector) const
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return FVector2D::ZeroVector;
    }

    // Normalize velocity (ignore Z component)
    FVector NormalizedVelocity = Velocity;
    NormalizedVelocity.Z = 0.0f;
    NormalizedVelocity.Normalize();

    // Calculate forward/backward component
    float ForwardComponent = FVector::DotProduct(NormalizedVelocity, ForwardVector);
    
    // Calculate right component
    FVector RightVector = FVector::CrossProduct(ForwardVector, FVector::UpVector);
    float RightComponent = FVector::DotProduct(NormalizedVelocity, RightVector);

    return FVector2D(ForwardComponent, RightComponent);
}

bool UAnim_BlendSpaceManager::ValidateBlendSpace(UBlendSpace* BlendSpace) const
{
    if (!BlendSpace)
    {
        return false;
    }

    // Check if blend space has valid sample data
    // Note: This is a simplified validation - real implementation would check sample points
    return true;
}

bool UAnim_BlendSpaceManager::ValidateBlendSpace1D(UBlendSpace1D* BlendSpace1D) const
{
    if (!BlendSpace1D)
    {
        return false;
    }

    // Check if 1D blend space has valid sample data
    return true;
}

void UAnim_BlendSpaceManager::RefreshAllBlendSpaces()
{
    SetupWalkBlendSpace();
    SetupRunBlendSpace();
    SetupCrouchBlendSpace();

    UE_LOG(LogTemp, Log, TEXT("Refreshed all blend spaces"));
}

TArray<UBlendSpace*> UAnim_BlendSpaceManager::GetAllBlendSpaces() const
{
    TArray<UBlendSpace*> BlendSpaces;
    
    if (LocomotionData.WalkBlendSpace)
    {
        BlendSpaces.Add(LocomotionData.WalkBlendSpace);
    }
    
    if (LocomotionData.RunBlendSpace)
    {
        BlendSpaces.Add(LocomotionData.RunBlendSpace);
    }
    
    if (LocomotionData.CrouchBlendSpace)
    {
        BlendSpaces.Add(LocomotionData.CrouchBlendSpace);
    }

    // Add custom blend spaces
    for (const FAnim_BlendSpaceData& CustomData : CustomBlendSpaces)
    {
        if (CustomData.BlendSpace)
        {
            BlendSpaces.Add(CustomData.BlendSpace);
        }
    }

    return BlendSpaces;
}

UBlendSpace* UAnim_BlendSpaceManager::GetBlendSpaceByType(EAnim_BlendSpaceType BlendSpaceType) const
{
    switch (BlendSpaceType)
    {
        case EAnim_BlendSpaceType::Locomotion:
            return LocomotionData.WalkBlendSpace;
        
        case EAnim_BlendSpaceType::Directional:
            return LocomotionData.RunBlendSpace;
        
        default:
            break;
    }

    // Search in custom blend spaces
    for (const FAnim_BlendSpaceData& CustomData : CustomBlendSpaces)
    {
        if (CustomData.BlendSpaceType == BlendSpaceType && CustomData.BlendSpace)
        {
            return CustomData.BlendSpace;
        }
    }

    return nullptr;
}

void UAnim_BlendSpaceManager::ConfigureBlendSpaceAxes(UBlendSpace* BlendSpace, 
                                                     const FString& XAxisName,
                                                     const FString& YAxisName) const
{
    if (!BlendSpace)
    {
        return;
    }

    // Note: In a real implementation, you would configure the blend space axes here
    // This would involve setting up the axis parameters, ranges, and interpolation settings
    // For now, this is a placeholder that logs the configuration
    
    UE_LOG(LogTemp, Log, TEXT("Configuring blend space axes: X=%s, Y=%s"), *XAxisName, *YAxisName);
}

void UAnim_BlendSpaceManager::AddAnimationToBlendSpace(UBlendSpace* BlendSpace, 
                                                      UAnimSequence* Animation,
                                                      const FVector& Position) const
{
    if (!BlendSpace || !Animation)
    {
        return;
    }

    // Note: In a real implementation, you would add the animation sample to the blend space
    // This would involve creating a blend sample and setting its position and animation
    
    UE_LOG(LogTemp, Log, TEXT("Adding animation %s to blend space at position (%f, %f, %f)"), 
           *Animation->GetName(), Position.X, Position.Y, Position.Z);
}

void UAnim_BlendSpaceManager::AddAnimationToBlendSpace1D(UBlendSpace1D* BlendSpace1D,
                                                        UAnimSequence* Animation,
                                                        float Position) const
{
    if (!BlendSpace1D || !Animation)
    {
        return;
    }

    // Note: In a real implementation, you would add the animation sample to the 1D blend space
    
    UE_LOG(LogTemp, Log, TEXT("Adding animation %s to 1D blend space at position %f"), 
           *Animation->GetName(), Position);
}