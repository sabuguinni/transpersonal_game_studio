#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogCore_RagdollSystem);

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default state
    CurrentState = ECore_RagdollState::Inactive;
    TargetMesh = nullptr;
    RecoveryTimer = 0.0f;
    LastImpactForce = FVector::ZeroVector;
    OriginalAnimBP = nullptr;
    
    // Set default ragdoll configuration
    RagdollConfig = FCore_RagdollConfig();
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (TargetMesh)
        {
            UE_LOG(LogCore_RagdollSystem, Log, TEXT("RagdollSystem initialized for %s"), *Owner->GetName());
            
            // Save original animation blueprint
            if (TargetMesh->GetAnimInstance())
            {
                OriginalAnimBP = TargetMesh->GetAnimInstance()->GetClass();
            }
            
            // Bind hit events
            TargetMesh->OnComponentHit.AddDynamic(this, &UCore_RagdollSystem::OnHit);
            
            // Initialize physics setup
            InitializeRagdollPhysics();
        }
        else
        {
            UE_LOG(LogCore_RagdollSystem, Warning, TEXT("No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update recovery process if in recovery state
    if (CurrentState == ECore_RagdollState::Recovering)
    {
        UpdateRecoveryProcess(DeltaTime);
    }
    
    // Auto-recovery check
    if (CurrentState == ECore_RagdollState::Active && RagdollConfig.bAutoRecover)
    {
        RecoveryTimer += DeltaTime;
        if (RecoveryTimer >= RagdollConfig.RecoveryTime)
        {
            CurrentState = ECore_RagdollState::Recovering;
            RecoveryTimer = 0.0f;
            OnRecoveryStarted();
            UE_LOG(LogCore_RagdollSystem, Log, TEXT("Auto-recovery started for %s"), *GetOwner()->GetName());
        }
    }
}

void UCore_RagdollSystem::ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!TargetMesh || CurrentState == ECore_RagdollState::Active)
    {
        return;
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Activating ragdoll for %s with force %s"), 
           *GetOwner()->GetName(), *ImpactForce.ToString());
    
    CurrentState = ECore_RagdollState::Activating;
    LastImpactForce = ImpactForce;
    RecoveryTimer = 0.0f;
    
    // Save current animation state
    SaveOriginalAnimationState();
    
    // Disable character collision
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Enable physics simulation
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetMesh->SetSimulatePhysics(true);
    TargetMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Apply masses to bones
    SetupBoneMasses();
    
    // Setup joint constraints
    SetupJointConstraints();
    
    // Apply initial impact force if provided
    if (!ImpactForce.IsZero())
    {
        FVector ApplyLocation = ImpactLocation.IsZero() ? TargetMesh->GetComponentLocation() : ImpactLocation;
        TargetMesh->AddImpulseAtLocation(ImpactForce, ApplyLocation);
    }
    
    CurrentState = ECore_RagdollState::Active;
    OnRagdollActivated(ImpactForce);
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Ragdoll activated successfully for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Deactivating ragdoll for %s"), *GetOwner()->GetName());
    
    // Restore original animation state
    RestoreOriginalAnimationState();
    
    // Disable physics simulation
    TargetMesh->SetSimulatePhysics(false);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable character collision
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    // Clean up constraint components
    for (UPhysicsConstraintComponent* Constraint : JointConstraints)
    {
        if (Constraint)
        {
            Constraint->DestroyComponent();
        }
    }
    JointConstraints.Empty();
    
    CurrentState = ECore_RagdollState::Inactive;
    RecoveryTimer = 0.0f;
    OnRagdollDeactivated();
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Ragdoll deactivated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::ForceRecovery()
{
    if (CurrentState != ECore_RagdollState::Active)
    {
        return;
    }
    
    CurrentState = ECore_RagdollState::Recovering;
    RecoveryTimer = 0.0f;
    OnRecoveryStarted();
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Force recovery initiated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
    
    // If ragdoll is currently active, update physics properties
    if (CurrentState == ECore_RagdollState::Active)
    {
        SetupBoneMasses();
        SetupJointConstraints();
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Ragdoll config updated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollSystem::SetBoneMass(const FName& BoneName, float Mass)
{
    BoneMassOverrides.Add(BoneName, Mass);
    
    // Apply immediately if ragdoll is active
    if (CurrentState == ECore_RagdollState::Active && TargetMesh)
    {
        FBodyInstance* BodyInst = TargetMesh->GetBodyInstance(BoneName);
        if (BodyInst)
        {
            BodyInst->SetMassOverride(Mass, true);
        }
    }
}

void UCore_RagdollSystem::SetJointConstraints(float Stiffness, float Damping)
{
    RagdollConfig.JointStiffness = Stiffness;
    RagdollConfig.JointDamping = Damping;
    
    // Update existing constraints
    for (UPhysicsConstraintComponent* Constraint : JointConstraints)
    {
        if (Constraint)
        {
            // Update constraint properties
            Constraint->SetAngularDriveParams(Stiffness, Damping, 0.0f);
        }
    }
}

float UCore_RagdollSystem::GetRecoveryProgress() const
{
    if (CurrentState != ECore_RagdollState::Recovering)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(RecoveryTimer / RagdollConfig.RecoveryTime, 0.0f, 1.0f);
}

void UCore_RagdollSystem::ApplyImpactToRagdoll(const FVector& Force, const FVector& Location, const FName& BoneName)
{
    if (CurrentState != ECore_RagdollState::Active || !TargetMesh)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        // Apply force to specific bone
        TargetMesh->AddImpulseToBody(Force, BoneName, true);
    }
    else
    {
        // Apply force at location
        TargetMesh->AddImpulseAtLocation(Force, Location);
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Applied impact force %s to %s"), 
           *Force.ToString(), *GetOwner()->GetName());
}

void UCore_RagdollSystem::SimulateExplosionImpact(const FVector& ExplosionCenter, float ExplosionForce, float ExplosionRadius)
{
    if (CurrentState != ECore_RagdollState::Active || !TargetMesh)
    {
        return;
    }
    
    // Calculate direction and distance from explosion
    FVector ActorLocation = TargetMesh->GetComponentLocation();
    FVector Direction = (ActorLocation - ExplosionCenter).GetSafeNormal();
    float Distance = FVector::Dist(ActorLocation, ExplosionCenter);
    
    // Apply falloff based on distance
    float ForceFalloff = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.0f, 1.0f);
    FVector FinalForce = Direction * ExplosionForce * ForceFalloff;
    
    // Apply force to all bones
    TArray<FName> BoneNames;
    GetAllBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        TargetMesh->AddImpulseToBody(FinalForce * 0.5f, BoneName, true);
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Explosion impact applied to %s with force %f"), 
           *GetOwner()->GetName(), ExplosionForce);
}

