#include "Core_RagdollPhysicsSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Animation/AnimInstance.h"

UCore_RagdollPhysicsSystem::UCore_RagdollPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize state
    CurrentState = ECore_RagdollState::Inactive;
    StateTimer = 0.0f;
    BlendAlpha = 0.0f;
    LastTriggerType = ECore_RagdollTrigger::Manual;
    LastImpactForce = FVector::ZeroVector;
    LastPerformanceUpdate = 0.0f;
    
    // Initialize components
    SkeletalMeshComponent = nullptr;
    ArchitecturalFramework = nullptr;
}

void UCore_RagdollPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find skeletal mesh component on owner
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_RagdollPhysicsSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
        else
        {
            SetupPhysicsAsset();
        }
    }
    
    // Register with architectural framework
    RegisterWithArchitecturalFramework();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysicsSystem initialized on %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Update ragdoll state machine
    UpdateRagdollState(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Report to architectural framework periodically
    if (GetWorld()->GetTimeSeconds() - LastPerformanceUpdate > 1.0f)
    {
        ReportPerformanceToFramework();
        LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UCore_RagdollPhysicsSystem::ActivateRagdoll(ECore_RagdollTrigger TriggerType, FVector ImpactForce)
{
    if (!SkeletalMeshComponent || CurrentState == ECore_RagdollState::Active)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Activating ragdoll on %s - Trigger: %d"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), (int32)TriggerType);
    
    // Store trigger information
    LastTriggerType = TriggerType;
    LastImpactForce = ImpactForce;
    
    // Transition to activating state
    CurrentState = ECore_RagdollState::Activating;
    StateTimer = 0.0f;
    BlendAlpha = 0.0f;
    
    // Configure physics based on trigger type
    switch (TriggerType)
    {
        case ECore_RagdollTrigger::Death:
            RagdollConfig.bAutoRecover = false;
            RagdollConfig.MaxRagdollTime = -1.0f; // Infinite
            break;
            
        case ECore_RagdollTrigger::DinosaurAttack:
            RagdollConfig.ActivationForce = 800.0f;
            RagdollConfig.RecoveryTime = 3.0f;
            break;
            
        case ECore_RagdollTrigger::HeavyImpact:
            RagdollConfig.ActivationForce = 600.0f;
            RagdollConfig.RecoveryTime = 2.5f;
            break;
            
        case ECore_RagdollTrigger::Falling:
            RagdollConfig.ActivationForce = 400.0f;
            RagdollConfig.RecoveryTime = 2.0f;
            break;
            
        default:
            break;
    }
    
    // Update performance metrics
    PerformanceMetrics.TotalActivations++;
    PerformanceMetrics.ActiveRagdolls++;
}

void UCore_RagdollPhysicsSystem::DeactivateRagdoll()
{
    if (CurrentState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Deactivating ragdoll on %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // Transition to recovering state
    CurrentState = ECore_RagdollState::Recovering;
    StateTimer = 0.0f;
    
    // Update performance metrics
    if (PerformanceMetrics.ActiveRagdolls > 0)
    {
        PerformanceMetrics.ActiveRagdolls--;
    }
}

void UCore_RagdollPhysicsSystem::SetRagdollConfig(const FCore_RagdollConfig& NewConfig)
{
    RagdollConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("Ragdoll config updated on %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_RagdollPhysicsSystem::ApplyDinosaurAttackRagdoll(FVector AttackDirection, float AttackForce)
{
    FVector ImpactForce = AttackDirection.GetSafeNormal() * AttackForce;
    ActivateRagdoll(ECore_RagdollTrigger::DinosaurAttack, ImpactForce);
    
    // Apply additional force for dramatic effect
    if (SkeletalMeshComponent && SkeletalMeshComponent->GetPhysicsAsset())
    {
        ApplyImpactForce(ImpactForce * 1.5f, GetOwner()->GetActorLocation());
    }
}

void UCore_RagdollPhysicsSystem::ApplyFallingRagdoll(float FallHeight, FVector ImpactPoint)
{
    // Calculate impact force based on fall height
    float ImpactForce = FMath::Clamp(FallHeight * 100.0f, 200.0f, 1000.0f);
    FVector DownwardForce = FVector(0, 0, -ImpactForce);
    
    ActivateRagdoll(ECore_RagdollTrigger::Falling, DownwardForce);
    
    if (SkeletalMeshComponent)
    {
        ApplyImpactForce(DownwardForce, ImpactPoint);
    }
}

void UCore_RagdollPhysicsSystem::ApplyWeaponImpactRagdoll(FVector ImpactPoint, FVector ImpactDirection, float ImpactForce)
{
    FVector Force = ImpactDirection.GetSafeNormal() * ImpactForce;
    ActivateRagdoll(ECore_RagdollTrigger::HeavyImpact, Force);
    
    if (SkeletalMeshComponent)
    {
        ApplyImpactForce(Force, ImpactPoint);
    }
}

void UCore_RagdollPhysicsSystem::RegisterWithArchitecturalFramework()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            ArchitecturalFramework = GameInstance->GetSubsystem<UEngineArchitecturalFramework>();
            if (ArchitecturalFramework)
            {
                UE_LOG(LogTemp, Log, TEXT("Core_RagdollPhysicsSystem registered with EngineArchitecturalFramework"));
            }
        }
    }
}

void UCore_RagdollPhysicsSystem::DebugDrawRagdollState()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FColor StateColor = FColor::White;
    FString StateText = TEXT("Unknown");
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Inactive:
            StateColor = FColor::Green;
            StateText = TEXT("Inactive");
            break;
        case ECore_RagdollState::Activating:
            StateColor = FColor::Yellow;
            StateText = TEXT("Activating");
            break;
        case ECore_RagdollState::Active:
            StateColor = FColor::Red;
            StateText = TEXT("Active");
            break;
        case ECore_RagdollState::Blending:
            StateColor = FColor::Orange;
            StateText = TEXT("Blending");
            break;
        case ECore_RagdollState::Recovering:
            StateColor = FColor::Blue;
            StateText = TEXT("Recovering");
            break;
    }
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), 
                   FString::Printf(TEXT("Ragdoll: %s (%.1fs)"), *StateText, StateTimer),
                   nullptr, StateColor, 0.0f);
}

