#include "Anim_AdvancedIKSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_AdvancedIKSystem::UAnim_AdvancedIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default settings
    bEnableFootIK = true;
    bEnableHandIK = true;
    bEnablePelvisAdjustment = true;
    IKUpdateRate = 60.0f;
    FootTraceDistance = 200.0f;
    IKInterpSpeed = 10.0f;
    MaxFootOffset = 50.0f;
    MaxPelvisOffset = 30.0f;

    // Debug settings
    bShowIKDebug = false;
    bShowFootTraces = false;
    bShowIKTargets = false;

    // Internal state
    LastIKUpdateTime = 0.0f;
    OwnerMeshComponent = nullptr;
    PelvisOffset = 0.0f;

    // Socket names
    LeftFootSocketName = FName("foot_l");
    RightFootSocketName = FName("foot_r");
    LeftHandSocketName = FName("hand_l");
    RightHandSocketName = FName("hand_r");
}

void UAnim_AdvancedIKSystem::BeginPlay()
{
    Super::BeginPlay();

    // Cache the skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("AdvancedIKSystem: No SkeletalMeshComponent found on owner"));
        }
    }

    InitializeIKTargets();
    CacheSocketNames();
}

void UAnim_AdvancedIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerMeshComponent)
        return;

    // Update IK at specified rate
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastIKUpdateTime >= (1.0f / IKUpdateRate))
    {
        UpdateIKTargets(DeltaTime);
        LastIKUpdateTime = CurrentTime;
    }

    // Draw debug information if enabled
    if (bShowIKDebug)
    {
        DrawIKDebugInfo();
    }
}

void UAnim_AdvancedIKSystem::UpdateIKTargets(float DeltaTime)
{
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }

    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }

    if (bEnablePelvisAdjustment)
    {
        UpdatePelvisOffset(DeltaTime);
    }
}

void UAnim_AdvancedIKSystem::SetIKTarget(EAnim_IKTargetType TargetType, FVector Location, FRotator Rotation, float Alpha)
{
    FAnim_IKTarget& Target = IKTargets.FindOrAdd(TargetType);
    Target.TargetType = TargetType;
    Target.TargetLocation = Location;
    Target.TargetRotation = Rotation;
    Target.IKAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    Target.bIsActive = true;
}

FAnim_IKTarget UAnim_AdvancedIKSystem::GetIKTarget(EAnim_IKTargetType TargetType) const
{
    if (const FAnim_IKTarget* Target = IKTargets.Find(TargetType))
    {
        return *Target;
    }
    return FAnim_IKTarget();
}

void UAnim_AdvancedIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent)
        return;

    // Update left foot IK
    FVector LeftFootLocation = GetSocketWorldLocation(LeftFootSocketName);
    if (!LeftFootLocation.IsZero())
    {
        LeftFootIK = CalculateFootPlacement(LeftFootLocation, LeftFootSocketName);
        
        // Set IK target for left foot
        FVector TargetLocation = LeftFootLocation + FVector(0, 0, LeftFootIK.FootOffset);
        SetIKTarget(EAnim_IKTargetType::LeftFoot, TargetLocation, LeftFootIK.FootRotation, LeftFootIK.IKAlpha);
    }

    // Update right foot IK
    FVector RightFootLocation = GetSocketWorldLocation(RightFootSocketName);
    if (!RightFootLocation.IsZero())
    {
        RightFootIK = CalculateFootPlacement(RightFootLocation, RightFootSocketName);
        
        // Set IK target for right foot
        FVector TargetLocation = RightFootLocation + FVector(0, 0, RightFootIK.FootOffset);
        SetIKTarget(EAnim_IKTargetType::RightFoot, TargetLocation, RightFootIK.FootRotation, RightFootIK.IKAlpha);
    }
}