void UCore_RagdollSystem::ConfigureForDinosaur(float DinosaurMass, bool bIsLargeDinosaur)
{
    RagdollConfig.TotalMass = DinosaurMass;
    
    if (bIsLargeDinosaur)
    {
        // Large dinosaurs (T-Rex, Brachiosaurus) need stronger constraints
        RagdollConfig.JointStiffness = 2000.0f;
        RagdollConfig.JointDamping = 100.0f;
        RagdollConfig.RecoveryTime = 5.0f;
        RagdollConfig.TorsoMassMultiplier = 0.6f; // Heavier torso
    }
    else
    {
        // Smaller dinosaurs (Raptors) are more agile
        RagdollConfig.JointStiffness = 800.0f;
        RagdollConfig.JointDamping = 40.0f;
        RagdollConfig.RecoveryTime = 2.0f;
        RagdollConfig.LimbMassMultiplier = 0.15f; // More mass in limbs for agility
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Configured ragdoll for dinosaur: Mass=%f, Large=%s"), 
           DinosaurMass, bIsLargeDinosaur ? TEXT("Yes") : TEXT("No"));
}

void UCore_RagdollSystem::ConfigureForHuman()
{
    RagdollConfig = FCore_RagdollConfig(); // Use default human configuration
    RagdollConfig.TotalMass = 70.0f;
    RagdollConfig.RecoveryTime = 3.0f;
    RagdollConfig.bAutoRecover = true;
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Configured ragdoll for human character"));
}