void UCore_RagdollPhysicsSystem::UpdateRagdollState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ECore_RagdollState::Activating:
            BlendToRagdoll(DeltaTime);
            break;
            
        case ECore_RagdollState::Active:
            // Check for auto-recovery
            if (RagdollConfig.bAutoRecover && 
                RagdollConfig.MaxRagdollTime > 0.0f && 
                StateTimer >= RagdollConfig.MaxRagdollTime)
            {
                DeactivateRagdoll();
            }
            break;
            
        case ECore_RagdollState::Recovering:
            BlendFromRagdoll(DeltaTime);
            break;
            
        case ECore_RagdollState::Blending:
            // Handle blending completion
            if (StateTimer >= RagdollConfig.BlendTime)
            {
                CurrentState = ECore_RagdollState::Active;
                StateTimer = 0.0f;
                BlendAlpha = 1.0f;
            }
            break;
            
        default:
            break;
    }
}

void UCore_RagdollPhysicsSystem::BlendToRagdoll(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    BlendAlpha = FMath::Clamp(StateTimer / RagdollConfig.BlendTime, 0.0f, 1.0f);
    
    if (BlendAlpha >= 1.0f)
    {
        // Enable full ragdoll physics
        SkeletalMeshComponent->SetSimulatePhysics(true);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Apply initial impact force
        if (!LastImpactForce.IsNearlyZero())
        {
            ApplyImpactForce(LastImpactForce, GetOwner()->GetActorLocation());
        }
        
        CurrentState = ECore_RagdollState::Active;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Ragdoll fully activated on %s"), *GetOwner()->GetName());
    }
}

