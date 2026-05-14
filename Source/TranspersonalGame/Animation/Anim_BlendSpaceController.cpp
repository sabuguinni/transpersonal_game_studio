#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    LeanAngle = 0.0f;
    CombatStance = 0.0f;
    WeaponType = 0.0f;
    BlendSpaceUpdateRate = 30.0f; // 30 updates per second
    LastUpdateTime = 0.0f;
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultBlendSpaces();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= (1.0f / BlendSpaceUpdateRate))
    {
        // Update movement blend space based on character movement
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                FVector Velocity = MovementComp->Velocity;
                float Speed = Velocity.Size();
                float MaxWalkSpeed = MovementComp->MaxWalkSpeed;
                
                // Normalize speed (0-1 range)
                MovementSpeed = FMath::Clamp(Speed / MaxWalkSpeed, 0.0f, 1.0f);
                
                // Calculate movement direction relative to character forward
                if (Speed > 10.0f) // Only calculate direction if moving
                {
                    FVector ForwardVector = Character->GetActorForwardVector();
                    FVector VelocityNormalized = Velocity.GetSafeNormal();
                    
                    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
                    FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
                    
                    MovementDirection = FMath::Atan2(CrossProduct.Z, DotProduct);
                    MovementDirection = FMath::RadiansToDegrees(MovementDirection);
                }
                else
                {
                    MovementDirection = 0.0f;
                }
                
                // Update locomotion blend space
                UpdateMovementBlendSpace(MovementSpeed, MovementDirection);
            }
        }
        
        UpdateBlendSpaceWeights(DeltaTime);
        LastUpdateTime = CurrentTime;
    }
}

void UAnim_BlendSpaceController::SetBlendSpaceValues(EAnim_BlendSpaceType BlendSpaceType, float XValue, float YValue, float Weight)
{
    if (FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        BlendSpaceData->XAxisValue = XValue;
        BlendSpaceData->YAxisValue = YValue;
        BlendSpaceData->BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    }
}

void UAnim_BlendSpaceController::ActivateBlendSpace(EAnim_BlendSpaceType BlendSpaceType, bool bActivate)
{
    if (FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        BlendSpaceData->bIsActive = bActivate;
    }
}

void UAnim_BlendSpaceController::UpdateMovementBlendSpace(float Speed, float Direction)
{
    // Map speed to X-axis (0 = idle, 0.5 = walk, 1.0 = run)
    float BlendSpaceSpeed = Speed;
    
    // Map direction to Y-axis (-180 to 180 degrees)
    float BlendSpaceDirection = FMath::Clamp(Direction, -180.0f, 180.0f);
    
    SetBlendSpaceValues(EAnim_BlendSpaceType::Locomotion, BlendSpaceSpeed, BlendSpaceDirection, 1.0f);
    ActivateBlendSpace(EAnim_BlendSpaceType::Locomotion, Speed > 0.01f);
}

void UAnim_BlendSpaceController::UpdateCombatBlendSpace(float Stance, float Weapon)
{
    CombatStance = FMath::Clamp(Stance, 0.0f, 1.0f);
    WeaponType = FMath::Clamp(Weapon, 0.0f, 1.0f);
    
    SetBlendSpaceValues(EAnim_BlendSpaceType::Combat, CombatStance, WeaponType, 1.0f);
}

FAnim_BlendSpaceData UAnim_BlendSpaceController::GetBlendSpaceData(EAnim_BlendSpaceType BlendSpaceType) const
{
    if (const FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        return *BlendSpaceData;
    }
    
    return FAnim_BlendSpaceData();
}

float UAnim_BlendSpaceController::GetNormalizedMovementSpeed() const
{
    return MovementSpeed;
}

float UAnim_BlendSpaceController::GetMovementDirection() const
{
    return MovementDirection;
}

void UAnim_BlendSpaceController::InitializeDefaultBlendSpaces()
{
    // Initialize locomotion blend space data
    FAnim_BlendSpaceData LocomotionData;
    LocomotionData.XAxisValue = 0.0f;
    LocomotionData.YAxisValue = 0.0f;
    LocomotionData.BlendWeight = 1.0f;
    LocomotionData.bIsActive = true;
    BlendSpaces.Add(EAnim_BlendSpaceType::Locomotion, LocomotionData);
    
    // Initialize combat blend space data
    FAnim_BlendSpaceData CombatData;
    CombatData.XAxisValue = 0.0f;
    CombatData.YAxisValue = 0.0f;
    CombatData.BlendWeight = 0.0f;
    CombatData.bIsActive = false;
    BlendSpaces.Add(EAnim_BlendSpaceType::Combat, CombatData);
    
    // Initialize interaction blend space data
    FAnim_BlendSpaceData InteractionData;
    InteractionData.XAxisValue = 0.0f;
    InteractionData.YAxisValue = 0.0f;
    InteractionData.BlendWeight = 0.0f;
    InteractionData.bIsActive = false;
    BlendSpaces.Add(EAnim_BlendSpaceType::Interaction, InteractionData);
    
    // Initialize emotion blend space data
    FAnim_BlendSpaceData EmotionData;
    EmotionData.XAxisValue = 0.0f;
    EmotionData.YAxisValue = 0.0f;
    EmotionData.BlendWeight = 0.0f;
    EmotionData.bIsActive = false;
    BlendSpaces.Add(EAnim_BlendSpaceType::Emotion, EmotionData);
}

void UAnim_BlendSpaceController::UpdateBlendSpaceWeights(float DeltaTime)
{
    // Smooth blend space weight transitions
    const float BlendSpeed = 5.0f; // Blend speed multiplier
    
    for (auto& BlendSpacePair : BlendSpaces)
    {
        FAnim_BlendSpaceData& BlendSpaceData = BlendSpacePair.Value;
        
        if (BlendSpaceData.bIsActive)
        {
            // Blend in
            BlendSpaceData.BlendWeight = FMath::FInterpTo(
                BlendSpaceData.BlendWeight, 
                1.0f, 
                DeltaTime, 
                BlendSpeed
            );
        }
        else
        {
            // Blend out
            BlendSpaceData.BlendWeight = FMath::FInterpTo(
                BlendSpaceData.BlendWeight, 
                0.0f, 
                DeltaTime, 
                BlendSpeed
            );
        }
    }
}