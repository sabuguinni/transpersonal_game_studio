#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default values
    bIKEnabled = true;
    bDebugDraw = false;
    CurrentHipOffset = 0.0f;
    TargetHipOffset = 0.0f;
    PreviousHipOffset = 0.0f;

    // Default socket names
    LeftFootSocketName = TEXT("foot_l");
    RightFootSocketName = TEXT("foot_r");

    // Initialize IK data
    LeftFootIKData = FAnim_FootIKData();
    RightFootIKData = FAnim_FootIKData();

    // Initialize target locations
    LeftFootTargetLocation = FVector::ZeroVector;
    RightFootTargetLocation = FVector::ZeroVector;
    LeftFootTargetRotation = FRotator::ZeroRotator;
    RightFootTargetRotation = FRotator::ZeroRotator;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_IKFootPlacement::InitializeComponent()
{
    // Get component references
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            MovementComponent = Character->GetCharacterMovement();
            if (!SkeletalMeshComponent)
            {
                SkeletalMeshComponent = Character->GetMesh();
            }
        }
    }

    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: No SkeletalMeshComponent found on owner"));
        bIKEnabled = false;
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIKEnabled && SkeletalMeshComponent)
    {
        UpdateFootIK(DeltaTime);
        UpdateHipOffset(DeltaTime);

        if (bDebugDraw)
        {
            DrawDebugInfo();
        }
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent || !GetOwner())
    {
        return;
    }

    FVector RootLocation = GetOwner()->GetActorLocation();

    // Calculate IK for both feet
    LeftFootIKData = CalculateFootIK(LeftFootSocketName, RootLocation);
    RightFootIKData = CalculateFootIK(RightFootSocketName, RootLocation);

    // Smooth interpolation
    if (LeftFootIKData.bIsGrounded)
    {
        LeftFootTargetLocation = FMath::VInterpTo(LeftFootTargetLocation, LeftFootIKData.FootLocation, DeltaTime, IKSettings.IKInterpSpeed);
        LeftFootTargetRotation = FMath::RInterpTo(LeftFootTargetRotation, LeftFootIKData.FootRotation, DeltaTime, IKSettings.IKInterpSpeed);
        LeftFootIKData.FootLocation = LeftFootTargetLocation;
        LeftFootIKData.FootRotation = LeftFootTargetRotation;
    }

    if (RightFootIKData.bIsGrounded)
    {
        RightFootTargetLocation = FMath::VInterpTo(RightFootTargetLocation, RightFootIKData.FootLocation, DeltaTime, IKSettings.IKInterpSpeed);
        RightFootTargetRotation = FMath::RInterpTo(RightFootTargetRotation, RightFootIKData.FootRotation, DeltaTime, IKSettings.IKInterpSpeed);
        RightFootIKData.FootLocation = RightFootTargetLocation;
        RightFootIKData.FootRotation = RightFootTargetRotation;
    }
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& SocketName, const FVector& RootLocation)
{
    FAnim_FootIKData FootData;

    if (!SkeletalMeshComponent)
    {
        return FootData;
    }

    // Get foot socket location
    FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
    if (SocketLocation.IsZero())
    {
        // Fallback to bone location if socket doesn't exist
        SocketLocation = SkeletalMeshComponent->GetBoneLocation(SocketName);
    }

    // Perform trace from above the foot
    FVector TraceStart = SocketLocation + FVector(0, 0, IKSettings.TraceDistance);
    FVector TraceEnd = SocketLocation - FVector(0, 0, IKSettings.TraceDistance);

    FVector HitLocation;
    FVector HitNormal;
    bool bHit = PerformFootTrace(TraceStart, HitLocation, HitNormal);

    if (bHit)
    {
        FootData.bIsGrounded = true;
        FootData.FootLocation = HitLocation + HitNormal * IKSettings.FootOffset;
        
        if (IKSettings.bEnableFootRotation)
        {
            FRotator CurrentRotation = SkeletalMeshComponent->GetSocketRotation(SocketName);
            FootData.FootRotation = CalculateFootRotation(HitNormal, CurrentRotation);
        }

        // Calculate IK alpha based on distance
        float Distance = FVector::Dist(SocketLocation, HitLocation);
        FootData.IKAlpha = FMath::Clamp(Distance / IKSettings.MaxFootAdjustment, 0.0f, 1.0f);
    }
    else
    {
        FootData.bIsGrounded = false;
        FootData.IKAlpha = 0.0f;
    }

    return FootData;
}

