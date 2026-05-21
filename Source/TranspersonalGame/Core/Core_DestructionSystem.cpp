#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Field/FieldSystemActor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize destruction profile with default values
    DestructionProfile.BreakThreshold = 1000.0f;
    DestructionProfile.ImpactDamageMultiplier = 1.5f;
    DestructionProfile.MaxFragments = 50;
    DestructionProfile.FragmentLifetime = 30.0f;
    DestructionProfile.bEnableDebris = true;
    DestructionProfile.DebrisScale = 0.1f;
    
    bEnableDestruction = true;
    bUseChaosPhysics = true;
    bUseGeometryCollection = false;
    MaxDestructionDistance = 10000.0f;
    bShowDebugInfo = false;
    bDrawDestructionSphere = true;
    
    TotalDestructionCount = 0;
    AverageDestructionProcessTime = 0.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDestructionSystem();
    
    // Setup cleanup timer for expired fragments
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FragmentCleanupTimer,
            this,
            &UCore_DestructionSystem::CleanupExpiredFragments,
            5.0f, // Every 5 seconds
            true  // Looping
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: BeginPlay completed"));
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bShowDebugInfo)
    {
        DebugDrawDestructionInfo();
    }
}

void UCore_DestructionSystem::InitializeDestructionSystem()
{
    if (!bEnableDestruction)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Destruction disabled"));
        return;
    }
    
    // Clear any existing data
    ActiveFragments.Empty();
    RecentDestructions.Empty();
    
    // Initialize Chaos Physics if enabled
    if (bUseChaosPhysics)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Chaos Physics enabled"));
    }
    
    // Initialize Geometry Collection if enabled
    if (bUseGeometryCollection)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Geometry Collection enabled"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Initialization complete"));
}

bool UCore_DestructionSystem::ProcessImpactDestruction(AActor* TargetActor, const FVector& ImpactLocation, const FVector& ImpactVelocity, float DamageAmount)
{
    if (!TargetActor || !bEnableDestruction)
    {
        return false;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Check if actor can be destroyed
    if (!CanDestroyActor(TargetActor))
    {
        return false;
    }
    
    // Calculate effective damage
    float EffectiveDamage = DamageAmount * DestructionProfile.ImpactDamageMultiplier;
    
    // Check if damage exceeds break threshold
    if (EffectiveDamage < DestructionProfile.BreakThreshold)
    {
        return false;
    }
    
    // Calculate fragment count based on damage
    int32 FragmentCount = FMath::Clamp(
        FMath::FloorToInt(EffectiveDamage / DestructionProfile.BreakThreshold * 10),
        1,
        DestructionProfile.MaxFragments
    );
    
    // Create destruction event
    FCore_DestructionEvent DestructionEvent;
    DestructionEvent.DestroyedActor = TargetActor;
    DestructionEvent.ImpactLocation = ImpactLocation;
    DestructionEvent.ImpactVelocity = ImpactVelocity;
    DestructionEvent.DamageAmount = EffectiveDamage;
    DestructionEvent.FragmentCount = FragmentCount;
    
    // Create fragments
    CreateDestructionFragments(TargetActor, ImpactLocation, FragmentCount);
    
    // Register the destruction event
    RegisterDestructionEvent(DestructionEvent);
    
    // Broadcast destruction event
    OnDestructionEvent.Broadcast(DestructionEvent);
    
    // Destroy the original actor
    TargetActor->Destroy();
    
    // Update performance stats
    float ProcessTime = FPlatformTime::Seconds() - StartTime;
    UpdatePerformanceStats(ProcessTime);
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Processed destruction of %s with %d fragments"), 
           *TargetActor->GetName(), FragmentCount);
    
    return true;
}

void UCore_DestructionSystem::CreateDestructionFragments(AActor* SourceActor, const FVector& ImpactLocation, int32 FragmentCount)
{
    if (!SourceActor || FragmentCount <= 0)
    {
        return;
    }
    
    // Get source actor's mesh component
    UStaticMeshComponent* SourceMesh = SourceActor->FindComponentByClass<UStaticMeshComponent>();
    if (!SourceMesh || !SourceMesh->GetStaticMesh())
    {
        return;
    }
    
    // Create physics fragments
    CreatePhysicsFragments(SourceActor, ImpactLocation, FragmentCount);
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Created %d fragments for %s"), 
           FragmentCount, *SourceActor->GetName());
}