FAnim_FootIKData UAnim_AdvancedIKSystem::CalculateFootPlacement(const FVector& FootLocation, const FName& SocketName)
{
    FAnim_FootIKData FootData;

    FVector HitLocation;
    FVector HitNormal;
    
    if (PerformFootTrace(FootLocation, HitLocation, HitNormal))
    {
        // Calculate foot offset
        float HeightDifference = HitLocation.Z - FootLocation.Z;
        FootData.FootOffset = FMath::Clamp(HeightDifference, -MaxFootOffset, MaxFootOffset);
        
        // Calculate foot rotation based on surface normal
        FootData.FootRotation = CalculateFootRotationFromNormal(HitNormal);
        
        // Set IK alpha based on ground distance
        float DistanceToGround = FMath::Abs(HeightDifference);
        FootData.IKAlpha = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, MaxFootOffset), 
            FVector2D(1.0f, 0.0f), 
            DistanceToGround
        );
        
        FootData.bIsGrounded = DistanceToGround < 10.0f;
    }
    else
    {
        // No ground found, disable IK
        FootData.FootOffset = 0.0f;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.IKAlpha = 0.0f;
        FootData.bIsGrounded = false;
    }

    return FootData;
}

float UAnim_AdvancedIKSystem::GetTerrainHeightAtLocation(const FVector& Location, float TraceDistance)
{
    FVector StartLocation = Location + FVector(0, 0, TraceDistance * 0.5f);
    FVector EndLocation = Location - FVector(0, 0, TraceDistance * 0.5f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location.Z;
    }

    return Location.Z;
}

void UAnim_AdvancedIKSystem::UpdateHandIK(float DeltaTime)
{
    // Hand IK implementation - can be extended for interaction with objects
    // For now, just maintain current hand positions
    
    FVector LeftHandLocation = GetSocketWorldLocation(LeftHandSocketName);
    if (!LeftHandLocation.IsZero())
    {
        SetIKTarget(EAnim_IKTargetType::LeftHand, LeftHandLocation, FRotator::ZeroRotator, 0.0f);
    }

    FVector RightHandLocation = GetSocketWorldLocation(RightHandSocketName);
    if (!RightHandLocation.IsZero())
    {
        SetIKTarget(EAnim_IKTargetType::RightHand, RightHandLocation, FRotator::ZeroRotator, 0.0f);
    }
}

void UAnim_AdvancedIKSystem::SetHandIKTarget(bool bIsLeftHand, AActor* TargetActor, FVector Offset)
{
    if (!TargetActor)
        return;

    FVector TargetLocation = TargetActor->GetActorLocation() + Offset;
    FRotator TargetRotation = TargetActor->GetActorRotation();

    EAnim_IKTargetType HandType = bIsLeftHand ? EAnim_IKTargetType::LeftHand : EAnim_IKTargetType::RightHand;
    SetIKTarget(HandType, TargetLocation, TargetRotation, 1.0f);
}

void UAnim_AdvancedIKSystem::UpdatePelvisOffset(float DeltaTime)
{
    float TargetPelvisOffset = CalculatePelvisOffset();
    
    // Smoothly interpolate to target offset
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKInterpSpeed);
    
    // Set pelvis IK target
    if (OwnerMeshComponent)
    {
        FVector PelvisLocation = OwnerMeshComponent->GetComponentLocation();
        PelvisLocation.Z += PelvisOffset;
        SetIKTarget(EAnim_IKTargetType::Pelvis, PelvisLocation, FRotator::ZeroRotator, 1.0f);
    }
}

float UAnim_AdvancedIKSystem::CalculatePelvisOffset() const
{
    // Calculate pelvis offset based on foot IK data
    float LeftOffset = LeftFootIK.bIsGrounded ? LeftFootIK.FootOffset : 0.0f;
    float RightOffset = RightFootIK.bIsGrounded ? RightFootIK.FootOffset : 0.0f;
    
    // Use the average of both feet, but bias towards the lower foot
    float AverageOffset = (LeftOffset + RightOffset) * 0.5f;
    float MinOffset = FMath::Min(LeftOffset, RightOffset);
    
    // Blend between average and minimum based on foot difference
    float FootDifference = FMath::Abs(LeftOffset - RightOffset);
    float BlendFactor = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 20.0f), FVector2D(0.0f, 1.0f), FootDifference);
    
    float TargetOffset = FMath::Lerp(AverageOffset, MinOffset, BlendFactor);
    return FMath::Clamp(TargetOffset, -MaxPelvisOffset, MaxPelvisOffset);
}

void UAnim_AdvancedIKSystem::ToggleIKDebugVisualization()
{
    bShowIKDebug = !bShowIKDebug;
    bShowFootTraces = bShowIKDebug;
    bShowIKTargets = bShowIKDebug;
}