bool UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        StartLocation - FVector(0, 0, IKSettings.TraceDistance * 2),
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
    }

    return bHit;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation)
{
    // Calculate rotation to align foot with surface normal
    FVector ForwardVector = CurrentRotation.Vector();
    FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();

    FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
    
    // Limit rotation to prevent extreme angles
    FRotator DeltaRotation = TargetRotation - CurrentRotation;
    DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -45.0f, 45.0f);
    DeltaRotation.Roll = FMath::Clamp(DeltaRotation.Roll, -45.0f, 45.0f);
    
    return CurrentRotation + DeltaRotation;
}

void UAnim_IKFootPlacement::UpdateHipOffset(float DeltaTime)
{
    TargetHipOffset = CalculateHipAdjustment(LeftFootIKData, RightFootIKData);
    CurrentHipOffset = FMath::FInterpTo(CurrentHipOffset, TargetHipOffset, DeltaTime, IKSettings.IKInterpSpeed);
}

float UAnim_IKFootPlacement::CalculateHipAdjustment(const FAnim_FootIKData& LeftFoot, const FAnim_FootIKData& RightFoot)
{
    if (!LeftFoot.bIsGrounded && !RightFoot.bIsGrounded)
    {
        return 0.0f;
    }

    float LeftOffset = LeftFoot.bIsGrounded ? (LeftFoot.FootLocation.Z - GetOwner()->GetActorLocation().Z) : 0.0f;
    float RightOffset = RightFoot.bIsGrounded ? (RightFoot.FootLocation.Z - GetOwner()->GetActorLocation().Z) : 0.0f;

    // Use the higher foot as reference to prevent sinking
    float HipAdjustment = FMath::Max(LeftOffset, RightOffset);
    
    // Limit adjustment
    return FMath::Clamp(HipAdjustment, -IKSettings.MaxFootAdjustment, IKSettings.MaxFootAdjustment);
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    if (!bEnabled)
    {
        ResetIKData();
    }
}

void UAnim_IKFootPlacement::ResetIKData()
{
    LeftFootIKData = FAnim_FootIKData();
    RightFootIKData = FAnim_FootIKData();
    CurrentHipOffset = 0.0f;
    TargetHipOffset = 0.0f;
    LeftFootTargetLocation = FVector::ZeroVector;
    RightFootTargetLocation = FVector::ZeroVector;
    LeftFootTargetRotation = FRotator::ZeroRotator;
    RightFootTargetRotation = FRotator::ZeroRotator;
}

void UAnim_IKFootPlacement::DrawDebugInfo()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Draw foot IK positions
    if (LeftFootIKData.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), LeftFootIKData.FootLocation, 5.0f, 12, FColor::Green, false, 0.0f);
        DrawDebugLine(GetWorld(), OwnerLocation, LeftFootIKData.FootLocation, FColor::Green, false, 0.0f);
    }

    if (RightFootIKData.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), RightFootIKData.FootLocation, 5.0f, 12, FColor::Blue, false, 0.0f);
        DrawDebugLine(GetWorld(), OwnerLocation, RightFootIKData.FootLocation, FColor::Blue, false, 0.0f);
    }

    // Draw hip offset
    if (FMath::Abs(CurrentHipOffset) > 0.1f)
    {
        FVector HipLocation = OwnerLocation + FVector(0, 0, CurrentHipOffset);
        DrawDebugSphere(GetWorld(), HipLocation, 8.0f, 12, FColor::Yellow, false, 0.0f);
    }
}