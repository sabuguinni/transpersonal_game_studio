// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AdvancedFootIKSystemV44.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdvancedFootIK, Log, All);

UAdvancedFootIKSystemV44::UAdvancedFootIKSystemV44()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default settings
    MaxFootOffset = 50.0f;
    FootTraceDistance = 100.0f;
    IKInterpSpeed = 15.0f;
    RotationInterpSpeed = 10.0f;
    bEnableFootPlanting = true;
    bEnableKneeTargeting = true;
    bEnableSurfaceAdaptation = true;
    
    // Hip adjustment
    HipBoneName = TEXT("pelvis");
    HipAdjustmentRatio = 0.5f;
    MaxHipOffset = 30.0f;
    HipInterpSpeed = 12.0f;
    
    // Terrain adaptation
    MaxSlopeAngle = 45.0f;
    SurfaceAdaptationStrength = 1.0f;
    
    // Performance
    UpdateFrequency = 60.0f;
    MaxTraceDistance = 150.0f;
    bUseAsyncTracing = false;
    
    // Debug
    bEnableDebugVisualization = false;
    bShowTraceLines = false;
    bShowFootTargets = false;
    bShowSurfaceNormals = false;
    
    // Runtime initialization
    HipOffset = FVector::ZeroVector;
    LastUpdateTime = 0.0f;
    bIKEnabled = true;
    bSpeedBasedPlantingEnabled = false;
    MinSpeedForPlanting = 50.0f;
    CurrentFootIndex = 0;
    TimeSinceLastUpdate = 0.0f;
    
    // Setup default trace object types
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
}

void UAdvancedFootIKSystemV44::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component
    SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogAdvancedFootIK, Error, TEXT("No SkeletalMeshComponent found on owner %s"), 
               *GetOwner()->GetName());
        return;
    }
    
    InitializeFootIKData();
    
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Advanced Foot IK System initialized with %d feet"), 
           FootIKData.Num());
}

void UAdvancedFootIKSystemV44::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIKEnabled || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Performance optimization: update at specified frequency
    TimeSinceLastUpdate += DeltaTime;
    float UpdateInterval = 1.0f / UpdateFrequency;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdateFootIK(TimeSinceLastUpdate);
        TimeSinceLastUpdate = 0.0f;
    }
    
    // Always update interpolation for smooth movement
    for (FFootIKData& FootData : FootIKData)
    {
        // Smooth interpolation of IK values
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 
                                           FootData.bIsPlanted ? 1.0f : 0.0f, 
                                           DeltaTime, IKInterpSpeed);
        
        if (bEnableKneeTargeting)
        {
            FootData.KneeTargetAlpha = FMath::FInterpTo(FootData.KneeTargetAlpha,
                                                       FootData.bIsPlanted ? 1.0f : 0.0f,
                                                       DeltaTime, IKInterpSpeed * 0.8f);
        }
    }
    
    // Update hip adjustment
    UpdateHipAdjustment(DeltaTime);
    
    // Debug visualization
    if (bEnableDebugVisualization)
    {
        DrawDebugVisualization();
    }
}

void UAdvancedFootIKSystemV44::InitializeFootIKData()
{
    if (FootIKData.Num() == 0)
    {
        // Setup default foot data for humanoid character
        FFootIKData LeftFootData;
        LeftFootData.FootBoneName = TEXT("foot_l");
        LeftFootData.IKBoneName = TEXT("ik_foot_l");
        LeftFootData.KneeBoneName = TEXT("calf_l");
        LeftFootData.PlantingThreshold = 50.0f;
        LeftFootData.LiftingThreshold = 100.0f;
        FootIKData.Add(LeftFootData);
        
        FFootIKData RightFootData;
        RightFootData.FootBoneName = TEXT("foot_r");
        RightFootData.IKBoneName = TEXT("ik_foot_r");
        RightFootData.KneeBoneName = TEXT("calf_r");
        RightFootData.PlantingThreshold = 50.0f;
        RightFootData.LiftingThreshold = 100.0f;
        FootIKData.Add(RightFootData);
    }
    
    // Initialize cached surface data
    for (const FFootIKData& FootData : FootIKData)
    {
        CachedSurfaceData.Add(FootData.FootBoneName, FTerrainSurfaceData());
    }
}

