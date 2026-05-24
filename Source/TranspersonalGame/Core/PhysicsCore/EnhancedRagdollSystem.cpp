#include "EnhancedRagdollSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogEnhancedRagdoll);

UEnhancedRagdollSystem::UEnhancedRagdollSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default values
    RagdollBlendTime = 0.5f;
    MaxRagdollDistance = 5000.0f;
    MinRagdollLifetime = 2.0f;
    MaxRagdollLifetime = 30.0f;
    PhysicsBlendWeight = 1.0f;
    
    bEnableRagdollOptimization = true;
    bEnableCollisionFiltering = true;
    bEnableConstraintDamping = true;
    bAutoCleanupRagdolls = true;
    
    RagdollState = ECore_RagdollState::Inactive;
    BlendAlpha = 0.0f;
    RagdollStartTime = 0.0f;
    
    // Initialize arrays
    ActiveRagdolls.Reserve(50);
    RagdollPool.Reserve(20);
}

void UEnhancedRagdollSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner reference
    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogEnhancedRagdoll, Error, TEXT("EnhancedRagdollSystem: No owner actor found"));
        return;
    }
    
    // Find skeletal mesh component
    SkeletalMeshComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogEnhancedRagdoll, Warning, TEXT("EnhancedRagdollSystem: No skeletal mesh component found on %s"), *OwnerActor->GetName());
    }
    
    // Initialize physics settings
    InitializePhysicsSettings();
    
    UE_LOG(LogEnhancedRagdoll, Log, TEXT("EnhancedRagdollSystem initialized for %s"), *OwnerActor->GetName());
}

void UEnhancedRagdollSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!SkeletalMeshComp || !OwnerActor)
    {
        return;
    }
    
    // Update ragdoll state
    UpdateRagdollState(DeltaTime);
    
    // Update active ragdolls
    UpdateActiveRagdolls(DeltaTime);
    
    // Handle blending
    if (RagdollState == ECore_RagdollState::Blending)
    {
        UpdateRagdollBlending(DeltaTime);
    }
    
    // Cleanup old ragdolls
    if (bAutoCleanupRagdolls)
    {
        CleanupOldRagdolls(DeltaTime);
    }
}

void UEnhancedRagdollSystem::ActivateRagdoll(const FVector& ImpactPoint, const FVector& ImpactForce, float BlendTime)
{
    if (!SkeletalMeshComp || RagdollState == ECore_RagdollState::Active)
    {
        return;
    }
    
    UE_LOG(LogEnhancedRagdoll, Log, TEXT("Activating ragdoll for %s with force %s"), 
           *OwnerActor->GetName(), *ImpactForce.ToString());
    
    // Store blend time
    RagdollBlendTime = BlendTime > 0.0f ? BlendTime : RagdollBlendTime;
    
    // Set physics simulation
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply impulse if provided
    if (!ImpactForce.IsZero())
    {
        FName ClosestBone = FindClosestBone(ImpactPoint);
        if (ClosestBone != NAME_None)
        {
            SkeletalMeshComp->AddImpulseAtLocation(ImpactForce, ImpactPoint, ClosestBone);
        }
        else
        {
            SkeletalMeshComp->AddImpulseAtLocation(ImpactForce, ImpactPoint);
        }
    }
    
    // Update state
    RagdollState = ECore_RagdollState::Blending;
    BlendAlpha = 0.0f;
    RagdollStartTime = GetWorld()->GetTimeSeconds();
    
    // Add to active ragdolls list
    FCore_RagdollInstance NewInstance;
    NewInstance.SkeletalMeshComponent = SkeletalMeshComp;
    NewInstance.OwnerActor = OwnerActor;
    NewInstance.StartTime = RagdollStartTime;
    NewInstance.BlendWeight = 0.0f;
    NewInstance.ImpactLocation = ImpactPoint;
    NewInstance.ImpactForce = ImpactForce;
    
    ActiveRagdolls.Add(NewInstance);
    
    // Broadcast event
    OnRagdollActivated.Broadcast(OwnerActor, ImpactPoint, ImpactForce);
}

void UEnhancedRagdollSystem::DeactivateRagdoll(float BlendTime)
{
    if (!SkeletalMeshComp || RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    UE_LOG(LogEnhancedRagdoll, Log, TEXT("Deactivating ragdoll for %s"), *OwnerActor->GetName());
    
    // Store blend time
    RagdollBlendTime = BlendTime > 0.0f ? BlendTime : RagdollBlendTime;
    
    // Start blending back to animation
    RagdollState = ECore_RagdollState::Blending;
    BlendAlpha = 1.0f; // Start from full ragdoll, blend to animation
    
    // Remove from active ragdolls
    ActiveRagdolls.RemoveAll([this](const FCore_RagdollInstance& Instance)
    {
        return Instance.SkeletalMeshComponent == SkeletalMeshComp;
    });
    
    // Broadcast event
    OnRagdollDeactivated.Broadcast(OwnerActor);
}

void UEnhancedRagdollSystem::SetRagdollPhysicsBlend(float BlendWeight)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    PhysicsBlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    
    // Apply blend weight to skeletal mesh
    SkeletalMeshComp->SetAllBodiesPhysicsBlendWeight(PhysicsBlendWeight);
}

