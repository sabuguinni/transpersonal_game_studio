#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bIKEnabled = true;
    TraceDistance = 100.0f;
    IKInterpSpeed = 10.0f;
    FootOffsetThreshold = 5.0f;
    LeftFootSocketName = TEXT("foot_l");
    RightFootSocketName = TEXT("foot_r");
    bShowDebugTraces = false;
    
    OwnerCharacter = nullptr;
    SkeletalMeshComponent = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIKEnabled)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }

    // Update left foot IK
    LeftFootIKData = CalculateFootIK(LeftFootSocketName, LeftFootIKData, DeltaTime);
    
    // Update right foot IK
    RightFootIKData = CalculateFootIK(RightFootSocketName, RightFootIKData, DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& SocketName, const FAnim_FootIKData& CurrentData, float DeltaTime)
{
    FAnim_FootIKData NewData = CurrentData;
    
    if (!SkeletalMeshComponent->DoesSocketExist(SocketName))
    {
        return NewData;
    }

    // Get foot socket world location
    FVector FootSocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
    
    // Trace downward from foot socket
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(FootSocketLocation, HitLocation, HitNormal);
    
    if (bHitGround)
    {
        // Calculate foot offset
        float FootOffset = FootSocketLocation.Z - HitLocation.Z;
        
        if (FMath::Abs(FootOffset) > FootOffsetThreshold)
        {
            // Interpolate to new foot position
            FVector TargetFootLocation = FVector(FootSocketLocation.X, FootSocketLocation.Y, HitLocation.Z);
            NewData.FootLocation = FMath::VInterpTo(CurrentData.FootLocation, TargetFootLocation, DeltaTime, IKInterpSpeed);
            
            // Calculate foot rotation based on ground normal
            FRotator TargetFootRotation = CalculateFootRotationFromNormal(HitNormal);
            NewData.FootRotation = FMath::RInterpTo(CurrentData.FootRotation, TargetFootRotation, DeltaTime, IKInterpSpeed);
            
            // Set IK alpha based on offset magnitude
            float MaxOffset = TraceDistance * 0.5f;
            NewData.IKAlpha = FMath::Clamp(FMath::Abs(FootOffset) / MaxOffset, 0.0f, 1.0f);
            NewData.bIsGrounded = true;
        }
        else
        {
            // Foot is close to ground, reduce IK influence
            NewData.IKAlpha = FMath::FInterpTo(CurrentData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
            NewData.bIsGrounded = true;
        }
    }
    else
    {
        // No ground hit, disable IK
        NewData.IKAlpha = FMath::FInterpTo(CurrentData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed * 2.0f);
        NewData.bIsGrounded = false;
    }
    
    return NewData;
}

bool UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TraceStart = StartLocation + FVector(0, 0, TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bShowDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotationFromNormal(const FVector& GroundNormal) const
{
    // Calculate foot rotation to align with ground normal
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, GroundNormal);
}