#include "RagdollPhysicsComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "PhysicsEngine/BodyInstance.h"

URagdollPhysicsComponent::URagdollPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Inactive;
    StateTimer = 0.0f;
    TransitionTimer = 0.0f;
    bWasSimulatingPhysics = false;
    
    SkeletalMeshComponent = nullptr;
    CachedPhysicsAsset = nullptr;
}

void URagdollPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            CachedPhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
            
            // Cache physics bone names
            if (CachedPhysicsAsset)
            {
                PhysicsBones.Empty();
                for (int32 i = 0; i < CachedPhysicsAsset->SkeletalBodySetups.Num(); i++)
                {
                    if (CachedPhysicsAsset->SkeletalBodySetups[i])
                    {
                        PhysicsBones.Add(CachedPhysicsAsset->SkeletalBodySetups[i]->BoneName);
                    }
                }
            }
        }
    }
}

void URagdollPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    UpdateRagdollState(DeltaTime);
}

void URagdollPhysicsComponent::ActivateRagdoll(const FCore_RagdollTransition& Transition)
{
    if (!SkeletalMeshComponent || CurrentState == ECore_RagdollState::Active)
    {
        return;
    }
    
    CurrentTransition = Transition;
    TransitionTimer = 0.0f;
    
    // Cache current bone transforms
    CacheBoneTransforms();
    
    // Start transition
    CurrentState = ECore_RagdollState::Transitioning;
    StateTimer = 0.0f;
    
    OnRagdollStateChanged.Broadcast(CurrentState);
    
    // Apply impact force if specified
    if (!Transition.ImpactForce.IsZero())
    {
        ApplyImpulseToRagdoll(Transition.ImpactForce);
        OnRagdollImpact.Broadcast(GetOwner()->GetActorLocation(), Transition.ImpactForce.Size());
    }
}

void URagdollPhysicsComponent::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    CurrentState = ECore_RagdollState::Recovering;
    StateTimer = 0.0f;
    TransitionTimer = 0.0f;
    
    OnRagdollStateChanged.Broadcast(CurrentState);
}

void URagdollPhysicsComponent::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName)
{
    if (!SkeletalMeshComponent || !IsRagdollActive())
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        // Apply to specific bone
        SkeletalMeshComponent->AddImpulseAtLocation(Impulse, SkeletalMeshComponent->GetBoneLocation(BoneName), BoneName);
    }
    else
    {
        // Apply to root bone or center of mass
        FName RootBone = SkeletalMeshComponent->GetBoneName(0);
        if (RootBone != NAME_None)
        {
            SkeletalMeshComponent->AddImpulseAtLocation(Impulse, SkeletalMeshComponent->GetBoneLocation(RootBone), RootBone);
        }
    }
}

void URagdollPhysicsComponent::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
    
    if (IsRagdollActive())
    {
        UpdatePhysicsSettings();
    }
}

void URagdollPhysicsComponent::SetBonePhysicsSettings(const FName& BoneName, float LinearDamping, float AngularDamping)
{
    if (!SkeletalMeshComponent || !IsRagdollActive())
    {
        return;
    }
    
    FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
    if (BodyInstance)
    {
        BodyInstance->LinearDamping = LinearDamping;
        BodyInstance->AngularDamping = AngularDamping;
    }
}

FVector URagdollPhysicsComponent::GetBoneVelocity(const FName& BoneName) const
{
    if (!SkeletalMeshComponent || !IsRagdollActive())
    {
        return FVector::ZeroVector;
    }
    
    FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
    if (BodyInstance)
    {
        return BodyInstance->GetUnrealWorldVelocity();
    }
    
    return FVector::ZeroVector;
}

void URagdollPhysicsComponent::FreezeRagdoll()
{
    if (!SkeletalMeshComponent || !IsRagdollActive())
    {
        return;
    }
    
    // Set all bodies to kinematic
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            BodyInstance->SetInstanceSimulatePhysics(false);
        }
    }
}

void URagdollPhysicsComponent::UnfreezeRagdoll()
{
    if (!SkeletalMeshComponent || !IsRagdollActive())
    {
        return;
    }
    
    // Restore physics simulation
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            BodyInstance->SetInstanceSimulatePhysics(true);
        }
    }
    
    UpdatePhysicsSettings();
}

void URagdollPhysicsComponent::UpdateRagdollState(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
        {
            TransitionTimer += DeltaTime;
            if (TransitionTimer >= CurrentTransition.TransitionDuration)
            {
                TransitionToRagdoll();
                CurrentState = ECore_RagdollState::Active;
                StateTimer = 0.0f;
                OnRagdollStateChanged.Broadcast(CurrentState);
            }
            break;
        }
        
        case ECore_RagdollState::Active:
        {
            if (RagdollSettings.bAutoRecover && ShouldAutoRecover())
            {
                DeactivateRagdoll();
            }
            break;
        }
        
        case ECore_RagdollState::Recovering:
        {
            TransitionTimer += DeltaTime;
            if (TransitionTimer >= RagdollSettings.RecoveryTime)
            {
                TransitionFromRagdoll();
                CurrentState = ECore_RagdollState::Inactive;
                StateTimer = 0.0f;
                OnRagdollStateChanged.Broadcast(CurrentState);
            }
            break;
        }
        
        default:
            break;
    }
}

void URagdollPhysicsComponent::TransitionToRagdoll()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Disable collision on character capsule if it's a character
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Enable physics simulation
    bWasSimulatingPhysics = SkeletalMeshComponent->IsSimulatingPhysics();
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    UpdatePhysicsSettings();
}

void URagdollPhysicsComponent::TransitionFromRagdoll()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable character collision
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    // Optionally restore bone transforms
    RestoreBoneTransforms();
}

void URagdollPhysicsComponent::UpdatePhysicsSettings()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Apply settings to all physics bodies
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            BodyInstance->LinearDamping = RagdollSettings.LinearDamping;
            BodyInstance->AngularDamping = RagdollSettings.AngularDamping;
            BodyInstance->SetMassScale(RagdollSettings.MassScale);
        }
    }
}

void URagdollPhysicsComponent::CacheBoneTransforms()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    PreRagdollBoneTransforms.Empty();
    
    for (const FName& BoneName : PhysicsBones)
    {
        FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(SkeletalMeshComponent->GetBoneIndex(BoneName));
        PreRagdollBoneTransforms.Add(BoneName, BoneTransform);
    }
}

void URagdollPhysicsComponent::RestoreBoneTransforms()
{
    if (!SkeletalMeshComponent || PreRagdollBoneTransforms.Num() == 0)
    {
        return;
    }
    
    // This would require custom animation blending - simplified for now
    // In a full implementation, you'd blend back to the cached poses
}

bool URagdollPhysicsComponent::ShouldAutoRecover() const
{
    if (StateTimer < RagdollSettings.RecoveryTime)
    {
        return false;
    }
    
    float CurrentVelocity = CalculateRagdollVelocity();
    return CurrentVelocity < RagdollSettings.MinVelocityThreshold;
}

float URagdollPhysicsComponent::CalculateRagdollVelocity() const
{
    if (!SkeletalMeshComponent || PhysicsBones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalVelocity = 0.0f;
    int32 ValidBones = 0;
    
    for (const FName& BoneName : PhysicsBones)
    {
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(BoneName);
        if (BodyInstance)
        {
            TotalVelocity += BodyInstance->GetUnrealWorldVelocity().Size();
            ValidBones++;
        }
    }
    
    return ValidBones > 0 ? TotalVelocity / ValidBones : 0.0f;
}