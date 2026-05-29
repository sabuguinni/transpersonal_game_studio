#include "AdaptiveIKSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Animation/AnimInstance.h"

UAdaptiveIKSystem::UAdaptiveIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    SetComponentTickEnabled(true);
    
    bTerrainAdaptationEnabled = true;
    bDebugMode = false;
    bIsCharacterMoving = false;
    CurrentMovementSpeed = 0.0f;
    CurrentStance = "Default";
    
    UpdateFrequency = 60.0f; // High frequency for smooth IK
    LastUpdateTime = 0.0f;
    IKSmoothingSpeed = 10.0f;
    
    // Initialize default terrain configuration
    TerrainConfig = FTerrainAdaptationConfig();
}

void UAdaptiveIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    // Set up default foot IK targets for humanoid characters
    AddIKTarget(FName("foot_l"), 1.0f);
    AddIKTarget(FName("foot_r"), 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Adaptive IK System initialized with %d IK targets"), IKTargets.Num());
}

void UAdaptiveIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!ShouldUpdateThisFrame() || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Update character movement state
    if (MovementComponent)
    {
        CurrentMovementSpeed = MovementComponent->Velocity.Size();
        bIsCharacterMoving = CurrentMovementSpeed > 1.0f;
    }
    
    // Update all IK targets
    UpdateIKTargets(DeltaTime);
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAdaptiveIKSystem::SetTerrainAdaptationConfig(const FTerrainAdaptationConfig& Config)
{
    TerrainConfig = Config;
    UE_LOG(LogTemp, Log, TEXT("Terrain adaptation configuration updated"));
}

void UAdaptiveIKSystem::AddIKTarget(const FName& BoneName, float Weight)
{
    // Check if target already exists
    if (FindIKTarget(BoneName))
    {
        UE_LOG(LogTemp, Warning, TEXT("IK Target already exists for bone: %s"), *BoneName.ToString());
        return;
    }
    
    FIKTarget NewTarget;
    NewTarget.BoneName = BoneName;
    NewTarget.IKWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    NewTarget.bIsActive = true;
    
    // Get initial bone position
    if (SkeletalMeshComponent)
    {
        NewTarget.OriginalLocation = GetBoneWorldLocation(BoneName);
        NewTarget.TargetLocation = NewTarget.OriginalLocation;
    }
    
    IKTargets.Add(NewTarget);
    
    UE_LOG(LogTemp, Log, TEXT("Added IK target for bone: %s"), *BoneName.ToString());
}

