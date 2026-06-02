#include "Perf_CharacterPhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"

UPerf_CharacterPhysicsOptimizer::UPerf_CharacterPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default optimization settings
    OptimizationSettings.OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
    OptimizationSettings.MaxPhysicsDistance = 2000.0f;
    OptimizationSettings.MaxSimultaneousRagdolls = 10;
    OptimizationSettings.PhysicsTickRateScale = 1.0f;
    OptimizationSettings.bEnableDistanceCulling = true;
    OptimizationSettings.bEnableRagdollLimiting = true;
    OptimizationSettings.bEnableCollisionOptimization = true;
    
    MetricsUpdateInterval = 0.5f;
    LastMetricsUpdate = 0.0f;
    bOptimizationActive = false;
    CurrentPhysicsTickRate = 1.0f;
    CurrentRagdollCount = 0;
}

void UPerf_CharacterPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics optimization
    ApplyOptimizationLevel();
    bOptimizationActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Character Physics Optimizer initialized with level: %d"), 
           static_cast<int32>(OptimizationSettings.OptimizationLevel));
}

void UPerf_CharacterPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bOptimizationActive)
        return;
    
    // Update metrics periodically
    LastMetricsUpdate += DeltaTime;
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdatePhysicsMetrics();
        LastMetricsUpdate = 0.0f;
    }
    
    // Apply continuous optimizations
    UpdateDistanceCulling();
    ManageRagdollInstances();
    OptimizeCollisionQueries();
}

void UPerf_CharacterPhysicsOptimizer::OptimizeCharacterPhysics(ACharacter* Character)
{
    if (!Character || !bOptimizationActive)
        return;
    
    // Add to tracked characters if not already present
    TWeakObjectPtr<ACharacter> CharacterPtr(Character);
    if (!TrackedCharacters.Contains(CharacterPtr))
    {
        TrackedCharacters.Add(CharacterPtr);
    }
    
    // Apply optimization based on current level
    switch (OptimizationSettings.OptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Disabled:
            // No optimization
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            // Basic distance culling
            if (OptimizationSettings.bEnableDistanceCulling)
            {
                float Distance = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.8f)
                {
                    Character->GetCharacterMovement()->SetComponentTickEnabled(false);
                }
            }
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            // Distance culling + reduced physics tick rate
            if (OptimizationSettings.bEnableDistanceCulling)
            {
                float Distance = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.6f)
                {
                    Character->GetCharacterMovement()->SetComponentTickInterval(0.2f);
                }
            }
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            // Full optimization suite
            if (OptimizationSettings.bEnableDistanceCulling)
            {
                float Distance = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.4f)
                {
                    Character->GetCharacterMovement()->SetComponentTickInterval(0.5f);
                    Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
            break;
            
        case EPerf_PhysicsOptimizationLevel::Ultra:
            // Maximum optimization
            if (OptimizationSettings.bEnableDistanceCulling)
            {
                float Distance = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.2f)
                {
                    Character->SetActorTickEnabled(false);
                    Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics optimization to character: %s"), *Character->GetName());
}

void UPerf_CharacterPhysicsOptimizer::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel)
{
    OptimizationSettings.OptimizationLevel = NewLevel;
    ApplyOptimizationLevel();
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization level changed to: %d"), static_cast<int32>(NewLevel));
}