void UAdvancedFootIKSystemV44::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent || FootIKData.Num() == 0)
    {
        return;
    }
    
    // Update each foot
    for (FFootIKData& FootData : FootIKData)
    {
        UpdateSingleFootIK(FootData, DeltaTime);
    }
    
    // Update knee targeting if enabled
    if (bEnableKneeTargeting)
    {
        UpdateKneeTargeting(DeltaTime);
    }
}

void UAdvancedFootIKSystemV44::UpdateSingleFootIK(FFootIKData& FootData, float DeltaTime)
{
    // Perform ground trace
    FTerrainSurfaceData SurfaceData = PerformFootTrace(FootData);
    
    // Cache surface data
    CachedSurfaceData.FindOrAdd(FootData.FootBoneName) = SurfaceData;
    
    if (!SurfaceData.bIsValidSurface)
    {
        // No valid surface found, gradually reduce IK influence
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }
    
    // Calculate foot offset
    FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
    float GroundHeight = SurfaceData.SurfaceLocation.Z;
    float FootHeight = FootWorldLocation.Z;
    float TargetOffset = FMath::Clamp(GroundHeight - FootHeight, -MaxFootOffset, MaxFootOffset);
    
    // Apply surface adaptation
    if (bEnableSurfaceAdaptation)
    {
        FVector SurfaceAdjustment = SurfaceData.SurfaceNormal * SurfaceAdaptationStrength;
        TargetOffset += SurfaceAdjustment.Z;
    }
    
    // Smooth interpolation
    FootData.FootOffset.Z = FMath::FInterpTo(FootData.FootOffset.Z, TargetOffset, DeltaTime, IKInterpSpeed);
    
    // Calculate foot rotation from surface normal
    if (bEnableSurfaceAdaptation)
    {
        FVector ForwardVector = SkeletalMeshComponent->GetForwardVector();
        FRotator TargetRotation = CalculateFootRotationFromSurface(SurfaceData.SurfaceNormal, ForwardVector);
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
    }
    
    // Update foot planting state
    if (bEnableFootPlanting)
    {
        UpdateFootPlanting(FootData, SurfaceData, DeltaTime);
    }
    
    // Calculate knee target if enabled
    if (bEnableKneeTargeting && !FootData.KneeBoneName.IsNone())
    {
        FVector FootTargetLocation = FootWorldLocation + FootData.FootOffset;
        FootData.KneeTargetLocation = CalculateKneeTarget(FootData, FootTargetLocation);
    }
}

void UAdvancedFootIKSystemV44::UpdateHipAdjustment(float DeltaTime)
{
    if (FootIKData.Num() < 2)
    {
        return;
    }
    
    // Calculate average foot offset
    float TotalOffset = 0.0f;
    int32 ValidFeet = 0;
    
    for (const FFootIKData& FootData : FootIKData)
    {
        if (FootData.IKAlpha > 0.1f)
        {
            TotalOffset += FootData.FootOffset.Z;
            ValidFeet++;
        }
    }
    
    if (ValidFeet > 0)
    {
        float AverageOffset = TotalOffset / ValidFeet;
        float TargetHipOffset = AverageOffset * HipAdjustmentRatio;
        TargetHipOffset = FMath::Clamp(TargetHipOffset, -MaxHipOffset, MaxHipOffset);
        
        HipOffset.Z = FMath::FInterpTo(HipOffset.Z, TargetHipOffset, DeltaTime, HipInterpSpeed);
    }
}