void UAdaptiveIKSystem::RemoveIKTarget(const FName& BoneName)
{
    IKTargets.RemoveAll([BoneName](const FIKTarget& Target)
    {
        return Target.BoneName == BoneName;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Removed IK target for bone: %s"), *BoneName.ToString());
}

void UAdaptiveIKSystem::SetIKTargetWeight(const FName& BoneName, float Weight)
{
    if (FIKTarget* Target = FindIKTarget(BoneName))
    {
        Target->IKWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    }
}

FIKTarget UAdaptiveIKSystem::GetIKTarget(const FName& BoneName) const
{
    if (const FIKTarget* Target = FindIKTarget(BoneName))
    {
        return *Target;
    }
    
    return FIKTarget(); // Return empty target if not found
}

TArray<FIKTarget> UAdaptiveIKSystem::GetAllIKTargets() const
{
    return IKTargets;
}

void UAdaptiveIKSystem::EnableTerrainAdaptation(bool bEnable)
{
    bTerrainAdaptationEnabled = bEnable;
    
    if (!bEnable)
    {
        // Reset all targets to original positions
        for (FIKTarget& Target : IKTargets)
        {
            Target.TargetLocation = Target.OriginalLocation;
            Target.TargetRotation = FRotator::ZeroRotator;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Terrain adaptation %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAdaptiveIKSystem::SetManualIKTarget(const FName& BoneName, const FVector& WorldLocation, const FRotator& WorldRotation, float Weight)
{
    if (FIKTarget* Target = FindIKTarget(BoneName))
    {
        Target->TargetLocation = WorldLocation;
        Target->TargetRotation = WorldRotation;
        Target->IKWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
        Target->bIsActive = true;
        
        // Calculate distance from original
        Target->DistanceFromOriginal = FVector::Dist(Target->TargetLocation, Target->OriginalLocation);
    }
}

void UAdaptiveIKSystem::ClearManualIKTarget(const FName& BoneName)
{
    if (FIKTarget* Target = FindIKTarget(BoneName))
    {
        Target->TargetLocation = Target->OriginalLocation;
        Target->TargetRotation = FRotator::ZeroRotator;
        Target->DistanceFromOriginal = 0.0f;
    }
}

bool UAdaptiveIKSystem::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!GetWorld())
        return false;
        
    FVector TraceStart = StartLocation + FVector(0, 0, 50); // Start slightly above
    FVector TraceEnd = StartLocation - FVector(0, 0, TerrainConfig.MaxTraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        TerrainConfig.GroundTraceChannel,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        
        if (bDebugMode)
        {
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Red, false, 0.1f, 0, 1.0f);
        }
        
        return true;
    }
    
    if (bDebugMode)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
    }
    
    return false;
}

float UAdaptiveIKSystem::GetGroundHeight(const FVector& Location) const
{
    FVector HitLocation;
    FVector HitNormal;
    
    if (TraceForGround(Location, HitLocation, HitNormal))
    {
        return HitLocation.Z;
    }
    
    return Location.Z; // Return original height if no ground found
}

void UAdaptiveIKSystem::SetCharacterMovementState(bool bIsMoving, float MovementSpeed)
{
    bIsCharacterMoving = bIsMoving;
    CurrentMovementSpeed = MovementSpeed;
}

void UAdaptiveIKSystem::SetCharacterStance(const FString& StanceName)
{
    CurrentStance = StanceName;
}

FVector UAdaptiveIKSystem::GetIKTargetLocation(const FName& BoneName) const
{
    if (const FIKTarget* Target = FindIKTarget(BoneName))
    {
        return Target->TargetLocation;
    }
    
    return FVector::ZeroVector;
}

FRotator UAdaptiveIKSystem::GetIKTargetRotation(const FName& BoneName) const
{
    if (const FIKTarget* Target = FindIKTarget(BoneName))
    {
        return Target->TargetRotation;
    }
    
    return FRotator::ZeroRotator;
}

float UAdaptiveIKSystem::GetIKTargetWeight(const FName& BoneName) const
{
    if (const FIKTarget* Target = FindIKTarget(BoneName))
    {
        return Target->IKWeight;
    }
    
    return 0.0f;
}

void UAdaptiveIKSystem::DebugDrawIKTargets() const
{
    if (!GetWorld() || !bDebugMode)
        return;
        
    for (const FIKTarget& Target : IKTargets)
    {
        if (!Target.bIsActive)
            continue;
            
        // Draw target location
        DrawDebugSphere(GetWorld(), Target.TargetLocation, 8.0f, 12, FColor::Blue, false, -1.0f, 0, 2.0f);
        
        // Draw line from original to target
        DrawDebugLine(GetWorld(), Target.OriginalLocation, Target.TargetLocation, FColor::Yellow, false, -1.0f, 0, 1.0f);
        
        // Draw bone name
        DrawDebugString(GetWorld(), Target.TargetLocation + FVector(0, 0, 20), Target.BoneName.ToString(), nullptr, FColor::White, -1.0f);
    }
}

void UAdaptiveIKSystem::SetDebugMode(bool bEnable)
{
    bDebugMode = bEnable;
    UE_LOG(LogTemp, Log, TEXT("IK Debug mode %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FString UAdaptiveIKSystem::GetDebugString() const
{
    FString DebugInfo = FString::Printf(TEXT("Adaptive IK System - Targets: %d\n"), IKTargets.Num());
    DebugInfo += FString::Printf(TEXT("Terrain Adaptation: %s\n"), bTerrainAdaptationEnabled ? TEXT("ON") : TEXT("OFF"));
    DebugInfo += FString::Printf(TEXT("Character Moving: %s (Speed: %.2f)\n"), bIsCharacterMoving ? TEXT("YES") : TEXT("NO"), CurrentMovementSpeed);
    DebugInfo += FString::Printf(TEXT("Current Stance: %s\n"), *CurrentStance);
    
    for (const FIKTarget& Target : IKTargets)
    {
        DebugInfo += FString::Printf(TEXT("%s: Weight %.2f, Distance %.2f\n"), 
            *Target.BoneName.ToString(), Target.IKWeight, Target.DistanceFromOriginal);
    }
    
    return DebugInfo;
}

void UAdaptiveIKSystem::UpdateIKTargets(float DeltaTime)
{
    for (FIKTarget& Target : IKTargets)
    {
        if (!Target.bIsActive)
            continue;
            
        // Update original bone position (in case character moved)
        Target.OriginalLocation = GetBoneWorldLocation(Target.BoneName);
        
        if (bTerrainAdaptationEnabled)
        {
            ProcessTerrainAdaptation(Target, DeltaTime);
        }
        
        // Apply smoothing
        SmoothIKTransitions(Target, DeltaTime);
        
        // Calculate distance from original
        Target.DistanceFromOriginal = FVector::Dist(Target.TargetLocation, Target.OriginalLocation);
    }
}

void UAdaptiveIKSystem::ProcessTerrainAdaptation(FIKTarget& Target, float DeltaTime)
{
    FVector PredictedLocation = PredictFootPlacement(Target);
    FVector GroundLocation;
    FVector GroundNormal;
    
    if (TraceForGround(PredictedLocation, GroundLocation, GroundNormal))
    {
        // Calculate height difference
        float HeightDifference = GroundLocation.Z - Target.OriginalLocation.Z;
        
        // Check if adaptation is needed
        if (FMath::Abs(HeightDifference) > 1.0f) // Minimum threshold
        {
            // Clamp to maximum adaptation height
            HeightDifference = FMath::Clamp(HeightDifference, -TerrainConfig.MaxAdaptationHeight, TerrainConfig.MaxAdaptationHeight);
            
            // Set target location
            FVector NewTargetLocation = Target.OriginalLocation;
            NewTargetLocation.Z += HeightDifference;
            Target.TargetLocation = NewTargetLocation;
            
            // Calculate rotation based on ground normal
            Target.TargetRotation = CalculateGroundRotation(GroundNormal);
            
            // Adjust weight based on movement state
            Target.IKWeight = CalculateAdaptationWeight(Target);
        }
        else
        {
            // Reset to original position if no significant height difference
            Target.TargetLocation = Target.OriginalLocation;
            Target.TargetRotation = FRotator::ZeroRotator;
        }
    }
}

void UAdaptiveIKSystem::SmoothIKTransitions(FIKTarget& Target, float DeltaTime)
{
    FName BoneName = Target.BoneName;
    
    // Get previous values
    FVector PreviousLocation = PreviousTargetLocations.FindRef(BoneName);
    FRotator PreviousRotation = PreviousTargetRotations.FindRef(BoneName);
    
    if (PreviousLocation.IsNearlyZero())
    {
        // First frame, no smoothing needed
        PreviousTargetLocations.Add(BoneName, Target.TargetLocation);
        PreviousTargetRotations.Add(BoneName, Target.TargetRotation);
        return;
    }
    
    // Apply smoothing
    float SmoothingAlpha = FMath::Clamp(IKSmoothingSpeed * DeltaTime, 0.0f, 1.0f);
    
    Target.TargetLocation = FMath::VInterpTo(PreviousLocation, Target.TargetLocation, DeltaTime, IKSmoothingSpeed);
    Target.TargetRotation = FMath::RInterpTo(PreviousRotation, Target.TargetRotation, DeltaTime, IKSmoothingSpeed);
    
    // Update previous values
    PreviousTargetLocations[BoneName] = Target.TargetLocation;
    PreviousTargetRotations[BoneName] = Target.TargetRotation;
}

FVector UAdaptiveIKSystem::GetBoneWorldLocation(const FName& BoneName) const
{
    if (!SkeletalMeshComponent)
        return FVector::ZeroVector;
        
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
        return FVector::ZeroVector;
        
    return SkeletalMeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
}

FRotator UAdaptiveIKSystem::CalculateGroundRotation(const FVector& GroundNormal) const
{
    // Calculate rotation to align with ground normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
    
    return FRotationMatrix::MakeFromXZ(ForwardVector, GroundNormal).Rotator();
}

FIKTarget* UAdaptiveIKSystem::FindIKTarget(const FName& BoneName)
{
    return IKTargets.FindByPredicate([BoneName](const FIKTarget& Target)
    {
        return Target.BoneName == BoneName;
    });
}

const FIKTarget* UAdaptiveIKSystem::FindIKTarget(const FName& BoneName) const
{
    return IKTargets.FindByPredicate([BoneName](const FIKTarget& Target)
    {
        return Target.BoneName == BoneName;
    });
}

bool UAdaptiveIKSystem::ShouldUpdateThisFrame() const
{
    if (!GetWorld())
        return false;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastUpdate = CurrentTime - LastUpdateTime;
    float UpdateInterval = 1.0f / UpdateFrequency;
    
    return TimeSinceLastUpdate >= UpdateInterval;
}

void UAdaptiveIKSystem::CacheComponentReferences()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        SkeletalMeshComponent = Character->GetMesh();
        MovementComponent = Character->GetCharacterMovement();
    }
}

FVector UAdaptiveIKSystem::PredictFootPlacement(const FIKTarget& Target) const
{
    FVector PredictedLocation = Target.OriginalLocation;
    
    if (bIsCharacterMoving && MovementComponent)
    {
        // Predict where the foot will be based on movement velocity
        FVector MovementDirection = MovementComponent->Velocity.GetSafeNormal();
        float PredictionDistance = FMath::Clamp(CurrentMovementSpeed * 0.1f, 0.0f, TerrainConfig.PredictionDistance);
        
        PredictedLocation += MovementDirection * PredictionDistance;
    }
    
    return PredictedLocation;
}

float UAdaptiveIKSystem::CalculateAdaptationWeight(const FIKTarget& Target) const
{
    float Weight = 1.0f;
    
    // Reduce IK influence when moving fast
    if (bIsCharacterMoving)
    {
        float SpeedFactor = FMath::Clamp(CurrentMovementSpeed / 600.0f, 0.0f, 1.0f); // 600 = fast run speed
        Weight *= FMath::Lerp(1.0f, 0.3f, SpeedFactor);
    }
    
    // Reduce IK influence for large adaptations
    float DistanceFactor = FMath::Clamp(Target.DistanceFromOriginal / TerrainConfig.MaxAdaptationHeight, 0.0f, 1.0f);
    Weight *= FMath::Lerp(1.0f, 0.5f, DistanceFactor);
    
    return Weight;
}