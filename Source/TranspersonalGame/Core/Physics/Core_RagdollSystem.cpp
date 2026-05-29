#include "Core_RagdollSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogRagdollSystem);

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentState = ECore_RagdollState::Disabled;
    CurrentBlendWeight = 0.0f;
    BlendTimer = 0.0f;
    bBlendingIn = false;
    
    // Set default configurations
    DefaultConfig.BlendTime = 0.5f;
    DefaultConfig.PhysicsBlendWeight = 1.0f;
    DefaultConfig.bEnableGravity = true;
    DefaultConfig.LinearDamping = 0.1f;
    DefaultConfig.AngularDamping = 0.1f;
    
    DeathConfig.BlendTime = 0.2f;
    DeathConfig.PhysicsBlendWeight = 1.0f;
    DeathConfig.bEnableGravity = true;
    DeathConfig.LinearDamping = 0.05f;
    DeathConfig.AngularDamping = 0.05f;
    
    bAutoActivateOnDeath = true;
    MinImpulseForPartialRagdoll = 500.0f;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeRagdollSystem();
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ECore_RagdollState::Blending)
    {
        UpdateBlending(DeltaTime);
    }
}

void UCore_RagdollSystem::InitializeRagdollSystem()
{
    // Get skeletal mesh component from owner
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("RagdollSystem: No owner found"));
        return;
    }
    
    // Try to get skeletal mesh from Character first
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        SkeletalMeshComp = Character->GetMesh();
    }
    else
    {
        // Fallback to component search
        SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("RagdollSystem: No skeletal mesh component found on %s"), *Owner->GetName());
        return;
    }
    
    // Get physics asset
    PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("RagdollSystem: No physics asset found on skeletal mesh"));
        return;
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Successfully initialized for %s"), *Owner->GetName());
}

void UCore_RagdollSystem::ActivateRagdoll(bool bFullBody)
{
    if (!SkeletalMeshComp || !PhysicsAsset)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("RagdollSystem: Cannot activate ragdoll - missing components"));
        return;
    }
    
    if (CurrentState != ECore_RagdollState::Disabled)
    {
        UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Ragdoll already active"));
        return;
    }
    
    // Set active configuration
    ActiveConfig = DefaultConfig;
    
    // Clear partial ragdoll bones for full body
    if (bFullBody)
    {
        PartialRagdollBones.Empty();
    }
    
    // Start blending
    CurrentState = ECore_RagdollState::Blending;
    bBlendingIn = true;
    BlendTimer = 0.0f;
    
    // Enable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    ApplyRagdollConfiguration();
    
    UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Activated ragdoll (FullBody: %s)"), bFullBody ? TEXT("true") : TEXT("false"));
    
    OnRagdollStateChanged.Broadcast(CurrentState);
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    // Start blending out
    CurrentState = ECore_RagdollState::Blending;
    bBlendingIn = false;
    BlendTimer = 0.0f;
    
    UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Deactivating ragdoll"));
    
    OnRagdollStateChanged.Broadcast(CurrentState);
}

void UCore_RagdollSystem::ActivatePartialRagdoll(const TArray<FName>& BoneNames)
{
    if (!SkeletalMeshComp || !PhysicsAsset)
    {
        UE_LOG(LogRagdollSystem, Warning, TEXT("RagdollSystem: Cannot activate partial ragdoll - missing components"));
        return;
    }
    
    PartialRagdollBones = BoneNames;
    ActiveConfig = DefaultConfig;
    
    CurrentState = ECore_RagdollState::Blending;
    bBlendingIn = true;
    BlendTimer = 0.0f;
    
    // Enable physics on specific bones
    EnablePhysicsOnBones(BoneNames);
    
    UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Activated partial ragdoll on %d bones"), BoneNames.Num());
    
    OnRagdollStateChanged.Broadcast(CurrentState);
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    ActiveConfig = NewConfig;
    
    if (CurrentState != ECore_RagdollState::Disabled)
    {
        ApplyRagdollConfiguration();
    }
}

void UCore_RagdollSystem::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName)
{
    if (!SkeletalMeshComp || CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, SkeletalMeshComp->GetBoneLocation(BoneName), BoneName);
    }
    else
    {
        SkeletalMeshComp->AddImpulse(Impulse);
    }
    
    UE_LOG(LogRagdollSystem, Log, TEXT("RagdollSystem: Applied impulse %s to bone %s"), *Impulse.ToString(), *BoneName.ToString());
}

void UCore_RagdollSystem::UpdateBlending(float DeltaTime)
{
    BlendTimer += DeltaTime;
    
    float BlendAlpha = FMath::Clamp(BlendTimer / ActiveConfig.BlendTime, 0.0f, 1.0f);
    
    if (bBlendingIn)
    {
        CurrentBlendWeight = FMath::Lerp(0.0f, ActiveConfig.PhysicsBlendWeight, BlendAlpha);
        
        if (BlendAlpha >= 1.0f)
        {
            CurrentState = PartialRagdollBones.Num() > 0 ? ECore_RagdollState::Partial : ECore_RagdollState::Full;
            OnRagdollStateChanged.Broadcast(CurrentState);
        }
    }
    else
    {
        CurrentBlendWeight = FMath::Lerp(ActiveConfig.PhysicsBlendWeight, 0.0f, BlendAlpha);
        
        if (BlendAlpha >= 1.0f)
        {
            CurrentState = ECore_RagdollState::Disabled;
            CurrentBlendWeight = 0.0f;
            
            // Disable physics simulation
            SkeletalMeshComp->SetSimulatePhysics(false);
            SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            
            OnRagdollStateChanged.Broadcast(CurrentState);
        }
    }
    
    SetPhysicsBlendWeight(CurrentBlendWeight);
}

void UCore_RagdollSystem::ApplyRagdollConfiguration()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Apply damping settings
    SkeletalMeshComp->SetLinearDamping(ActiveConfig.LinearDamping);
    SkeletalMeshComp->SetAngularDamping(ActiveConfig.AngularDamping);
    
    // Apply gravity setting
    SkeletalMeshComp->SetEnableGravity(ActiveConfig.bEnableGravity);
}

void UCore_RagdollSystem::SetPhysicsBlendWeight(float Weight)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Set physics blend weight on animation instance
    if (UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance())
    {
        // Note: This would typically use a custom animation blueprint node
        // For now, we'll use the skeletal mesh component's physics blend weight
        SkeletalMeshComp->SetPhysicsBlendWeight(Weight);
    }
}

void UCore_RagdollSystem::EnablePhysicsOnBones(const TArray<FName>& BoneNames)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // First disable all physics
    DisablePhysicsOnAllBones();
    
    // Then enable physics on specified bones
    for (const FName& BoneName : BoneNames)
    {
        int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BoneName);
        if (BoneIndex != INDEX_NONE)
        {
            SkeletalMeshComp->SetBodyNotifyRigidBodyCollision(true, BoneName);
        }
    }
}

void UCore_RagdollSystem::DisablePhysicsOnAllBones()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // This would typically iterate through all bodies in the physics asset
    // For now, we'll use the general physics simulation setting
    SkeletalMeshComp->SetSimulatePhysics(false);
}