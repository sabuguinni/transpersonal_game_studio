#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize destruction data
    DestructionData.DestructionType = ECore_DestructionType::Fracture;
    DestructionData.Health = 100.0f;
    DestructionData.ImpactThreshold = 50.0f;
    DestructionData.FragmentCount = 8;
    DestructionData.FragmentLifetime = 10.0f;
    DestructionData.bCreateDebris = true;

    CurrentHealth = DestructionData.Health;
    bIsDestroyed = false;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.Health;
    InitializeFragmentMeshes();

    // Enable tick only if we have destruction capability
    if (DestructionData.DestructionType != ECore_DestructionType::None)
    {
        SetComponentTickEnabled(true);
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Clean up fragments that are too far or too old
    if (SpawnedFragments.Num() > 0)
    {
        for (int32 i = SpawnedFragments.Num() - 1; i >= 0; i--)
        {
            if (!IsValid(SpawnedFragments[i]))
            {
                SpawnedFragments.RemoveAt(i);
            }
        }
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (bIsDestroyed || DamageAmount <= 0.0f)
    {
        return;
    }

    float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    // Broadcast damage event
    OnDamageTaken.Broadcast(GetOwner(), DamageAmount, CurrentHealth);

    // Check if damage threshold reached for destruction
    if (DamageAmount >= DestructionData.ImpactThreshold || CurrentHealth <= 0.0f)
    {
        TriggerDestruction(ImpactLocation, ImpactDirection);
    }

    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Applied %.1f damage, Health: %.1f/%.1f"), 
           DamageAmount, CurrentHealth, DestructionData.Health);
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (bIsDestroyed)
    {
        return;
    }

    bIsDestroyed = true;

    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(GetOwner(), ImpactLocation);

    // Create fragments based on destruction type
    if (DestructionData.bCreateDebris)
    {
        CreateFragments(ImpactLocation, ImpactDirection);
    }

    // Spawn destruction effects
    SpawnDestructionEffects(ImpactLocation);

    // Hide or destroy the original actor
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetVisibility(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // Schedule cleanup
        GetWorld()->GetTimerManager().SetTimer(FragmentCleanupTimer, this, 
                                             &UCore_DestructionSystem::CleanupFragments, 
                                             DestructionData.FragmentLifetime, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Object destroyed at %s"), *ImpactLocation.ToString());
}

void UCore_DestructionSystem::CreateFragments(const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (!GetWorld() || DestructionData.FragmentCount <= 0)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    UStaticMeshComponent* OriginalMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
    {
        return;
    }

    // Get bounds for fragment distribution
    FVector Origin, BoxExtent;
    Owner->GetActorBounds(false, Origin, BoxExtent);

    // Create fragments
    for (int32 i = 0; i < DestructionData.FragmentCount; i++)
    {
        // Random position within bounds
        FVector FragmentLocation = Origin + FVector(
            FMath::RandRange(-BoxExtent.X, BoxExtent.X),
            FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
            FMath::RandRange(-BoxExtent.Z, BoxExtent.Z)
        );

        // Use original mesh or fragment mesh if available
        UStaticMesh* FragmentMesh = OriginalMesh->GetStaticMesh();
        if (FragmentMeshes.IsValidIndex(i % FragmentMeshes.Num()) && FragmentMeshes[i % FragmentMeshes.Num()])
        {
            FragmentMesh = FragmentMeshes[i % FragmentMeshes.Num()];
        }

        // Calculate fragment velocity
        FVector FragmentVelocity = CalculateFragmentVelocity(ImpactLocation, FragmentLocation, ImpactDirection);

        // Create fragment actor
        AActor* Fragment = CreateFragmentActor(FragmentMesh, FragmentLocation, FragmentVelocity);
        if (Fragment)
        {
            SpawnedFragments.Add(Fragment);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Created %d fragments"), SpawnedFragments.Num());
}

AActor* UCore_DestructionSystem::CreateFragmentActor(UStaticMesh* FragmentMesh, const FVector& Location, const FVector& Velocity)
{
    if (!GetWorld() || !FragmentMesh)
    {
        return nullptr;
    }

    // Spawn static mesh actor for fragment
    AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!Fragment)
    {
        return nullptr;
    }

    // Setup mesh component
    UStaticMeshComponent* FragmentMesh_Comp = Fragment->GetStaticMeshComponent();
    if (FragmentMesh_Comp)
    {
        FragmentMesh_Comp->SetStaticMesh(FragmentMesh);
        
        // Scale down fragment
        float Scale = FMath::RandRange(0.3f, 0.8f);
        Fragment->SetActorScale3D(FVector(Scale));

        // Setup physics
        SetupFragmentPhysics(Fragment, Velocity);

        // Set material if available
        if (AActor* Owner = GetOwner())
        {
            if (UStaticMeshComponent* OriginalMesh = Owner->FindComponentByClass<UStaticMeshComponent>())
            {
                if (UMaterialInterface* Material = OriginalMesh->GetMaterial(0))
                {
                    FragmentMesh_Comp->SetMaterial(0, Material);
                }
            }
        }
    }

    return Fragment;
}

void UCore_DestructionSystem::SetupFragmentPhysics(AActor* Fragment, const FVector& Velocity)
{
    if (!Fragment)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = Fragment->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }

    // Enable physics simulation
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

    // Apply initial velocity
    MeshComp->SetPhysicsLinearVelocity(Velocity);

    // Add random angular velocity
    FVector AngularVelocity = FVector(
        FMath::RandRange(-360.0f, 360.0f),
        FMath::RandRange(-360.0f, 360.0f),
        FMath::RandRange(-360.0f, 360.0f)
    );
    MeshComp->SetPhysicsAngularVelocityInDegrees(AngularVelocity);

    // Set mass
    MeshComp->SetMassOverrideInKg(NAME_None, FMath::RandRange(1.0f, 10.0f));
}

FVector UCore_DestructionSystem::CalculateFragmentVelocity(const FVector& ImpactLocation, const FVector& FragmentLocation, const FVector& ImpactDirection)
{
    FVector DirectionFromImpact = (FragmentLocation - ImpactLocation).GetSafeNormal();
    
    // Combine impact direction with radial direction
    FVector FinalDirection = (ImpactDirection.GetSafeNormal() + DirectionFromImpact).GetSafeNormal();
    
    // Random velocity magnitude based on destruction type
    float VelocityMagnitude = 300.0f;
    switch (DestructionData.DestructionType)
    {
        case ECore_DestructionType::Explode:
            VelocityMagnitude = FMath::RandRange(500.0f, 1000.0f);
            break;
        case ECore_DestructionType::Shatter:
            VelocityMagnitude = FMath::RandRange(200.0f, 600.0f);
            break;
        case ECore_DestructionType::Fracture:
            VelocityMagnitude = FMath::RandRange(100.0f, 400.0f);
            break;
        case ECore_DestructionType::Crumble:
            VelocityMagnitude = FMath::RandRange(50.0f, 200.0f);
            break;
        default:
            VelocityMagnitude = FMath::RandRange(100.0f, 300.0f);
            break;
    }

    return FinalDirection * VelocityMagnitude;
}

void UCore_DestructionSystem::SpawnDestructionEffects(const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn particle effect
    if (DestructionData.DestructionEffect.IsValid())
    {
        UParticleSystem* ParticleSystem = DestructionData.DestructionEffect.LoadSynchronous();
        if (ParticleSystem)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem, Location);
        }
    }

    // Play destruction sound
    if (DestructionData.DestructionSound.IsValid())
    {
        USoundCue* SoundCue = DestructionData.DestructionSound.LoadSynchronous();
        if (SoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Location);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Spawned effects at %s"), *Location.ToString());
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

    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Cleaned up fragments"));
}

void UCore_DestructionSystem::InitializeFragmentMeshes()
{
    // This would typically load fragment meshes from content
    // For now, we'll use the original mesh scaled down
    FragmentMeshes.Empty();
    
    // In a real implementation, you would load specific fragment meshes here
    // FragmentMeshes.Add(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Fragments/Fragment01")));
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return !bIsDestroyed && DestructionData.DestructionType != ECore_DestructionType::None;
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    if (DestructionData.Health <= 0.0f)
    {
        return 0.0f;
    }
    return CurrentHealth / DestructionData.Health;
}