void UCore_RagdollSystem::InitializeRagdollPhysics()
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Ensure physics asset exists
    UPhysicsAsset* PhysicsAsset = TargetMesh->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogCore_RagdollSystem, Warning, TEXT("No Physics Asset found for %s"), *GetOwner()->GetName());
        return;
    }
    
    // Initialize bone mass overrides map
    TArray<FName> BoneNames;
    GetAllBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        if (!BoneMassOverrides.Contains(BoneName))
        {
            float CalculatedMass = CalculateBoneMass(BoneName);
            BoneMassOverrides.Add(BoneName, CalculatedMass);
        }
    }
    
    UE_LOG(LogCore_RagdollSystem, Log, TEXT("Ragdoll physics initialized for %s with %d bones"), 
           *GetOwner()->GetName(), BoneNames.Num());
}

void UCore_RagdollSystem::SetupBoneMasses()
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Apply mass overrides to all bones
    for (const auto& MassOverride : BoneMassOverrides)
    {
        FBodyInstance* BodyInst = TargetMesh->GetBodyInstance(MassOverride.Key);
        if (BodyInst)
        {
            BodyInst->SetMassOverride(MassOverride.Value, true);
        }
    }
}

void UCore_RagdollSystem::SetupJointConstraints()
{
    // This would typically create physics constraints between bones
    // For now, we rely on the physics asset's built-in constraints
    // and modify their properties
    
    if (!TargetMesh)
    {
        return;
    }
    
    // Get all body instances and modify their constraint properties
    TArray<FName> BoneNames;
    GetAllBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        FBodyInstance* BodyInst = TargetMesh->GetBodyInstance(BoneName);
        if (BodyInst)
        {
            // Configure collision properties
            BodyInst->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            BodyInst->SetObjectType(ECC_Pawn);
            BodyInst->SetResponseToAllChannels(ECR_Block);
            
            // Set damping values
            BodyInst->LinearDamping = RagdollConfig.JointDamping * 0.01f;
            BodyInst->AngularDamping = RagdollConfig.JointDamping * 0.02f;
        }
    }
}

void UCore_RagdollSystem::UpdateRecoveryProcess(float DeltaTime)
{
    RecoveryTimer += DeltaTime;
    float RecoveryProgress = GetRecoveryProgress();
    
    // Blend from ragdoll to animation
    BlendToRecoveryPose(RecoveryProgress);
    
    // Complete recovery when timer expires
    if (RecoveryProgress >= 1.0f)
    {
        DeactivateRagdoll();
        OnRecoveryCompleted();
        UE_LOG(LogCore_RagdollSystem, Log, TEXT("Recovery completed for %s"), *GetOwner()->GetName());
    }
}

void UCore_RagdollSystem::BlendToRecoveryPose(float BlendAlpha)
{
    if (!TargetMesh)
    {
        return;
    }
    
    // Gradually reduce physics influence
    float PhysicsWeight = 1.0f - BlendAlpha;
    SetPhysicsBlendWeight(PhysicsWeight);
    
    // Gradually restore animation
    if (BlendAlpha > 0.5f && OriginalAnimBP)
    {
        // Start blending back to animation when halfway through recovery
        float AnimBlendAlpha = (BlendAlpha - 0.5f) * 2.0f;
        // Implementation would blend animation back in
    }
}

