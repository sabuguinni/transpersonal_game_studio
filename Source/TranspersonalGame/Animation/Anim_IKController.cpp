#include "Anim_IKController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_IKController::UAnim_IKController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    FootIKTraceDistance = 150.0f;
    FootIKInterpSpeed = 15.0f;
    FootIKCapsuleHalfHeight = 88.0f;
    bEnableFootIK = true;
    bEnableHandIK = false;
    bEnableLookAtIK = false;
    
    LookAtTarget = FVector::ZeroVector;
    LookAtWeight = 0.0f;
    LastUpdateTime = 0.0f;
    OwnerMeshComponent = nullptr;
}

void UAnim_IKController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the skeletal mesh component from the owner
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    InitializeIKBones();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_IKController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    UpdateIKWeights(DeltaTime);
}

void UAnim_IKController::SetIKTarget(EAnim_IKBoneType BoneType, FVector TargetLocation, FRotator TargetRotation, float Weight)
{
    if (FAnim_IKBoneData* IKBoneData = IKBones.Find(BoneType))
    {
        IKBoneData->TargetLocation = TargetLocation;
        IKBoneData->TargetRotation = TargetRotation;
        IKBoneData->IKWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
        IKBoneData->bIsActive = Weight > 0.0f;
    }
}

void UAnim_IKController::ActivateIK(EAnim_IKBoneType BoneType, bool bActivate)
{
    if (FAnim_IKBoneData* IKBoneData = IKBones.Find(BoneType))
    {
        IKBoneData->bIsActive = bActivate;
    }
}

void UAnim_IKController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !GetOwner())
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    // Get character capsule component for ground reference
    UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent();
    if (!CapsuleComponent)
    {
        return;
    }

    FVector CapsuleLocation = CapsuleComponent->GetComponentLocation();
    float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

    // Update left foot IK
    if (FAnim_IKBoneData* LeftFootData = IKBones.Find(EAnim_IKBoneType::LeftFoot))
    {
        FVector LeftFootWorldLocation = OwnerMeshComponent->GetSocketLocation(LeftFootData->BoneName);
        FVector LeftTraceStart = FVector(LeftFootWorldLocation.X, LeftFootWorldLocation.Y, CapsuleLocation.Z);
        FVector LeftTraceEnd = LeftTraceStart - FVector(0, 0, FootIKTraceDistance);
        
        FVector LeftHitLocation;
        FVector LeftHitNormal;
        bool bLeftFootHit = TraceForGround(LeftTraceStart, LeftHitLocation, LeftHitNormal);
        
        if (bLeftFootHit)
        {
            float LeftFootOffset = (CapsuleLocation.Z - CapsuleHalfHeight) - LeftHitLocation.Z;
            LeftFootIK.FootLocation = FVector(0, 0, LeftFootOffset);
            LeftFootIK.FootRotation = CalculateFootRotationFromNormal(LeftHitNormal);
            LeftFootIK.IKWeight = FMath::FInterpTo(LeftFootIK.IKWeight, 1.0f, DeltaTime, FootIKInterpSpeed);
            LeftFootIK.bIsGrounded = true;
            LeftFootIK.GroundDistance = FMath::Abs(LeftFootOffset);
        }
        else
        {
            LeftFootIK.IKWeight = FMath::FInterpTo(LeftFootIK.IKWeight, 0.0f, DeltaTime, FootIKInterpSpeed);
            LeftFootIK.bIsGrounded = false;
        }
    }

    // Update right foot IK
    if (FAnim_IKBoneData* RightFootData = IKBones.Find(EAnim_IKBoneType::RightFoot))
    {
        FVector RightFootWorldLocation = OwnerMeshComponent->GetSocketLocation(RightFootData->BoneName);
        FVector RightTraceStart = FVector(RightFootWorldLocation.X, RightFootWorldLocation.Y, CapsuleLocation.Z);
        FVector RightTraceEnd = RightTraceStart - FVector(0, 0, FootIKTraceDistance);
        
        FVector RightHitLocation;
        FVector RightHitNormal;
        bool bRightFootHit = TraceForGround(RightTraceStart, RightHitLocation, RightHitNormal);
        
        if (bRightFootHit)
        {
            float RightFootOffset = (CapsuleLocation.Z - CapsuleHalfHeight) - RightHitLocation.Z;
            RightFootIK.FootLocation = FVector(0, 0, RightFootOffset);
            RightFootIK.FootRotation = CalculateFootRotationFromNormal(RightHitNormal);
            RightFootIK.IKWeight = FMath::FInterpTo(RightFootIK.IKWeight, 1.0f, DeltaTime, FootIKInterpSpeed);
            RightFootIK.bIsGrounded = true;
            RightFootIK.GroundDistance = FMath::Abs(RightFootOffset);
        }
        else
        {
            RightFootIK.IKWeight = FMath::FInterpTo(RightFootIK.IKWeight, 0.0f, DeltaTime, FootIKInterpSpeed);
            RightFootIK.bIsGrounded = false;
        }
    }
}

