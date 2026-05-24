#include "AdvancedDestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdvancedDestruction, Log, All);

UAdvancedDestructionSystem::UAdvancedDestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Default destruction parameters
    DestructionThreshold = 1000.0f;
    FragmentCount = 50;
    FragmentLifetime = 30.0f;
    ExplosionForce = 5000.0f;
    ExplosionRadius = 500.0f;
    bUseGeometryCollection = true;
    bEnableFieldSystem = true;
    
    // Performance settings
    MaxActiveFragments = 200;
    FragmentCullingDistance = 2000.0f;
    bEnableLODSystem = true;
    
    // Initialize arrays
    ActiveFragments.Reserve(MaxActiveFragments);
    PendingDestructions.Reserve(10);
}

void UAdvancedDestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize destruction system
    InitializeDestructionSystem();
    
    UE_LOG(LogAdvancedDestruction, Log, TEXT("AdvancedDestructionSystem initialized for %s"), 
           *GetOwner()->GetName());
}

void UAdvancedDestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active fragments
    UpdateActiveFragments(DeltaTime);
    
    // Process pending destructions
    ProcessPendingDestructions(DeltaTime);
    
    // Cleanup expired fragments
    CleanupExpiredFragments();
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
}

void UAdvancedDestructionSystem::InitializeDestructionSystem()
{
    // Get the owner's mesh component
    OwnerMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!OwnerMeshComponent)
    {
        UE_LOG(LogAdvancedDestruction, Warning, 
               TEXT("No StaticMeshComponent found on %s"), *GetOwner()->GetName());
        return;
    }
    
    // Setup geometry collection if enabled
    if (bUseGeometryCollection)
    {
        SetupGeometryCollection();
    }
    
    // Setup field system if enabled
    if (bEnableFieldSystem)
    {
        SetupFieldSystem();
    }
    
    // Cache original mesh data
    CacheOriginalMeshData();
    
    bIsInitialized = true;
}

void UAdvancedDestructionSystem::SetupGeometryCollection()
{
    // Create geometry collection component if not exists
    GeometryCollectionComponent = GetOwner()->FindComponentByClass<UGeometryCollectionComponent>();
    if (!GeometryCollectionComponent)
    {
        GeometryCollectionComponent = NewObject<UGeometryCollectionComponent>(GetOwner());
        GetOwner()->AddInstanceComponent(GeometryCollectionComponent);
        GeometryCollectionComponent->AttachToComponent(
            GetOwner()->GetRootComponent(),
            FAttachmentTransformRules::KeepWorldTransform
        );
    }
    
    // Configure geometry collection settings
    if (GeometryCollectionComponent)
    {
        GeometryCollectionComponent->SetNotifyBreaks(true);
        GeometryCollectionComponent->SetDamageThreshold(DestructionThreshold);
        
        // Bind to break events
        GeometryCollectionComponent->OnChaosBreakEvent.AddDynamic(
            this, &UAdvancedDestructionSystem::OnGeometryCollectionBreak
        );
    }
}

void UAdvancedDestructionSystem::SetupFieldSystem()
{
    // Create field system component
    FieldSystemComponent = GetOwner()->FindComponentByClass<UFieldSystemComponent>();
    if (!FieldSystemComponent)
    {
        FieldSystemComponent = NewObject<UFieldSystemComponent>(GetOwner());
        GetOwner()->AddInstanceComponent(FieldSystemComponent);
    }
}

void UAdvancedDestructionSystem::CacheOriginalMeshData()
{
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetStaticMesh())
    {
        return;
    }
    
    OriginalMesh = OwnerMeshComponent->GetStaticMesh();
    OriginalMaterials = OwnerMeshComponent->GetMaterials();
    OriginalTransform = OwnerMeshComponent->GetComponentTransform();
}

