#include "Anim_RealTimeIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UAnim_RealTimeIKSystem::UAnim_RealTimeIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Default IK settings
    bEnableFootIK = true;
    bEnableHandIK = false;
    bEnableSpineIK = true;
    bEnableLookAtIK = false;
    
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    HandIKInterpSpeed = 10.0f;
    SpineIKInterpSpeed = 8.0f;
    LookAtIKInterpSpeed = 5.0f;
    
    MaxFootOffset = 25.0f;
    MaxHandOffset = 30.0f;
    MaxSpineOffset = 15.0f;
    
    FootIKSocketNames.Add(TEXT("foot_l"));
    FootIKSocketNames.Add(TEXT("foot_r"));
    HandIKSocketNames.Add(TEXT("hand_l"));
    HandIKSocketNames.Add(TEXT("hand_r"));
    
    SpineBoneName = TEXT("spine_03");
    NeckBoneName = TEXT("neck_01");
    
    bDebugDraw = false;
    DebugDrawDuration = 0.0f;
}

void UAnim_RealTimeIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache skeletal mesh component
    OwnerSkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    
    if (OwnerSkeletalMesh)
    {
        // Initialize IK data structures
        InitializeIKData();
        
        UE_LOG(LogTemp, Log, TEXT("RealTimeIKSystem initialized for %s"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RealTimeIKSystem: No SkeletalMeshComponent found on %s"), *GetOwner()->GetName());
    }
}

void UAnim_RealTimeIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerSkeletalMesh || !OwnerSkeletalMesh->GetAnimInstance())
    {
        return;
    }
    
    // Update IK systems
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }
    
    if (bEnableSpineIK)
    {
        UpdateSpineIK(DeltaTime);
    }
    
    if (bEnableLookAtIK)
    {
        UpdateLookAtIK(DeltaTime);
    }
    
    // Debug drawing
    if (bDebugDraw)
    {
        DrawDebugInfo();
    }
}

void UAnim_RealTimeIKSystem::InitializeIKData()
{
    if (!OwnerSkeletalMesh)
        return;
    
    // Initialize foot IK data
    FootIKData.Empty();
    for (const FName& SocketName : FootIKSocketNames)
    {
        if (OwnerSkeletalMesh->DoesSocketExist(SocketName))
        {
            FAnim_FootIKData FootData;
            FootData.SocketName = SocketName;
            FootData.TargetLocation = FVector::ZeroVector;
            FootData.CurrentOffset = 0.0f;
            FootData.TargetOffset = 0.0f;
            FootData.bIsGrounded = false;
            
            FootIKData.Add(FootData);
        }
    }
    
    // Initialize hand IK data
    HandIKData.Empty();
    for (const FName& SocketName : HandIKSocketNames)
    {
        if (OwnerSkeletalMesh->DoesSocketExist(SocketName))
        {
            FAnim_HandIKData HandData;
            HandData.SocketName = SocketName;
            HandData.TargetLocation = FVector::ZeroVector;
            HandData.CurrentLocation = FVector::ZeroVector;
            HandData.bIsActive = false;
            
            HandIKData.Add(HandData);
        }
    }
    
    // Initialize spine IK data
    SpineIKData.TargetRotation = FRotator::ZeroRotator;
    SpineIKData.CurrentRotation = FRotator::ZeroRotator;
    SpineIKData.bIsActive = false;
    
    // Initialize look-at IK data
    LookAtIKData.TargetLocation = FVector::ZeroVector;
    LookAtIKData.CurrentRotation = FRotator::ZeroRotator;
    LookAtIKData.bIsActive = false;
}