void UPerf_CharacterPhysicsOptimizer::EnableRagdollOptimization(bool bEnabled)
{
    OptimizationSettings.bEnableRagdollLimiting = bEnabled;
    
    if (bEnabled)
    {
        OptimizeRagdollPerformance();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll optimization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_CharacterPhysicsOptimizer::SetMaxPhysicsDistance(float Distance)
{
    OptimizationSettings.MaxPhysicsDistance = FMath::Clamp(Distance, 100.0f, 5000.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Max physics distance set to: %.2f"), OptimizationSettings.MaxPhysicsDistance);
}

FPerf_PhysicsMetrics UPerf_CharacterPhysicsOptimizer::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_CharacterPhysicsOptimizer::UpdatePhysicsMetrics()
{
    if (!GetWorld())
        return;
    
    PhysicsStartTime = FPlatformTime::Seconds();
    
    // Count active physics bodies
    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.RagdollInstances = 0;
    
    // Clean up invalid character references
    TrackedCharacters.RemoveAll([](const TWeakObjectPtr<ACharacter>& CharPtr) {
        return !CharPtr.IsValid();
    });
    
    ActiveRagdolls.RemoveAll([](const TWeakObjectPtr<ACharacter>& CharPtr) {
        return !CharPtr.IsValid();
    });
    
    // Count physics bodies and ragdolls
    for (const TWeakObjectPtr<ACharacter>& CharPtr : TrackedCharacters)
    {
        if (ACharacter* Character = CharPtr.Get())
        {
            if (Character->GetMesh() && Character->GetMesh()->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsBodies++;
                
                // Check if in ragdoll state
                if (Character->GetMesh()->IsSimulatingPhysics())
                {
                    CurrentMetrics.RagdollInstances++;
                }
            }
        }
    }
    
    CurrentRagdollCount = CurrentMetrics.RagdollInstances;
    
    // Calculate physics tick time (simplified)
    CurrentMetrics.PhysicsTickTime = (FPlatformTime::Seconds() - PhysicsStartTime) * 1000.0f;
    
    // Estimate collision query time
    CurrentMetrics.CollisionQueryTime = CurrentMetrics.ActivePhysicsBodies * 0.1f;
    
    // Count physics constraints (simplified)
    CurrentMetrics.PhysicsConstraints = CurrentMetrics.RagdollInstances * 15; // Approximate bones per ragdoll
    
    // Estimate memory usage
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActivePhysicsBodies * 0.5f) + (CurrentMetrics.RagdollInstances * 2.0f);
}

bool UPerf_CharacterPhysicsOptimizer::IsPhysicsOptimizationActive() const
{
    return bOptimizationActive && OptimizationSettings.OptimizationLevel != EPerf_PhysicsOptimizationLevel::Disabled;
}

void UPerf_CharacterPhysicsOptimizer::OptimizeRagdollPerformance()
{
    if (!OptimizationSettings.bEnableRagdollLimiting)
        return;
    
    LimitRagdollInstances();
    
    // Apply ragdoll-specific optimizations
    for (const TWeakObjectPtr<ACharacter>& CharPtr : ActiveRagdolls)
    {
        if (ACharacter* Character = CharPtr.Get())
        {
            if (USkeletalMeshComponent* Mesh = Character->GetMesh())
            {
                // Reduce ragdoll physics quality for distant characters
                float Distance = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                
                if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.5f)
                {
                    // Reduce physics substeps for distant ragdolls
                    Mesh->SetPhysicsLinearVelocity(Mesh->GetPhysicsLinearVelocity() * 0.9f);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll performance optimization applied to %d instances"), ActiveRagdolls.Num());
}

int32 UPerf_CharacterPhysicsOptimizer::GetActiveRagdollCount() const
{
    return CurrentRagdollCount;
}

void UPerf_CharacterPhysicsOptimizer::LimitRagdollInstances()
{
    if (ActiveRagdolls.Num() <= OptimizationSettings.MaxSimultaneousRagdolls)
        return;
    
    // Sort ragdolls by distance and disable the furthest ones
    ActiveRagdolls.Sort([this](const TWeakObjectPtr<ACharacter>& A, const TWeakObjectPtr<ACharacter>& B) {
        if (!A.IsValid() || !B.IsValid() || !GetOwner())
            return false;
        
        float DistA = FVector::DistSquared(A->GetActorLocation(), GetOwner()->GetActorLocation());
        float DistB = FVector::DistSquared(B->GetActorLocation(), GetOwner()->GetActorLocation());
        return DistA < DistB;
    });
    
    // Disable ragdoll physics for excess instances
    int32 ExcessCount = ActiveRagdolls.Num() - OptimizationSettings.MaxSimultaneousRagdolls;
    for (int32 i = OptimizationSettings.MaxSimultaneousRagdolls; i < ActiveRagdolls.Num(); ++i)
    {
        if (ACharacter* Character = ActiveRagdolls[i].Get())
        {
            if (USkeletalMeshComponent* Mesh = Character->GetMesh())
            {
                Mesh->SetSimulatePhysics(false);
                Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }
    }
    
    // Remove disabled ragdolls from active list
    ActiveRagdolls.RemoveAt(OptimizationSettings.MaxSimultaneousRagdolls, ExcessCount);
    
    UE_LOG(LogTemp, Log, TEXT("Limited ragdoll instances: disabled %d excess ragdolls"), ExcessCount);
}

void UPerf_CharacterPhysicsOptimizer::ApplyOptimizationLevel()
{
    switch (OptimizationSettings.OptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Disabled:
            CurrentPhysicsTickRate = 1.0f;
            OptimizationSettings.bEnableDistanceCulling = false;
            OptimizationSettings.bEnableRagdollLimiting = false;
            OptimizationSettings.bEnableCollisionOptimization = false;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            CurrentPhysicsTickRate = 0.9f;
            OptimizationSettings.MaxPhysicsDistance = 2500.0f;
            OptimizationSettings.MaxSimultaneousRagdolls = 15;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            CurrentPhysicsTickRate = 0.8f;
            OptimizationSettings.MaxPhysicsDistance = 2000.0f;
            OptimizationSettings.MaxSimultaneousRagdolls = 10;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            CurrentPhysicsTickRate = 0.6f;
            OptimizationSettings.MaxPhysicsDistance = 1500.0f;
            OptimizationSettings.MaxSimultaneousRagdolls = 8;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Ultra:
            CurrentPhysicsTickRate = 0.4f;
            OptimizationSettings.MaxPhysicsDistance = 1000.0f;
            OptimizationSettings.MaxSimultaneousRagdolls = 5;
            break;
    }
    
    UpdatePhysicsTickRate();
}

void UPerf_CharacterPhysicsOptimizer::UpdateDistanceCulling()
{
    if (!OptimizationSettings.bEnableDistanceCulling || !GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (const TWeakObjectPtr<ACharacter>& CharPtr : TrackedCharacters)
    {
        if (ACharacter* Character = CharPtr.Get())
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), OwnerLocation);
            
            // Enable/disable physics based on distance
            if (Distance > OptimizationSettings.MaxPhysicsDistance)
            {
                Character->GetCharacterMovement()->SetComponentTickEnabled(false);
                Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            else
            {
                Character->GetCharacterMovement()->SetComponentTickEnabled(true);
                Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
}

void UPerf_CharacterPhysicsOptimizer::ManageRagdollInstances()
{
    if (!OptimizationSettings.bEnableRagdollLimiting)
        return;
    
    // Update active ragdoll list
    ActiveRagdolls.RemoveAll([](const TWeakObjectPtr<ACharacter>& CharPtr) {
        if (!CharPtr.IsValid())
            return true;
        
        ACharacter* Character = CharPtr.Get();
        return !Character->GetMesh() || !Character->GetMesh()->IsSimulatingPhysics();
    });
    
    // Add new ragdolls
    for (const TWeakObjectPtr<ACharacter>& CharPtr : TrackedCharacters)
    {
        if (ACharacter* Character = CharPtr.Get())
        {
            if (Character->GetMesh() && Character->GetMesh()->IsSimulatingPhysics())
            {
                if (!ActiveRagdolls.Contains(CharPtr))
                {
                    ActiveRagdolls.Add(CharPtr);
                }
            }
        }
    }
    
    LimitRagdollInstances();
}

void UPerf_CharacterPhysicsOptimizer::OptimizeCollisionQueries()
{
    if (!OptimizationSettings.bEnableCollisionOptimization)
        return;
    
    // Reduce collision complexity for distant characters
    FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    for (const TWeakObjectPtr<ACharacter>& CharPtr : TrackedCharacters)
    {
        if (ACharacter* Character = CharPtr.Get())
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), OwnerLocation);
            
            if (Distance > OptimizationSettings.MaxPhysicsDistance * 0.7f)
            {
                // Use simpler collision for distant characters
                if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
                {
                    Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
                    Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                }
            }
        }
    }
}

void UPerf_CharacterPhysicsOptimizer::UpdatePhysicsTickRate()
{
    OptimizationSettings.PhysicsTickRateScale = CurrentPhysicsTickRate;
    
    // Apply tick rate to component
    SetComponentTickInterval(0.1f / CurrentPhysicsTickRate);
}