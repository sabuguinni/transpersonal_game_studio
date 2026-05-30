#include "Anim_CharacterMovementBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterMovementBlueprint::UAnim_CharacterMovementBlueprint()
{
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    JumpAnimation = nullptr;
    CrouchAnimation = nullptr;
}

void UAnim_CharacterMovementBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get the owning character
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Animation Blueprint initialized for: %s"), 
                *OwningCharacter->GetName()));
        }
    }
}

void UAnim_CharacterMovementBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (OwningCharacter && MovementComponent)
    {
        UpdateMovementData();
        UpdateMovementState();
    }
}

void UAnim_CharacterMovementBlueprint::UpdateMovementData()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character facing
    if (MovementData.Speed > 0.1f)
    {
        CalculateMovementDirection();
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Check if character is in air
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Check if character is crouching
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Determine current movement state
    MovementData.MovementState = DetermineMovementState();
}

void UAnim_CharacterMovementBlueprint::CalculateMovementDirection()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    
    // Normalize velocity (remove Z component for ground movement)
    FVector GroundVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    GroundVelocity.Normalize();
    
    // Calculate dot product to get direction
    float DotProduct = FVector::DotProduct(ForwardVector, GroundVelocity);
    
    // Calculate cross product to determine left/right
    FVector CrossProduct = FVector::CrossProduct(ForwardVector, GroundVelocity);
    float CrossZ = CrossProduct.Z;
    
    // Convert to angle (-180 to 180)
    MovementData.Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, DotProduct));
}

EAnim_MovementState UAnim_CharacterMovementBlueprint::DetermineMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if falling/jumping first
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check if swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check movement speed for ground movement
    if (MovementData.Speed > 10.0f)
    {
        // Determine walk vs run based on speed threshold
        float WalkThreshold = MovementComponent->MaxWalkSpeed * 0.6f;
        
        if (MovementData.Speed > WalkThreshold)
        {
            return EAnim_MovementState::Running;
        }
        else
        {
            return EAnim_MovementState::Walking;
        }
    }
    
    return EAnim_MovementState::Idle;
}

void UAnim_CharacterMovementBlueprint::UpdateMovementState()
{
    // This function can be called from Blueprint to force an update
    if (OwningCharacter && MovementComponent)
    {
        UpdateMovementData();
        
        // Debug output for movement state changes
        static EAnim_MovementState LastState = EAnim_MovementState::Idle;
        if (MovementData.MovementState != LastState)
        {
            if (GEngine)
            {
                FString StateName;
                switch (MovementData.MovementState)
                {
                    case EAnim_MovementState::Idle: StateName = TEXT("Idle"); break;
                    case EAnim_MovementState::Walking: StateName = TEXT("Walking"); break;
                    case EAnim_MovementState::Running: StateName = TEXT("Running"); break;
                    case EAnim_MovementState::Jumping: StateName = TEXT("Jumping"); break;
                    case EAnim_MovementState::Falling: StateName = TEXT("Falling"); break;
                    case EAnim_MovementState::Crouching: StateName = TEXT("Crouching"); break;
                    case EAnim_MovementState::Swimming: StateName = TEXT("Swimming"); break;
                    case EAnim_MovementState::Climbing: StateName = TEXT("Climbing"); break;
                }
                
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                    FString::Printf(TEXT("Movement State: %s (Speed: %.1f)"), 
                    *StateName, MovementData.Speed));
            }
            
            LastState = MovementData.MovementState;
        }
    }
}

void UAnim_CharacterMovementBlueprint::SetArchetypeState(EAnim_TribalArchetype NewArchetype)
{
    MovementData.ArchetypeState = NewArchetype;
    
    // Apply archetype-specific animation modifications
    switch (NewArchetype)
    {
        case EAnim_TribalArchetype::Hunter:
            // Hunters move more cautiously, with lower stance
            break;
            
        case EAnim_TribalArchetype::Gatherer:
            // Gatherers have more relaxed, flowing movements
            break;
            
        case EAnim_TribalArchetype::Elder:
            // Elders move slower, with more deliberate steps
            break;
            
        case EAnim_TribalArchetype::Shaman:
            // Shamans have mystical, flowing movements
            break;
            
        case EAnim_TribalArchetype::Warrior:
            // Warriors have aggressive, powerful movements
            break;
            
        case EAnim_TribalArchetype::Scout:
            // Scouts move quickly and efficiently
            break;
    }
    
    if (GEngine)
    {
        FString ArchetypeName;
        switch (NewArchetype)
        {
            case EAnim_TribalArchetype::Hunter: ArchetypeName = TEXT("Hunter"); break;
            case EAnim_TribalArchetype::Gatherer: ArchetypeName = TEXT("Gatherer"); break;
            case EAnim_TribalArchetype::Elder: ArchetypeName = TEXT("Elder"); break;
            case EAnim_TribalArchetype::Shaman: ArchetypeName = TEXT("Shaman"); break;
            case EAnim_TribalArchetype::Warrior: ArchetypeName = TEXT("Warrior"); break;
            case EAnim_TribalArchetype::Scout: ArchetypeName = TEXT("Scout"); break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("Character Archetype set to: %s"), *ArchetypeName));
    }
}