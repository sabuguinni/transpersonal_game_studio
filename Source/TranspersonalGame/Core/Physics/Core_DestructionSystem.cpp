#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize destruction data with defaults
    DestructionData.Health = 100.0f;
    DestructionData.DamageThreshold = 50.0f;
    DestructionData.DestructionType = ECore_DestructionType::Fragmentation;
    DestructionData.FragmentCount = 8;
    DestructionData.FragmentLifetime = 10.0f;
    DestructionData.bApplyPhysicsToFragments = true;
    
    bCanBeDestroyed = true;
    bDestroyOnZeroHealth = true;
    ImpactForceMultiplier = 1.0f;
    bIsDestroyed = false;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure we have a valid mesh component
    UStaticMeshComponent* MeshComp = GetOwnerMeshComponent();
    if (MeshComp)
    {
        // Enable collision events if not already enabled
        MeshComp->SetNotifyRigidBodyCollision(true);
        UE_LOG(LogTemp, Log, TEXT("DestructionSystem initialized for actor: %s"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: No StaticMeshComponent found on owner actor"));
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up destroyed fragments periodically
    if (SpawnedFragments.Num() > 0)
    {
        SpawnedFragments.RemoveAll([](AActor* Fragment) {
            return !IsValid(Fragment);
        });
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, FVector ImpactLocation, FVector ImpactForce)
{
    if (!bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }
    
    float PreviousHealth = DestructionData.Health;
    DestructionData.Health = FMath::Max(0.0f, DestructionData.Health - DamageAmount);
    
    // Broadcast damage event
    OnDamageReceived.Broadcast(DamageAmount, DestructionData.Health);
    
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Applied %.2f damage, health: %.2f/%.2f"), 
           DamageAmount, DestructionData.Health, 100.0f);
    
    // Check if we should trigger destruction
    if (DestructionData.Health <= 0.0f && bDestroyOnZeroHealth)
    {
        TriggerDestruction(ImpactLocation, ImpactForce);
    }
    else if (DamageAmount >= DestructionData.DamageThreshold)
    {
        // Apply impact force to the mesh if available
        UStaticMeshComponent* MeshComp = GetOwnerMeshComponent();
        if (MeshComp && MeshComp->IsSimulatingPhysics())
        {
            FVector AdjustedForce = ImpactForce * ImpactForceMultiplier;
            MeshComp->AddImpulseAtLocation(AdjustedForce, ImpactLocation);
        }
    }
}

void UCore_DestructionSystem::TriggerDestruction(FVector ImpactLocation, FVector ImpactForce)
{
    if (bIsDestroyed || !bCanBeDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    // Use owner's location if impact location is zero
    if (ImpactLocation.IsZero())
    {
        ImpactLocation = GetOwner()->GetActorLocation();
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Triggering destruction of type %d"), 
           static_cast<int32>(DestructionData.DestructionType));
    
    // Handle different destruction types
    switch (DestructionData.DestructionType)
    {
        case ECore_DestructionType::Fragmentation:
            CreateFragments(ImpactLocation, ImpactForce);
            break;
            
        case ECore_DestructionType::Collapse:
            HandleCollapse(ImpactLocation);
            break;
            
        case ECore_DestructionType::Explosion:
            HandleExplosion(ImpactLocation, 500.0f);
            break;
            
        case ECore_DestructionType::Erosion:
            // Gradual destruction - could be implemented with timeline
            CreateFragments(ImpactLocation, ImpactForce * 0.5f);
            break;
            
        default:
            CreateFragments(ImpactLocation, ImpactForce);
            break;
    }
    
    // Broadcast destruction event
    OnDestruction.Broadcast(GetOwner(), ImpactLocation, DestructionData.DestructionType);
    
    // Hide or destroy the original actor after a short delay
    FTimerHandle DestroyTimer;
    GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
    {
        if (IsValid(GetOwner()))
        {
            GetOwner()->SetActorHiddenInGame(true);
            GetOwner()->SetActorEnableCollision(false);
        }
    }, 0.1f, false);
}

void UCore_DestructionSystem::CreateFragments(FVector ImpactLocation, FVector ImpactForce)
{
    UStaticMeshComponent* OriginalMesh = GetOwnerMeshComponent();
    if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Cannot create fragments - no valid mesh"));
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FRotator OwnerRotation = GetOwner()->GetActorRotation();
    FVector OwnerScale = GetOwner()->GetActorScale3D();
    
    // Create fragments around the impact location
    for (int32 i = 0; i < DestructionData.FragmentCount; i++)
    {
        // Calculate fragment spawn location
        float Angle = (2.0f * PI * i) / DestructionData.FragmentCount;
        float Radius = FMath::RandRange(50.0f, 200.0f);
        FVector FragmentOffset = FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        FVector FragmentLocation = OwnerLocation + FragmentOffset;
        FRotator FragmentRotation = FRotator(
            FMath::RandRange(-45.0f, 45.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-45.0f, 45.0f)
        );
        
        // Spawn fragment actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            FragmentLocation,
            FragmentRotation,
            SpawnParams
        );
        
        if (Fragment)
        {
            UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
            if (FragmentMesh)
            {
                // Set the same mesh as original (in a real implementation, you'd use fractured pieces)
                FragmentMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
                FragmentMesh->SetMaterial(0, OriginalMesh->GetMaterial(0));
                
                // Scale down the fragment
                float FragmentScale = FMath::RandRange(0.2f, 0.6f);
                Fragment->SetActorScale3D(OwnerScale * FragmentScale);
                
                // Enable physics if requested
                if (DestructionData.bApplyPhysicsToFragments)
                {
                    FragmentMesh->SetSimulatePhysics(true);
                    FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    
                    // Apply impulse based on impact force and fragment location
                    FVector FragmentVelocity = CalculateFragmentVelocity(ImpactLocation, FragmentLocation, ImpactForce);
                    FragmentMesh->SetPhysicsLinearVelocity(FragmentVelocity);
                    
                    // Add some angular velocity for realistic tumbling
                    FVector AngularVelocity = FVector(
                        FMath::RandRange(-720.0f, 720.0f),
                        FMath::RandRange(-720.0f, 720.0f),
                        FMath::RandRange(-720.0f, 720.0f)
                    );
                    FragmentMesh->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
                }
                
                // Set fragment lifetime
                Fragment->SetLifeSpan(DestructionData.FragmentLifetime);
                
                // Track the fragment
                SpawnedFragments.Add(Fragment);
                
                UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Created fragment %d at location %s"), 
                       i, *FragmentLocation.ToString());
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Created %d fragments"), DestructionData.FragmentCount);
}

void UCore_DestructionSystem::HandleCollapse(FVector ImpactLocation)
{
    // For collapse, create fewer, larger fragments that fall downward
    int32 CollapseFragments = FMath::Max(3, DestructionData.FragmentCount / 2);
    
    // Temporarily modify fragment count for collapse
    int32 OriginalCount = DestructionData.FragmentCount;
    DestructionData.FragmentCount = CollapseFragments;
    
    // Create fragments with downward force
    FVector DownwardForce = FVector(0.0f, 0.0f, -1000.0f);
    CreateFragments(ImpactLocation, DownwardForce);
    
    // Restore original fragment count
    DestructionData.FragmentCount = OriginalCount;
}

void UCore_DestructionSystem::HandleExplosion(FVector ImpactLocation, float ExplosionRadius)
{
    // Create fragments with outward explosive force
    FVector ExplosiveForce = FVector(0.0f, 0.0f, 500.0f); // Upward component
    CreateFragments(ImpactLocation, ExplosiveForce);
    
    // Apply additional explosive force to nearby objects
    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        ImpactLocation,
        ExplosionRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AStaticMeshActor::StaticClass(),
        TArray<AActor*>(),
        NearbyActors
    );
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != GetOwner())
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                FVector Direction = (Actor->GetActorLocation() - ImpactLocation).GetSafeNormal();
                float Distance = FVector::Dist(Actor->GetActorLocation(), ImpactLocation);
                float ForceMagnitude = FMath::Lerp(2000.0f, 500.0f, Distance / ExplosionRadius);
                
                MeshComp->AddImpulse(Direction * ForceMagnitude, NAME_None, true);
            }
        }
    }
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    return (DestructionData.Health / 100.0f) * 100.0f;
}

