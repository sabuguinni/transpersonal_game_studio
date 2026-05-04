#include "Core_RagdollSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Kismet/GameplayStatics.h"

// UCore_RagdollComponent Implementation
UCore_RagdollComponent::UCore_RagdollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default settings
    RagdollSettings = FCore_RagdollSettings();
    CurrentState = ECore_RagdollState::Disabled;
    StateTime = 0.0f;
    BlendWeight = 0.0f;
    bWasSimulatingPhysics = false;
}

void UCore_RagdollComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeRagdollSystem();
    
    // Register with ragdoll manager if it exists
    if (ACore_RagdollManager* Manager = Cast<ACore_RagdollManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ACore_RagdollManager::StaticClass())))
    {
        Manager->RegisterRagdollComponent(this);
    }
}

void UCore_RagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState != ECore_RagdollState::Disabled)
    {
        UpdateRagdollState(DeltaTime);
        
        // Performance culling
        if (bUseDistanceCulling && ShouldCullRagdoll())
        {
            DeactivateRagdoll();
        }
    }
}

void UCore_RagdollComponent::InitializeRagdollSystem()
{
    // Find target skeletal mesh if not set
    if (!TargetMesh)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (TargetMesh)
    {
        // Store original physics state
        bWasSimulatingPhysics = TargetMesh->IsSimulatingPhysics();
        
        // Set custom physics asset if provided
        if (CustomPhysicsAsset)
        {
            TargetMesh->SetPhysicsAsset(CustomPhysicsAsset);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Ragdoll system initialized for %s"), 
               *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh found for ragdoll component on %s"), 
               *GetOwner()->GetName());
    }
}

void UCore_RagdollComponent::ActivateRagdoll(bool bForceActivation)
{
    if (!TargetMesh || (CurrentState == ECore_RagdollState::Active && !bForceActivation))
    {
        return;
    }
    
    // Check velocity threshold unless forced
    if (!bForceActivation)
    {
        FVector Velocity = TargetMesh->GetComponentVelocity();
        if (Velocity.Size() < RagdollSettings.MinVelocityForRagdoll)
        {
            return;
        }
    }
    
    // Disable character collision
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Enable physics simulation
    TargetMesh->SetSimulatePhysics(true);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Start blending
    SetRagdollState(ECore_RagdollState::Transitioning);
    BlendWeight = 0.0f;
    StateTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll activated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollComponent::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Disabled)
    {
        return;
    }
    
    SetRagdollState(ECore_RagdollState::Recovering);
    StateTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll deactivating for %s"), *GetOwner()->GetName());
}

void UCore_RagdollComponent::SetRagdollState(ECore_RagdollState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTime = 0.0f;
        
        // Handle state-specific logic
        switch (NewState)
        {
            case ECore_RagdollState::Disabled:
                BlendWeight = 0.0f;
                if (TargetMesh)
                {
                    TargetMesh->SetSimulatePhysics(false);
                    // Re-enable character collision
                    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
                    {
                        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
                        {
                            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                        }
                    }
                }
                break;
                
            case ECore_RagdollState::Active:
                BlendWeight = 1.0f;
                break;
        }
    }
}

bool UCore_RagdollComponent::IsRagdollActive() const
{
    return CurrentState == ECore_RagdollState::Active || CurrentState == ECore_RagdollState::Transitioning;
}

void UCore_RagdollComponent::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName)
{
    if (!TargetMesh || !IsRagdollActive())
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        TargetMesh->AddImpulseAtLocation(Impulse, TargetMesh->GetBoneLocation(BoneName), BoneName);
    }
    else
    {
        TargetMesh->AddImpulse(Impulse);
    }
}

void UCore_RagdollComponent::SetPhysicsBlendWeight(float BlendWeight)
{
    this->BlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    
    if (TargetMesh && RagdollSettings.bUsePhysicsBlending)
    {
        TargetMesh->SetPhysicsBlendWeight(this->BlendWeight);
    }
}

FVector UCore_RagdollComponent::GetRagdollVelocity() const
{
    if (TargetMesh && IsRagdollActive())
    {
        return TargetMesh->GetComponentVelocity();
    }
    return FVector::ZeroVector;
}

void UCore_RagdollComponent::UpdateRagdollState(float DeltaTime)
{
    StateTime += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Transitioning:
            BlendToRagdoll(DeltaTime);
            break;
            
        case ECore_RagdollState::Active:
            // Check for auto-recovery
            if (RagdollSettings.bAutoRecover)
            {
                CheckRecoveryConditions();
            }
            
            // Check max ragdoll time
            if (StateTime >= RagdollSettings.MaxRagdollTime)
            {
                DeactivateRagdoll();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            BlendFromRagdoll(DeltaTime);
            break;
    }
    
    // Update position tracking
    if (TargetMesh)
    {
        LastPosition = TargetMesh->GetComponentLocation();
    }
}

void UCore_RagdollComponent::BlendToRagdoll(float DeltaTime)
{
    float BlendSpeed = 1.0f / FMath::Max(RagdollSettings.BlendInTime, 0.01f);
    BlendWeight = FMath::Min(BlendWeight + (BlendSpeed * DeltaTime), 1.0f);
    
    SetPhysicsBlendWeight(BlendWeight);
    
    if (BlendWeight >= 1.0f)
    {
        SetRagdollState(ECore_RagdollState::Active);
    }
}

