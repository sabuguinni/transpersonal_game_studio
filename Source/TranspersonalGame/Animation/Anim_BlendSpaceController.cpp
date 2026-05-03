#include "Anim_BlendSpaceController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_BlendSpaceController::UAnim_BlendSpaceController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    ActiveBlendSpaceIndex = -1;
    SmoothingSpeed = 5.0f;
    ParameterSmoothingTime = 0.1f;
    PreviousParameters = FVector2D::ZeroVector;
    TargetParameters = FVector2D::ZeroVector;
}

void UAnim_BlendSpaceController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
    InitializeDefaultBlendSpaces();
    
    // Set locomotion as default active blend space
    SetActiveBlendSpace(EAnim_BlendSpaceType::Locomotion);
}

void UAnim_BlendSpaceController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update blend space parameters based on current state
    if (ActiveBlendSpaceIndex >= 0 && ActiveBlendSpaceIndex < BlendSpaces.Num())
    {
        FAnim_BlendSpaceData& ActiveBlendSpace = BlendSpaces[ActiveBlendSpaceIndex];
        
        switch (ActiveBlendSpace.BlendSpaceType)
        {
            case EAnim_BlendSpaceType::Locomotion:
                CalculateLocomotionParameters();
                break;
                
            case EAnim_BlendSpaceType::Combat:
                CalculateCombatParameters();
                break;
                
            default:
                break;
        }
    }
    
    // Smooth parameter transitions
    SmoothBlendSpaceParameters(DeltaTime);
}

void UAnim_BlendSpaceController::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp)
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
}

void UAnim_BlendSpaceController::InitializeDefaultBlendSpaces()
{
    // Initialize with default blend space entries
    BlendSpaces.Empty();
    
    // Add locomotion blend space
    FAnim_BlendSpaceData LocomotionData;
    LocomotionData.BlendSpaceType = EAnim_BlendSpaceType::Locomotion;
    LocomotionData.bIsActive = true;
    BlendSpaces.Add(LocomotionData);
    
    // Add combat blend space
    FAnim_BlendSpaceData CombatData;
    CombatData.BlendSpaceType = EAnim_BlendSpaceType::Combat;
    CombatData.bIsActive = false;
    BlendSpaces.Add(CombatData);
    
    // Add emotional state blend spaces
    FAnim_BlendSpaceData InjuredData;
    InjuredData.BlendSpaceType = EAnim_BlendSpaceType::Injured;
    InjuredData.bIsActive = false;
    BlendSpaces.Add(InjuredData);
    
    FAnim_BlendSpaceData ExhaustedData;
    ExhaustedData.BlendSpaceType = EAnim_BlendSpaceType::Exhausted;
    ExhaustedData.bIsActive = false;
    BlendSpaces.Add(ExhaustedData);
    
    FAnim_BlendSpaceData AfraidData;
    AfraidData.BlendSpaceType = EAnim_BlendSpaceType::Afraid;
    AfraidData.bIsActive = false;
    BlendSpaces.Add(AfraidData);
}

void UAnim_BlendSpaceController::SetActiveBlendSpace(EAnim_BlendSpaceType BlendSpaceType)
{
    int32 OldIndex = ActiveBlendSpaceIndex;
    int32 NewIndex = FindBlendSpaceIndex(BlendSpaceType);
    
    if (NewIndex != -1 && NewIndex != ActiveBlendSpaceIndex)
    {
        // Deactivate old blend space
        if (ActiveBlendSpaceIndex >= 0 && ActiveBlendSpaceIndex < BlendSpaces.Num())
        {
            BlendSpaces[ActiveBlendSpaceIndex].bIsActive = false;
        }
        
        // Activate new blend space
        ActiveBlendSpaceIndex = NewIndex;
        BlendSpaces[ActiveBlendSpaceIndex].bIsActive = true;
        
        // Trigger blueprint event
        EAnim_BlendSpaceType OldType = (OldIndex >= 0 && OldIndex < BlendSpaces.Num()) ? 
            BlendSpaces[OldIndex].BlendSpaceType : EAnim_BlendSpaceType::Locomotion;
        OnBlendSpaceChanged(OldType, BlendSpaceType);
        
        UE_LOG(LogTemp, Log, TEXT("BlendSpaceController: Changed active blend space to %s"), 
            *UEnum::GetValueAsString(BlendSpaceType));
    }
}

void UAnim_BlendSpaceController::UpdateBlendSpaceParameters(float XValue, float YValue)
{
    if (ActiveBlendSpaceIndex >= 0 && ActiveBlendSpaceIndex < BlendSpaces.Num())
    {
        TargetParameters = FVector2D(XValue, YValue);
        OnBlendSpaceParametersUpdated(XValue, YValue);
    }
}

void UAnim_BlendSpaceController::SetBlendSpaceWeight(EAnim_BlendSpaceType BlendSpaceType, float Weight)
{
    int32 Index = FindBlendSpaceIndex(BlendSpaceType);
    if (Index != -1)
    {
        BlendSpaces[Index].BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    }
}

FAnim_BlendSpaceData UAnim_BlendSpaceController::GetActiveBlendSpaceData() const
{
    if (ActiveBlendSpaceIndex >= 0 && ActiveBlendSpaceIndex < BlendSpaces.Num())
    {
        return BlendSpaces[ActiveBlendSpaceIndex];
    }
    
    return FAnim_BlendSpaceData();
}

bool UAnim_BlendSpaceController::IsBlendSpaceActive(EAnim_BlendSpaceType BlendSpaceType) const
{
    int32 Index = FindBlendSpaceIndex(BlendSpaceType);
    return Index != -1 && BlendSpaces[Index].bIsActive;
}

