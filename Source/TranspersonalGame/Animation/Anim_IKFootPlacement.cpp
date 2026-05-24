#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the skeletal mesh component from the owner
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: No SkeletalMeshComponent found on owner"));
        }
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableIK && SkeletalMeshComponent)
    {
        UpdateFootIK();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance())
    {
        return;
    }

    // Get current foot positions
    FVector LeftFootLocation = GetBoneWorldLocation(LeftFootBoneName);
    FVector RightFootLocation = GetBoneWorldLocation(RightFootBoneName);

    if (LeftFootLocation != FVector::ZeroVector && RightFootLocation != FVector::ZeroVector)
    {
        // Calculate IK data for both feet
        LeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootLocation);
        RightFootIK = CalculateFootIK(RightFootBoneName, RightFootLocation);

        // Calculate hip offset to keep character grounded
        HipIKOffset = CalculateHipOffset();

        // Interpolate for smooth transitions
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 
            LeftFootIK.DistanceFromGround > 0.0f ? 1.0f : 0.0f, 
            DeltaTime, IKInterpSpeed);
            
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 
            RightFootIK.DistanceFromGround > 0.0f ? 1.0f : 0.0f, 
            DeltaTime, IKInterpSpeed);

        HipIKOffset = FMath::FInterpTo(PreviousHipOffset, HipIKOffset, DeltaTime, IKInterpSpeed);

        // Store for next frame
        PreviousHipOffset = HipIKOffset;
    }
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, const FVector& FootLocation)
{
    FAnim_FootIKData FootIKData;
    
    // Perform trace from foot position downward
    FVector HitLocation;
    FVector HitNormal;
    
    if (PerformFootTrace(FootLocation, HitLocation, HitNormal))
    {
        // Calculate the distance the foot needs to move
        float DistanceToGround = FootLocation.Z - HitLocation.Z;
        FootIKData.DistanceFromGround = DistanceToGround;
        
        if (DistanceToGround > 0.0f)
        {
            // Foot is above ground, need to lower it
            FootIKData.IKLocation = FVector(0.0f, 0.0f, -DistanceToGround);
            
            // Calculate rotation to align with ground normal
            FVector ForwardVector = SkeletalMeshComponent->GetForwardVector();
            FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
            ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();
            
            FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
            FRotator CurrentRotation = SkeletalMeshComponent->GetComponentRotation();
            
            FootIKData.IKRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
        }
    }
    
    return FootIKData;
}

FVector UAnim_IKFootPlacement::GetBoneWorldLocation(const FName& BoneName)
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex != INDEX_NONE)
    {
        return SkeletalMeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    }
    
    return FVector::ZeroVector;
}

bool UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Set up trace parameters
    FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, 50.0f); // Start slightly above foot
    FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        
        // Debug drawing
        if (bDrawDebugTraces)
        {
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f, 0, 1.0f);
            DrawDebugLine(GetWorld(), HitResult.Location, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Blue, false, 0.1f);
        }
        
        return true;
    }
    else
    {
        // Debug drawing for missed traces
        if (bDrawDebugTraces)
        {
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
        }
        
        return false;
    }
}

float UAnim_IKFootPlacement::CalculateHipOffset()
{
    // Calculate hip offset based on foot IK data
    float LeftOffset = LeftFootIK.IKLocation.Z;
    float RightOffset = RightFootIK.IKLocation.Z;
    
    // Use the higher foot as reference to keep character grounded
    float MaxOffset = FMath::Max(LeftOffset, RightOffset);
    
    // Only apply positive offset (lowering the character)
    return FMath::Min(MaxOffset, 0.0f);
}