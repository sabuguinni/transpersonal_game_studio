#include "Core_DestructionSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default settings
    DestructionSettings = FCore_DestructionSettings();
    CurrentHealth = DestructionSettings.HealthPoints;
    
    // Material multipliers for realistic destruction
    WoodDestructionMultiplier = 1.0f;
    StoneDestructionMultiplier = 2.0f;
    MetalDestructionMultiplier = 3.0f;
    BoneDestructionMultiplier = 1.5f;
    
    bIsDestroyed = false;
    ActiveFragments.Empty();
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize health from settings
    CurrentHealth = DestructionSettings.HealthPoints;
    
    // Bind to actor's damage events if available
    if (AActor* Owner = GetOwner())
    {
        // Setup collision detection for impact-based destruction
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetNotifyRigidBodyCollision(true);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem initialized for %s with %f health"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), CurrentHealth);
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process fragment lifetimes
    if (ActiveFragments.Num() > 0)
    {
        ProcessFragmentLifetime(DeltaTime);
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (bIsDestroyed || DamageAmount <= 0.0f)
    {
        return;
    }
    
    // Apply material-specific damage multiplier
    float MaterialMultiplier = 1.0f;
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            MaterialMultiplier = GetMaterialDestructionMultiplier(PrimComp);
        }
    }
    
    float AdjustedDamage = DamageAmount * MaterialMultiplier;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - AdjustedDamage);
    
    UE_LOG(LogTemp, Log, TEXT("Destruction damage applied: %f (adjusted: %f), Health remaining: %f"), 
           DamageAmount, AdjustedDamage, CurrentHealth);
    
    // Check if destruction threshold is reached
    if (CurrentHealth <= 0.0f || ImpactForce.Size() >= DestructionSettings.DestructionThreshold)
    {
        TriggerDestruction(ImpactLocation, ImpactForce);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Destruction triggered for %s at location %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), *ImpactLocation.ToString());
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(GetOwner(), ImpactLocation);
    
    // Create fragments based on destruction type
    if (DestructionSettings.bCreateDebris)
    {
        CreateFragments(ImpactLocation, ImpactForce);
    }
    
    // Hide or destroy the original actor
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetVisibility(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Start cleanup timer for fragments
    StartFragmentCleanupTimer();
}