void UAdvancedDestructionSystem::TriggerDestruction(const FVector& ImpactPoint, 
                                                   const FVector& ImpactDirection, 
                                                   float ImpactForce)
{
    if (!bIsInitialized || bIsDestroyed)
    {
        return;
    }
    
    // Check if force exceeds threshold
    if (ImpactForce < DestructionThreshold)
    {
        // Apply damage but don't destroy
        ApplyDamage(ImpactPoint, ImpactDirection, ImpactForce);
        return;
    }
    
    UE_LOG(LogAdvancedDestruction, Log, 
           TEXT("Triggering destruction on %s with force %f"), 
           *GetOwner()->GetName(), ImpactForce);
    
    // Create destruction data
    FCore_DestructionData DestructionData;
    DestructionData.ImpactPoint = ImpactPoint;
    DestructionData.ImpactDirection = ImpactDirection.GetSafeNormal();
    DestructionData.ImpactForce = ImpactForce;
    DestructionData.Timestamp = GetWorld()->GetTimeSeconds();
    
    // Add to pending destructions for next tick processing
    PendingDestructions.Add(DestructionData);
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(ImpactPoint, ImpactDirection, ImpactForce);
}

void UAdvancedDestructionSystem::ProcessPendingDestructions(float DeltaTime)
{
    for (int32 i = PendingDestructions.Num() - 1; i >= 0; i--)
    {
        const FCore_DestructionData& DestructionData = PendingDestructions[i];
        
        // Process the destruction
        ExecuteDestruction(DestructionData);
        
        // Remove from pending
        PendingDestructions.RemoveAt(i);
    }
}

void UAdvancedDestructionSystem::ExecuteDestruction(const FCore_DestructionData& DestructionData)
{
    if (bUseGeometryCollection && GeometryCollectionComponent)
    {
        // Use geometry collection for destruction
        ExecuteGeometryCollectionDestruction(DestructionData);
    }
    else
    {
        // Use custom fragmentation
        ExecuteCustomDestruction(DestructionData);
    }
    
    // Apply field forces if enabled
    if (bEnableFieldSystem && FieldSystemComponent)
    {
        ApplyFieldForces(DestructionData);
    }
    
    // Mark as destroyed
    bIsDestroyed = true;
    
    // Hide original mesh
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->SetVisibility(false);
        OwnerMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Broadcast completion event
    OnDestructionCompleted.Broadcast(DestructionData.ImpactPoint);
}

void UAdvancedDestructionSystem::ExecuteGeometryCollectionDestruction(const FCore_DestructionData& DestructionData)
{
    if (!GeometryCollectionComponent)
    {
        return;
    }
    
    // Apply break impulse at impact point
    GeometryCollectionComponent->ApplyBreakingLinearVelocity(
        DestructionData.ImpactPoint,
        DestructionData.ImpactDirection * DestructionData.ImpactForce
    );
    
    // Apply radial force
    GeometryCollectionComponent->ApplyBreakingAngularVelocity(
        DestructionData.ImpactPoint,
        ExplosionRadius
    );
}

void UAdvancedDestructionSystem::ExecuteCustomDestruction(const FCore_DestructionData& DestructionData)
{
    if (!OwnerMeshComponent || !OriginalMesh)
    {
        return;
    }
    
    // Generate fragments around impact point
    TArray<FVector> FragmentPositions;
    GenerateFragmentPositions(DestructionData.ImpactPoint, FragmentPositions);
    
    // Create fragment actors
    for (int32 i = 0; i < FMath::Min(FragmentPositions.Num(), FragmentCount); i++)
    {
        CreateFragment(FragmentPositions[i], DestructionData);
    }
}

void UAdvancedDestructionSystem::GenerateFragmentPositions(const FVector& ImpactPoint, 
                                                          TArray<FVector>& OutPositions)
{
    OutPositions.Empty();
    
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // Get mesh bounds
    FBoxSphereBounds MeshBounds = OwnerMeshComponent->GetStaticMesh()->GetBounds();
    FVector BoundsExtent = MeshBounds.BoxExtent;
    FVector MeshCenter = OwnerMeshComponent->GetComponentLocation();
    
    // Generate random positions within mesh bounds
    for (int32 i = 0; i < FragmentCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BoundsExtent.X, BoundsExtent.X),
            FMath::RandRange(-BoundsExtent.Y, BoundsExtent.Y),
            FMath::RandRange(-BoundsExtent.Z, BoundsExtent.Z)
        );
        
        FVector FragmentPosition = MeshCenter + RandomOffset;
        OutPositions.Add(FragmentPosition);
    }
}