void UAdvancedFootIKSystemV44::UpdateFootPlanting(FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData, float DeltaTime)
{
    // Check character velocity for speed-based planting
    if (bSpeedBasedPlantingEnabled)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                float CurrentSpeed = MovementComp->Velocity.Size();
                if (CurrentSpeed > MinSpeedForPlanting)
                {
                    // Moving too fast for planting
                    FootData.bIsPlanted = false;
                    return;
                }
            }
        }
    }
    
    // Determine planting state based on surface distance
    float SurfaceDistance = FMath::Abs(FootData.FootOffset.Z);
    
    if (!FootData.bIsPlanted && ShouldPlantFoot(FootData, SurfaceData))
    {
        FootData.bIsPlanted = true;
        OnFootPlanted.Broadcast(FootData.FootBoneName, true);
        
        UE_LOG(LogAdvancedFootIK, Verbose, TEXT("Foot %s planted"), *FootData.FootBoneName.ToString());
    }
    else if (FootData.bIsPlanted && ShouldLiftFoot(FootData, SurfaceData))
    {
        FootData.bIsPlanted = false;
        OnFootPlanted.Broadcast(FootData.FootBoneName, false);
        
        UE_LOG(LogAdvancedFootIK, Verbose, TEXT("Foot %s lifted"), *FootData.FootBoneName.ToString());
    }
}

FTerrainSurfaceData UAdvancedFootIKSystemV44::PerformFootTrace(const FFootIKData& FootData)
{
    FTerrainSurfaceData SurfaceData;
    
    if (!SkeletalMeshComponent || !GetWorld())
    {
        return SurfaceData;
    }
    
    // Get foot world location
    FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
    
    // Setup trace
    FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootWorldLocation - FVector(0, 0, FootTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    // Perform trace
    bool bHit = GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        TraceStart,
        TraceEnd,
        FCollisionObjectQueryParams(TraceObjectTypes),
        QueryParams
    );
    
    if (bHit)
    {
        SurfaceData.bIsValidSurface = true;
        SurfaceData.SurfaceLocation = HitResult.Location;
        SurfaceData.SurfaceNormal = HitResult.Normal;
        SurfaceData.SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceData.SurfaceNormal, FVector::UpVector)));
        SurfaceData.SurfaceMaterial = HitResult.PhysMaterial.Get();
        
        // Calculate surface friction
        if (SurfaceData.SurfaceMaterial)
        {
            SurfaceData.SurfaceFriction = SurfaceData.SurfaceMaterial->Friction;
        }
        else
        {
            SurfaceData.SurfaceFriction = 1.0f;
        }
        
        // Check if surface angle is within acceptable range
        if (SurfaceData.SurfaceAngle > MaxSlopeAngle)
        {
            SurfaceData.bIsValidSurface = false;
        }
    }
    
    return SurfaceData;
}

FVector UAdvancedFootIKSystemV44::CalculateKneeTarget(const FFootIKData& FootData, const FVector& FootTargetLocation)
{
    if (!SkeletalMeshComponent || FootData.KneeBoneName.IsNone())
    {
        return FVector::ZeroVector;
    }
    
    // Get current knee location
    FVector KneeLocation = SkeletalMeshComponent->GetBoneLocation(FootData.KneeBoneName, EBoneSpaces::WorldSpace);
    
    // Calculate knee direction based on foot target
    FVector HipLocation = SkeletalMeshComponent->GetBoneLocation(HipBoneName, EBoneSpaces::WorldSpace);
    FVector FootLocation = SkeletalMeshComponent->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
    
    // Calculate desired knee position
    FVector HipToFoot = FootTargetLocation - HipLocation;
    FVector HipToKnee = KneeLocation - HipLocation;
    
    // Project knee position to maintain natural bend
    FVector KneeDirection = FVector::CrossProduct(HipToFoot, SkeletalMeshComponent->GetRightVector()).GetSafeNormal();
    FVector DesiredKneeLocation = HipLocation + (HipToFoot * 0.5f) + (KneeDirection * 30.0f);
    
    return DesiredKneeLocation;
}

FRotator UAdvancedFootIKSystemV44::CalculateFootRotationFromSurface(const FVector& SurfaceNormal, const FVector& ForwardVector)
{
    // Calculate foot rotation to align with surface
    FVector RightVector = FVector::CrossProduct(ForwardVector, SurfaceNormal).GetSafeNormal();
    FVector AdjustedForward = FVector::CrossProduct(SurfaceNormal, RightVector).GetSafeNormal();
    
    return FRotationMatrix::MakeFromXZ(AdjustedForward, SurfaceNormal).Rotator();
}

