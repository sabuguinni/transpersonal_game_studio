#include "PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize performance data
    PerformanceData = FCore_PhysicsPerformanceData();
    PerformanceData.ActiveBodiesCount = 0;
    PerformanceData.PhysicsUpdateTime = 0.0f;
    PerformanceData.CollisionChecksPerFrame = 0;
    PerformanceData.MemoryUsageMB = 0.0f;
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing physics system..."));
    
    InitializePhysicsSystem();
    CreateDefaultPhysicsMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics system initialized successfully"));
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsSimulation)
    {
        return;
    }
    
    // Update physics simulation at specified frequency
    LastPhysicsUpdateTime += DeltaTime;
    float UpdateInterval = 1.0f / PhysicsUpdateFrequency;
    
    if (LastPhysicsUpdateTime >= UpdateInterval)
    {
        UpdatePhysicsSimulation(LastPhysicsUpdateTime);
        LastPhysicsUpdateTime = 0.0f;
    }
    
    // Cleanup old ragdolls
    CleanupRagdolls();
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void UPhysicsSystemManager::InitializePhysicsSystem()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot initialize - no valid world"));
        return;
    }
    
    // Set global physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Apply gravity multiplier
        FVector CurrentGravity = PhysicsSettings->DefaultGravityZ * FVector(0, 0, 1);
        PhysicsSettings->DefaultGravityZ = CurrentGravity.Z * GravityMultiplier;
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Applied gravity multiplier %.2f"), GravityMultiplier);
    }
    
    // Initialize active physics bodies array
    ActivePhysicsBodies.Empty();
    UpdateActivePhysicsBodies();
    
    // Initialize ragdoll tracking
    ActiveRagdolls.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Found %d physics bodies in world"), ActivePhysicsBodies.Num());
}

void UPhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update active physics bodies list
    UpdateActivePhysicsBodies();
    
    // Performance tracking
    PerformanceData.PhysicsUpdateTime = DeltaTime;
    PerformanceData.ActiveBodiesCount = ActivePhysicsBodies.Num();
    
    // Limit physics simulation to nearby objects for performance
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        
        for (UPrimitiveComponent* PhysicsBody : ActivePhysicsBodies)
        {
            if (!PhysicsBody || !PhysicsBody->IsValidLowLevel())
            {
                continue;
            }
            
            float Distance = FVector::Dist(PhysicsBody->GetComponentLocation(), PlayerLocation);
            bool bShouldSimulate = Distance <= MaxPhysicsDistance;
            
            if (PhysicsBody->IsSimulatingPhysics() != bShouldSimulate)
            {
                PhysicsBody->SetSimulatePhysics(bShouldSimulate);
            }
        }
    }
}

void UPhysicsSystemManager::ApplyPhysicsMaterial(UPrimitiveComponent* Component, ECore_PhysicsMaterialType MaterialType)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot apply physics material - invalid component"));
        return;
    }
    
    UPhysicalMaterial* MaterialToApply = nullptr;
    
    switch (MaterialType)
    {
        case ECore_PhysicsMaterialType::Rock:
            MaterialToApply = RockPhysicsMaterial;
            break;
        case ECore_PhysicsMaterialType::Wood:
            MaterialToApply = WoodPhysicsMaterial;
            break;
        case ECore_PhysicsMaterialType::Flesh:
            MaterialToApply = FleshPhysicsMaterial;
            break;
        case ECore_PhysicsMaterialType::Ground:
            MaterialToApply = GroundPhysicsMaterial;
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Unknown physics material type"));
            return;
    }
    
    if (MaterialToApply)
    {
        Component->SetPhysMaterialOverride(MaterialToApply);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied physics material to component %s"), 
               *Component->GetName());
    }
}

