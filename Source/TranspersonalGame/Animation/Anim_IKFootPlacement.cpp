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
    
    // Initialize IK data
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    HipOffset = 0.0f;
    
    // Set default socket names (common UE5 skeleton naming)
    LeftFootSocketName = TEXT("foot_l");
    RightFootSocketName = TEXT("foot_r");
    
    // Initialize settings with reasonable defaults
    IKSettings = FAnim_IKSettings();
    IKSettings.MaxIKDistance = 50.0f;
    IKSettings.IKInterpSpeed = 15.0f;
    IKSettings.TraceDistance = 100.0f;
    IKSettings.FootHeight = 10.0f;
    IKSettings.bEnableIK = true;
    IKSettings.bShowDebugTraces = false;
    
    TargetHipOffset = 0.0f;
    CurrentHipOffset = 0.0f;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMesh = OwnerCharacter->GetMesh();
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("Foot IK System initialized for character: %s"), 
               *OwnerCharacter->GetName());
        
        // Verify socket names exist
        if (SkeletalMesh)
        {
            if (!SkeletalMesh->DoesSocketExist(LeftFootSocketName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Left foot socket '%s' not found, trying alternatives"), 
                       *LeftFootSocketName.ToString());
                // Try common alternatives
                if (SkeletalMesh->DoesSocketExist(TEXT("LeftFoot")))
                    LeftFootSocketName = TEXT("LeftFoot");
                else if (SkeletalMesh->DoesSocketExist(TEXT("foot_L")))
                    LeftFootSocketName = TEXT("foot_L");
            }
            
            if (!SkeletalMesh->DoesSocketExist(RightFootSocketName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Right foot socket '%s' not found, trying alternatives"), 
                       *RightFootSocketName.ToString());
                // Try common alternatives
                if (SkeletalMesh->DoesSocketExist(TEXT("RightFoot")))
                    RightFootSocketName = TEXT("RightFoot");
                else if (SkeletalMesh->DoesSocketExist(TEXT("foot_R")))
                    RightFootSocketName = TEXT("foot_R");
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Foot IK System: Owner is not a Character!"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!ShouldPerformIK())
    {
        return;
    }
    
    UpdateFootIK(DeltaTime);
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMesh || !IKSettings.bEnableIK)
    {
        return;
    }
    
    // Update individual foot IK
    LeftFootIK = TraceFootIK(LeftFootSocketName, DeltaTime);
    RightFootIK = TraceFootIK(RightFootSocketName, DeltaTime);
    
    // Calculate hip offset to keep character grounded
    TargetHipOffset = CalculateHipOffset(LeftFootIK, RightFootIK);
    
    // Smoothly interpolate hip offset
    CurrentHipOffset = FMath::FInterpTo(CurrentHipOffset, TargetHipOffset, DeltaTime, IKSettings.IKInterpSpeed);
    HipOffset = CurrentHipOffset;
    
    // Adjust foot positions relative to hip offset
    LeftFootIK.FootLocation.Z -= HipOffset;
    RightFootIK.FootLocation.Z -= HipOffset;
    
    // Update IK alpha based on movement state
    float MovementSpeed = CharacterMovement ? CharacterMovement->Velocity.Size2D() : 0.0f;
    float MaxSpeed = CharacterMovement ? CharacterMovement->MaxWalkSpeed : 600.0f;
    float SpeedRatio = FMath::Clamp(MovementSpeed / MaxSpeed, 0.0f, 1.0f);
    
    // Reduce IK influence at higher speeds (running/sprinting)
    float BaseIKAlpha = FMath::Lerp(1.0f, 0.3f, SpeedRatio);
    
    LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, BaseIKAlpha, DeltaTime, IKSettings.IKInterpSpeed);
    RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, BaseIKAlpha, DeltaTime, IKSettings.IKInterpSpeed);
}

