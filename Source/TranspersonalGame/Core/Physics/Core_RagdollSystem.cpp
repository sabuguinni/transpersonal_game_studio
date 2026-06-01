#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    TargetMesh = nullptr;
    ActivationBlendTime = 0.5f;
    DeactivationBlendTime = 1.0f;
    bAutoSetupConstraints = true;
    GlobalMassScale = 1.0f;
    GlobalDampingScale = 1.0f;
    
    CurrentState = ECore_RagdollState::Inactive;
    StateTransitionTime = 0.0f;
    TransitionTimer = 0.0f;
    bRagdollInitialized = false;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-detect skeletal mesh if not set
    if (!TargetMesh)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            TargetMesh = Character->GetMesh();
        }
        else
        {
            TargetMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (TargetMesh)
    {
        InitializeRagdollBones();
        if (bAutoSetupConstraints)
        {
            SetupRagdollConstraints();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No target mesh found on %s"), *GetOwner()->GetName());
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ECore_RagdollState::Activating || CurrentState == ECore_RagdollState::Deactivating)
    {
        UpdateStateTransition(DeltaTime);
    }
}

void UCore_RagdollSystem::ActivateRagdoll()
{
    if (!TargetMesh || CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Activating)
    {
        return;
    }
    
    // Store pre-ragdoll bone transforms
    PreRagdollBoneTransforms.Empty();
    if (TargetMesh->GetSkeletalMeshAsset())
    {
        const int32 BoneCount = TargetMesh->GetNumBones();
        PreRagdollBoneTransforms.Reserve(BoneCount);
        
        for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
        {
            PreRagdollBoneTransforms.Add(TargetMesh->GetBoneTransform(BoneIndex));
        }
    }
    
    CurrentState = ECore_RagdollState::Activating;
    TransitionTimer = 0.0f;
    StateTransitionTime = ActivationBlendTime;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Activating ragdoll on %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!TargetMesh || CurrentState == ECore_RagdollState::Inactive || CurrentState == ECore_RagdollState::Deactivating)
    {
        return;
    }
    
    CurrentState = ECore_RagdollState::Deactivating;
    TransitionTimer = 0.0f;
    StateTransitionTime = DeactivationBlendTime;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Deactivating ragdoll on %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::SetRagdollImpulse(const FVector& Impulse, const FName& BoneName)
{
    if (!TargetMesh || !IsRagdollActive())
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        // Apply impulse to specific bone
        TargetMesh->AddImpulseAtLocation(Impulse, TargetMesh->GetBoneLocation(BoneName), BoneName);
    }
    else
    {
        // Apply impulse to root bone or center of mass
        FVector CenterOfMass = TargetMesh->GetCenterOfMass();
        TargetMesh->AddImpulseAtLocation(Impulse, CenterOfMass);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applied impulse %s to bone %s"), *Impulse.ToString(), *BoneName.ToString());
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active;
}

void UCore_RagdollSystem::SetupRagdollConstraints()
{
    if (!TargetMesh || !TargetMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Cannot setup constraints - no physics asset"));
        return;
    }
    
    // Clear existing constraints
    RagdollConstraints.Empty();
    
    // Auto-generate common constraints for humanoid skeleton
    TArray<FName> CommonBones = {
        TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
        TEXT("neck_01"), TEXT("head"),
        TEXT("clavicle_l"), TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
        TEXT("clavicle_r"), TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"),
        TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
        TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r")
    };
    
    // Create parent-child constraint pairs
    TArray<TPair<FName, FName>> ConstraintPairs = {
        {TEXT("pelvis"), TEXT("spine_01")},
        {TEXT("spine_01"), TEXT("spine_02")},
        {TEXT("spine_02"), TEXT("spine_03")},
        {TEXT("spine_03"), TEXT("neck_01")},
        {TEXT("neck_01"), TEXT("head")},
        {TEXT("spine_03"), TEXT("clavicle_l")},
        {TEXT("clavicle_l"), TEXT("upperarm_l")},
        {TEXT("upperarm_l"), TEXT("lowerarm_l")},
        {TEXT("lowerarm_l"), TEXT("hand_l")},
        {TEXT("spine_03"), TEXT("clavicle_r")},
        {TEXT("clavicle_r"), TEXT("upperarm_r")},
        {TEXT("upperarm_r"), TEXT("lowerarm_r")},
        {TEXT("lowerarm_r"), TEXT("hand_r")},
        {TEXT("pelvis"), TEXT("thigh_l")},
        {TEXT("thigh_l"), TEXT("calf_l")},
        {TEXT("calf_l"), TEXT("foot_l")},
        {TEXT("pelvis"), TEXT("thigh_r")},
        {TEXT("thigh_r"), TEXT("calf_r")},
        {TEXT("calf_r"), TEXT("foot_r")}
    };
    
    for (const auto& Pair : ConstraintPairs)
    {
        FCore_RagdollConstraint Constraint;
        Constraint.ParentBone = Pair.Key;
        Constraint.ChildBone = Pair.Value;
        
        // Set different limits based on bone type
        if (Pair.Value.ToString().Contains(TEXT("head")) || Pair.Value.ToString().Contains(TEXT("neck")))
        {
            Constraint.AngularLimit = 30.0f;
            Constraint.Strength = 1500.0f;
        }
        else if (Pair.Value.ToString().Contains(TEXT("spine")))
        {
            Constraint.AngularLimit = 20.0f;
            Constraint.Strength = 2000.0f;
        }
        else
        {
            Constraint.AngularLimit = 45.0f;
            Constraint.Strength = 1000.0f;
        }
        
        RagdollConstraints.Add(Constraint);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Setup %d ragdoll constraints"), RagdollConstraints.Num());
}

void UCore_RagdollSystem::ConfigureBonePhysics(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping)
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Find or create bone configuration
    FCore_RagdollBone* ExistingBone = RagdollBones.FindByPredicate([BoneName](const FCore_RagdollBone& Bone)
    {
        return Bone.BoneName == BoneName;
    });
    
    if (ExistingBone)
    {
        ExistingBone->Mass = Mass * GlobalMassScale;
        ExistingBone->LinearDamping = LinearDamping * GlobalDampingScale;
        ExistingBone->AngularDamping = AngularDamping * GlobalDampingScale;
    }
    else
    {
        FCore_RagdollBone NewBone;
        NewBone.BoneName = BoneName;
        NewBone.Mass = Mass * GlobalMassScale;
        NewBone.LinearDamping = LinearDamping * GlobalDampingScale;
        NewBone.AngularDamping = AngularDamping * GlobalDampingScale;
        RagdollBones.Add(NewBone);
    }
    
    // Apply physics settings if ragdoll is active
    if (IsRagdollActive())
    {
        TargetMesh->SetBoneLinearDamping(BoneName, LinearDamping * GlobalDampingScale);
        TargetMesh->SetBoneAngularDamping(BoneName, AngularDamping * GlobalDampingScale);
    }
}

void UCore_RagdollSystem::InitializeRagdollBones()
{
    if (!TargetMesh || !TargetMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Initialize default bone configurations for common bones
    TArray<TPair<FName, float>> DefaultBoneMasses = {
        {TEXT("pelvis"), 15.0f},
        {TEXT("spine_01"), 8.0f},
        {TEXT("spine_02"), 8.0f},
        {TEXT("spine_03"), 8.0f},
        {TEXT("neck_01"), 3.0f},
        {TEXT("head"), 5.0f},
        {TEXT("upperarm_l"), 4.0f},
        {TEXT("upperarm_r"), 4.0f},
        {TEXT("lowerarm_l"), 3.0f},
        {TEXT("lowerarm_r"), 3.0f},
        {TEXT("hand_l"), 1.0f},
        {TEXT("hand_r"), 1.0f},
        {TEXT("thigh_l"), 10.0f},
        {TEXT("thigh_r"), 10.0f},
        {TEXT("calf_l"), 6.0f},
        {TEXT("calf_r"), 6.0f},
        {TEXT("foot_l"), 2.0f},
        {TEXT("foot_r"), 2.0f}
    };
    
    for (const auto& BoneMass : DefaultBoneMasses)
    {
        ConfigureBonePhysics(BoneMass.Key, BoneMass.Value, 0.1f, 0.1f);
    }
    
    bRagdollInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized %d ragdoll bones"), RagdollBones.Num());
}

void UCore_RagdollSystem::UpdateStateTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / StateTransitionTime, 0.0f, 1.0f);
    
    if (CurrentState == ECore_RagdollState::Activating)
    {
        BlendToRagdoll(Alpha);
        
        if (Alpha >= 1.0f)
        {
            CurrentState = ECore_RagdollState::Active;
            if (TargetMesh)
            {
                TargetMesh->SetSimulatePhysics(true);
                TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    else if (CurrentState == ECore_RagdollState::Deactivating)
    {
        BlendFromRagdoll(1.0f - Alpha);
        
        if (Alpha >= 1.0f)
        {
            CurrentState = ECore_RagdollState::Inactive;
            if (TargetMesh)
            {
                TargetMesh->SetSimulatePhysics(false);
                TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }
    }
}

void UCore_RagdollSystem::BlendToRagdoll(float Alpha)
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Gradually enable physics simulation
    if (Alpha > 0.5f && !TargetMesh->IsSimulatingPhysics())
    {
        TargetMesh->SetSimulatePhysics(true);
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Apply bone physics settings
        for (const FCore_RagdollBone& Bone : RagdollBones)
        {
            TargetMesh->SetBoneLinearDamping(Bone.BoneName, Bone.LinearDamping);
            TargetMesh->SetBoneAngularDamping(Bone.BoneName, Bone.AngularDamping);
        }
    }
}

void UCore_RagdollSystem::BlendFromRagdoll(float Alpha)
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Gradually disable physics simulation
    if (Alpha < 0.5f && TargetMesh->IsSimulatingPhysics())
    {
        TargetMesh->SetSimulatePhysics(false);
        TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}