bool UAdvancedFootIKSystemV44::ShouldPlantFoot(const FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData) const
{
    if (!SurfaceData.bIsValidSurface)
    {
        return false;
    }
    
    float SurfaceDistance = FMath::Abs(FootData.FootOffset.Z);
    return SurfaceDistance <= FootData.PlantingThreshold;
}

bool UAdvancedFootIKSystemV44::ShouldLiftFoot(const FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData) const
{
    if (!SurfaceData.bIsValidSurface)
    {
        return true;
    }
    
    float SurfaceDistance = FMath::Abs(FootData.FootOffset.Z);
    return SurfaceDistance > FootData.LiftingThreshold;
}

void UAdvancedFootIKSystemV44::UpdateKneeTargeting(float DeltaTime)
{
    for (FFootIKData& FootData : FootIKData)
    {
        if (!FootData.KneeBoneName.IsNone() && FootData.IKAlpha > 0.1f)
        {
            FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
            FVector FootTargetLocation = FootWorldLocation + FootData.FootOffset;
            FootData.KneeTargetLocation = CalculateKneeTarget(FootData, FootTargetLocation);
        }
    }
}

void UAdvancedFootIKSystemV44::DrawDebugVisualization() const
{
    if (!GetWorld() || !SkeletalMeshComponent)
    {
        return;
    }
    
    for (const FFootIKData& FootData : FootIKData)
    {
        FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
        
        // Show trace lines
        if (bShowTraceLines)
        {
            FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
            FVector TraceEnd = FootWorldLocation - FVector(0, 0, FootTraceDistance);
            
            const FTerrainSurfaceData* SurfaceData = CachedSurfaceData.Find(FootData.FootBoneName);
            FColor TraceColor = (SurfaceData && SurfaceData->bIsValidSurface) ? FColor::Green : FColor::Red;
            
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 2.0f);
        }
        
        // Show foot targets
        if (bShowFootTargets)
        {
            FVector TargetLocation = FootWorldLocation + FootData.FootOffset;
            FColor TargetColor = FootData.bIsPlanted ? FColor::Blue : FColor::Yellow;
            
            DrawDebugSphere(GetWorld(), TargetLocation, 8.0f, 8, TargetColor, false, 0.1f);
            
            // Show IK alpha as sphere size
            float SphereSize = 5.0f + (FootData.IKAlpha * 10.0f);
            DrawDebugSphere(GetWorld(), TargetLocation, SphereSize, 8, FColor::White, false, 0.1f);
        }
        
        // Show surface normals
        if (bShowSurfaceNormals)
        {
            const FTerrainSurfaceData* SurfaceData = CachedSurfaceData.Find(FootData.FootBoneName);
            if (SurfaceData && SurfaceData->bIsValidSurface)
            {
                FVector NormalStart = SurfaceData->SurfaceLocation;
                FVector NormalEnd = NormalStart + (SurfaceData->SurfaceNormal * 50.0f);
                
                DrawDebugDirectionalArrow(GetWorld(), NormalStart, NormalEnd, 20.0f, FColor::Cyan, false, 0.1f, 0, 3.0f);
            }
        }
        
        // Show knee targets
        if (bEnableKneeTargeting && !FootData.KneeBoneName.IsNone() && FootData.KneeTargetAlpha > 0.1f)
        {
            DrawDebugSphere(GetWorld(), FootData.KneeTargetLocation, 6.0f, 8, FColor::Orange, false, 0.1f);
        }
    }
    
    // Show hip offset
    if (HipOffset.SizeSquared() > 1.0f)
    {
        FVector HipLocation = SkeletalMeshComponent->GetBoneLocation(HipBoneName, EBoneSpaces::WorldSpace);
        FVector HipTarget = HipLocation + HipOffset;
        
        DrawDebugSphere(GetWorld(), HipTarget, 10.0f, 8, FColor::Purple, false, 0.1f);
        DrawDebugLine(GetWorld(), HipLocation, HipTarget, FColor::Purple, false, 0.1f, 0, 3.0f);
    }
}