FAnim_FootIKData UAnim_IKFootPlacement::TraceFootIK(FName SocketName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!SkeletalMesh || !SkeletalMesh->DoesSocketExist(SocketName))
    {
        return FootData;
    }
    
    // Get foot world location
    FVector FootWorldLocation = GetFootWorldLocation(SocketName);
    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
    
    // Perform ground trace
    FVector HitLocation;
    FVector HitNormal;
    float HitDistance;
    
    bool bHit = PerformFootTrace(TraceStart, HitLocation, HitNormal, HitDistance);
    
    if (bHit)
    {
        // Calculate foot adjustment
        float GroundZ = HitLocation.Z + IKSettings.FootHeight;
        float FootZ = FootWorldLocation.Z;
        float ZDifference = GroundZ - FootZ;
        
        // Only apply IK if within reasonable range
        if (FMath::Abs(ZDifference) <= IKSettings.MaxIKDistance)
        {
            FootData.FootLocation = FVector(0.0f, 0.0f, ZDifference);
            FootData.GroundDistance = HitDistance;
            FootData.GroundNormal = HitNormal;
            FootData.IKAlpha = 1.0f;
            
            // Calculate foot rotation based on ground normal
            FRotator CurrentRotation = SkeletalMesh->GetSocketRotation(SocketName);
            FootData.FootRotation = CalculateFootRotation(HitNormal, CurrentRotation);
        }
    }
    
    return FootData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(FName SocketName) const
{
    if (SkeletalMesh && SkeletalMesh->DoesSocketExist(SocketName))
    {
        return SkeletalMesh->GetSocketLocation(SocketName);
    }
    
    return FVector::ZeroVector;
}

bool UAnim_IKFootPlacement::PerformFootTrace(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal, float& OutDistance)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        OutDistance = HitResult.Distance;
        
        // Optional debug drawing
        if (IKSettings.bShowDebugTraces && GetWorld()->IsPlayInEditor())
        {
            DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Green, false, 0.1f, 0, 2.0f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Red, false, 0.1f);
            DrawDebugDirectionalArrow(GetWorld(), HitResult.Location, 
                                    HitResult.Location + HitResult.Normal * 20.0f, 
                                    5.0f, FColor::Blue, false, 0.1f, 0, 2.0f);
        }
        
        return true;
    }
    else
    {
        // Debug drawing for missed traces
        if (IKSettings.bShowDebugTraces && GetWorld()->IsPlayInEditor())
        {
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 0.1f, 0, 1.0f);
        }
    }
    
    return false;
}

float UAnim_IKFootPlacement::CalculateHipOffset(const FAnim_FootIKData& LeftFoot, const FAnim_FootIKData& RightFoot)
{
    // Find the lowest foot position to determine hip adjustment
    float LeftFootZ = LeftFoot.IKAlpha > 0.0f ? LeftFoot.FootLocation.Z : 0.0f;
    float RightFootZ = RightFoot.IKAlpha > 0.0f ? RightFoot.FootLocation.Z : 0.0f;
    
    // Use the lower foot as reference (negative values mean foot needs to go down)
    float LowestFootZ = FMath::Min(LeftFootZ, RightFootZ);
    
    // Only adjust hip if one or both feet need to go down
    if (LowestFootZ < 0.0f)
    {
        return LowestFootZ; // This will be negative, lowering the hip
    }
    
    return 0.0f;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation)
{
    // Calculate rotation to align foot with ground normal
    FVector UpVector = FVector::UpVector;
    FVector ForwardVector = CurrentRotation.Vector();
    
    // Project forward vector onto the ground plane
    FVector ProjectedForward = ForwardVector - FVector::DotProduct(ForwardVector, GroundNormal) * GroundNormal;
    ProjectedForward.Normalize();
    
    // Calculate right vector
    FVector RightVector = FVector::CrossProduct(GroundNormal, ProjectedForward);
    RightVector.Normalize();
    
    // Recalculate forward vector to ensure orthogonality
    ProjectedForward = FVector::CrossProduct(RightVector, GroundNormal);
    
    // Create rotation matrix and convert to rotator
    FMatrix RotationMatrix = FMatrix(ProjectedForward, RightVector, GroundNormal, FVector::ZeroVector);
    FRotator TargetRotation = RotationMatrix.Rotator();
    
    // Limit rotation angles to prevent extreme foot bending
    float MaxFootAngle = 30.0f; // Maximum degrees the foot can rotate
    
    FRotator DeltaRotation = (TargetRotation - CurrentRotation).GetNormalized();
    DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -MaxFootAngle, MaxFootAngle);
    DeltaRotation.Roll = FMath::Clamp(DeltaRotation.Roll, -MaxFootAngle, MaxFootAngle);
    
    return CurrentRotation + DeltaRotation;
}

bool UAnim_IKFootPlacement::ShouldPerformIK() const
{
    if (!IKSettings.bEnableIK || !OwnerCharacter || !SkeletalMesh || !CharacterMovement)
    {
        return false;
    }
    
    // Don't perform IK if character is in air for too long
    if (CharacterMovement->IsFalling())
    {
        return false;
    }
    
    // Don't perform IK if character is swimming
    if (CharacterMovement->IsSwimming())
    {
        return false;
    }
    
    return true;
}