void UAdvancedDestructionSystem::CreateFragment(const FVector& Position, 
                                               const FCore_DestructionData& DestructionData)
{
    if (ActiveFragments.Num() >= MaxActiveFragments)
    {
        return;
    }
    
    // Spawn fragment actor
    AActor* FragmentActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Position, FRotator::ZeroRotator);
    if (!FragmentActor)
    {
        return;
    }
    
    // Add mesh component
    UStaticMeshComponent* FragmentMesh = NewObject<UStaticMeshComponent>(FragmentActor);
    FragmentActor->SetRootComponent(FragmentMesh);
    
    // Set fragment mesh (simplified version of original)
    if (OriginalMesh)
    {
        FragmentMesh->SetStaticMesh(OriginalMesh);
        FragmentMesh->SetWorldScale3D(FVector(0.1f)); // Scale down fragments
    }
    
    // Set materials
    for (int32 i = 0; i < OriginalMaterials.Num(); i++)
    {
        FragmentMesh->SetMaterial(i, OriginalMaterials[i]);
    }
    
    // Enable physics
    FragmentMesh->SetSimulatePhysics(true);
    FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply impulse
    FVector ImpulseDirection = (Position - DestructionData.ImpactPoint).GetSafeNormal();
    float ImpulseStrength = ExplosionForce * FMath::RandRange(0.5f, 1.5f);
    FragmentMesh->AddImpulse(ImpulseDirection * ImpulseStrength);
    
    // Create fragment data
    FCore_FragmentData FragmentData;
    FragmentData.Actor = FragmentActor;
    FragmentData.MeshComponent = FragmentMesh;
    FragmentData.CreationTime = GetWorld()->GetTimeSeconds();
    FragmentData.Lifetime = FragmentLifetime;
    
    ActiveFragments.Add(FragmentData);
    
    // Schedule cleanup
    FTimerHandle CleanupTimer;
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimer,
        [this, FragmentActor]()
        {
            if (IsValid(FragmentActor))
            {
                FragmentActor->Destroy();
            }
        },
        FragmentLifetime,
        false
    );
}

void UAdvancedDestructionSystem::ApplyFieldForces(const FCore_DestructionData& DestructionData)
{
    if (!FieldSystemComponent)
    {
        return;
    }
    
    // Create radial force field
    // Note: Field system implementation would require more complex setup
    // This is a simplified version for demonstration
    
    UE_LOG(LogAdvancedDestruction, Log, 
           TEXT("Applying field forces at %s"), 
           *DestructionData.ImpactPoint.ToString());
}

void UAdvancedDestructionSystem::UpdateActiveFragments(float DeltaTime)
{
    // Update fragment states and apply LOD
    for (int32 i = ActiveFragments.Num() - 1; i >= 0; i--)
    {
        FCore_FragmentData& Fragment = ActiveFragments[i];
        
        if (!IsValid(Fragment.Actor))
        {
            ActiveFragments.RemoveAt(i);
            continue;
        }
        
        // Update lifetime
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float Age = CurrentTime - Fragment.CreationTime;
        
        if (Age >= Fragment.Lifetime)
        {
            Fragment.Actor->Destroy();
            ActiveFragments.RemoveAt(i);
            continue;
        }
        
        // Apply LOD based on distance
        if (bEnableLODSystem)
        {
            ApplyFragmentLOD(Fragment);
        }
    }
}