void UEnhancedRagdollSystem::ApplyImpulseToRagdoll(const FVector& Impulse, const FVector& Location, FName BoneName)
{
    if (!SkeletalMeshComp || RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location, BoneName);
    }
    else
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location);
    }
    
    UE_LOG(LogEnhancedRagdoll, Log, TEXT("Applied impulse %s to bone %s at location %s"), 
           *Impulse.ToString(), *BoneName.ToString(), *Location.ToString());
}

void UEnhancedRagdollSystem::SetConstraintDamping(FName BoneName, float LinearDamping, float AngularDamping)
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Apply damping to specific bone or all bones
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->SetAllBodiesBelowLinearDamping(BoneName, LinearDamping, true);
        SkeletalMeshComp->SetAllBodiesBelowAngularDamping(BoneName, AngularDamping, true);
    }
    else
    {
        SkeletalMeshComp->SetAllBodiesLinearDamping(LinearDamping);
        SkeletalMeshComp->SetAllBodiesAngularDamping(AngularDamping);
    }
}

bool UEnhancedRagdollSystem::IsRagdollActive() const
{
    return RagdollState != ECore_RagdollState::Inactive;
}

FName UEnhancedRagdollSystem::FindClosestBone(const FVector& WorldLocation) const
{
    if (!SkeletalMeshComp)
    {
        return NAME_None;
    }
    
    FName ClosestBone = NAME_None;
    float ClosestDistance = MAX_FLT;
    
    const TArray<FName>& BoneNames = SkeletalMeshComp->GetAllSocketNames();
    
    for (const FName& BoneName : BoneNames)
    {
        FVector BoneLocation = SkeletalMeshComp->GetSocketLocation(BoneName);
        float Distance = FVector::Dist(WorldLocation, BoneLocation);
        
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBone = BoneName;
        }
    }
    
    return ClosestBone;
}

void UEnhancedRagdollSystem::InitializePhysicsSettings()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    // Set default physics settings
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Initialize constraint settings if enabled
    if (bEnableConstraintDamping)
    {
        SetConstraintDamping(NAME_None, 0.1f, 0.1f);
    }
    
    UE_LOG(LogEnhancedRagdoll, Log, TEXT("Physics settings initialized for %s"), *OwnerActor->GetName());
}

void UEnhancedRagdollSystem::UpdateRagdollState(float DeltaTime)
{
    if (RagdollState == ECore_RagdollState::Inactive)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float RagdollAge = CurrentTime - RagdollStartTime;
    
    // Check if ragdoll should be deactivated due to lifetime
    if (RagdollAge > MaxRagdollLifetime)
    {
        DeactivateRagdoll();
        return;
    }
    
    // Check distance-based deactivation
    if (bEnableRagdollOptimization)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(OwnerActor->GetActorLocation(), PlayerPawn->GetActorLocation());
            if (DistanceToPlayer > MaxRagdollDistance && RagdollAge > MinRagdollLifetime)
            {
                DeactivateRagdoll();
                return;
            }
        }
    }
}

void UEnhancedRagdollSystem::UpdateActiveRagdolls(float DeltaTime)
{
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        FCore_RagdollInstance& Instance = ActiveRagdolls[i];
        
        if (!Instance.SkeletalMeshComponent || !Instance.OwnerActor)
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        // Update blend weight
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float Age = CurrentTime - Instance.StartTime;
        Instance.BlendWeight = FMath::Clamp(Age / RagdollBlendTime, 0.0f, 1.0f);
        
        // Apply blend weight
        Instance.SkeletalMeshComponent->SetAllBodiesPhysicsBlendWeight(Instance.BlendWeight);
    }
}

void UEnhancedRagdollSystem::UpdateRagdollBlending(float DeltaTime)
{
    if (RagdollBlendTime <= 0.0f)
    {
        // Instant transition
        BlendAlpha = (RagdollState == ECore_RagdollState::Active) ? 1.0f : 0.0f;
        RagdollState = (BlendAlpha > 0.5f) ? ECore_RagdollState::Active : ECore_RagdollState::Inactive;
        return;
    }
    
    // Update blend alpha
    float BlendSpeed = 1.0f / RagdollBlendTime;
    
    if (RagdollState == ECore_RagdollState::Blending)
    {
        if (BlendAlpha < 1.0f)
        {
            BlendAlpha += BlendSpeed * DeltaTime;
            if (BlendAlpha >= 1.0f)
            {
                BlendAlpha = 1.0f;
                RagdollState = ECore_RagdollState::Active;
            }
        }
        else
        {
            BlendAlpha -= BlendSpeed * DeltaTime;
            if (BlendAlpha <= 0.0f)
            {
                BlendAlpha = 0.0f;
                RagdollState = ECore_RagdollState::Inactive;
                
                // Disable physics simulation
                if (SkeletalMeshComp)
                {
                    SkeletalMeshComp->SetSimulatePhysics(false);
                    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
            }
        }
    }
    
    // Apply blend weight
    SetRagdollPhysicsBlend(BlendAlpha);
}

void UEnhancedRagdollSystem::CleanupOldRagdolls(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        const FCore_RagdollInstance& Instance = ActiveRagdolls[i];
        float Age = CurrentTime - Instance.StartTime;
        
        if (Age > MaxRagdollLifetime)
        {
            // Move to pool for reuse
            if (RagdollPool.Num() < 20) // Limit pool size
            {
                RagdollPool.Add(Instance);
            }
            
            ActiveRagdolls.RemoveAt(i);
            UE_LOG(LogEnhancedRagdoll, Log, TEXT("Cleaned up old ragdoll instance"));
        }
    }
}