void UAnim_RealTimeIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerSkeletalMesh || FootIKData.Num() == 0)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    for (FAnim_FootIKData& FootData : FootIKData)
    {
        // Get foot socket location in world space
        FVector SocketLocation = OwnerSkeletalMesh->GetSocketLocation(FootData.SocketName);
        
        // Trace downward from foot socket
        FVector TraceStart = SocketLocation + FVector(0, 0, 20.0f);
        FVector TraceEnd = SocketLocation - FVector(0, 0, FootIKTraceDistance);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        
        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_WorldStatic,
            QueryParams
        );
        
        if (bHit)
        {
            // Calculate foot offset
            float GroundDistance = FVector::Dist(SocketLocation, HitResult.Location);
            FootData.TargetOffset = FMath::Clamp(
                FootIKTraceDistance - GroundDistance,
                -MaxFootOffset,
                MaxFootOffset
            );
            FootData.bIsGrounded = true;
            FootData.TargetLocation = HitResult.Location;
        }
        else
        {
            FootData.TargetOffset = 0.0f;
            FootData.bIsGrounded = false;
        }
        
        // Interpolate to target offset
        FootData.CurrentOffset = FMath::FInterpTo(
            FootData.CurrentOffset,
            FootData.TargetOffset,
            DeltaTime,
            FootIKInterpSpeed
        );
    }
}

void UAnim_RealTimeIKSystem::UpdateHandIK(float DeltaTime)
{
    if (!OwnerSkeletalMesh || HandIKData.Num() == 0)
        return;
    
    for (FAnim_HandIKData& HandData : HandIKData)
    {
        if (!HandData.bIsActive)
            continue;
        
        // Interpolate hand position to target
        HandData.CurrentLocation = FMath::VInterpTo(
            HandData.CurrentLocation,
            HandData.TargetLocation,
            DeltaTime,
            HandIKInterpSpeed
        );
    }
}

void UAnim_RealTimeIKSystem::UpdateSpineIK(float DeltaTime)
{
    if (!OwnerSkeletalMesh || !SpineIKData.bIsActive)
        return;
    
    // Interpolate spine rotation to target
    SpineIKData.CurrentRotation = FMath::RInterpTo(
        SpineIKData.CurrentRotation,
        SpineIKData.TargetRotation,
        DeltaTime,
        SpineIKInterpSpeed
    );
}

void UAnim_RealTimeIKSystem::UpdateLookAtIK(float DeltaTime)
{
    if (!OwnerSkeletalMesh || !LookAtIKData.bIsActive)
        return;
    
    // Calculate look-at rotation
    FVector HeadLocation = OwnerSkeletalMesh->GetSocketLocation(NeckBoneName);
    FVector LookDirection = (LookAtIKData.TargetLocation - HeadLocation).GetSafeNormal();
    FRotator TargetRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
    
    // Interpolate to target rotation
    LookAtIKData.CurrentRotation = FMath::RInterpTo(
        LookAtIKData.CurrentRotation,
        TargetRotation,
        DeltaTime,
        LookAtIKInterpSpeed
    );
}

void UAnim_RealTimeIKSystem::SetHandIKTarget(const FName& HandSocketName, const FVector& TargetLocation, bool bEnable)
{
    for (FAnim_HandIKData& HandData : HandIKData)
    {
        if (HandData.SocketName == HandSocketName)
        {
            HandData.TargetLocation = TargetLocation;
            HandData.bIsActive = bEnable;
            break;
        }
    }
}

void UAnim_RealTimeIKSystem::SetSpineIKTarget(const FRotator& TargetRotation, bool bEnable)
{
    SpineIKData.TargetRotation = TargetRotation;
    SpineIKData.bIsActive = bEnable;
}

void UAnim_RealTimeIKSystem::SetLookAtIKTarget(const FVector& TargetLocation, bool bEnable)
{
    LookAtIKData.TargetLocation = TargetLocation;
    LookAtIKData.bIsActive = bEnable;
}

float UAnim_RealTimeIKSystem::GetFootIKOffset(const FName& FootSocketName) const
{
    for (const FAnim_FootIKData& FootData : FootIKData)
    {
        if (FootData.SocketName == FootSocketName)
        {
            return FootData.CurrentOffset;
        }
    }
    return 0.0f;
}