void UAnim_BlendSpaceController::AddBlendSpace(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace* BlendSpaceAsset)
{
    int32 Index = FindBlendSpaceIndex(BlendSpaceType);
    if (Index != -1)
    {
        BlendSpaces[Index].BlendSpaceAsset = BlendSpaceAsset;
    }
    else
    {
        FAnim_BlendSpaceData NewData;
        NewData.BlendSpaceType = BlendSpaceType;
        NewData.BlendSpaceAsset = BlendSpaceAsset;
        BlendSpaces.Add(NewData);
    }
}

void UAnim_BlendSpaceController::AddBlendSpace1D(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace1D* BlendSpace1DAsset)
{
    int32 Index = FindBlendSpaceIndex(BlendSpaceType);
    if (Index != -1)
    {
        BlendSpaces[Index].BlendSpace1DAsset = BlendSpace1DAsset;
    }
    else
    {
        FAnim_BlendSpaceData NewData;
        NewData.BlendSpaceType = BlendSpaceType;
        NewData.BlendSpace1DAsset = BlendSpace1DAsset;
        BlendSpaces.Add(NewData);
    }
}

void UAnim_BlendSpaceController::RemoveBlendSpace(EAnim_BlendSpaceType BlendSpaceType)
{
    int32 Index = FindBlendSpaceIndex(BlendSpaceType);
    if (Index != -1)
    {
        if (Index == ActiveBlendSpaceIndex)
        {
            ActiveBlendSpaceIndex = -1;
        }
        BlendSpaces.RemoveAt(Index);
    }
}

void UAnim_BlendSpaceController::CalculateLocomotionParameters()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FVector2D MovementDirection = CalculateDirectionalMovement();
    float MovementSpeed = CalculateMovementSpeed();
    
    // X-axis: Forward/Backward movement (-1 to 1)
    // Y-axis: Left/Right movement (-1 to 1) 
    float XValue = MovementDirection.X * MovementSpeed;
    float YValue = MovementDirection.Y * MovementSpeed;
    
    UpdateBlendSpaceParameters(XValue, YValue);
}

void UAnim_BlendSpaceController::CalculateCombatParameters()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Combat blend space parameters could be based on:
    // X-axis: Attack intensity (0 to 1)
    // Y-axis: Defense stance (-1 to 1)
    
    // For now, use basic movement but with different scaling
    FVector2D MovementDirection = CalculateDirectionalMovement();
    float MovementSpeed = CalculateMovementSpeed() * 0.5f; // Slower in combat
    
    float XValue = MovementSpeed; // Attack readiness
    float YValue = MovementDirection.Y * 0.7f; // Defensive positioning
    
    UpdateBlendSpaceParameters(XValue, YValue);
}

void UAnim_BlendSpaceController::CalculateEmotionalParameters(float FearLevel, float HealthPercentage, float StaminaPercentage)
{
    // Emotional state affects movement parameters
    // X-axis: Urgency (based on fear and health)
    // Y-axis: Stability (based on stamina and health)
    
    float Urgency = FMath::Clamp(FearLevel + (1.0f - HealthPercentage), 0.0f, 1.0f);
    float Stability = FMath::Clamp((HealthPercentage + StaminaPercentage) * 0.5f, 0.0f, 1.0f);
    
    UpdateBlendSpaceParameters(Urgency, Stability);
}

int32 UAnim_BlendSpaceController::FindBlendSpaceIndex(EAnim_BlendSpaceType BlendSpaceType) const
{
    for (int32 i = 0; i < BlendSpaces.Num(); i++)
    {
        if (BlendSpaces[i].BlendSpaceType == BlendSpaceType)
        {
            return i;
        }
    }
    return -1;
}

void UAnim_BlendSpaceController::SmoothBlendSpaceParameters(float DeltaTime)
{
    if (ActiveBlendSpaceIndex >= 0 && ActiveBlendSpaceIndex < BlendSpaces.Num())
    {
        FAnim_BlendSpaceData& ActiveBlendSpace = BlendSpaces[ActiveBlendSpaceIndex];
        
        // Smooth interpolation towards target parameters
        PreviousParameters = FMath::Vector2DInterpTo(
            PreviousParameters, 
            TargetParameters, 
            DeltaTime, 
            SmoothingSpeed
        );
        
        // Update the blend space data
        ActiveBlendSpace.XAxisValue = PreviousParameters.X;
        ActiveBlendSpace.YAxisValue = PreviousParameters.Y;
    }
}

FVector2D UAnim_BlendSpaceController::CalculateDirectionalMovement() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return FVector2D::ZeroVector;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Project velocity onto character's local axes
    float ForwardSpeed = FVector::DotProduct(Velocity, ForwardVector);
    float RightSpeed = FVector::DotProduct(Velocity, RightVector);
    
    // Normalize to -1 to 1 range
    float MaxSpeed = MovementComponent->MaxWalkSpeed;
    if (MaxSpeed > 0.0f)
    {
        ForwardSpeed /= MaxSpeed;
        RightSpeed /= MaxSpeed;
    }
    
    return FVector2D(
        FMath::Clamp(ForwardSpeed, -1.0f, 1.0f),
        FMath::Clamp(RightSpeed, -1.0f, 1.0f)
    );
}

float UAnim_BlendSpaceController::CalculateMovementSpeed() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    float CurrentSpeed = MovementComponent->Velocity.Size2D();
    float MaxSpeed = MovementComponent->MaxWalkSpeed;
    
    if (MaxSpeed > 0.0f)
    {
        return FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
    }
    
    return 0.0f;
}