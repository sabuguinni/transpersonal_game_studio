#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "PhysicsEngine/RadialForceComponent.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Default destruction settings
    DestructionType = ECore_DestructionType::Fracture;
    FragmentLifetime = 10.0f;
    MaxFragments = 20;
    FragmentScaleVariation = 0.3f;
    FragmentMass = 1.0f;
    ExplosionForce = 1000.0f;
    ExplosionRadius = 500.0f;
    bUseRadialDamage = true;
    bSpawnDebris = true;
    SoundVolumeMultiplier = 1.0f;

    // Initialize destruction data
    DestructionData.Health = 100.0f;
    DestructionData.MaxHealth = 100.0f;
    DestructionData.bCanBeDestroyed = true;
    DestructionData.DestructionForceThreshold = 500.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize destruction system
    bIsDestroyed = false;
    CurrentDestructionStage = 0;
    
    // Set up cleanup timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FragmentCleanupTimer,
            this,
            &UCore_DestructionSystem::CleanupFragments,
            FragmentLifetime,
            true
        );
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update destruction stage based on health
    UpdateDestructionStage();
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (!DestructionData.bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }

    // Apply damage
    DestructionData.Health = FMath::Max(0.0f, DestructionData.Health - DamageAmount);
    
    // Check if force threshold is exceeded
    float ForceStrength = ImpactForce.Size();
    if (ForceStrength >= DestructionData.DestructionForceThreshold)
    {
        TriggerDestruction(ImpactLocation, ForceStrength);
    }
    
    // Trigger destruction if health is depleted
    if (DestructionData.Health <= 0.0f)
    {
        TriggerDestruction(ImpactLocation, ForceStrength);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& DestructionLocation, float Force)
{
    if (bIsDestroyed)
    {
        return;
    }

    bIsDestroyed = true;
    
    // Play destruction effects
    PlayDestructionEffects(DestructionLocation);
    
    // Create fragments based on destruction type
    int32 NumFragments = FMath::Min(MaxFragments, FMath::RandRange(8, MaxFragments));
    CreateFragments(DestructionLocation, NumFragments);
    
    // Apply radial damage if enabled
    if (bUseRadialDamage && GetWorld())
    {
        UGameplayStatics::ApplyRadialDamage(
            GetWorld(),
            Force * 0.1f,
            DestructionLocation,
            ExplosionRadius,
            nullptr,
            TArray<AActor*>(),
            GetOwner(),
            nullptr,
            true
        );
    }
    
    // Hide or destroy the original actor
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorHiddenInGame(true);
        Owner->SetActorEnableCollision(false);
    }
}

void UCore_DestructionSystem::CreateFragments(const FVector& Origin, int32 NumFragments)
{
    if (!GetWorld() || !bSpawnDebris)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get the original mesh component
    UStaticMeshComponent* OriginalMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
    {
        return;
    }

    for (int32 i = 0; i < NumFragments; i++)
    {
        // Generate random fragment properties
        FVector FragmentLocation = Origin + FMath::VRand() * 100.0f;
        FVector FragmentVelocity = FMath::VRand() * ExplosionForce * FMath::RandRange(0.5f, 1.5f);
        float FragmentScale = FMath::RandRange(0.1f, 0.5f) * (1.0f + FMath::RandRange(-FragmentScaleVariation, FragmentScaleVariation));
        
        // Create fragment
        UStaticMeshComponent* Fragment = CreateFragment(FragmentLocation, FragmentVelocity, FragmentScale);
        if (Fragment)
        {
            SpawnedFragments.Add(Fragment);
            
            // Set fragment material if available
            if (FragmentMaterial)
            {
                Fragment->SetMaterial(0, FragmentMaterial);
            }
            else if (OriginalMesh->GetMaterial(0))
            {
                Fragment->SetMaterial(0, OriginalMesh->GetMaterial(0));
            }
        }
    }
}

UStaticMeshComponent* UCore_DestructionSystem::CreateFragment(const FVector& Location, const FVector& Velocity, float Scale)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Spawn fragment actor
    AStaticMeshActor* FragmentActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator
    );

    if (!FragmentActor)
    {
        return nullptr;
    }

    UStaticMeshComponent* FragmentMesh = FragmentActor->GetStaticMeshComponent();
    if (!FragmentMesh)
    {
        FragmentActor->Destroy();
        return nullptr;
    }

    // Get original mesh
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UStaticMeshComponent* OriginalMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (OriginalMesh && OriginalMesh->GetStaticMesh())
        {
            FragmentMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
        }
    }

    // Set fragment properties
    FragmentMesh->SetWorldScale3D(FVector(Scale));
    FragmentMesh->SetSimulatePhysics(true);
    FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FragmentMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    FragmentMesh->SetMassOverrideInKg(NAME_None, FragmentMass * Scale, true);

    // Apply initial velocity
    FragmentMesh->AddImpulse(Velocity * FragmentMass * Scale, NAME_None, true);
    
    // Add random angular velocity
    FVector AngularVelocity = FMath::VRand() * 360.0f;
    FragmentMesh->AddAngularImpulseInDegrees(AngularVelocity, NAME_None, true);

    return FragmentMesh;
}

void UCore_DestructionSystem::PlayDestructionEffects(const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }

    // Play particle effect
    if (DestructionParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DestructionParticles,
            Location,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true
        );
    }

    // Play destruction sound
    if (DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DestructionSound,
            Location,
            SoundVolumeMultiplier
        );
    }
}