void UCore_DestructionSystem::CreatePhysicsFragments(AActor* SourceActor, const FVector& ImpactLocation, int32 FragmentCount)
{
    UWorld* World = GetWorld();
    if (!World || !SourceActor)
    {
        return;
    }
    
    UStaticMeshComponent* SourceMesh = SourceActor->FindComponentByClass<UStaticMeshComponent>();
    if (!SourceMesh)
    {
        return;
    }
    
    FVector SourceLocation = SourceActor->GetActorLocation();
    FVector SourceScale = SourceActor->GetActorScale3D();
    
    for (int32 i = 0; i < FragmentCount; ++i)
    {
        // Create fragment actor
        AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>();
        if (!Fragment)
        {
            continue;
        }
        
        // Setup fragment mesh
        UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
        if (FragmentMesh)
        {
            FragmentMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
            FragmentMesh->SetMaterial(0, SourceMesh->GetMaterial(0));
            
            // Scale down fragment
            float FragmentScale = DestructionProfile.DebrisScale * FMath::RandRange(0.5f, 1.5f);
            Fragment->SetActorScale3D(SourceScale * FragmentScale);
            
            // Position fragment near impact location
            FVector FragmentLocation = ImpactLocation + FMath::VRand() * 100.0f;
            Fragment->SetActorLocation(FragmentLocation);
            
            // Enable physics
            FragmentMesh->SetSimulatePhysics(true);
            FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            FragmentMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            
            // Apply random impulse
            FVector RandomImpulse = FMath::VRand() * FMath::RandRange(500.0f, 2000.0f);
            FragmentMesh->AddImpulse(RandomImpulse);
            
            // Set fragment lifetime
            Fragment->SetLifeSpan(DestructionProfile.FragmentLifetime);
            
            // Add to active fragments
            ActiveFragments.Add(Fragment);
        }
    }
}

void UCore_DestructionSystem::ApplyDestructionForce(const FVector& ForceLocation, float ForceRadius, float ForceStrength)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all actors within force radius
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World,
        ForceLocation,
        ForceRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AStaticMeshActor::StaticClass(),
        TArray<AActor*>(),
        OverlappingActors
    );
    
    for (AActor* Actor : OverlappingActors)
    {
        if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            if (MeshComp->IsSimulatingPhysics())
            {
                FVector ForceDirection = (Actor->GetActorLocation() - ForceLocation).GetSafeNormal();
                float Distance = FVector::Dist(Actor->GetActorLocation(), ForceLocation);
                float ForceMagnitude = ForceStrength * (1.0f - Distance / ForceRadius);
                
                MeshComp->AddImpulse(ForceDirection * ForceMagnitude);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Applied destruction force to %d actors"), 
           OverlappingActors.Num());
}

void UCore_DestructionSystem::SetDestructionProfile(const FCore_DestructionProfile& NewProfile)
{
    DestructionProfile = NewProfile;
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Destruction profile updated"));
}

void UCore_DestructionSystem::CleanupExpiredFragments()
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveFragments.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ActiveFragments[i]))
        {
            ActiveFragments.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Cleaned up %d expired fragments"), RemovedCount);
    }
}

void UCore_DestructionSystem::EnableChaosDestruction(AActor* TargetActor)
{
    if (!TargetActor || !bUseChaosPhysics)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = TargetActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        ConfigureChaosBreaking(MeshComp, DestructionProfile.BreakThreshold);
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Enabled Chaos destruction for %s"), 
               *TargetActor->GetName());
    }
}

void UCore_DestructionSystem::ConfigureChaosBreaking(UStaticMeshComponent* MeshComponent, float BreakThreshold)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Enable physics simulation
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Configure breaking parameters
    FBodyInstance* BodyInstance = MeshComponent->GetBodyInstance();
    if (BodyInstance)
    {
        BodyInstance->bNotifyRigidBodyCollision = true;
        // Additional Chaos-specific configuration would go here
    }
}