// Blueprint interface implementations
FVector UAdvancedFootIKSystemV44::GetFootIKOffset(FName FootBoneName) const
{
    for (const FFootIKData& FootData : FootIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            return FootData.FootOffset * FootData.IKAlpha;
        }
    }
    return FVector::ZeroVector;
}

FRotator UAdvancedFootIKSystemV44::GetFootIKRotation(FName FootBoneName) const
{
    for (const FFootIKData& FootData : FootIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            return FootData.FootRotation;
        }
    }
    return FRotator::ZeroRotator;
}

float UAdvancedFootIKSystemV44::GetFootIKAlpha(FName FootBoneName) const
{
    for (const FFootIKData& FootData : FootIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            return FootData.IKAlpha;
        }
    }
    return 0.0f;
}

FVector UAdvancedFootIKSystemV44::GetHipOffset() const
{
    return HipOffset;
}

bool UAdvancedFootIKSystemV44::IsFootPlanted(FName FootBoneName) const
{
    for (const FFootIKData& FootData : FootIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            return FootData.bIsPlanted;
        }
    }
    return false;
}

FTerrainSurfaceData UAdvancedFootIKSystemV44::GetSurfaceDataForFoot(FName FootBoneName) const
{
    if (const FTerrainSurfaceData* SurfaceData = CachedSurfaceData.Find(FootBoneName))
    {
        return *SurfaceData;
    }
    return FTerrainSurfaceData();
}

void UAdvancedFootIKSystemV44::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    UE_LOG(LogAdvancedFootIK, Log, TEXT("IK %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAdvancedFootIKSystemV44::SetFootPlantingEnabled(bool bEnabled)
{
    bEnableFootPlanting = bEnabled;
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Foot planting %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAdvancedFootIKSystemV44::AddFootIKTarget(FName FootBoneName, FName IKBoneName, FName KneeBoneName)
{
    FFootIKData NewFootData;
    NewFootData.FootBoneName = FootBoneName;
    NewFootData.IKBoneName = IKBoneName;
    NewFootData.KneeBoneName = KneeBoneName;
    
    FootIKData.Add(NewFootData);
    CachedSurfaceData.Add(FootBoneName, FTerrainSurfaceData());
    
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Added foot IK target: %s"), *FootBoneName.ToString());
}

void UAdvancedFootIKSystemV44::RemoveFootIKTarget(FName FootBoneName)
{
    FootIKData.RemoveAll([FootBoneName](const FFootIKData& FootData)
    {
        return FootData.FootBoneName == FootBoneName;
    });
    
    CachedSurfaceData.Remove(FootBoneName);
    
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Removed foot IK target: %s"), *FootBoneName.ToString());
}

void UAdvancedFootIKSystemV44::SetFootPlantingThresholds(FName FootBoneName, float PlantingThreshold, float LiftingThreshold)
{
    for (FFootIKData& FootData : FootIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            FootData.PlantingThreshold = PlantingThreshold;
            FootData.LiftingThreshold = LiftingThreshold;
            break;
        }
    }
}

void UAdvancedFootIKSystemV44::EnableSpeedBasedPlanting(bool bEnabled, float MinSpeedForPlanting)
{
    bSpeedBasedPlantingEnabled = bEnabled;
    this->MinSpeedForPlanting = MinSpeedForPlanting;
    
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Speed-based planting %s (min speed: %f)"), 
           bEnabled ? TEXT("Enabled") : TEXT("Disabled"), MinSpeedForPlanting);
}

void UAdvancedFootIKSystemV44::SetSurfaceAdaptationStrength(float Strength)
{
    SurfaceAdaptationStrength = FMath::Clamp(Strength, 0.0f, 2.0f);
    UE_LOG(LogAdvancedFootIK, Log, TEXT("Surface adaptation strength set to %f"), SurfaceAdaptationStrength);
}