void UCore_RagdollComponent::BlendFromRagdoll(float DeltaTime)
{
    float BlendSpeed = 1.0f / FMath::Max(RagdollSettings.BlendOutTime, 0.01f);
    BlendWeight = FMath::Max(BlendWeight - (BlendSpeed * DeltaTime), 0.0f);
    
    SetPhysicsBlendWeight(BlendWeight);
    
    if (BlendWeight <= 0.0f)
    {
        SetRagdollState(ECore_RagdollState::Disabled);
    }
}

void UCore_RagdollComponent::CheckRecoveryConditions()
{
    if (!TargetMesh)
    {
        return;
    }
    
    FVector CurrentVelocity = TargetMesh->GetComponentVelocity();
    if (CurrentVelocity.Size() < RagdollSettings.RecoveryThreshold)
    {
        DeactivateRagdoll();
    }
}

bool UCore_RagdollComponent::ShouldCullRagdoll() const
{
    if (!bUseDistanceCulling || !TargetMesh)
    {
        return false;
    }
    
    // Get player camera location
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        FVector RagdollLocation = TargetMesh->GetComponentLocation();
        
        float Distance = FVector::Dist(CameraLocation, RagdollLocation);
        return Distance > MaxRagdollDistance;
    }
    
    return false;
}

void UCore_RagdollComponent::TestRagdollActivation()
{
    ActivateRagdoll(true);
}

void UCore_RagdollComponent::ValidatePhysicsAsset()
{
    if (TargetMesh && TargetMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Log, TEXT("Physics asset validated for %s: %s"), 
               *GetOwner()->GetName(), 
               *TargetMesh->GetPhysicsAsset()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No physics asset found for %s"), 
               *GetOwner()->GetName());
    }
}

// ACore_RagdollManager Implementation
ACore_RagdollManager::ACore_RagdollManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    
    // Initialize settings
    GlobalRagdollSettings = FCore_RagdollSettings();
    MaxActiveRagdolls = 10;
    RagdollUpdateFrequency = 30.0f;
    CurrentActiveRagdolls = 0;
    
    UpdateInterval = 1.0f / RagdollUpdateFrequency;
}

void ACore_RagdollManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Manager initialized"));
}

void ACore_RagdollManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateAllRagdolls();
        ManageRagdollLimits();
        UpdatePerformanceMetrics();
        LastUpdateTime = 0.0f;
    }
}

void ACore_RagdollManager::RegisterRagdollComponent(UCore_RagdollComponent* Component)
{
    if (Component && !RegisteredRagdolls.Contains(Component))
    {
        RegisteredRagdolls.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Ragdoll component registered: %s"), 
               *Component->GetOwner()->GetName());
    }
}

void ACore_RagdollManager::UnregisterRagdollComponent(UCore_RagdollComponent* Component)
{
    if (Component)
    {
        RegisteredRagdolls.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Ragdoll component unregistered: %s"), 
               *Component->GetOwner()->GetName());
    }
}

void ACore_RagdollManager::UpdateAllRagdolls()
{
    // Clean up null references
    RegisteredRagdolls.RemoveAll([](UCore_RagdollComponent* Component)
    {
        return !IsValid(Component) || !IsValid(Component->GetOwner());
    });
}

void ACore_RagdollManager::OptimizeRagdollPerformance()
{
    CullDistantRagdolls();
    ManageRagdollLimits();
}

TArray<UCore_RagdollComponent*> ACore_RagdollManager::GetActiveRagdolls() const
{
    TArray<UCore_RagdollComponent*> ActiveRagdolls;
    
    for (UCore_RagdollComponent* Component : RegisteredRagdolls)
    {
        if (IsValid(Component) && Component->IsRagdollActive())
        {
            ActiveRagdolls.Add(Component);
        }
    }
    
    return ActiveRagdolls;
}

void ACore_RagdollManager::ManageRagdollLimits()
{
    TArray<UCore_RagdollComponent*> ActiveRagdolls = GetActiveRagdolls();
    CurrentActiveRagdolls = ActiveRagdolls.Num();
    
    // Deactivate oldest ragdolls if over limit
    if (CurrentActiveRagdolls > MaxActiveRagdolls)
    {
        int32 ToDeactivate = CurrentActiveRagdolls - MaxActiveRagdolls;
        
        // Sort by state time (oldest first)
        ActiveRagdolls.Sort([](const UCore_RagdollComponent& A, const UCore_RagdollComponent& B)
        {
            return A.StateTime > B.StateTime;
        });
        
        for (int32 i = 0; i < ToDeactivate && i < ActiveRagdolls.Num(); i++)
        {
            ActiveRagdolls[i]->DeactivateRagdoll();
        }
    }
}

void ACore_RagdollManager::CullDistantRagdolls()
{
    for (UCore_RagdollComponent* Component : RegisteredRagdolls)
    {
        if (IsValid(Component) && Component->IsRagdollActive())
        {
            if (Component->bUseDistanceCulling && Component->ShouldCullRagdoll())
            {
                Component->DeactivateRagdoll();
            }
        }
    }
}

void ACore_RagdollManager::UpdatePerformanceMetrics()
{
    CurrentActiveRagdolls = GetActiveRagdolls().Num();
}

void ACore_RagdollManager::TestAllRagdolls()
{
    for (UCore_RagdollComponent* Component : RegisteredRagdolls)
    {
        if (IsValid(Component))
        {
            Component->TestRagdollActivation();
        }
    }
}

void ACore_RagdollManager::ResetAllRagdolls()
{
    for (UCore_RagdollComponent* Component : RegisteredRagdolls)
    {
        if (IsValid(Component))
        {
            Component->SetRagdollState(ECore_RagdollState::Disabled);
        }
    }
}