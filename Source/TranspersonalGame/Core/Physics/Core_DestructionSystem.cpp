#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    // Initialize destruction data with realistic defaults
    DestructionData.DestructionType = ECore_DestructionType::Fracture;
    DestructionData.DamageThreshold = 100.0f;
    DestructionData.FragmentCount = 8;
    DestructionData.FragmentLifetime = 10.0f;
    DestructionData.ImpulseStrength = 500.0f;
    DestructionData.bUsePhysicsFragments = true;
    
    bIsDestroyed = false;
    CurrentDamage = 0.0f;
    bAutoCleanupFragments = true;
    FragmentCleanupTimer = 0.0f;
    OriginalMesh = nullptr;
    OriginalMaterial = nullptr;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDestructionSystem();
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoCleanupFragments && ActiveFragments.Num() > 0)
    {
        UpdateFragmentLifetime(DeltaTime);
    }
}

void UCore_DestructionSystem::InitializeDestructionSystem()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: No owner actor found"));
        return;
    }
    
    // Store original mesh and material for potential restoration
    UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        OriginalMesh = MeshComp->GetStaticMesh();
        OriginalMaterial = MeshComp->GetMaterial(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem initialized for actor: %s"), *Owner->GetName());
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection, float Damage)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    CurrentDamage += Damage;
    
    // Check if damage threshold is reached
    if (CurrentDamage >= DestructionData.DamageThreshold)
    {
        bIsDestroyed = true;
        
        // Handle destruction based on type
        switch (DestructionData.DestructionType)
        {
            case ECore_DestructionType::Fracture:
                HandleFractureDestruction(ImpactPoint, ImpactDirection);
                break;
            case ECore_DestructionType::Shatter:
                HandleShatterDestruction(ImpactPoint, ImpactDirection);
                break;
            case ECore_DestructionType::Crumble:
                HandleCrumbleDestruction(ImpactPoint, ImpactDirection);
                break;
            case ECore_DestructionType::Explode:
                HandleExplodeDestruction(ImpactPoint, ImpactDirection);
                break;
            case ECore_DestructionType::Burn:
                HandleBurnDestruction(ImpactPoint, ImpactDirection);
                break;
            default:
                HandleFractureDestruction(ImpactPoint, ImpactDirection);
                break;
        }
        
        // Play destruction effects
        PlayDestructionEffects(ImpactPoint);
        
        // Broadcast destruction event
        OnDestructionTriggered.Broadcast(GetOwner(), DestructionData.DestructionType);
        
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Destruction triggered for %s"), *GetOwner()->GetName());
    }
}

void UCore_DestructionSystem::SetDestructionData(const FCore_DestructionData& NewData)
{
    DestructionData = NewData;
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return !bIsDestroyed && CurrentDamage < DestructionData.DamageThreshold;
}

void UCore_DestructionSystem::RepairObject()
{
    if (!bIsDestroyed)
    {
        return;
    }
    
    // Clean up existing fragments
    CleanupFragments();
    
    // Reset destruction state
    bIsDestroyed = false;
    CurrentDamage = 0.0f;
    
    // Restore original mesh if available
    AActor* Owner = GetOwner();
    if (Owner && OriginalMesh)
    {
        UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(OriginalMesh);
            if (OriginalMaterial)
            {
                MeshComp->SetMaterial(0, OriginalMaterial);
            }
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Object repaired"));
}

void UCore_DestructionSystem::CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate fragment positions around the impact point
    for (int32 i = 0; i < DestructionData.FragmentCount; i++)
    {
        // Generate random offset from impact point
        FVector Offset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        FVector FragmentLocation = ImpactPoint + Offset;
        
        // Calculate fragment velocity based on impact direction and random variation
        FVector BaseVelocity = ImpactDirection.GetSafeNormal() * DestructionData.ImpulseStrength;
        FVector RandomVelocity = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(0.0f, 300.0f)
        );
        FVector FragmentVelocity = BaseVelocity + RandomVelocity;
        
        // Create fragment with random scale variation
        float FragmentScale = FMath::RandRange(0.3f, 0.8f);
        AActor* Fragment = CreateFragment(FragmentLocation, FragmentVelocity, FragmentScale);
        
        if (Fragment)
        {
            ActiveFragments.Add(Fragment);
            OnFragmentCreated.Broadcast(Fragment);
        }
    }
    
    // Hide original mesh
    UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetVisibility(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UCore_DestructionSystem::CleanupFragments()
{
    for (AActor* Fragment : ActiveFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    ActiveFragments.Empty();
    FragmentCleanupTimer = 0.0f;
}

void UCore_DestructionSystem::HandleFractureDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    // Fracture creates moderate-sized fragments with realistic physics
    CreateFragments(ImpactPoint, ImpactDirection);
}

void UCore_DestructionSystem::HandleShatterDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    // Shatter creates many small fragments with high velocity
    int32 OriginalCount = DestructionData.FragmentCount;
    DestructionData.FragmentCount = FMath::Max(12, OriginalCount * 2);
    DestructionData.ImpulseStrength *= 1.5f;
    
    CreateFragments(ImpactPoint, ImpactDirection);
    
    // Restore original settings
    DestructionData.FragmentCount = OriginalCount;
    DestructionData.ImpulseStrength /= 1.5f;
}

