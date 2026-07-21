#include "Anim_IKSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UAnim_IKSystem::UAnim_IKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize IK settings
    bEnableFootIK = true;
    bEnableHandIK = true;
    bEnableLookAtIK = true;
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    
    // Initialize component references
    OwnerMeshComponent = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_IKSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeIKSystem();
}

void UAnim_IKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMeshComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Update all IK systems
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }
    
    if (bEnableLookAtIK)
    {
        UpdateLookAtIK(DeltaTime);
    }
}

void UAnim_IKSystem::InitializeIKSystem()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMeshComponent = OwnerCharacter->GetMesh();
        
        if (OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("IK System initialized for character: %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("IK System: No skeletal mesh component found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("IK System: Owner is not a Character"));
    }
}

void UAnim_IKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Perform foot traces
    float LeftFootRotation = 0.0f;
    float RightFootRotation = 0.0f;
    
    float LeftFootOffset = PerformFootTrace(FName("foot_l"), LeftFootRotation);
    float RightFootOffset = PerformFootTrace(FName("foot_r"), RightFootRotation);
    
    // Calculate hip offset (lowest foot determines hip position)
    float TargetHipOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    if (TargetHipOffset > 0.0f)
    {
        TargetHipOffset = 0.0f; // Don't raise hips, only lower
    }
    
    // Adjust foot offsets relative to hip
    LeftFootOffset -= TargetHipOffset;
    RightFootOffset -= TargetHipOffset;
    
    // Interpolate to smooth values
    FootIKData.LeftFootOffset = FMath::FInterpTo(FootIKData.LeftFootOffset, LeftFootOffset, DeltaTime, FootIKData.IKInterpSpeed);
    FootIKData.RightFootOffset = FMath::FInterpTo(FootIKData.RightFootOffset, RightFootOffset, DeltaTime, FootIKData.IKInterpSpeed);
    FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, TargetHipOffset, DeltaTime, FootIKData.IKInterpSpeed);
    
    // Update foot rotations
    FRotator TargetLeftRotation = FRotator(0.0f, 0.0f, LeftFootRotation);
    FRotator TargetRightRotation = FRotator(0.0f, 0.0f, RightFootRotation);
    
    FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, TargetLeftRotation, DeltaTime, FootIKData.IKInterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, TargetRightRotation, DeltaTime, FootIKData.IKInterpSpeed);
}

float UAnim_IKSystem::PerformFootTrace(FName SocketName, float& OutFootRotation)
{
    if (!OwnerMeshComponent || !OwnerCharacter)
    {
        OutFootRotation = 0.0f;
        return 0.0f;
    }
    
    // Get foot socket location
    FVector SocketLocation = GetSocketWorldLocation(SocketName);
    if (SocketLocation.IsZero())
    {
        OutFootRotation = 0.0f;
        return 0.0f;
    }
    
    // Setup trace parameters
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate foot offset
        float FootOffset = (HitResult.Location.Z - SocketLocation.Z);
        
        // Calculate foot rotation from surface normal
        FRotator SurfaceRotation = CalculateFootRotationFromNormal(HitResult.Normal);
        OutFootRotation = SurfaceRotation.Roll;
        
        return FootOffset;
    }
    
    OutFootRotation = 0.0f;
    return 0.0f;
}

void UAnim_IKSystem::UpdateHandIK(float DeltaTime)
{
    // Interpolate hand IK targets
    if (LeftHandTarget.bIsActive)
    {
        LeftHandTarget.IKAlpha = FMath::FInterpTo(LeftHandTarget.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        LeftHandTarget.IKAlpha = FMath::FInterpTo(LeftHandTarget.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
    
    if (RightHandTarget.bIsActive)
    {
        RightHandTarget.IKAlpha = FMath::FInterpTo(RightHandTarget.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        RightHandTarget.IKAlpha = FMath::FInterpTo(RightHandTarget.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
}

void UAnim_IKSystem::UpdateLookAtIK(float DeltaTime)
{
    if (LookAtTarget.bIsActive)
    {
        LookAtTarget.IKAlpha = FMath::FInterpTo(LookAtTarget.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        LookAtTarget.IKAlpha = FMath::FInterpTo(LookAtTarget.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
}

FVector UAnim_IKSystem::GetSocketWorldLocation(FName SocketName) const
{
    if (!OwnerMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    if (OwnerMeshComponent->DoesSocketExist(SocketName))
    {
        return OwnerMeshComponent->GetSocketLocation(SocketName);
    }
    
    return FVector::ZeroVector;
}

FRotator UAnim_IKSystem::CalculateFootRotationFromNormal(FVector SurfaceNormal) const
{
    // Calculate rotation to align foot with surface
    FVector ForwardVector = OwnerCharacter ? OwnerCharacter->GetActorForwardVector() : FVector::ForwardVector;
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
}

void UAnim_IKSystem::SetFootIKEnabled(bool bEnabled)
{
    bEnableFootIK = bEnabled;
}

void UAnim_IKSystem::SetLeftHandIKTarget(FVector Location, FRotator Rotation, float Alpha)
{
    LeftHandTarget.TargetLocation = Location;
    LeftHandTarget.TargetRotation = Rotation;
    LeftHandTarget.IKAlpha = Alpha;
    LeftHandTarget.bIsActive = true;
}

void UAnim_IKSystem::SetRightHandIKTarget(FVector Location, FRotator Rotation, float Alpha)
{
    RightHandTarget.TargetLocation = Location;
    RightHandTarget.TargetRotation = Rotation;
    RightHandTarget.IKAlpha = Alpha;
    RightHandTarget.bIsActive = true;
}

void UAnim_IKSystem::ClearHandIKTargets()
{
    LeftHandTarget.bIsActive = false;
    RightHandTarget.bIsActive = false;
}

void UAnim_IKSystem::SetLookAtTarget(FVector Location, float Alpha)
{
    LookAtTarget.TargetLocation = Location;
    LookAtTarget.IKAlpha = Alpha;
    LookAtTarget.bIsActive = true;
}

void UAnim_IKSystem::ClearLookAtTarget()
{
    LookAtTarget.bIsActive = false;
}

void UAnim_IKSystem::ResetAllIK()
{
    // Reset foot IK
    FootIKData = FAnim_FootIKData();
    
    // Clear hand IK
    ClearHandIKTargets();
    
    // Clear look at IK
    ClearLookAtTarget();
}