void UPhysicsSystemManager::EnableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh || !bEnableRagdollPhysics)
    {
        return;
    }
    
    // Check if we're at the ragdoll limit
    if (ActiveRagdolls.Num() >= MaxActiveRagdolls)
    {
        // Remove oldest ragdoll
        if (ActiveRagdolls.Num() > 0)
        {
            USkeletalMeshComponent* OldestRagdoll = ActiveRagdolls[0];
            if (OldestRagdoll && OldestRagdoll->IsValidLowLevel())
            {
                DisableRagdollPhysics(OldestRagdoll);
            }
            ActiveRagdolls.RemoveAt(0);
        }
    }
    
    // Enable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Add to active ragdolls list
    ActiveRagdolls.Add(SkeletalMesh);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Enabled ragdoll physics for %s"), 
           *SkeletalMesh->GetName());
}

void UPhysicsSystemManager::DisableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Disable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Remove from active ragdolls list
    ActiveRagdolls.Remove(SkeletalMesh);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Disabled ragdoll physics for %s"), 
           *SkeletalMesh->GetName());
}

UPhysicsConstraintComponent* UPhysicsSystemManager::CreatePhysicsConstraint(
    UPrimitiveComponent* ComponentA, 
    UPrimitiveComponent* ComponentB,
    FVector ConstraintLocation)
{
    if (!ComponentA || !ComponentB)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot create constraint - invalid components"));
        return nullptr;
    }
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Cannot create constraint - no owner actor"));
        return nullptr;
    }
    
    // Create physics constraint component
    UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(OwnerActor);
    if (!Constraint)
    {
        return nullptr;
    }
    
    // Configure constraint
    Constraint->SetWorldLocation(ConstraintLocation);
    Constraint->SetConstrainedComponents(ComponentA, NAME_None, ComponentB, NAME_None);
    
    // Set constraint properties for realistic behavior
    Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);
    Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);
    Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Created physics constraint between %s and %s"), 
           *ComponentA->GetName(), *ComponentB->GetName());
    
    return Constraint;
}

void UPhysicsSystemManager::ApplyImpulse(UPrimitiveComponent* Component, FVector Impulse, FVector Location)
{
    if (!Component)
    {
        return;
    }
    
    if (Location == FVector::ZeroVector)
    {
        Location = Component->GetComponentLocation();
    }
    
    Component->AddImpulseAtLocation(Impulse, Location);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied impulse %.2f,%.2f,%.2f to %s"), 
           Impulse.X, Impulse.Y, Impulse.Z, *Component->GetName());
}

bool UPhysicsSystemManager::IsPhysicsSimulationActive(UPrimitiveComponent* Component) const
{
    if (!Component)
    {
        return false;
    }
    
    return Component->IsSimulatingPhysics();
}

ECore_PhysicsMaterialType UPhysicsSystemManager::GetPhysicsMaterialType(UPrimitiveComponent* Component) const
{
    if (!Component)
    {
        return ECore_PhysicsMaterialType::Ground;
    }
    
    UPhysicalMaterial* PhysMat = Component->GetBodyInstance()->GetSimplePhysicalMaterial();
    if (!PhysMat)
    {
        return ECore_PhysicsMaterialType::Ground;
    }
    
    // Compare with known materials
    if (PhysMat == RockPhysicsMaterial)
        return ECore_PhysicsMaterialType::Rock;
    if (PhysMat == WoodPhysicsMaterial)
        return ECore_PhysicsMaterialType::Wood;
    if (PhysMat == FleshPhysicsMaterial)
        return ECore_PhysicsMaterialType::Flesh;
    
    return ECore_PhysicsMaterialType::Ground;
}

FCore_PhysicsPerformanceData UPhysicsSystemManager::GetPhysicsPerformanceData() const
{
    return PerformanceData;
}

int32 UPhysicsSystemManager::GetActivePhysicsBodiesCount() const
{
    return ActivePhysicsBodies.Num();
}

