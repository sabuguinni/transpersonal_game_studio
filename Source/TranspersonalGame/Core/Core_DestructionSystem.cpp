#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"

// UCore_DestructionComponent Implementation

UCore_DestructionComponent::UCore_DestructionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize destruction data
    DestructionData = FCore_DestructionData();
    CurrentHealth = DestructionData.HealthPoints;
    bIsDestructible = true;
    bIsDestroyed = false;
}

void UCore_DestructionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.HealthPoints;
    
    // Bind to owner's hit events if it has a mesh component
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->OnComponentHit.AddDynamic(this, &UCore_DestructionComponent::ApplyDamage);
        }
    }
}

void UCore_DestructionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup old fragments periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    
    if (CleanupTimer >= 5.0f)
    {
        CleanupFragments();
        CleanupTimer = 0.0f;
    }
}

void UCore_DestructionComponent::ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (!bIsDestructible || bIsDestroyed)
    {
        return;
    }
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    
    // Broadcast damage event
    OnDamageTaken.Broadcast(DamageAmount, CurrentHealth);
    
    // Check if we should trigger destruction
    if (CurrentHealth <= 0.0f || DamageAmount >= DestructionData.DamageThreshold)
    {
        TriggerDestruction(ImpactPoint, ImpactDirection);
    }
}

void UCore_DestructionComponent::TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    // Create fragments based on destruction type
    CreateFragments(ImpactPoint, ImpactDirection);
    
    // Play effects
    if (DestructionData.bPlayEffects)
    {
        PlayDestructionEffects(ImpactPoint);
    }
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(GetOwner(), ImpactPoint, ImpactDirection);
    
    // Hide or destroy the original actor
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorHiddenInGame(true);
        Owner->SetActorEnableCollision(false);
        
        // Schedule destruction after a delay to allow effects to play
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [Owner]()
        {
            if (IsValid(Owner))
            {
                Owner->Destroy();
            }
        }, 2.0f, false);
    }
}

void UCore_DestructionComponent::CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (!DestructionData.bCreateDebris)
    {
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    UStaticMeshComponent* OriginalMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (!OriginalMesh)
    {
        return;
    }
    
    // Get original mesh bounds
    FVector Origin, BoxExtent;
    Owner->GetActorBounds(false, Origin, BoxExtent);
    
    // Create fragments
    for (int32 i = 0; i < DestructionData.FragmentCount; ++i)
    {
        FVector FragmentLocation = Origin + FVector(
            FMath::RandRange(-BoxExtent.X * 0.5f, BoxExtent.X * 0.5f),
            FMath::RandRange(-BoxExtent.Y * 0.5f, BoxExtent.Y * 0.5f),
            FMath::RandRange(-BoxExtent.Z * 0.5f, BoxExtent.Z * 0.5f)
        );
        
        FVector FragmentVelocity = ImpactDirection * DestructionData.ImpulseStrength;
        FragmentVelocity += FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(100.0f, 400.0f)
        );
        
        AActor* Fragment = CreateFragment(FragmentLocation, FragmentVelocity, i);
        if (Fragment)
        {
            SpawnedFragments.Add(Fragment);
        }
    }
}

AActor* UCore_DestructionComponent::CreateFragment(const FVector& Location, const FVector& Velocity, int32 FragmentIndex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn a simple static mesh actor as fragment
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* Fragment = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (!Fragment)
    {
        return nullptr;
    }
    
    // Add static mesh component
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Fragment);
    Fragment->SetRootComponent(MeshComp);
    
    // Set a basic cube mesh for fragments (in a real implementation, you'd use fractured pieces)
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh)
    {
        MeshComp->SetStaticMesh(CubeMesh);
        
        // Scale fragment randomly
        float Scale = FMath::RandRange(0.1f, 0.3f);
        Fragment->SetActorScale3D(FVector(Scale));
        
        // Enable physics
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        
        // Apply initial velocity
        MeshComp->SetPhysicsLinearVelocity(Velocity);
        MeshComp->SetPhysicsAngularVelocityInDegrees(FVector(
            FMath::RandRange(-360.0f, 360.0f),
            FMath::RandRange(-360.0f, 360.0f),
            FMath::RandRange(-360.0f, 360.0f)
        ));
    }
    
    // Schedule fragment destruction
    FTimerHandle FragmentTimer;
    World->GetTimerManager().SetTimer(FragmentTimer, [Fragment]()
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }, DestructionData.FragmentLifetime, false);
    
    return Fragment;
}

