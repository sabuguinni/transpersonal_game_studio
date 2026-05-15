#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Default settings
    bEnableFootIK = true;
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    FootOffset = 5.0f;
    bShowDebugTraces = false;

    // Default bone names (UE5 mannequin)
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    LeftFootIKBoneName = FName("ik_foot_l");
    RightFootIKBoneName = FName("ik_foot_r");

    HipOffset = 0.0f;
    OwnerMeshComponent = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();

    // Get the skeletal mesh component from the owner
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMeshComponent)
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                OwnerMeshComponent = Character->GetMesh();
            }
        }

        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: No SkeletalMeshComponent found on owner %s"), *Owner->GetName());
        }
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableFootIK && OwnerMeshComponent)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetAnimInstance())
    {
        return;
    }

    // Trace for both feet
    FAnim_FootIKData LeftFootTarget = TraceForFoot(LeftFootBoneName);
    FAnim_FootIKData RightFootTarget = TraceForFoot(RightFootBoneName);

    // Interpolate to smooth IK data
    InterpolateIKData(LeftFootIK, LeftFootTarget, DeltaTime);
    InterpolateIKData(RightFootIK, RightFootTarget, DeltaTime);

    // Calculate hip offset to keep character grounded
    HipOffset = FMath::FInterpTo(HipOffset, CalculateHipOffset(), DeltaTime, IKInterpSpeed);
}

FAnim_FootIKData UAnim_IKFootPlacement::TraceForFoot(const FName& FootBoneName, float TraceDistanceOverride)
{
    FAnim_FootIKData FootData;

    if (!OwnerMeshComponent)
    {
        return FootData;
    }

    // Get foot world location
    FVector FootWorldLocation = GetFootWorldLocation(FootBoneName);
    if (FootWorldLocation == FVector::ZeroVector)
    {
        return FootData;
    }

    // Setup trace parameters
    float UseTraceDistance = TraceDistanceOverride > 0 ? TraceDistanceOverride : TraceDistance;
    FVector TraceStart = FootWorldLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootWorldLocation - FVector(0, 0, UseTraceDistance);

    // Perform line trace
    FHitResult HitResult = PerformFootTrace(TraceStart, TraceEnd);

    if (HitResult.bBlockingHit)
    {
        // Calculate IK data
        FootData.FootLocation = HitResult.ImpactPoint + FVector(0, 0, FootOffset);
        FootData.FootRotation = UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal);
        FootData.DistanceFromGround = FVector::Dist(FootWorldLocation, HitResult.ImpactPoint);
        FootData.IKAlpha = FMath::Clamp((UseTraceDistance - FootData.DistanceFromGround) / UseTraceDistance, 0.0f, 1.0f);
    }
    else
    {
        // No ground hit - disable IK for this foot
        FootData.FootLocation = FootWorldLocation;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.DistanceFromGround = UseTraceDistance;
        FootData.IKAlpha = 0.0f;
    }

    return FootData;
}

FHitResult UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, const FVector& EndLocation)
{
    FHitResult HitResult;

    if (!GetWorld())
    {
        return HitResult;
    }

    // Setup trace parameters
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = false;
    TraceParams.bReturnPhysicalMaterial = true;
    TraceParams.AddIgnoredActor(GetOwner());

    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        TraceParams
    );

    // Debug visualization
    if (bShowDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }

    return HitResult;
}

float UAnim_IKFootPlacement::CalculateHipOffset()
{
    // Calculate the lowest foot position to adjust hip height
    float LeftFootDistance = LeftFootIK.DistanceFromGround;
    float RightFootDistance = RightFootIK.DistanceFromGround;

    float MinDistance = FMath::Min(LeftFootDistance, RightFootDistance);
    
    // Only apply offset if foot is significantly below ground level
    if (MinDistance < TraceDistance * 0.5f)
    {
        return -(TraceDistance * 0.5f - MinDistance);
    }

    return 0.0f;
}

void UAnim_IKFootPlacement::InterpolateIKData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime)
{
    CurrentData.FootLocation = FMath::VInterpTo(CurrentData.FootLocation, TargetData.FootLocation, DeltaTime, IKInterpSpeed);
    CurrentData.FootRotation = FMath::RInterpTo(CurrentData.FootRotation, TargetData.FootRotation, DeltaTime, IKInterpSpeed);
    CurrentData.IKAlpha = FMath::FInterpTo(CurrentData.IKAlpha, TargetData.IKAlpha, DeltaTime, IKInterpSpeed);
    CurrentData.DistanceFromGround = FMath::FInterpTo(CurrentData.DistanceFromGround, TargetData.DistanceFromGround, DeltaTime, IKInterpSpeed);
}

void UAnim_IKFootPlacement::SetFootIKEnabled(bool bEnabled)
{
    bEnableFootIK = bEnabled;
    
    if (!bEnabled)
    {
        ResetIKData();
    }
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& FootBoneName)
{
    if (!OwnerMeshComponent || FootBoneName == NAME_None)
    {
        return FVector::ZeroVector;
    }

    // Check if bone exists
    int32 BoneIndex = OwnerMeshComponent->GetBoneIndex(FootBoneName);
    if (BoneIndex == INDEX_NONE)
    {
        return FVector::ZeroVector;
    }

    // Get bone world transform
    FTransform BoneTransform = OwnerMeshComponent->GetBoneTransform(BoneIndex, FTransform::Identity);
    return BoneTransform.GetLocation();
}

void UAnim_IKFootPlacement::ResetIKData()
{
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    HipOffset = 0.0f;
}