#include "Core_RagdollPhysics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_RagdollPhysics::UCore_RagdollPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Inactive;
    StateTimer = 0.0f;
    BlendAlpha = 0.0f;
    LastImpactTime = 0.0f;
    LastImpactStrength = 0.0f;
    
    SkeletalMeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UCore_RagdollPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeRagdollSystem();
}

void UCore_RagdollPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRagdollState(DeltaTime);
    
    if (CurrentState == ECore_RagdollState::Blending && RagdollSettings.bUsePhysicsBlending)
    {
        BlendPhysicsAndAnimation(DeltaTime);
    }
}

void UCore_RagdollPhysics::InitializeRagdollSystem()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: No owner found"));
        return;
    }
    
    // Find skeletal mesh component
    SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: No skeletal mesh component found"));
        return;
    }
    
    // Get animation instance
    AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: No animation instance found"));
    }
    
    // Configure physics asset if available
    ConfigurePhysicsAsset();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: System initialized successfully"));
}

void UCore_RagdollPhysics::ConfigurePhysicsAsset()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: No physics asset found"));
        return;
    }
    
    // Apply bone configurations
    for (const FCore_RagdollBoneData& BoneData : BoneConfigurations)
    {
        SetBonePhysicsProperties(BoneData.BoneName, BoneData.Mass, BoneData.LinearDamping, BoneData.AngularDamping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Physics asset configured with %d bone configurations"), BoneConfigurations.Num());
}

void UCore_RagdollPhysics::ActivateRagdoll(ECore_RagdollTrigger Trigger)
{
    if (CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Activating)
    {
        return;
    }
    
    if (!ShouldActivateRagdoll(Trigger))
    {
        return;
    }
    
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysics: Cannot activate ragdoll - no skeletal mesh component"));
        return;
    }
    
    // Cache current animation pose
    CacheAnimationPose();
    
    // Disable collision on character capsule
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Enable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(RagdollSettings.bEnableCollisionWithWorld ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::PhysicsOnly);
    
    if (!RagdollSettings.bEnableCollisionWithPawns)
    {
        SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }
    
    SetRagdollState(ECore_RagdollState::Activating);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Ragdoll activated with trigger: %d"), (int32)Trigger);
}

void UCore_RagdollPhysics::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive || CurrentState == ECore_RagdollState::Deactivating)
    {
        return;
    }
    
    SetRagdollState(ECore_RagdollState::Deactivating);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Ragdoll deactivation started"));
}

void UCore_RagdollPhysics::BlendToAnimation(float BlendTime)
{
    if (CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    RagdollSettings.BlendOutTime = BlendTime;
    SetRagdollState(ECore_RagdollState::Blending);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Blending to animation over %f seconds"), BlendTime);
}

bool UCore_RagdollPhysics::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Blending;
}

ECore_RagdollState UCore_RagdollPhysics::GetRagdollState() const
{
    return CurrentState;
}

void UCore_RagdollPhysics::ApplyImpactForce(const FVector& Force, const FVector& Location, const FName& BoneName)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location);
    }
    
    // Track impact for auto-activation
    LastImpactTime = GetWorld()->GetTimeSeconds();
    LastImpactStrength = Force.Size();
    
    // Check if impact should trigger ragdoll
    if (CurrentState == ECore_RagdollState::Inactive && LastImpactStrength >= RagdollSettings.ActivationThreshold)
    {
        ActivateRagdoll(ECore_RagdollTrigger::ImpactForce);
    }
    
    OnRagdollImpact.Broadcast(Location, LastImpactStrength);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Impact force applied - Strength: %f, Location: %s"), LastImpactStrength, *Location.ToString());
}

void UCore_RagdollPhysics::ApplyRadialImpulse(const FVector& Origin, float Radius, float Strength)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    SkeletalMeshComponent->AddRadialImpulse(Origin, Radius, Strength, RIF_Linear, true);
    
    // Track impact
    LastImpactTime = GetWorld()->GetTimeSeconds();
    LastImpactStrength = Strength;
    
    if (CurrentState == ECore_RagdollState::Inactive && Strength >= RagdollSettings.ActivationThreshold)
    {
        ActivateRagdoll(ECore_RagdollTrigger::ImpactForce);
    }
    
    OnRagdollImpact.Broadcast(Origin, Strength);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Radial impulse applied - Strength: %f, Origin: %s, Radius: %f"), Strength, *Origin.ToString(), Radius);
}

void UCore_RagdollPhysics::SetBonePhysicsProperties(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
    if (BodyInstance)
    {
        BodyInstance->SetMassOverride(Mass, true);
        BodyInstance->LinearDamping = LinearDamping;
        BodyInstance->AngularDamping = AngularDamping;
        BodyInstance->UpdateMassProperties();
    }
}

