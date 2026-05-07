#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/GameplayStatics.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Disabled;
    TransitionTimer = 0.0f;
    RecoveryTimer = 0.0f;
    PhysicsBlendWeight = 0.0f;
    bIsTransitioning = false;
    bIsRecovering = false;
    
    SkeletalMeshComponent = nullptr;
    CapsuleComponent = nullptr;
    AnimInstance = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    SetupPhysicsProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Initialized for actor %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateRagdollTransition(DeltaTime);
    }
    
    if (bIsRecovering)
    {
        UpdateRecovery(DeltaTime);
    }
}

void UCore_RagdollSystem::InitializeComponents()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_RagdollSystem: No owner found"));
        return;
    }
    
    // Find skeletal mesh component
    SkeletalMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No SkeletalMeshComponent found on %s"), *GetOwner()->GetName());
    }
    
    // Find capsule component
    CapsuleComponent = GetOwner()->FindComponentByClass<UCapsuleComponent>();
    if (!CapsuleComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No CapsuleComponent found on %s"), *GetOwner()->GetName());
    }
    
    // Get animation instance
    if (SkeletalMeshComponent)
    {
        AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (!AnimInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: No AnimInstance found"));
        }
        
        // Bind to hit events
        SkeletalMeshComponent->OnComponentHit.AddDynamic(this, &UCore_RagdollSystem::OnHit);
    }
}

void UCore_RagdollSystem::SetupPhysicsProperties()
{
    if (!SkeletalMeshComponent)
        return;
    
    // Set initial physics properties
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Disable physics simulation initially
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetAllBodiesSimulatePhysics(false);
}

void UCore_RagdollSystem::EnableRagdoll(ECore_RagdollTrigger Trigger)
{
    if (CurrentState == ECore_RagdollState::Active)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Ragdoll already active"));
        return;
    }
    
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_RagdollSystem: Cannot enable ragdoll - no SkeletalMeshComponent"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Enabling ragdoll (Trigger: %d)"), (int32)Trigger);
    
    // Cache current pose
    CacheCurrentPose();
    
    // Disable capsule collision
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Enable physics on skeletal mesh
    SkeletalMeshComponent->SetSimulatePhysics(true);
    SkeletalMeshComponent->SetAllBodiesSimulatePhysics(true);
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply physics properties
    SkeletalMeshComponent->SetAllBodiesLinearDamping(RagdollConfig.LinearDamping);
    SkeletalMeshComponent->SetAllBodiesAngularDamping(RagdollConfig.AngularDamping);
    
    // Set state
    SetRagdollState(ECore_RagdollState::Transitioning);
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
    
    // Start auto-recovery timer if enabled
    if (RagdollConfig.bAutoRecover)
    {
        RecoveryTimer = RagdollConfig.RecoveryTime;
    }
}

void UCore_RagdollSystem::DisableRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_RagdollSystem: Ragdoll already disabled"));
        return;
    }
    
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_RagdollSystem: Cannot disable ragdoll - no SkeletalMeshComponent"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Disabling ragdoll"));
    
    // Disable physics
    SkeletalMeshComponent->SetSimulatePhysics(false);
    SkeletalMeshComponent->SetAllBodiesSimulatePhysics(false);
    
    // Re-enable capsule collision
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Restore cached pose if available
    RestoreCachedPose();
    
    // Set state
    SetRagdollState(ECore_RagdollState::Disabled);
    bIsTransitioning = false;
    bIsRecovering = false;
    PhysicsBlendWeight = 0.0f;
}

void UCore_RagdollSystem::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState == NewState)
        return;
    
    ECore_RagdollState PreviousState = CurrentState;
    CurrentState = NewState;
    
    HandleStateTransition(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: State changed from %d to %d"), 
           (int32)PreviousState, (int32)NewState);
}