void UAdvancedDestructionSystem::ApplyFragmentLOD(FCore_FragmentData& Fragment)
{
    if (!Fragment.MeshComponent || !GetWorld())
    {
        return;
    }
    
    // Get distance to camera/player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float Distance = FVector::Dist(
        Fragment.Actor->GetActorLocation(),
        PlayerPawn->GetActorLocation()
    );
    
    // Apply LOD based on distance
    if (Distance > FragmentCullingDistance)
    {
        Fragment.MeshComponent->SetVisibility(false);
    }
    else
    {
        Fragment.MeshComponent->SetVisibility(true);
        
        // Scale detail based on distance
        float LODScale = FMath::Clamp(1.0f - (Distance / FragmentCullingDistance), 0.1f, 1.0f);
        Fragment.MeshComponent->SetWorldScale3D(FVector(LODScale * 0.1f));
    }
}

void UAdvancedDestructionSystem::CleanupExpiredFragments()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveFragments.Num() - 1; i >= 0; i--)
    {
        const FCore_FragmentData& Fragment = ActiveFragments[i];
        
        if (!IsValid(Fragment.Actor))
        {
            ActiveFragments.RemoveAt(i);
            continue;
        }
        
        float Age = CurrentTime - Fragment.CreationTime;
        if (Age >= Fragment.Lifetime)
        {
            Fragment.Actor->Destroy();
            ActiveFragments.RemoveAt(i);
        }
    }
}

void UAdvancedDestructionSystem::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update performance counters
    PerformanceMetrics.ActiveFragmentCount = ActiveFragments.Num();
    PerformanceMetrics.PendingDestructionCount = PendingDestructions.Num();
    PerformanceMetrics.FrameTime = DeltaTime;
    
    // Log performance warnings
    if (ActiveFragments.Num() > MaxActiveFragments * 0.8f)
    {
        UE_LOG(LogAdvancedDestruction, Warning, 
               TEXT("High fragment count: %d/%d"), 
               ActiveFragments.Num(), MaxActiveFragments);
    }
}

void UAdvancedDestructionSystem::ApplyDamage(const FVector& ImpactPoint, 
                                            const FVector& ImpactDirection, 
                                            float Damage)
{
    // Accumulate damage
    AccumulatedDamage += Damage;
    
    // Check if accumulated damage exceeds threshold
    if (AccumulatedDamage >= DestructionThreshold)
    {
        TriggerDestruction(ImpactPoint, ImpactDirection, AccumulatedDamage);
    }
    else
    {
        // Apply visual damage effects
        ApplyDamageEffects(ImpactPoint, Damage / DestructionThreshold);
    }
}

void UAdvancedDestructionSystem::ApplyDamageEffects(const FVector& ImpactPoint, float DamageRatio)
{
    // Apply visual damage effects (cracks, deformation, etc.)
    // This would typically involve material parameter changes or decals
    
    OnDamageApplied.Broadcast(ImpactPoint, DamageRatio);
}

void UAdvancedDestructionSystem::OnGeometryCollectionBreak(const FChaosBreakEvent& BreakEvent)
{
    UE_LOG(LogAdvancedDestruction, Log, 
           TEXT("Geometry collection break event at %s"), 
           *BreakEvent.Location.ToString());
    
    // Handle geometry collection break event
    OnDestructionCompleted.Broadcast(BreakEvent.Location);
}

void UAdvancedDestructionSystem::ResetDestruction()
{
    if (!bIsDestroyed)
    {
        return;
    }
    
    // Cleanup all fragments
    for (const FCore_FragmentData& Fragment : ActiveFragments)
    {
        if (IsValid(Fragment.Actor))
        {
            Fragment.Actor->Destroy();
        }
    }
    ActiveFragments.Empty();
    
    // Clear pending destructions
    PendingDestructions.Empty();
    
    // Reset state
    bIsDestroyed = false;
    AccumulatedDamage = 0.0f;
    
    // Restore original mesh
    if (OwnerMeshComponent)
    {
        OwnerMeshComponent->SetVisibility(true);
        OwnerMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    UE_LOG(LogAdvancedDestruction, Log, 
           TEXT("Destruction reset for %s"), 
           *GetOwner()->GetName());
}

FCore_DestructionPerformanceMetrics UAdvancedDestructionSystem::GetPerformanceMetrics() const
{
    return PerformanceMetrics;
}