void UAnim_AdvancedIKSystem::DrawIKDebugInfo()
{
    if (!GetWorld() || !OwnerMeshComponent)
        return;

    // Draw foot traces
    if (bShowFootTraces)
    {
        FVector LeftFootLoc = GetSocketWorldLocation(LeftFootSocketName);
        FVector RightFootLoc = GetSocketWorldLocation(RightFootSocketName);
        
        if (!LeftFootLoc.IsZero())
        {
            FVector TraceStart = LeftFootLoc + FVector(0, 0, FootTraceDistance * 0.5f);
            FVector TraceEnd = LeftFootLoc - FVector(0, 0, FootTraceDistance * 0.5f);
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, -1, 0, 2.0f);
        }
        
        if (!RightFootLoc.IsZero())
        {
            FVector TraceStart = RightFootLoc + FVector(0, 0, FootTraceDistance * 0.5f);
            FVector TraceEnd = RightFootLoc - FVector(0, 0, FootTraceDistance * 0.5f);
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, -1, 0, 2.0f);
        }
    }

    // Draw IK targets
    if (bShowIKTargets)
    {
        for (const auto& IKPair : IKTargets)
        {
            const FAnim_IKTarget& Target = IKPair.Value;
            if (Target.bIsActive && Target.IKAlpha > 0.0f)
            {
                FColor TargetColor = FColor::Red;
                switch (Target.TargetType)
                {
                case EAnim_IKTargetType::LeftFoot:
                    TargetColor = FColor::Green;
                    break;
                case EAnim_IKTargetType::RightFoot:
                    TargetColor = FColor::Blue;
                    break;
                case EAnim_IKTargetType::LeftHand:
                    TargetColor = FColor::Yellow;
                    break;
                case EAnim_IKTargetType::RightHand:
                    TargetColor = FColor::Orange;
                    break;
                case EAnim_IKTargetType::Pelvis:
                    TargetColor = FColor::Purple;
                    break;
                }
                
                DrawDebugSphere(GetWorld(), Target.TargetLocation, 5.0f, 8, TargetColor, false, -1, 0, 2.0f);
            }
        }
    }
}

void UAnim_AdvancedIKSystem::InitializeIKTargets()
{
    // Initialize all IK targets with default values
    IKTargets.Empty();
    
    TArray<EAnim_IKTargetType> TargetTypes = {
        EAnim_IKTargetType::LeftFoot,
        EAnim_IKTargetType::RightFoot,
        EAnim_IKTargetType::LeftHand,
        EAnim_IKTargetType::RightHand,
        EAnim_IKTargetType::Pelvis,
        EAnim_IKTargetType::Spine
    };

    for (EAnim_IKTargetType TargetType : TargetTypes)
    {
        FAnim_IKTarget& Target = IKTargets.Add(TargetType);
        Target.TargetType = TargetType;
        Target.bIsActive = false;
    }
}

void UAnim_AdvancedIKSystem::CacheSocketNames()
{
    // Cache socket names - these can be customized based on the skeletal mesh
    if (OwnerMeshComponent && OwnerMeshComponent->GetSkeletalMeshAsset())
    {
        // Try to find actual socket names from the mesh
        // For now, use standard naming convention
        LeftFootSocketName = FName("foot_l");
        RightFootSocketName = FName("foot_r");
        LeftHandSocketName = FName("hand_l");
        RightHandSocketName = FName("hand_r");
    }
}

FVector UAnim_AdvancedIKSystem::GetSocketWorldLocation(const FName& SocketName) const
{
    if (OwnerMeshComponent && OwnerMeshComponent->DoesSocketExist(SocketName))
    {
        return OwnerMeshComponent->GetSocketLocation(SocketName);
    }
    return FVector::ZeroVector;
}

FRotator UAnim_AdvancedIKSystem::GetSocketWorldRotation(const FName& SocketName) const
{
    if (OwnerMeshComponent && OwnerMeshComponent->DoesSocketExist(SocketName))
    {
        return OwnerMeshComponent->GetSocketRotation(SocketName);
    }
    return FRotator::ZeroRotator;
}

bool UAnim_AdvancedIKSystem::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    FVector TraceStart = StartLocation + FVector(0, 0, FootTraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, FootTraceDistance * 0.5f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }

    OutHitLocation = FVector::ZeroVector;
    OutHitNormal = FVector::UpVector;
    return false;
}

FRotator UAnim_AdvancedIKSystem::CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const
{
    // Calculate foot rotation to align with surface normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();

    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
}