void UCore_DestructionComponent::PlayDestructionEffects(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Play particle effect if specified
    if (DestructionData.DestructionEffect.IsValid())
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, DestructionData.DestructionEffect.Get(), Location);
    }
    
    // Play sound effect if specified
    if (DestructionData.DestructionSound.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(World, DestructionData.DestructionSound.Get(), Location);
    }
}

void UCore_DestructionComponent::CleanupFragments()
{
    SpawnedFragments.RemoveAll([](const AActor* Fragment)
    {
        return !IsValid(Fragment);
    });
}

bool UCore_DestructionComponent::CanBeDestroyed() const
{
    return bIsDestructible && !bIsDestroyed && CurrentHealth > 0.0f;
}

float UCore_DestructionComponent::GetHealthPercentage() const
{
    if (DestructionData.HealthPoints <= 0.0f)
    {
        return 0.0f;
    }
    return CurrentHealth / DestructionData.HealthPoints;
}

// ACore_DestructibleActor Implementation

ACore_DestructibleActor::ACore_DestructibleActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    // Create destruction component
    DestructionComponent = CreateDefaultSubobject<UCore_DestructionComponent>(TEXT("DestructionComponent"));
    
    // Create effects component
    EffectsComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EffectsComponent"));
    EffectsComponent->SetupAttachment(RootComponent);
    EffectsComponent->SetAutoActivate(false);
    
    // Set default mesh properties
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MeshComponent->SetGenerateOverlapEvents(true);
    
    // Bind hit events
    MeshComponent->OnComponentHit.AddDynamic(this, &ACore_DestructibleActor::OnHit);
}

void ACore_DestructibleActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind destruction events
    if (DestructionComponent)
    {
        DestructionComponent->OnDestructionTriggered.AddDynamic(this, &ACore_DestructibleActor::OnDestructionTriggered);
        DestructionComponent->OnDamageTaken.AddDynamic(this, &ACore_DestructibleActor::OnDamageTaken);
    }
    
    // Register with destruction manager
    if (UCore_DestructionManager* Manager = GetGameInstance()->GetSubsystem<UCore_DestructionManager>())
    {
        Manager->RegisterDestructibleActor(this);
    }
}

void ACore_DestructibleActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACore_DestructibleActor::TakeDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (DestructionComponent)
    {
        DestructionComponent->ApplyDamage(DamageAmount, ImpactPoint, ImpactDirection);
    }
}

void ACore_DestructibleActor::SetDestructionType(ECore_DestructionType NewType)
{
    if (DestructionComponent)
    {
        DestructionComponent->DestructionData.DestructionType = NewType;
    }
}

ECore_DestructionType ACore_DestructibleActor::GetDestructionType() const
{
    if (DestructionComponent)
    {
        return DestructionComponent->DestructionData.DestructionType;
    }
    return ECore_DestructionType::None;
}

bool ACore_DestructibleActor::IsDestroyed() const
{
    if (DestructionComponent)
    {
        return DestructionComponent->bIsDestroyed;
    }
    return false;
}

void ACore_DestructibleActor::OnDestructionTriggered(AActor* DestroyedActor, FVector ImpactPoint, FVector ImpactDirection)
{
    // Handle destruction logic here
    // This is called when the actor is destroyed
}