FVector UAnim_RealTimeIKSystem::GetHandIKLocation(const FName& HandSocketName) const
{
    for (const FAnim_HandIKData& HandData : HandIKData)
    {
        if (HandData.SocketName == HandSocketName)
        {
            return HandData.CurrentLocation;
        }
    }
    return FVector::ZeroVector;
}

FRotator UAnim_RealTimeIKSystem::GetSpineIKRotation() const
{
    return SpineIKData.CurrentRotation;
}

FRotator UAnim_RealTimeIKSystem::GetLookAtIKRotation() const
{
    return LookAtIKData.CurrentRotation;
}

bool UAnim_RealTimeIKSystem::IsFootGrounded(const FName& FootSocketName) const
{
    for (const FAnim_FootIKData& FootData : FootIKData)
    {
        if (FootData.SocketName == FootSocketName)
        {
            return FootData.bIsGrounded;
        }
    }
    return false;
}

void UAnim_RealTimeIKSystem::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
}

void UAnim_RealTimeIKSystem::EnableHandIK(bool bEnable)
{
    bEnableHandIK = bEnable;
}

void UAnim_RealTimeIKSystem::EnableSpineIK(bool bEnable)
{
    bEnableSpineIK = bEnable;
}

void UAnim_RealTimeIKSystem::EnableLookAtIK(bool bEnable)
{
    bEnableLookAtIK = bEnable;
}

void UAnim_RealTimeIKSystem::SetDebugDraw(bool bEnable, float Duration)
{
    bDebugDraw = bEnable;
    DebugDrawDuration = Duration;
}

void UAnim_RealTimeIKSystem::DrawDebugInfo()
{
    if (!OwnerSkeletalMesh)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Draw foot IK debug info
    for (const FAnim_FootIKData& FootData : FootIKData)
    {
        FVector SocketLocation = OwnerSkeletalMesh->GetSocketLocation(FootData.SocketName);
        
        // Draw foot socket
        DrawDebugSphere(World, SocketLocation, 3.0f, 12, FColor::Blue, false, DebugDrawDuration);
        
        if (FootData.bIsGrounded)
        {
            // Draw ground target
            DrawDebugSphere(World, FootData.TargetLocation, 2.0f, 12, FColor::Green, false, DebugDrawDuration);
            
            // Draw line to ground
            DrawDebugLine(World, SocketLocation, FootData.TargetLocation, FColor::Yellow, false, DebugDrawDuration);
        }
        
        // Draw offset value
        FString OffsetText = FString::Printf(TEXT("%.1f"), FootData.CurrentOffset);
        DrawDebugString(World, SocketLocation + FVector(0, 0, 10), OffsetText, nullptr, FColor::White, DebugDrawDuration);
    }
    
    // Draw hand IK debug info
    for (const FAnim_HandIKData& HandData : HandIKData)
    {
        if (HandData.bIsActive)
        {
            FVector SocketLocation = OwnerSkeletalMesh->GetSocketLocation(HandData.SocketName);
            
            DrawDebugSphere(World, SocketLocation, 2.0f, 12, FColor::Red, false, DebugDrawDuration);
            DrawDebugSphere(World, HandData.TargetLocation, 2.0f, 12, FColor::Orange, false, DebugDrawDuration);
            DrawDebugLine(World, SocketLocation, HandData.TargetLocation, FColor::Red, false, DebugDrawDuration);
        }
    }
    
    // Draw look-at IK debug info
    if (LookAtIKData.bIsActive)
    {
        FVector HeadLocation = OwnerSkeletalMesh->GetSocketLocation(NeckBoneName);
        DrawDebugSphere(World, LookAtIKData.TargetLocation, 3.0f, 12, FColor::Purple, false, DebugDrawDuration);
        DrawDebugLine(World, HeadLocation, LookAtIKData.TargetLocation, FColor::Purple, false, DebugDrawDuration);
    }
}