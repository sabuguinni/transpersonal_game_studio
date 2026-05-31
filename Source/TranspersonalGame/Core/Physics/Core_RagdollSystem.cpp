#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Disabled;
    ActiveProfileName = TEXT("Default");
    RagdollActiveTime = 0.0f;
    BlendTimer = 0.0f;
    bAutoBlendBack = true;
    AutoBlendBackDelay = 3.0f;
    bPreserveMomentum = true;
    MomentumScale = 1.0f;
    SkeletalMeshComponent = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component automatically
    if (!SkeletalMeshComponent)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
            if (!SkeletalMeshComponent)
            {
                // Try to get from Character class
                ACharacter* Character = Cast<ACharacter>(Owner);
                if (Character)
                {
                    SkeletalMeshComponent = Character->GetMesh();
                }
            }
        }
    }
    
    InitializeRagdollProfiles();
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRagdollState(DeltaTime);
    
    if (CurrentState == ECore_RagdollState::Blending)
    {
        UpdateBlending(DeltaTime);
    }
}

void UCore_RagdollSystem::ActivateRagdoll(const FString& ProfileName)
{
    if (!SkeletalMeshComponent || CurrentState == ECore_RagdollState::Active)
    {
        return;
    }
    
    FCore_RagdollProfile* Profile = FindProfile(ProfileName);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll profile '%s' not found, using default"), *ProfileName);
        Profile = FindProfile(TEXT("Default"));
    }
    
    if (Profile)
    {
        // Cache current bone transforms for blending back
        CacheBoneTransforms();
        
        // Preserve momentum if enabled
        if (bPreserveMomentum && SkeletalMeshComponent->GetOwner())
        {
            FVector Velocity = SkeletalMeshComponent->GetOwner()->GetVelocity();
            if (!Velocity.IsNearlyZero())
            {
                Velocity *= MomentumScale;
                // Apply velocity to root bone
                SkeletalMeshComponent->SetAllBodiesLinearVelocity(Velocity);
            }
        }
        
        // Apply ragdoll profile
        ApplyRagdollProfile(*Profile);
        
        // Set ragdoll state
        SkeletalMeshComponent->SetSimulatePhysics(true);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        CurrentState = ECore_RagdollState::Active;
        ActiveProfileName = ProfileName;
        RagdollActiveTime = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Ragdoll activated with profile: %s"), *ProfileName);
    }
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetSimulatePhysics(false);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // Reset to animation
        if (SkeletalMeshComponent->GetAnimInstance())
        {
            SkeletalMeshComponent->GetAnimInstance()->Montage_Resume(nullptr);
        }
    }
    
    CurrentState = ECore_RagdollState::Disabled;
    RagdollActiveTime = 0.0f;
    BlendTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll deactivated"));
}

void UCore_RagdollSystem::BlendToAnimation(float BlendTime)
{
    if (CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    CurrentState = ECore_RagdollState::Blending;
    BlendTimer = BlendTime;
    
    UE_LOG(LogTemp, Log, TEXT("Starting ragdoll blend to animation over %f seconds"), BlendTime);
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Blending;
}

ECore_RagdollState UCore_RagdollSystem::GetRagdollState() const
{
    return CurrentState;
}

void UCore_RagdollSystem::AddRagdollProfile(const FCore_RagdollProfile& Profile)
{
    // Remove existing profile with same name
    RagdollProfiles.RemoveAll([&Profile](const FCore_RagdollProfile& ExistingProfile)
    {
        return ExistingProfile.ProfileName == Profile.ProfileName;
    });
    
    RagdollProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("Added ragdoll profile: %s"), *Profile.ProfileName);
}

void UCore_RagdollSystem::SetSkeletalMeshComponent(USkeletalMeshComponent* InMeshComponent)
{
    SkeletalMeshComponent = InMeshComponent;
    InitializeRagdollProfiles();
}

void UCore_RagdollSystem::SetBonePhysicsProperties(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping)
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

void UCore_RagdollSystem::ApplyImpulseAtBone(const FName& BoneName, const FVector& Impulse)
{
    if (!SkeletalMeshComponent || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    SkeletalMeshComponent->AddImpulseAtLocation(Impulse, SkeletalMeshComponent->GetBoneLocation(BoneName), BoneName);
}

void UCore_RagdollSystem::ApplyRadialImpulse(const FVector& Origin, float Radius, float Strength, bool bVelChange)
{
    if (!SkeletalMeshComponent || CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    SkeletalMeshComponent->AddRadialImpulse(Origin, Radius, Strength, ERadialImpulseFalloff::RIF_Linear, bVelChange);
}

void UCore_RagdollSystem::SetConstraintLimits(const FName& BoneName, bool bEnableSwing1, bool bEnableSwing2, bool bEnableTwist)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // This would require access to physics constraints
    // Implementation depends on specific constraint setup
    UE_LOG(LogTemp, Log, TEXT("Setting constraint limits for bone: %s"), *BoneName.ToString());
}

void UCore_RagdollSystem::ModifyConstraintStrength(const FName& BoneName, float Strength)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // This would require access to physics constraints
    // Implementation depends on specific constraint setup
    UE_LOG(LogTemp, Log, TEXT("Modifying constraint strength for bone: %s to %f"), *BoneName.ToString(), Strength);
}