void UCore_DestructionSystem::CleanupFragments()
{
    // Remove old fragments
    for (int32 i = SpawnedFragments.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(SpawnedFragments[i]) || !IsValid(SpawnedFragments[i]->GetOwner()))
        {
            SpawnedFragments.RemoveAt(i);
            continue;
        }

        // Check if fragment should be cleaned up based on lifetime
        AActor* FragmentActor = SpawnedFragments[i]->GetOwner();
        if (FragmentActor && (GetWorld()->GetTimeSeconds() - FragmentActor->GetActorTimestamp()) > FragmentLifetime)
        {
            FragmentActor->Destroy();
            SpawnedFragments.RemoveAt(i);
        }
    }
}

void UCore_DestructionSystem::UpdateDestructionStage()
{
    if (bIsDestroyed)
    {
        return;
    }

    float HealthPercentage = GetHealthPercentage();
    int32 NewStage = 0;

    // Determine destruction stage based on health
    if (HealthPercentage > 0.75f)
    {
        NewStage = 0; // Pristine
    }
    else if (HealthPercentage > 0.5f)
    {
        NewStage = 1; // Slightly damaged
    }
    else if (HealthPercentage > 0.25f)
    {
        NewStage = 2; // Moderately damaged
    }
    else
    {
        NewStage = 3; // Heavily damaged
    }

    if (NewStage != CurrentDestructionStage)
    {
        CurrentDestructionStage = NewStage;
        
        // Apply visual changes based on destruction stage
        if (DestructionData.DestructionStages.IsValidIndex(CurrentDestructionStage))
        {
            AActor* Owner = GetOwner();
            if (Owner)
            {
                UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
                if (MeshComp && DestructionData.DestructionStages[CurrentDestructionStage])
                {
                    MeshComp->SetStaticMesh(DestructionData.DestructionStages[CurrentDestructionStage]);
                }
            }
        }
    }
}

void UCore_DestructionSystem::SetDestructionType(ECore_DestructionType NewType)
{
    DestructionType = NewType;
    
    // Adjust parameters based on destruction type
    switch (DestructionType)
    {
        case ECore_DestructionType::Fracture:
            MaxFragments = 15;
            ExplosionForce = 800.0f;
            FragmentScaleVariation = 0.3f;
            break;
            
        case ECore_DestructionType::Shatter:
            MaxFragments = 25;
            ExplosionForce = 1200.0f;
            FragmentScaleVariation = 0.5f;
            break;
            
        case ECore_DestructionType::Crumble:
            MaxFragments = 30;
            ExplosionForce = 400.0f;
            FragmentScaleVariation = 0.2f;
            break;
            
        case ECore_DestructionType::Explode:
            MaxFragments = 20;
            ExplosionForce = 2000.0f;
            FragmentScaleVariation = 0.4f;
            bUseRadialDamage = true;
            break;
            
        case ECore_DestructionType::Dissolve:
            MaxFragments = 5;
            ExplosionForce = 100.0f;
            FragmentScaleVariation = 0.1f;
            break;
            
        default:
            break;
    }
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return DestructionData.bCanBeDestroyed && !bIsDestroyed;
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    if (DestructionData.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    
    return DestructionData.Health / DestructionData.MaxHealth;
}

void UCore_DestructionSystem::SetupDinosaurDestruction(float DinosaurMass, ECore_DestructionType Type)
{
    SetDestructionType(Type);
    
    // Scale destruction parameters based on dinosaur mass
    float MassScale = FMath::Clamp(DinosaurMass / 1000.0f, 0.1f, 10.0f); // Normalize to 1000kg baseline
    
    DestructionData.Health = 50.0f * MassScale;
    DestructionData.MaxHealth = DestructionData.Health;
    DestructionData.DestructionForceThreshold = 300.0f * MassScale;
    
    ExplosionForce = 1000.0f * MassScale;
    ExplosionRadius = 300.0f * FMath::Sqrt(MassScale);
    MaxFragments = FMath::Clamp(FMath::RoundToInt(15.0f * MassScale), 5, 40);
    FragmentMass = 10.0f * MassScale;
}

void UCore_DestructionSystem::ApplyDinosaurImpact(float DinosaurMass, const FVector& ImpactVelocity)
{
    // Calculate impact force based on mass and velocity
    float ImpactForce = DinosaurMass * ImpactVelocity.Size();
    float DamageAmount = ImpactForce * 0.01f; // Convert force to damage
    
    ApplyDamage(DamageAmount, GetOwner()->GetActorLocation(), ImpactVelocity * DinosaurMass);
}

void UCore_DestructionSystem::SetupTreeDestruction()
{
    SetDestructionType(ECore_DestructionType::Fracture);
    
    DestructionData.Health = 150.0f;
    DestructionData.MaxHealth = 150.0f;
    DestructionData.DestructionForceThreshold = 800.0f;
    
    MaxFragments = 12;
    ExplosionForce = 600.0f;
    FragmentMass = 5.0f;
    FragmentScaleVariation = 0.4f;
}

void UCore_DestructionSystem::SetupRockDestruction()
{
    SetDestructionType(ECore_DestructionType::Shatter);
    
    DestructionData.Health = 300.0f;
    DestructionData.MaxHealth = 300.0f;
    DestructionData.DestructionForceThreshold = 1200.0f;
    
    MaxFragments = 20;
    ExplosionForce = 1000.0f;
    FragmentMass = 15.0f;
    FragmentScaleVariation = 0.3f;
}

void UCore_DestructionSystem::SetupStructureDestruction()
{
    SetDestructionType(ECore_DestructionType::Crumble);
    
    DestructionData.Health = 500.0f;
    DestructionData.MaxHealth = 500.0f;
    DestructionData.DestructionForceThreshold = 1500.0f;
    
    MaxFragments = 25;
    ExplosionForce = 800.0f;
    FragmentMass = 20.0f;
    FragmentScaleVariation = 0.35f;
}