void ACore_DestructibleActor::OnDamageTaken(float DamageAmount, float RemainingHealth)
{
    // Handle damage feedback here
    // Could trigger damage effects, screen shake, etc.
}

void ACore_DestructibleActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Calculate damage based on impact force
    float ImpactForce = NormalImpulse.Size();
    float Damage = FMath::Max(0.0f, (ImpactForce - 1000.0f) * 0.01f); // Threshold and scaling
    
    if (Damage > 0.0f)
    {
        TakeDamage(Damage, Hit.ImpactPoint, Hit.ImpactNormal);
    }
}

// UCore_DestructionManager Implementation

void UCore_DestructionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UCore_DestructionManager::PerformCleanup, FragmentCleanupInterval, true);
    }
}

void UCore_DestructionManager::Deinitialize()
{
    // Clear cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    RegisteredActors.Empty();
    ActiveFragments.Empty();
    
    Super::Deinitialize();
}

void UCore_DestructionManager::RegisterDestructibleActor(ACore_DestructibleActor* Actor)
{
    if (Actor && !RegisteredActors.Contains(Actor))
    {
        RegisteredActors.Add(Actor);
    }
}

void UCore_DestructionManager::UnregisterDestructibleActor(ACore_DestructibleActor* Actor)
{
    RegisteredActors.Remove(Actor);
}

void UCore_DestructionManager::TriggerAreaDestruction(const FVector& Center, float Radius, float DamageAmount)
{
    TArray<ACore_DestructibleActor*> ActorsInRadius = GetDestructibleActorsInRadius(Center, Radius);
    
    for (ACore_DestructibleActor* Actor : ActorsInRadius)
    {
        if (Actor && !Actor->IsDestroyed())
        {
            FVector Direction = (Actor->GetActorLocation() - Center).GetSafeNormal();
            Actor->TakeDamage(DamageAmount, Actor->GetActorLocation(), Direction);
        }
    }
}

void UCore_DestructionManager::CleanupDestroyedActors()
{
    RegisteredActors.RemoveAll([](const ACore_DestructibleActor* Actor)
    {
        return !IsValid(Actor) || Actor->IsDestroyed();
    });
}

int32 UCore_DestructionManager::GetActiveDestructibleCount() const
{
    return RegisteredActors.Num();
}

TArray<ACore_DestructibleActor*> UCore_DestructionManager::GetDestructibleActorsInRadius(const FVector& Center, float Radius)
{
    TArray<ACore_DestructibleActor*> Result;
    
    for (ACore_DestructibleActor* Actor : RegisteredActors)
    {
        if (Actor && !Actor->IsDestroyed())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                Result.Add(Actor);
            }
        }
    }
    
    return Result;
}

void UCore_DestructionManager::PerformCleanup()
{
    // Remove invalid fragments
    ActiveFragments.RemoveAll([](const AActor* Fragment)
    {
        return !IsValid(Fragment);
    });
    
    // Cleanup destroyed actors
    CleanupDestroyedActors();
    
    // Limit active fragments for performance
    if (ActiveFragments.Num() > MaxActiveFragments)
    {
        int32 ToRemove = ActiveFragments.Num() - MaxActiveFragments;
        for (int32 i = 0; i < ToRemove; ++i)
        {
            if (ActiveFragments.IsValidIndex(i) && IsValid(ActiveFragments[i]))
            {
                ActiveFragments[i]->Destroy();
            }
        }
        ActiveFragments.RemoveAt(0, ToRemove);
    }
}

bool UCore_DestructionManager::ShouldCreateFragment(const FVector& Location) const
{
    if (!bEnableDestructionLOD)
    {
        return true;
    }
    
    // LOD based on distance to player
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            
            // Don't create fragments too far from player
            if (Distance > 5000.0f)
            {
                return false;
            }
            
            // Reduce fragment count at medium distances
            if (Distance > 2000.0f && FMath::RandRange(0.0f, 1.0f) > 0.5f)
            {
                return false;
            }
        }
    }
    
    return true;
}