void UCore_RagdollSystem::InitializeRagdollProfiles()
{
    if (RagdollProfiles.Num() == 0)
    {
        SetupDefaultProfile();
    }
}

void UCore_RagdollSystem::CacheBoneTransforms()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    CachedBoneTransforms.Empty();
    RagdollBoneNames.Empty();
    
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetRefSkeleton();
    
    for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
    {
        FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
        FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
        
        CachedBoneTransforms.Add(BoneTransform);
        RagdollBoneNames.Add(BoneName);
    }
}

void UCore_RagdollSystem::ApplyRagdollProfile(const FCore_RagdollProfile& Profile)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    for (const FCore_RagdollBone& RagdollBone : Profile.Bones)
    {
        SetBonePhysicsProperties(RagdollBone.BoneName, RagdollBone.Mass, 
                               RagdollBone.LinearDamping, RagdollBone.AngularDamping);
        
        FBodyInstance* BodyInstance = SkeletalMeshComponent->GetBodyInstance(RagdollBone.BoneName);
        if (BodyInstance)
        {
            BodyInstance->SetEnableGravity(RagdollBone.bEnableGravity);
        }
    }
}

void UCore_RagdollSystem::UpdateBlending(float DeltaTime)
{
    BlendTimer -= DeltaTime;
    
    if (BlendTimer <= 0.0f)
    {
        // Blend complete, return to animation
        DeactivateRagdoll();
        return;
    }
    
    // Implement blending logic here
    // This would involve interpolating between ragdoll and animation poses
    float BlendAlpha = BlendTimer / (FindProfile(ActiveProfileName) ? FindProfile(ActiveProfileName)->BlendOutTime : 1.0f);
    
    // Apply blended transforms
    // Implementation would depend on specific animation system integration
}

void UCore_RagdollSystem::UpdateRagdollState(float DeltaTime)
{
    if (CurrentState == ECore_RagdollState::Active)
    {
        RagdollActiveTime += DeltaTime;
        
        // Auto blend back if enabled
        if (bAutoBlendBack && RagdollActiveTime >= AutoBlendBackDelay)
        {
            FCore_RagdollProfile* Profile = FindProfile(ActiveProfileName);
            float BlendTime = Profile ? Profile->BlendOutTime : 1.0f;
            BlendToAnimation(BlendTime);
        }
        
        // Check max ragdoll time
        FCore_RagdollProfile* Profile = FindProfile(ActiveProfileName);
        if (Profile && RagdollActiveTime >= Profile->MaxRagdollTime)
        {
            DeactivateRagdoll();
        }
    }
}

FCore_RagdollProfile* UCore_RagdollSystem::FindProfile(const FString& ProfileName)
{
    for (FCore_RagdollProfile& Profile : RagdollProfiles)
    {
        if (Profile.ProfileName == ProfileName)
        {
            return &Profile;
        }
    }
    return nullptr;
}

void UCore_RagdollSystem::SetupDefaultProfile()
{
    FCore_RagdollProfile DefaultProfile;
    DefaultProfile.ProfileName = TEXT("Default");
    DefaultProfile.BlendInTime = 0.2f;
    DefaultProfile.BlendOutTime = 1.0f;
    DefaultProfile.MaxRagdollTime = 10.0f;
    
    // Add common bones with default properties
    TArray<FString> CommonBones = {
        TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
        TEXT("head"), TEXT("neck_01"),
        TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
        TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"),
        TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
        TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r")
    };
    
    for (const FString& BoneNameStr : CommonBones)
    {
        FCore_RagdollBone RagdollBone;
        RagdollBone.BoneName = FName(*BoneNameStr);
        RagdollBone.Mass = 10.0f;
        RagdollBone.LinearDamping = 0.1f;
        RagdollBone.AngularDamping = 0.1f;
        RagdollBone.bEnableGravity = true;
        
        DefaultProfile.Bones.Add(RagdollBone);
    }
    
    RagdollProfiles.Add(DefaultProfile);
    UE_LOG(LogTemp, Log, TEXT("Created default ragdoll profile with %d bones"), DefaultProfile.Bones.Num());
}