void UCore_RagdollPhysicsSystem::BlendFromRagdoll(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    BlendAlpha = FMath::Clamp(1.0f - (StateTimer / RagdollConfig.RecoveryTime), 0.0f, 1.0f);
    
    if (BlendAlpha <= 0.0f)
    {
        // Disable ragdoll physics
        SkeletalMeshComponent->SetSimulatePhysics(false);
        SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        CurrentState = ECore_RagdollState::Inactive;
        StateTimer = 0.0f;
        BlendAlpha = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Ragdoll recovery complete on %s"), *GetOwner()->GetName());
    }
}

void UCore_RagdollPhysicsSystem::UpdatePerformanceMetrics(float DeltaTime)
{
    PerformanceMetrics.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Track blend time history for average calculation
    if (CurrentState == ECore_RagdollState::Activating || CurrentState == ECore_RagdollState::Recovering)
    {
        BlendTimeHistory.Add(StateTimer);
        
        // Keep only recent history (last 10 blends)
        if (BlendTimeHistory.Num() > 10)
        {
            BlendTimeHistory.RemoveAt(0);
        }
        
        // Calculate average blend time
        float TotalBlendTime = 0.0f;
        for (float BlendTime : BlendTimeHistory)
        {
            TotalBlendTime += BlendTime;
        }
        PerformanceMetrics.AverageBlendTime = BlendTimeHistory.Num() > 0 ? 
            TotalBlendTime / BlendTimeHistory.Num() : 0.0f;
    }
}

void UCore_RagdollPhysicsSystem::SetupPhysicsAsset()
{
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Ensure physics asset is available
    UPhysicsAsset* PhysicsAsset = SkeletalMeshComponent->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Physics Asset found for %s"), *GetOwner()->GetName());
        return;
    }
    
    // Configure ragdoll constraints for realistic prehistoric movement
    ConfigureRagdollConstraints();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Asset configured for ragdoll on %s"), *GetOwner()->GetName());
}

void UCore_RagdollPhysicsSystem::ApplyImpactForce(FVector Force, FVector ImpactPoint)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Apply force to the closest bone
    FName ClosestBone = SkeletalMeshComponent->FindClosestBone(ImpactPoint);
    if (ClosestBone != NAME_None)
    {
        SkeletalMeshComponent->AddImpulseAtLocation(Force, ImpactPoint, ClosestBone);
        UE_LOG(LogTemp, Log, TEXT("Applied impact force %.1f to bone %s"), 
               Force.Size(), *ClosestBone.ToString());
    }
}

void UCore_RagdollPhysicsSystem::ConfigureRagdollConstraints()
{
    // Configure physics constraints for realistic prehistoric creature movement
    // This would be expanded based on specific creature types (human, dinosaur, etc.)
    
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetPhysicsAsset())
    {
        return;
    }
    
    // Set appropriate collision responses for prehistoric environment
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void UCore_RagdollPhysicsSystem::ReportPerformanceToFramework()
{
    if (!ArchitecturalFramework)
    {
        return;
    }
    
    // Report performance metrics to architectural framework
    // This integrates with the performance monitoring system
    UE_LOG(LogTemp, VeryVerbose, TEXT("Reporting ragdoll performance: %d active, %.2f avg blend time"), 
           PerformanceMetrics.ActiveRagdolls, PerformanceMetrics.AverageBlendTime);
}

bool UCore_RagdollPhysicsSystem::ValidateArchitecturalCompliance()
{
    // Validate that the ragdoll system meets architectural requirements
    bool bCompliant = true;
    
    if (PerformanceMetrics.AverageBlendTime > 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll blend time exceeds architectural limits"));
        bCompliant = false;
    }
    
    if (PerformanceMetrics.ActiveRagdolls > 20)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too many active ragdolls for performance target"));
        bCompliant = false;
    }
    
    return bCompliant;
}