void UCore_RagdollSystem::HandleStateTransition(ECore_RagdollState NewState)
{
    switch (NewState)
    {
        case ECore_RagdollState::Disabled:
            PhysicsBlendWeight = 0.0f;
            break;
            
        case ECore_RagdollState::Transitioning:
            TransitionTimer = 0.0f;
            bIsTransitioning = true;
            break;
            
        case ECore_RagdollState::Active:
            PhysicsBlendWeight = 1.0f;
            bIsTransitioning = false;
            break;
            
        case ECore_RagdollState::Recovering:
            bIsRecovering = true;
            RecoveryTimer = RagdollConfig.RecoveryTime;
            break;
    }
}

void UCore_RagdollSystem::ApplyImpactForce(FVector Force, FVector Location, FName BoneName)
{
    if (!SkeletalMeshComponent)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Applying impact force %s at %s"), 
           *Force.ToString(), *Location.ToString());
    
    // Enable ragdoll if not already active
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        EnableRagdoll(ECore_RagdollTrigger::Impact);
    }
    
    // Apply force to specific bone or all bodies
    if (BoneName != NAME_None)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, Location);
    }
}

void UCore_RagdollSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
    
    // Apply new settings if ragdoll is active
    if (SkeletalMeshComponent && CurrentState != ECore_RagdollState::Disabled)
    {
        SkeletalMeshComponent->SetAllBodiesLinearDamping(RagdollConfig.LinearDamping);
        SkeletalMeshComponent->SetAllBodiesAngularDamping(RagdollConfig.AngularDamping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Updated ragdoll configuration"));
}

void UCore_RagdollSystem::SetPhysicsBlendWeight(float BlendWeight)
{
    PhysicsBlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    
    if (SkeletalMeshComponent && RagdollConfig.bBlendPhysics)
    {
        SkeletalMeshComponent->SetAllBodiesPhysicsBlendWeight(PhysicsBlendWeight);
    }
}

void UCore_RagdollSystem::BlendToPhysics(float BlendTime)
{
    if (BlendTime <= 0.0f)
    {
        SetPhysicsBlendWeight(1.0f);
        return;
    }
    
    // Start transition to physics
    SetRagdollState(ECore_RagdollState::Transitioning);
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
}

void UCore_RagdollSystem::BlendToAnimation(float BlendTime)
{
    if (BlendTime <= 0.0f)
    {
        SetPhysicsBlendWeight(0.0f);
        SetRagdollState(ECore_RagdollState::Disabled);
        return;
    }
    
    // Start recovery transition
    StartRecovery();
}

void UCore_RagdollSystem::SetBonePhysicsEnabled(FName BoneName, bool bEnabled)
{
    if (!SkeletalMeshComponent)
        return;
    
    SkeletalMeshComponent->SetBodySimulatePhysics(BoneName, bEnabled);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Set bone %s physics to %s"), 
           *BoneName.ToString(), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_RagdollSystem::SetAllBonesPhysicsEnabled(bool bEnabled)
{
    if (!SkeletalMeshComponent)
        return;
    
    SkeletalMeshComponent->SetAllBodiesSimulatePhysics(bEnabled);
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Set all bones physics to %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_RagdollSystem::StartRecovery()
{
    if (CurrentState == ECore_RagdollState::Disabled)
        return;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Starting recovery"));
    
    SetRagdollState(ECore_RagdollState::Recovering);
    RecoveryTimer = RagdollConfig.RecoveryTime;
    bIsRecovering = true;
}

bool UCore_RagdollSystem::CanRecover() const
{
    // Check if character is in a stable position for recovery
    if (!SkeletalMeshComponent)
        return false;
    
    // Simple check: ensure character isn't moving too fast
    FVector Velocity = SkeletalMeshComponent->GetPhysicsLinearVelocity();
    return Velocity.Size() < 100.0f; // Arbitrary threshold
}

void UCore_RagdollSystem::UpdateRagdollTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    
    if (TransitionTimer >= RagdollConfig.TransitionTime)
    {
        // Transition complete
        SetRagdollState(ECore_RagdollState::Active);
        bIsTransitioning = false;
        return;
    }
    
    // Update blend weight during transition
    if (RagdollConfig.bBlendPhysics)
    {
        float BlendAlpha = TransitionTimer / RagdollConfig.TransitionTime;
        SetPhysicsBlendWeight(BlendAlpha);
    }
}

void UCore_RagdollSystem::UpdateRecovery(float DeltaTime)
{
    RecoveryTimer -= DeltaTime;
    
    if (RecoveryTimer <= 0.0f)
    {
        // Recovery complete
        DisableRagdoll();
        return;
    }
    
    // Update blend weight during recovery
    if (RagdollConfig.bBlendPhysics)
    {
        float BlendAlpha = RecoveryTimer / RagdollConfig.RecoveryTime;
        SetPhysicsBlendWeight(BlendAlpha);
    }
}

void UCore_RagdollSystem::CacheCurrentPose()
{
    if (!SkeletalMeshComponent)
        return;
    
    LastValidLocation = GetOwner()->GetActorLocation();
    LastValidRotation = GetOwner()->GetActorRotation();
    
    // Cache bone transforms
    BoneTransforms.Empty();
    const USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
    if (SkeletalMesh)
    {
        int32 BoneCount = SkeletalMesh->GetRefSkeleton().GetNum();
        BoneTransforms.Reserve(BoneCount);
        
        for (int32 i = 0; i < BoneCount; ++i)
        {
            FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(i);
            BoneTransforms.Add(BoneTransform);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Cached current pose (%d bones)"), BoneTransforms.Num());
}

void UCore_RagdollSystem::RestoreCachedPose()
{
    if (!SkeletalMeshComponent || BoneTransforms.Num() == 0)
        return;
    
    // Restore actor transform
    GetOwner()->SetActorLocation(LastValidLocation);
    GetOwner()->SetActorRotation(LastValidRotation);
    
    // Restore bone transforms
    const USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
    if (SkeletalMesh)
    {
        int32 BoneCount = FMath::Min(SkeletalMesh->GetRefSkeleton().GetNum(), BoneTransforms.Num());
        
        for (int32 i = 0; i < BoneCount; ++i)
        {
            FName BoneName = SkeletalMesh->GetRefSkeleton().GetBoneName(i);
            SkeletalMeshComponent->SetBoneTransformByName(BoneName, BoneTransforms[i], EBoneSpaces::WorldSpace);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Restored cached pose"));
}

void UCore_RagdollSystem::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || CurrentState == ECore_RagdollState::Active)
        return;
    
    float ImpactMagnitude = NormalImpulse.Size();
    
    if (ImpactMagnitude > RagdollConfig.ImpactThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollSystem: Impact detected (%.2f) - enabling ragdoll"), ImpactMagnitude);
        
        EnableRagdoll(ECore_RagdollTrigger::Impact);
        ApplyImpactForce(NormalImpulse, Hit.Location);
    }
}

// Test Actor Implementation
ACore_RagdollTestActor::ACore_RagdollTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create capsule collision
    CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
    RootComponent = CapsuleCollision;
    CapsuleCollision->SetCapsuleSize(40.0f, 90.0f);
    
    // Create skeletal mesh
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootComponent);
    
    // Create ragdoll system
    RagdollSystem = CreateDefaultSubobject<UCore_RagdollSystem>(TEXT("RagdollSystem"));
}

void ACore_RagdollTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollTestActor: Test actor initialized"));
}

void ACore_RagdollTestActor::TestRagdollActivation()
{
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollTestActor: Testing ragdoll activation"));
        RagdollSystem->EnableRagdoll(ECore_RagdollTrigger::Manual);
    }
}

void ACore_RagdollTestActor::TestRagdollDeactivation()
{
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollTestActor: Testing ragdoll deactivation"));
        RagdollSystem->DisableRagdoll();
    }
}

void ACore_RagdollTestActor::TestImpactForce()
{
    if (RagdollSystem)
    {
        FVector Force = FVector(0, 0, 1000) + FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0);
        FVector Location = GetActorLocation() + FVector(0, 0, 50);
        
        UE_LOG(LogTemp, Log, TEXT("Core_RagdollTestActor: Testing impact force %s"), *Force.ToString());
        RagdollSystem->ApplyImpactForce(Force, Location);
    }
}