void UCore_DestructionSystem::HandleCrumbleDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    // Crumble creates fragments that fall mostly downward
    FVector DownwardDirection = FVector(0, 0, -1);
    FVector ModifiedDirection = (ImpactDirection + DownwardDirection * 2.0f).GetSafeNormal();
    
    CreateFragments(ImpactPoint, ModifiedDirection);
}

void UCore_DestructionSystem::HandleExplodeDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    // Explode creates fragments flying in all directions with high force
    float OriginalStrength = DestructionData.ImpulseStrength;
    DestructionData.ImpulseStrength *= 2.0f;
    
    // Create fragments with radial explosion pattern
    for (int32 i = 0; i < DestructionData.FragmentCount; i++)
    {
        // Generate radial direction
        float Angle = (float(i) / float(DestructionData.FragmentCount)) * 2.0f * PI;
        FVector RadialDirection = FVector(
            FMath::Cos(Angle),
            FMath::Sin(Angle),
            FMath::RandRange(0.2f, 0.8f)
        ).GetSafeNormal();
        
        CreateFragments(ImpactPoint, RadialDirection);
    }
    
    // Restore original strength
    DestructionData.ImpulseStrength = OriginalStrength;
}

void UCore_DestructionSystem::HandleBurnDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    // Burn creates charred fragments with smoke effects
    CreateFragments(ImpactPoint, FVector(0, 0, 1)); // Fragments rise with heat
    
    // TODO: Add fire/smoke particle effects when particle system is available
}

void UCore_DestructionSystem::UpdateFragmentLifetime(float DeltaTime)
{
    FragmentCleanupTimer += DeltaTime;
    
    if (FragmentCleanupTimer >= DestructionData.FragmentLifetime)
    {
        CleanupFragments();
    }
}

AActor* UCore_DestructionSystem::CreateFragment(const FVector& Location, const FVector& Velocity, float Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create a static mesh actor for the fragment
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (Fragment)
    {
        UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
        if (FragmentMesh)
        {
            // Set fragment mesh (use simplified version of original or basic cube)
            UStaticMesh* FragmentMeshAsset = GetFragmentMesh();
            if (FragmentMeshAsset)
            {
                FragmentMesh->SetStaticMesh(FragmentMeshAsset);
            }
            
            // Set scale
            Fragment->SetActorScale3D(FVector(Scale));
            
            // Apply physics if enabled
            if (DestructionData.bUsePhysicsFragments)
            {
                ApplyFragmentPhysics(Fragment, Velocity);
            }
            
            // Set material (darker/damaged version of original)
            if (OriginalMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, FragmentMesh);
                if (DynamicMaterial)
                {
                    // Make fragment darker to simulate damage
                    DynamicMaterial->SetScalarParameterValue(TEXT("Brightness"), 0.3f);
                    FragmentMesh->SetMaterial(0, DynamicMaterial);
                }
            }
        }
    }
    
    return Fragment;
}

void UCore_DestructionSystem::ApplyFragmentPhysics(AActor* Fragment, const FVector& Velocity)
{
    if (!Fragment)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Fragment->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Enable physics simulation
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        
        // Apply initial velocity
        MeshComp->SetPhysicsLinearVelocity(Velocity);
        
        // Add random angular velocity for realistic tumbling
        FVector AngularVelocity = FVector(
            FMath::RandRange(-10.0f, 10.0f),
            FMath::RandRange(-10.0f, 10.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );
        MeshComp->SetPhysicsAngularVelocityInRadians(AngularVelocity);
    }
}

void UCore_DestructionSystem::PlayDestructionEffects(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Play particle effect if available
    if (DestructionData.DestructionEffect.IsValid())
    {
        UParticleSystem* ParticleSystem = DestructionData.DestructionEffect.LoadSynchronous();
        if (ParticleSystem)
        {
            UGameplayStatics::SpawnEmitterAtLocation(World, ParticleSystem, Location);
        }
    }
    
    // Play sound effect if available
    if (DestructionData.DestructionSound.IsValid())
    {
        USoundCue* SoundCue = DestructionData.DestructionSound.LoadSynchronous();
        if (SoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(World, SoundCue, Location);
        }
    }
}

UStaticMesh* UCore_DestructionSystem::GetFragmentMesh() const
{
    // For now, return the original mesh (in production, this would return a fractured version)
    return OriginalMesh;
}