void UCore_RagdollSystem::GetAllBoneNames(TArray<FName>& OutBoneNames) const
{
    OutBoneNames.Empty();
    
    if (!TargetMesh || !TargetMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    const FReferenceSkeleton& RefSkeleton = TargetMesh->GetSkeletalMeshAsset()->GetRefSkeleton();
    
    for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
    {
        OutBoneNames.Add(RefSkeleton.GetBoneName(BoneIndex));
    }
}

float UCore_RagdollSystem::CalculateBoneMass(const FName& BoneName) const
{
    FString BoneNameStr = BoneName.ToString().ToLower();
    
    // Determine bone type and calculate appropriate mass
    if (BoneNameStr.Contains(TEXT("head")) || BoneNameStr.Contains(TEXT("skull")))
    {
        return RagdollConfig.TotalMass * RagdollConfig.HeadMassMultiplier;
    }
    else if (BoneNameStr.Contains(TEXT("spine")) || BoneNameStr.Contains(TEXT("chest")) || 
             BoneNameStr.Contains(TEXT("pelvis")) || BoneNameStr.Contains(TEXT("torso")))
    {
        return RagdollConfig.TotalMass * RagdollConfig.TorsoMassMultiplier;
    }
    else
    {
        // Arms, legs, tail, etc.
        return RagdollConfig.TotalMass * RagdollConfig.LimbMassMultiplier;
    }
}

bool UCore_RagdollSystem::IsCriticalBone(const FName& BoneName) const
{
    FString BoneNameStr = BoneName.ToString().ToLower();
    
    // Critical bones that should have special handling
    return BoneNameStr.Contains(TEXT("head")) || 
           BoneNameStr.Contains(TEXT("spine")) || 
           BoneNameStr.Contains(TEXT("pelvis"));
}

void UCore_RagdollSystem::SaveOriginalAnimationState()
{
    if (TargetMesh && TargetMesh->GetAnimInstance())
    {
        OriginalAnimBP = TargetMesh->GetAnimInstance()->GetClass();
    }
}

void UCore_RagdollSystem::RestoreOriginalAnimationState()
{
    if (TargetMesh && OriginalAnimBP)
    {
        TargetMesh->SetAnimInstanceClass(OriginalAnimBP);
    }
}

void UCore_RagdollSystem::SetPhysicsBlendWeight(float BlendWeight)
{
    if (TargetMesh)
    {
        TargetMesh->SetAllBodiesPhysicsBlendWeight(BlendWeight);
    }
}

void UCore_RagdollSystem::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
                                FVector NormalImpulse, const FHitResult& Hit)
{
    // Auto-activate ragdoll on significant impacts
    if (CurrentState == ECore_RagdollState::Inactive && NormalImpulse.Size() > 1000.0f)
    {
        ActivateRagdoll(NormalImpulse, Hit.Location);
    }
}

void UCore_RagdollSystem::DebugDrawRagdollInfo()
{
    if (!TargetMesh || !GetWorld())
    {
        return;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    
    // Draw state information
    FString StateText = FString::Printf(TEXT("Ragdoll State: %s"), 
        *UEnum::GetValueAsString(CurrentState));
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), StateText, 
                   nullptr, FColor::Yellow, 0.0f);
    
    // Draw recovery progress if recovering
    if (CurrentState == ECore_RagdollState::Recovering)
    {
        FString ProgressText = FString::Printf(TEXT("Recovery: %.1f%%"), GetRecoveryProgress() * 100.0f);
        DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 180), ProgressText, 
                       nullptr, FColor::Green, 0.0f);
    }
    
    // Draw mass information
    FString MassText = FString::Printf(TEXT("Total Mass: %.1f kg"), RagdollConfig.TotalMass);
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 160), MassText, 
                   nullptr, FColor::Cyan, 0.0f);
}

void UCore_RagdollSystem::TestRagdollActivation()
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        FVector TestForce = FVector(0, 0, 1000);
        ActivateRagdoll(TestForce);
    }
    else
    {
        DeactivateRagdoll();
    }
}