void UCore_DestructionSystem::CreateFragments(const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (!GetOwner() || ActiveFragments.Num() > 0)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate fragment positions based on destruction type
    TArray<FVector> FragmentPositions;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector OwnerExtent = GetOwner()->GetActorBounds(false).BoxExtent;
    
    for (int32 i = 0; i < DestructionSettings.FragmentCount; i++)
    {
        FVector RandomOffset;
        
        switch (DestructionSettings.DestructionType)
        {
            case ECore_DestructionType::Fracture:
                // Fracture creates larger pieces near impact
                RandomOffset = FMath::VRand() * OwnerExtent * FMath::RandRange(0.3f, 0.8f);
                break;
                
            case ECore_DestructionType::Shatter:
                // Shatter creates many small pieces
                RandomOffset = FMath::VRand() * OwnerExtent * FMath::RandRange(0.1f, 0.5f);
                break;
                
            case ECore_DestructionType::Explode:
                // Explosion spreads fragments outward from impact
                FVector Direction = (OwnerLocation - ImpactLocation).GetSafeNormal();
                if (Direction.IsNearlyZero())
                {
                    Direction = FVector::UpVector;
                }
                RandomOffset = Direction * FMath::RandRange(50.0f, 200.0f) + FMath::VRand() * 100.0f;
                break;
                
            default:
                RandomOffset = FMath::VRand() * OwnerExtent * 0.5f;
                break;
        }
        
        FragmentPositions.Add(OwnerLocation + RandomOffset);
    }
    
    // Create fragment actors
    for (int32 i = 0; i < FragmentPositions.Num(); i++)
    {
        FVector FragmentPos = FragmentPositions[i];
        FVector FragmentVelocity = CalculateFragmentVelocity(ImpactLocation, FragmentPos, ImpactForce);
        float FragmentScale = FMath::RandRange(0.1f, 0.4f);
        
        CreateSingleFragment(FragmentPos, FragmentVelocity, FragmentScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d destruction fragments"), ActiveFragments.Num());
}

void UCore_DestructionSystem::CreateSingleFragment(const FVector& Position, const FVector& Velocity, float Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create fragment mesh component
    UStaticMeshComponent* FragmentMesh = GenerateFragmentMesh(Position, Scale);
    if (!FragmentMesh)
    {
        return;
    }
    
    // Setup physics
    if (DestructionSettings.bUsePhysicsFragments)
    {
        FragmentMesh->SetSimulatePhysics(true);
        FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        FragmentMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Apply initial velocity
        if (!Velocity.IsNearlyZero())
        {
            FragmentMesh->SetPhysicsLinearVelocity(Velocity);
            FragmentMesh->SetPhysicsAngularVelocityInDegrees(FMath::VRand() * 360.0f);
        }
    }
    
    // Create fragment data
    FCore_DestructionFragment Fragment;
    Fragment.FragmentMesh = FragmentMesh;
    Fragment.InitialVelocity = Velocity;
    Fragment.LifetimeRemaining = DestructionSettings.FragmentLifetime;
    Fragment.bIsPhysicsEnabled = DestructionSettings.bUsePhysicsFragments;
    
    ActiveFragments.Add(Fragment);
    
    // Broadcast fragment creation event
    OnFragmentCreated.Broadcast(FragmentMesh, Velocity, DestructionSettings.FragmentLifetime);
}

UStaticMeshComponent* UCore_DestructionSystem::GenerateFragmentMesh(const FVector& Position, float Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create a simple fragment actor
    AActor* FragmentActor = World->SpawnActor<AActor>();
    if (!FragmentActor)
    {
        return nullptr;
    }
    
    // Create mesh component
    UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FragmentMesh"));
    if (!MeshComp)
    {
        FragmentActor->Destroy();
        return nullptr;
    }
    
    FragmentActor->SetRootComponent(MeshComp);
    FragmentActor->SetActorLocation(Position);
    FragmentActor->SetActorScale3D(FVector(Scale));
    
    // Try to use a basic cube mesh for fragments
    // In a real implementation, this would use the original mesh with procedural fracturing
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh)
    {
        MeshComp->SetStaticMesh(CubeMesh);
    }
    
    return MeshComp;
}

FVector UCore_DestructionSystem::CalculateFragmentVelocity(const FVector& ImpactLocation, const FVector& FragmentPosition, const FVector& ImpactForce)
{
    FVector Direction = (FragmentPosition - ImpactLocation).GetSafeNormal();
    if (Direction.IsNearlyZero())
    {
        Direction = FVector::UpVector;
    }
    
    float ForceMultiplier = FMath::RandRange(0.5f, 1.5f);
    float BaseForce = FMath::Min(ImpactForce.Size(), DestructionSettings.ExplosionForce);
    
    FVector Velocity = Direction * BaseForce * ForceMultiplier * 0.01f; // Scale down for reasonable velocities
    
    // Add some randomness
    Velocity += FMath::VRand() * BaseForce * 0.005f;
    
    return Velocity;
}

void UCore_DestructionSystem::ProcessFragmentLifetime(float DeltaTime)
{
    for (int32 i = ActiveFragments.Num() - 1; i >= 0; i--)
    {
        FCore_DestructionFragment& Fragment = ActiveFragments[i];
        Fragment.LifetimeRemaining -= DeltaTime;
        
        if (Fragment.LifetimeRemaining <= 0.0f)
        {
            // Destroy fragment
            if (Fragment.FragmentMesh && Fragment.FragmentMesh->GetOwner())
            {
                Fragment.FragmentMesh->GetOwner()->Destroy();
            }
            ActiveFragments.RemoveAt(i);
        }
        else if (Fragment.LifetimeRemaining <= 2.0f && Fragment.FragmentMesh)
        {
            // Fade out fragment in last 2 seconds
            float Alpha = Fragment.LifetimeRemaining / 2.0f;
            // In a real implementation, this would modify material opacity
        }
    }
}

float UCore_DestructionSystem::GetMaterialDestructionMultiplier(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return 1.0f;
    }
    
    // Get material from component
    UMaterialInterface* Material = Component->GetMaterial(0);
    if (!Material)
    {
        return 1.0f;
    }
    
    // Simple material detection based on name
    FString MaterialName = Material->GetName().ToLower();
    
    if (MaterialName.Contains(TEXT("wood")) || MaterialName.Contains(TEXT("tree")))
    {
        return WoodDestructionMultiplier;
    }
    else if (MaterialName.Contains(TEXT("stone")) || MaterialName.Contains(TEXT("rock")))
    {
        return StoneDestructionMultiplier;
    }
    else if (MaterialName.Contains(TEXT("metal")) || MaterialName.Contains(TEXT("iron")))
    {
        return MetalDestructionMultiplier;
    }
    else if (MaterialName.Contains(TEXT("bone")))
    {
        return BoneDestructionMultiplier;
    }
    
    return 1.0f; // Default multiplier
}

void UCore_DestructionSystem::CleanupFragments()
{
    for (FCore_DestructionFragment& Fragment : ActiveFragments)
    {
        if (Fragment.FragmentMesh && Fragment.FragmentMesh->GetOwner())
        {
            Fragment.FragmentMesh->GetOwner()->Destroy();
        }
    }
    
    ActiveFragments.Empty();
    UE_LOG(LogTemp, Log, TEXT("All destruction fragments cleaned up"));
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return !bIsDestroyed && CurrentHealth > 0.0f;
}

float UCore_DestructionSystem::GetDestructionPercentage() const
{
    if (DestructionSettings.HealthPoints <= 0.0f)
    {
        return 0.0f;
    }
    
    return (DestructionSettings.HealthPoints - CurrentHealth) / DestructionSettings.HealthPoints;
}

void UCore_DestructionSystem::ResetDestruction()
{
    bIsDestroyed = false;
    CurrentHealth = DestructionSettings.HealthPoints;
    CleanupFragments();
    
    // Restore original actor visibility
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetVisibility(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Destruction system reset for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_DestructionSystem::SetDestructionSettings(const FCore_DestructionSettings& NewSettings)
{
    DestructionSettings = NewSettings;
    
    // Update current health if not destroyed
    if (!bIsDestroyed)
    {
        CurrentHealth = DestructionSettings.HealthPoints;
    }
}

void UCore_DestructionSystem::StartFragmentCleanupTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FragmentCleanupTimer,
            this,
            &UCore_DestructionSystem::CleanupFragments,
            DestructionSettings.FragmentLifetime + 5.0f, // Extra time buffer
            false
        );
    }
}