void UPhysicsSystemManager::UpdateActivePhysicsBodies()
{
    if (!GetWorld())
    {
        return;
    }
    
    ActivePhysicsBodies.Empty();
    
    // Find all physics-enabled components in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->IsValidLowLevel())
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->IsSimulatingPhysics())
            {
                ActivePhysicsBodies.Add(Component);
            }
        }
    }
}

void UPhysicsSystemManager::CleanupRagdolls()
{
    // Remove invalid or expired ragdolls
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; --i)
    {
        USkeletalMeshComponent* Ragdoll = ActiveRagdolls[i];
        if (!Ragdoll || !Ragdoll->IsValidLowLevel())
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        // Check if ragdoll has been active too long
        AActor* RagdollOwner = Ragdoll->GetOwner();
        if (RagdollOwner)
        {
            float TimeSinceSpawn = GetWorld()->GetTimeSeconds() - RagdollOwner->GetGameTimeSinceCreation();
            if (TimeSinceSpawn > RagdollLifetime)
            {
                DisableRagdollPhysics(Ragdoll);
                ActiveRagdolls.RemoveAt(i);
            }
        }
    }
}

void UPhysicsSystemManager::UpdatePerformanceMetrics()
{
    // Update collision checks counter
    PerformanceData.CollisionChecksPerFrame = FMath::Min(ActivePhysicsBodies.Num() * 2, MaxCollisionChecksPerFrame);
    
    // Estimate memory usage (rough calculation)
    PerformanceData.MemoryUsageMB = (ActivePhysicsBodies.Num() * 0.1f) + (ActiveRagdolls.Num() * 2.0f);
    
    // Log performance warnings if needed
    if (PerformanceData.ActiveBodiesCount > 1000)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: High physics body count: %d"), 
               PerformanceData.ActiveBodiesCount);
    }
}

void UPhysicsSystemManager::CreateDefaultPhysicsMaterials()
{
    // Note: In a real implementation, these would be loaded from assets
    // For now, we'll create basic materials programmatically
    
    if (!RockPhysicsMaterial)
    {
        RockPhysicsMaterial = NewObject<UPhysicalMaterial>();
        if (RockPhysicsMaterial)
        {
            RockPhysicsMaterial->Friction = 0.8f;
            RockPhysicsMaterial->Restitution = 0.1f;
            RockPhysicsMaterial->Density = 2.5f;
        }
    }
    
    if (!WoodPhysicsMaterial)
    {
        WoodPhysicsMaterial = NewObject<UPhysicalMaterial>();
        if (WoodPhysicsMaterial)
        {
            WoodPhysicsMaterial->Friction = 0.6f;
            WoodPhysicsMaterial->Restitution = 0.3f;
            WoodPhysicsMaterial->Density = 0.8f;
        }
    }
    
    if (!FleshPhysicsMaterial)
    {
        FleshPhysicsMaterial = NewObject<UPhysicalMaterial>();
        if (FleshPhysicsMaterial)
        {
            FleshPhysicsMaterial->Friction = 0.5f;
            FleshPhysicsMaterial->Restitution = 0.2f;
            FleshPhysicsMaterial->Density = 1.0f;
        }
    }
    
    if (!GroundPhysicsMaterial)
    {
        GroundPhysicsMaterial = NewObject<UPhysicalMaterial>();
        if (GroundPhysicsMaterial)
        {
            GroundPhysicsMaterial->Friction = 0.7f;
            GroundPhysicsMaterial->Restitution = 0.1f;
            GroundPhysicsMaterial->Density = 1.8f;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Created default physics materials"));
}

bool UPhysicsSystemManager::ValidatePhysicsConfiguration() const
{
    if (GravityMultiplier <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Invalid gravity multiplier"));
        return false;
    }
    
    if (MaxPhysicsDistance <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Invalid max physics distance"));
        return false;
    }
    
    if (PhysicsUpdateFrequency < 30 || PhysicsUpdateFrequency > 120)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Invalid physics update frequency"));
        return false;
    }
    
    return true;
}