void UCore_DestructionSystem::SetupGeometryCollection(AActor* TargetActor, UGeometryCollection* GeometryCollection)
{
    if (!TargetActor || !GeometryCollection || !bUseGeometryCollection)
    {
        return;
    }
    
    // This would setup Geometry Collection component
    // Implementation depends on specific Geometry Collection setup
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Geometry Collection setup for %s"), 
           *TargetActor->GetName());
}

void UCore_DestructionSystem::TriggerGeometryCollectionBreak(AActor* TargetActor, const FVector& BreakLocation)
{
    if (!TargetActor || !bUseGeometryCollection)
    {
        return;
    }
    
    // Trigger Geometry Collection breaking at specified location
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Triggered Geometry Collection break for %s"), 
           *TargetActor->GetName());
}

void UCore_DestructionSystem::CreateDestructionField(const FVector& FieldLocation, float FieldRadius, float FieldStrength)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create Field System actor for destruction field
    AFieldSystemActor* FieldActor = World->SpawnActor<AFieldSystemActor>();
    if (FieldActor)
    {
        FieldActor->SetActorLocation(FieldLocation);
        // Configure field parameters
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Created destruction field at %s"), 
               *FieldLocation.ToString());
    }
}

void UCore_DestructionSystem::DebugDrawDestructionInfo()
{
    UWorld* World = GetWorld();
    if (!World || !bShowDebugInfo)
    {
        return;
    }
    
    // Draw active fragments
    for (AActor* Fragment : ActiveFragments)
    {
        if (IsValid(Fragment))
        {
            DrawDebugSphere(World, Fragment->GetActorLocation(), 50.0f, 12, FColor::Red, false, 0.1f);
        }
    }
    
    // Draw destruction sphere if enabled
    if (bDrawDestructionSphere && GetOwner())
    {
        DrawDebugSphere(World, GetOwner()->GetActorLocation(), MaxDestructionDistance, 24, FColor::Orange, false, 0.1f);
    }
}

void UCore_DestructionSystem::GetDestructionStats(int32& TotalDestructions, int32& ActiveFragments_Out, float& AverageProcessTime)
{
    TotalDestructions = TotalDestructionCount;
    ActiveFragments_Out = ActiveFragments.Num();
    AverageProcessTime = AverageDestructionProcessTime;
}

bool UCore_DestructionSystem::CanDestroyActor(AActor* Actor) const
{
    if (!Actor || !bEnableDestruction)
    {
        return false;
    }
    
    // Check distance from owner
    if (GetOwner())
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance > MaxDestructionDistance)
        {
            return false;
        }
    }
    
    // Check if actor has a mesh component
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return false;
    }
    
    return true;
}

void UCore_DestructionSystem::ApplyFragmentPhysics(AActor* Fragment, const FVector& ImpactVelocity)
{
    if (!Fragment)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Fragment->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->IsSimulatingPhysics())
    {
        FVector RandomizedVelocity = ImpactVelocity + FMath::VRand() * 500.0f;
        MeshComp->SetPhysicsLinearVelocity(RandomizedVelocity);
        
        FVector RandomAngularVelocity = FMath::VRand() * 10.0f;
        MeshComp->SetPhysicsAngularVelocityInRadians(RandomAngularVelocity);
    }
}

void UCore_DestructionSystem::RegisterDestructionEvent(const FCore_DestructionEvent& Event)
{
    RecentDestructions.Add(Event);
    
    // Keep only recent events (last 100)
    if (RecentDestructions.Num() > 100)
    {
        RecentDestructions.RemoveAt(0);
    }
    
    TotalDestructionCount++;
}

void UCore_DestructionSystem::UpdatePerformanceStats(float ProcessTime)
{
    // Update rolling average
    if (TotalDestructionCount == 1)
    {
        AverageDestructionProcessTime = ProcessTime;
    }
    else
    {
        AverageDestructionProcessTime = (AverageDestructionProcessTime * 0.9f) + (ProcessTime * 0.1f);
    }
}