void UCore_RagdollPhysics::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Settings updated"));
}

FCore_RagdollSettings UCore_RagdollPhysics::GetRagdollSettings() const
{
    return RagdollSettings;
}

void UCore_RagdollPhysics::AddBoneConfiguration(const FCore_RagdollBoneData& BoneData)
{
    BoneConfigurations.Add(BoneData);
    
    // Apply immediately if system is initialized
    if (SkeletalMeshComponent)
    {
        SetBonePhysicsProperties(BoneData.BoneName, BoneData.Mass, BoneData.LinearDamping, BoneData.AngularDamping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Bone configuration added for %s"), *BoneData.BoneName.ToString());
}

void UCore_RagdollPhysics::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Activating:
            if (StateTimer >= RagdollSettings.BlendInTime)
            {
                SetRagdollState(ECore_RagdollState::Active);
            }
            break;
            
        case ECore_RagdollState::Active:
            // Check for auto-deactivation
            if (RagdollSettings.DeactivationDelay > 0.0f && StateTimer >= RagdollSettings.DeactivationDelay)
            {
                DeactivateRagdoll();
            }
            break;
            
        case ECore_RagdollState::Blending:
            BlendAlpha = FMath::Clamp(StateTimer / RagdollSettings.BlendOutTime, 0.0f, 1.0f);
            if (BlendAlpha >= 1.0f)
            {
                SetRagdollState(ECore_RagdollState::Deactivating);
            }
            break;
            
        case ECore_RagdollState::Deactivating:
            if (StateTimer >= 0.1f) // Small delay for cleanup
            {
                // Restore character collision
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
                    {
                        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                }
                
                // Disable physics simulation
                if (SkeletalMeshComponent)
                {
                    SkeletalMeshComponent->SetSimulatePhysics(false);
                    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                
                RestoreAnimationPose();
                SetRagdollState(ECore_RagdollState::Inactive);
            }
            break;
    }
}

void UCore_RagdollPhysics::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        BlendAlpha = 0.0f;
        
        OnRagdollStateChanged.Broadcast(CurrentState);
        
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: State changed to %d"), (int32)CurrentState);
    }
}

void UCore_RagdollPhysics::BlendPhysicsAndAnimation(float DeltaTime)
{
    if (!SkeletalMeshComponent || !AnimInstance)
    {
        return;
    }
    
    // This would require more complex implementation with animation blueprints
    // For now, we'll use a simple physics weight approach
    float PhysicsWeight = FMath::Lerp(RagdollSettings.PhysicsBlendWeight, 0.0f, BlendAlpha);
    SkeletalMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(NAME_None, PhysicsWeight);
}

bool UCore_RagdollPhysics::ShouldActivateRagdoll(ECore_RagdollTrigger Trigger) const
{
    switch (Trigger)
    {
        case ECore_RagdollTrigger::Manual:
            return true;
            
        case ECore_RagdollTrigger::ImpactForce:
            return LastImpactStrength >= RagdollSettings.ActivationThreshold;
            
        case ECore_RagdollTrigger::Death:
        case ECore_RagdollTrigger::Stun:
            return true;
            
        case ECore_RagdollTrigger::HealthThreshold:
            // Would need health component integration
            return true;
            
        default:
            return false;
    }
}

void UCore_RagdollPhysics::CacheAnimationPose()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    CachedBoneTransforms.Empty();
    CachedBoneVelocities.Empty();
    
    // Cache current bone transforms and velocities
    const TArray<FTransform>& BoneTransforms = SkeletalMeshComponent->GetComponentSpaceTransforms();
    for (int32 BoneIndex = 0; BoneIndex < BoneTransforms.Num(); ++BoneIndex)
    {
        FName BoneName = SkeletalMeshComponent->GetBoneName(BoneIndex);
        CachedBoneTransforms.Add(BoneName, BoneTransforms[BoneIndex]);
        
        // Get bone velocity if physics is enabled
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            CachedBoneVelocities.Add(BoneName, BodyInstance->GetUnrealWorldVelocity());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Cached %d bone transforms"), CachedBoneTransforms.Num());
}

void UCore_RagdollPhysics::RestoreAnimationPose()
{
    if (!SkeletalMeshComponent || !AnimInstance)
    {
        return;
    }
    
    // Restore animation control
    // This would typically involve resetting animation blueprint state
    // For now, we'll just ensure physics is disabled
    SkeletalMeshComponent->SetSimulatePhysics(false);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysics: Animation pose restored"));
}