void UCore_DestructionSystem::RepairDamage(float RepairAmount)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    float PreviousHealth = DestructionData.Health;
    DestructionData.Health = FMath::Min(100.0f, DestructionData.Health + RepairAmount);
    
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Repaired %.2f damage, health: %.2f/100.0f"), 
           RepairAmount, DestructionData.Health);
}

void UCore_DestructionSystem::SetDestructionType(ECore_DestructionType NewType)
{
    DestructionData.DestructionType = NewType;
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Changed destruction type to %d"), static_cast<int32>(NewType));
}

UStaticMeshComponent* UCore_DestructionSystem::GetOwnerMeshComponent() const
{
    if (!GetOwner())
    {
        return nullptr;
    }
    
    return GetOwner()->FindComponentByClass<UStaticMeshComponent>();
}

void UCore_DestructionSystem::CleanupFragments()
{
    for (AActor* Fragment : SpawnedFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    SpawnedFragments.Empty();
}

FVector UCore_DestructionSystem::CalculateFragmentVelocity(FVector ImpactLocation, FVector FragmentLocation, FVector ImpactForce) const
{
    // Calculate direction from impact to fragment
    FVector Direction = (FragmentLocation - ImpactLocation).GetSafeNormal();
    
    // Add some randomization
    Direction += FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(0.1f, 0.5f)  // Slight upward bias
    );
    Direction.Normalize();
    
    // Calculate velocity magnitude based on impact force
    float VelocityMagnitude = FMath::Clamp(ImpactForce.Size() * 0.1f, 200.0f, 1000.0f);
    
    return Direction * VelocityMagnitude;
}