#include "Core_RagdollSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Disabled;
    SkeletalMeshComp = nullptr;
    AnimInstance = nullptr;
    RecoveryTimer = 0.0f;
    bIsRecovering = false;
    
    // Default settings
    Settings.ActivationForce = 1000.0f;
    Settings.RecoveryTime = 3.0f;
    Settings.BlendOutTime = 1.0f;
    Settings.bAutoRecover = true;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRagdollSystem();
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsRecovering)
    {
        UpdateRecoverySystem(DeltaTime);
    }
}

void UCore_RagdollSystem::InitializeRagdollSystem()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: No owner found"));
        return;
    }
    
    // Find skeletal mesh component
    SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: No SkeletalMeshComponent found on owner"));
        return;
    }
    
    // Get animation instance
    AnimInstance = SkeletalMeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollSystem: No AnimInstance found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Successfully initialized"));
}

void UCore_RagdollSystem::ActivateRagdoll(float Force, FVector ImpactLocation)
{
    if (!SkeletalMeshComp || CurrentState == ECore_RagdollState::Full)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Activating ragdoll with force %f"), Force);
    
    // Set physics simulation
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply impact force if specified
    if (Force > 0.0f && ImpactLocation != FVector::ZeroVector)
    {
        ApplyImpactForce(FVector(0, 0, Force), ImpactLocation);
    }
    
    // Update state
    SetRagdollState(ECore_RagdollState::Full);
    
    // Start recovery timer if auto-recovery is enabled
    if (Settings.bAutoRecover)
    {
        RecoveryTimer = Settings.RecoveryTime;
        bIsRecovering = true;
    }
    
    OnRagdollActivated.Broadcast();
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (!SkeletalMeshComp || CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Deactivating ragdoll"));
    
    // Restore animation
    RestoreAnimation();
    
    // Update state
    SetRagdollState(ECore_RagdollState::Disabled);
    
    // Stop recovery
    bIsRecovering = false;
    RecoveryTimer = 0.0f;
    
    OnRagdollDeactivated.Broadcast();
}

void UCore_RagdollSystem::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState != NewState)
    {
        ECore_RagdollState OldState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("RagdollSystem: State changed from %d to %d"), (int32)OldState, (int32)NewState);
        
        BroadcastStateChange(NewState);
    }
}

void UCore_RagdollSystem::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    Settings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Settings updated"));
}

void UCore_RagdollSystem::ApplyImpactForce(FVector Force, FVector Location, FName BoneName)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->AddImpulseAtLocation(Force, Location, BoneName);
    }
    else
    {
        SkeletalMeshComp->AddImpulseAtLocation(Force, Location);
    }
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Applied impact force %s at location %s"), 
           *Force.ToString(), *Location.ToString());
}

void UCore_RagdollSystem::SetBonePhysicsBlendWeight(FName BoneName, float BlendWeight)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    BoneBlendWeights.Add(BoneName, BlendWeight);
    
    // Apply blend weight to specific bone
    SkeletalMeshComp->SetAllBodiesBelowPhysicsBlendWeight(BoneName, BlendWeight);
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Set bone %s physics blend weight to %f"), 
           *BoneName.ToString(), BlendWeight);
}

void UCore_RagdollSystem::UpdateRecoverySystem(float DeltaTime)
{
    if (!bIsRecovering || CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    RecoveryTimer -= DeltaTime;
    
    if (RecoveryTimer <= 0.0f)
    {
        // Start recovery process
        SetRagdollState(ECore_RagdollState::Recovering);
        
        // Begin blend out
        float BlendOutProgress = FMath::Clamp((-RecoveryTimer) / Settings.BlendOutTime, 0.0f, 1.0f);
        BlendToRagdoll(1.0f - BlendOutProgress);
        
        if (BlendOutProgress >= 1.0f)
        {
            // Recovery complete
            DeactivateRagdoll();
        }
    }
}

void UCore_RagdollSystem::BlendToRagdoll(float BlendWeight)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    SetPhysicsBlendWeight(BlendWeight);
}

void UCore_RagdollSystem::RestoreAnimation()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Reset physics blend weight
    SetPhysicsBlendWeight(0.0f);
    
    // Clear bone blend weights
    BoneBlendWeights.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("RagdollSystem: Animation restored"));
}

void UCore_RagdollSystem::SetPhysicsBlendWeight(float BlendWeight)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    SkeletalMeshComp->SetAllBodiesPhysicsBlendWeight(BlendWeight);
}

void UCore_RagdollSystem::BroadcastStateChange(ECore_RagdollState NewState)
{
    OnRagdollStateChanged.Broadcast(NewState);
}