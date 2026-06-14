#include "Core_RagdollSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_RagdollSystem::UCore_RagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ECore_RagdollState::Inactive;
    StateTimer = 0.0f;
    RagdollActiveTime = 0.0f;
    bWasSimulatingPhysics = false;
    CachedAnimInstance = nullptr;
    CurrentPhysicsBlend = 0.0f;
    TargetPhysicsBlend = 0.0f;
    TargetMesh = nullptr;
}

void UCore_RagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find skeletal mesh component if not set
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
        CachedAnimInstance = TargetMesh->GetAnimInstance();
        bWasSimulatingPhysics = TargetMesh->IsSimulatingPhysics();
    }
}

void UCore_RagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateRagdollState(DeltaTime);
}

void UCore_RagdollSystem::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
            HandleStateTransition(DeltaTime);
            break;
            
        case ECore_RagdollState::Active:
            HandleActiveRagdoll(DeltaTime);
            break;
            
        case ECore_RagdollState::Recovering:
            HandleRecovery(DeltaTime);
            break;
            
        default:
            break;
    }
    
    // Update physics blending
    if (RagdollSettings.bBlendPhysics && TargetMesh)
    {
        BlendPhysicsWeight(TargetPhysicsBlend, DeltaTime);
    }
}

void UCore_RagdollSystem::TransitionToState(ECore_RagdollState NewState)
{
    if (CurrentState == NewState)
        return;
        
    CurrentState = NewState;
    StateTimer = 0.0f;
    
    OnRagdollStateChanged.Broadcast(NewState);
    
    switch (NewState)
    {
        case ECore_RagdollState::Transitioning:
            TargetPhysicsBlend = 1.0f;
            break;
            
        case ECore_RagdollState::Active:
            if (TargetMesh)
            {
                TargetMesh->SetSimulatePhysics(true);
                TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                
                // Disable capsule collision for character
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
            RagdollActiveTime = 0.0f;
            break;
            
        case ECore_RagdollState::Recovering:
            TargetPhysicsBlend = 0.0f;
            break;
            
        case ECore_RagdollState::Inactive:
            if (TargetMesh)
            {
                TargetMesh->SetSimulatePhysics(false);
                TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                
                // Re-enable capsule collision
                if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                {
                    Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }
            CurrentPhysicsBlend = 0.0f;
            TargetPhysicsBlend = 0.0f;
            break;
    }
}

void UCore_RagdollSystem::HandleStateTransition(float DeltaTime)
{
    if (StateTimer >= RagdollSettings.TransitionTime)
    {
        TransitionToState(ECore_RagdollState::Active);
    }
}

void UCore_RagdollSystem::HandleActiveRagdoll(float DeltaTime)
{
    RagdollActiveTime += DeltaTime;
    
    // Auto-recovery after max time
    if (RagdollSettings.bAutoRecover && RagdollActiveTime >= RagdollSettings.MaxRagdollTime)
    {
        TransitionToState(ECore_RagdollState::Recovering);
    }
}

void UCore_RagdollSystem::HandleRecovery(float DeltaTime)
{
    if (StateTimer >= RagdollSettings.RecoveryTime)
    {
        TransitionToState(ECore_RagdollState::Inactive);
    }
}

void UCore_RagdollSystem::BlendPhysicsWeight(float TargetWeight, float DeltaTime)
{
    float BlendSpeed = 1.0f / FMath::Max(RagdollSettings.TransitionTime, 0.1f);
    CurrentPhysicsBlend = FMath::FInterpTo(CurrentPhysicsBlend, TargetWeight, DeltaTime, BlendSpeed);
    
    if (TargetMesh && CachedAnimInstance)
    {
        // Set physics blend weight on anim instance if available
        // This would typically be done through animation blueprint
    }
}

void UCore_RagdollSystem::ActivateRagdoll(ECore_RagdollTrigger Trigger, float ImpactForce)
{
    if (CurrentState != ECore_RagdollState::Inactive)
        return;
        
    // Check minimum impact force requirement
    if (Trigger == ECore_RagdollTrigger::Impact && ImpactForce < RagdollSettings.MinImpactForce)
        return;
        
    OnRagdollTriggered.Broadcast(Trigger, ImpactForce);
    TransitionToState(ECore_RagdollState::Transitioning);
}

void UCore_RagdollSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive)
        return;
        
    TransitionToState(ECore_RagdollState::Recovering);
}

void UCore_RagdollSystem::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
    RagdollSettings = NewSettings;
}

bool UCore_RagdollSystem::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Transitioning;
}

float UCore_RagdollSystem::GetRagdollActiveTime() const
{
    return RagdollActiveTime;
}

void UCore_RagdollSystem::ApplyImpactForce(const FVector& Force, const FVector& Location)
{
    if (!TargetMesh || !IsRagdollActive())
        return;
        
    TargetMesh->AddImpulseAtLocation(Force, Location);
}

void UCore_RagdollSystem::SetPhysicsBlendWeight(float BlendWeight)
{
    TargetPhysicsBlend = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
}