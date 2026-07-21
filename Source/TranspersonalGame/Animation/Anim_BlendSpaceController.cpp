#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    BlendSpeedMultiplier = 1.0f;
    InputSmoothingSpeed = 5.0f;
    CurrentBlendSpaceType = EAnim_BlendSpaceType::Locomotion;
    SmoothedInput = FVector2D::ZeroVector;
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default blend spaces
    BlendSpaces.Add(EAnim_BlendSpaceType::Locomotion, FAnim_BlendSpaceData());
    BlendSpaces.Add(EAnim_BlendSpaceType::Combat, FAnim_BlendSpaceData());
    BlendSpaces.Add(EAnim_BlendSpaceType::Interaction, FAnim_BlendSpaceData());
    BlendSpaces.Add(EAnim_BlendSpaceType::Emotional, FAnim_BlendSpaceData());
    
    ValidateBlendSpaces();
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    SmoothInputTransition(DeltaTime);
    UpdateBlendWeights(DeltaTime);
    
    // Auto-update locomotion based on character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            float Speed = Velocity.Size();
            float Direction = 0.0f;
            
            if (Speed > 0.1f)
            {
                FVector Forward = Character->GetActorForwardVector();
                Direction = FMath::Atan2(FVector::CrossProduct(Forward, Velocity.GetSafeNormal()).Z, 
                                       FVector::DotProduct(Forward, Velocity.GetSafeNormal()));
                Direction = FMath::RadiansToDegrees(Direction);
            }
            
            UpdateLocomotionBlendSpace(Speed, Direction);
        }
    }
}

void UAnim_BlendSpaceController::SetBlendSpaceInput(EAnim_BlendSpaceType BlendSpaceType, const FVector2D& Input)
{
    if (FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        BlendSpaceData->CurrentInput = Input;
        BlendSpaceData->bIsActive = true;
    }
}

void UAnim_BlendSpaceController::SetBlendSpace1DInput(EAnim_BlendSpaceType BlendSpaceType, float Input)
{
    SetBlendSpaceInput(BlendSpaceType, FVector2D(Input, 0.0f));
}

void UAnim_BlendSpaceController::SwitchToBlendSpace(EAnim_BlendSpaceType NewBlendSpaceType)
{
    if (CurrentBlendSpaceType != NewBlendSpaceType)
    {
        // Deactivate current blend space
        if (FAnim_BlendSpaceData* CurrentData = BlendSpaces.Find(CurrentBlendSpaceType))
        {
            CurrentData->bIsActive = false;
            CurrentData->BlendWeight = 0.0f;
        }
        
        // Activate new blend space
        CurrentBlendSpaceType = NewBlendSpaceType;
        if (FAnim_BlendSpaceData* NewData = BlendSpaces.Find(NewBlendSpaceType))
        {
            NewData->bIsActive = true;
            NewData->BlendWeight = 1.0f;
        }
    }
}

void UAnim_BlendSpaceController::RegisterBlendSpace2D(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace* BlendSpace)
{
    if (BlendSpace)
    {
        if (FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
        {
            BlendSpaceData->BlendSpace2D = BlendSpace;
        }
        else
        {
            FAnim_BlendSpaceData NewData;
            NewData.BlendSpace2D = BlendSpace;
            BlendSpaces.Add(BlendSpaceType, NewData);
        }
    }
}

void UAnim_BlendSpaceController::RegisterBlendSpace1D(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace1D* BlendSpace)
{
    if (BlendSpace)
    {
        if (FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
        {
            BlendSpaceData->BlendSpace1D = BlendSpace;
        }
        else
        {
            FAnim_BlendSpaceData NewData;
            NewData.BlendSpace1D = BlendSpace;
            BlendSpaces.Add(BlendSpaceType, NewData);
        }
    }
}

FVector2D UAnim_BlendSpaceController::GetCurrentBlendSpaceInput(EAnim_BlendSpaceType BlendSpaceType) const
{
    if (const FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        return BlendSpaceData->CurrentInput;
    }
    return FVector2D::ZeroVector;
}

float UAnim_BlendSpaceController::GetBlendSpaceWeight(EAnim_BlendSpaceType BlendSpaceType) const
{
    if (const FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        return BlendSpaceData->BlendWeight;
    }
    return 0.0f;
}

bool UAnim_BlendSpaceController::IsBlendSpaceActive(EAnim_BlendSpaceType BlendSpaceType) const
{
    if (const FAnim_BlendSpaceData* BlendSpaceData = BlendSpaces.Find(BlendSpaceType))
    {
        return BlendSpaceData->bIsActive;
    }
    return false;
}

void UAnim_BlendSpaceController::UpdateLocomotionBlendSpace(float Speed, float Direction)
{
    // Map speed to blend space input (0-600 units/sec to 0-1)
    float NormalizedSpeed = FMath::Clamp(Speed / 600.0f, 0.0f, 1.0f);
    
    // Map direction to blend space input (-180 to 180 degrees to -1 to 1)
    float NormalizedDirection = FMath::Clamp(Direction / 180.0f, -1.0f, 1.0f);
    
    SetBlendSpaceInput(EAnim_BlendSpaceType::Locomotion, FVector2D(NormalizedSpeed, NormalizedDirection));
}

void UAnim_BlendSpaceController::UpdateCombatBlendSpace(float AttackIntensity, float DefenseStance)
{
    float ClampedIntensity = FMath::Clamp(AttackIntensity, 0.0f, 1.0f);
    float ClampedStance = FMath::Clamp(DefenseStance, 0.0f, 1.0f);
    
    SetBlendSpaceInput(EAnim_BlendSpaceType::Combat, FVector2D(ClampedIntensity, ClampedStance));
}

void UAnim_BlendSpaceController::SmoothInputTransition(float DeltaTime)
{
    if (FAnim_BlendSpaceData* CurrentData = BlendSpaces.Find(CurrentBlendSpaceType))
    {
        SmoothedInput = FMath::Vector2DInterpTo(SmoothedInput, CurrentData->CurrentInput, 
                                              DeltaTime, InputSmoothingSpeed);
        CurrentData->CurrentInput = SmoothedInput;
    }
}

void UAnim_BlendSpaceController::UpdateBlendWeights(float DeltaTime)
{
    for (auto& BlendSpacePair : BlendSpaces)
    {
        FAnim_BlendSpaceData& BlendSpaceData = BlendSpacePair.Value;
        
        float TargetWeight = BlendSpaceData.bIsActive ? 1.0f : 0.0f;
        BlendSpaceData.BlendWeight = FMath::FInterpTo(BlendSpaceData.BlendWeight, TargetWeight, 
                                                    DeltaTime, BlendSpeedMultiplier * 3.0f);
    }
}

void UAnim_BlendSpaceController::ValidateBlendSpaces()
{
    for (auto& BlendSpacePair : BlendSpaces)
    {
        const EAnim_BlendSpaceType BlendSpaceType = BlendSpacePair.Key;
        FAnim_BlendSpaceData& BlendSpaceData = BlendSpacePair.Value;
        
        if (!BlendSpaceData.BlendSpace2D && !BlendSpaceData.BlendSpace1D)
        {
            UE_LOG(LogTemp, Warning, TEXT("BlendSpace not assigned for type: %d"), (int32)BlendSpaceType);
        }
    }
}