void UAnim_IKController::SetLookAtTarget(FVector Target, float Weight)
{
    LookAtTarget = Target;
    LookAtWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    bEnableLookAtIK = Weight > 0.0f;
}

FAnim_IKBoneData UAnim_IKController::GetIKBoneData(EAnim_IKBoneType BoneType) const
{
    if (const FAnim_IKBoneData* IKBoneData = IKBones.Find(BoneType))
    {
        return *IKBoneData;
    }
    
    return FAnim_IKBoneData();
}

void UAnim_IKController::InitializeIKBones()
{
    // Initialize left foot IK
    FAnim_IKBoneData LeftFootData;
    LeftFootData.BoneName = FName("foot_l");
    LeftFootData.BlendSpeed = FootIKInterpSpeed;
    IKBones.Add(EAnim_IKBoneType::LeftFoot, LeftFootData);
    
    // Initialize right foot IK
    FAnim_IKBoneData RightFootData;
    RightFootData.BoneName = FName("foot_r");
    RightFootData.BlendSpeed = FootIKInterpSpeed;
    IKBones.Add(EAnim_IKBoneType::RightFoot, RightFootData);
    
    // Initialize left hand IK
    FAnim_IKBoneData LeftHandData;
    LeftHandData.BoneName = FName("hand_l");
    LeftHandData.BlendSpeed = 10.0f;
    IKBones.Add(EAnim_IKBoneType::LeftHand, LeftHandData);
    
    // Initialize right hand IK
    FAnim_IKBoneData RightHandData;
    RightHandData.BoneName = FName("hand_r");
    RightHandData.BlendSpeed = 10.0f;
    IKBones.Add(EAnim_IKBoneType::RightHand, RightHandData);
    
    // Initialize head IK
    FAnim_IKBoneData HeadData;
    HeadData.BoneName = FName("head");
    HeadData.BlendSpeed = 8.0f;
    IKBones.Add(EAnim_IKBoneType::Head, HeadData);
    
    // Initialize spine IK
    FAnim_IKBoneData SpineData;
    SpineData.BoneName = FName("spine_03");
    SpineData.BlendSpeed = 5.0f;
    IKBones.Add(EAnim_IKBoneType::Spine, SpineData);
}

void UAnim_IKController::UpdateIKWeights(float DeltaTime)
{
    for (auto& IKBonePair : IKBones)
    {
        FAnim_IKBoneData& IKBoneData = IKBonePair.Value;
        
        if (IKBoneData.bIsActive)
        {
            // Blend in
            IKBoneData.IKWeight = FMath::FInterpTo(
                IKBoneData.IKWeight, 
                1.0f, 
                DeltaTime, 
                IKBoneData.BlendSpeed
            );
        }
        else
        {
            // Blend out
            IKBoneData.IKWeight = FMath::FInterpTo(
                IKBoneData.IKWeight, 
                0.0f, 
                DeltaTime, 
                IKBoneData.BlendSpeed
            );
        }
    }
    
    // Update look at weight
    if (bEnableLookAtIK)
    {
        LookAtWeight = FMath::FInterpTo(LookAtWeight, 1.0f, DeltaTime, 5.0f);
    }
    else
    {
        LookAtWeight = FMath::FInterpTo(LookAtWeight, 0.0f, DeltaTime, 5.0f);
    }
}

bool UAnim_IKController::TraceForGround(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TraceEnd = StartLocation - FVector(0, 0, FootIKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}

FRotator UAnim_IKController::CalculateFootRotationFromNormal(FVector GroundNormal)
{
    // Calculate foot rotation to align with ground normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
    
    FRotator FootRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, GroundNormal);
    
    // Limit rotation angles to prevent extreme foot bending
    FootRotation.Pitch = FMath::Clamp(FootRotation.Pitch, -45.0f, 45.0f);
    FootRotation.Roll = FMath::Clamp(FootRotation.Roll, -30.0f, 30.0f);
    
    return FootRotation;
}