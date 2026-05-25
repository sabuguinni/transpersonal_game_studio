#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogDestructionSystem);

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Default destruction settings
    MaterialType = ECore_MaterialType::Wood;
    DestructionThreshold = 100.0f;
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    bCanBeDestroyed = true;
    bCreateDebris = true;
    MaxDebrisCount = 8;
    DebrisLifetime = 30.0f;

    // Physics settings
    DebrisMass = 10.0f;
    DebrisBounciness = 0.3f;
    DebrisFriction = 0.7f;

    // Internal state
    bIsDestroyed = false;
    DestructionTime = 0.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDestructionSystem();
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDestroyed)
    {
        DestructionTime += DeltaTime;
    }
}

void UCore_DestructionSystem::InitializeDestructionSystem()
{
    UE_LOG(LogDestructionSystem, Log, TEXT("Initializing Destruction System for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));

    // Set up material-specific defaults
    switch (MaterialType)
    {
        case ECore_MaterialType::Wood:
            DestructionThreshold = 80.0f;
            MaxDebrisCount = 6;
            DebrisMass = 5.0f;
            break;
        case ECore_MaterialType::Stone:
            DestructionThreshold = 150.0f;
            MaxDebrisCount = 10;
            DebrisMass = 20.0f;
            break;
        case ECore_MaterialType::Glass:
            DestructionThreshold = 30.0f;
            MaxDebrisCount = 15;
            DebrisMass = 2.0f;
            break;
        case ECore_MaterialType::Metal:
            DestructionThreshold = 200.0f;
            MaxDebrisCount = 4;
            DebrisMass = 30.0f;
            break;
        case ECore_MaterialType::Bone:
            DestructionThreshold = 60.0f;
            MaxDebrisCount = 8;
            DebrisMass = 3.0f;
            break;
        case ECore_MaterialType::Flesh:
            DestructionThreshold = 40.0f;
            MaxDebrisCount = 0; // No debris for flesh
            bCreateDebris = false;
            break;
        case ECore_MaterialType::Ice:
            DestructionThreshold = 50.0f;
            MaxDebrisCount = 12;
            DebrisMass = 8.0f;
            break;
        case ECore_MaterialType::Crystal:
            DestructionThreshold = 70.0f;
            MaxDebrisCount = 20;
            DebrisMass = 4.0f;
            break;
    }

    CurrentHealth = MaxHealth;
}

void UCore_DestructionSystem::TakeDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection, AActor* DamageCauser)
{
    if (!bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }

    CurrentHealth -= DamageAmount;
    
    UE_LOG(LogDestructionSystem, Log, TEXT("%s took %.1f damage, health: %.1f/%.1f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           DamageAmount, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        FCore_DestructionEvent DestructionEvent;
        DestructionEvent.DestructionType = (DamageAmount >= DestructionThreshold) ? 
            ECore_DestructionType::Explode : ECore_DestructionType::Crumble;
        DestructionEvent.ImpactLocation = ImpactLocation;
        DestructionEvent.ImpactDirection = ImpactDirection;
        DestructionEvent.ImpactForce = DamageAmount;
        DestructionEvent.InstigatorActor = DamageCauser;

        DestroyObject(DestructionEvent);
    }
}

void UCore_DestructionSystem::DestroyObject(const FCore_DestructionEvent& DestructionEvent)
{
    if (bIsDestroyed)
    {
        return;
    }

    bIsDestroyed = true;
    
    UE_LOG(LogDestructionSystem, Log, TEXT("Destroying object %s with type %d"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           (int32)DestructionEvent.DestructionType);

    // Apply material-specific destruction
    ApplyMaterialSpecificDestruction(MaterialType, DestructionEvent);

    // Create debris if enabled
    if (bCreateDebris && MaxDebrisCount > 0)
    {
        CreateDebris(DestructionEvent.ImpactLocation, DestructionEvent.ImpactDirection, DestructionEvent.ImpactForce);
    }

    // Play destruction effects
    PlayDestructionEffects(DestructionEvent.ImpactLocation);

    // Hide or replace the original mesh
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            if (DestroyedMesh)
            {
                MeshComp->SetStaticMesh(DestroyedMesh);
            }
            else
            {
                MeshComp->SetVisibility(false);
            }
        }
    }

    // Set cleanup timer
    if (DebrisLifetime > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DebrisCleanupTimer,
            this,
            &UCore_DestructionSystem::CleanupDebris,
            DebrisLifetime,
            false
        );
    }
}

void UCore_DestructionSystem::CreateDebris(const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce)
{
    if (!GetWorld() || DebrisMeshes.Num() == 0)
    {
        return;
    }

    int32 DebrisToSpawn = FMath::Min(MaxDebrisCount, DebrisMeshes.Num());
    
    for (int32 i = 0; i < DebrisToSpawn; i++)
    {
        if (DebrisMeshes[i])
        {
            // Calculate spawn location with some randomness
            FVector SpawnLocation = ImpactLocation + FVector(
                FMath::RandRange(-50.0f, 50.0f),
                FMath::RandRange(-50.0f, 50.0f),
                FMath::RandRange(0.0f, 25.0f)
            );

            // Calculate velocity based on impact direction and force
            FVector Velocity = CalculateDebrisVelocity(ImpactDirection, ImpactForce);
            
            SpawnDebrisActor(SpawnLocation, Velocity, DebrisMeshes[i]);
        }
    }

    UE_LOG(LogDestructionSystem, Log, TEXT("Created %d debris pieces"), DebrisToSpawn);
}

void UCore_DestructionSystem::PlayDestructionEffects(const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }

    // Play destruction sound
    if (DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, Location);
    }

    // Play particle effect
    if (DestructionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionEffect, Location);
    }

    // Play Niagara effect
    if (DestructionNiagara)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestructionNiagara, Location);
    }
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool UCore_DestructionSystem::IsDestroyed() const
{
    return bIsDestroyed;
}

bool UCore_DestructionSystem::CanTakeDamage() const
{
    return bCanBeDestroyed && !bIsDestroyed;
}

void UCore_DestructionSystem::ApplyMaterialSpecificDestruction(ECore_MaterialType Material, const FCore_DestructionEvent& Event)
{
    switch (Material)
    {
        case ECore_MaterialType::Wood:
            // Wood splinters and breaks apart
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying wood destruction pattern"));
            break;
        case ECore_MaterialType::Stone:
            // Stone crumbles into chunks
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying stone destruction pattern"));
            break;
        case ECore_MaterialType::Glass:
            // Glass shatters into many small pieces
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying glass destruction pattern"));
            break;
        case ECore_MaterialType::Metal:
            // Metal dents and tears
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying metal destruction pattern"));
            break;
        case ECore_MaterialType::Bone:
            // Bone fractures and splinters
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying bone destruction pattern"));
            break;
        case ECore_MaterialType::Flesh:
            // Flesh tears and bleeds
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying flesh destruction pattern"));
            break;
        case ECore_MaterialType::Ice:
            // Ice shatters and melts
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying ice destruction pattern"));
            break;
        case ECore_MaterialType::Crystal:
            // Crystal fractures with prismatic effects
            UE_LOG(LogDestructionSystem, Log, TEXT("Applying crystal destruction pattern"));
            break;
    }
}

FVector UCore_DestructionSystem::CalculateDebrisVelocity(const FVector& ImpactDirection, float Force) const
{
    // Base velocity from impact direction
    FVector BaseVelocity = ImpactDirection.GetSafeNormal() * FMath::Sqrt(Force) * 10.0f;
    
    // Add random spread
    FVector RandomSpread = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(0.2f, 1.0f)
    ) * 200.0f;
    
    return BaseVelocity + RandomSpread;
}

void UCore_DestructionSystem::SpawnDebrisActor(const FVector& Location, const FVector& Velocity, UStaticMesh* DebrisMesh)
{
    if (!GetWorld() || !DebrisMesh)
    {
        return;
    }

    // Spawn debris actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* DebrisActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    
    if (DebrisActor)
    {
        // Set up mesh
        UStaticMeshComponent* MeshComp = DebrisActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(DebrisMesh);
            
            // Enable physics
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Set physics properties
            MeshComp->SetMassOverrideInKg(NAME_None, DebrisMass);
            
            // Apply initial velocity
            MeshComp->SetPhysicsLinearVelocity(Velocity);
            
            // Add random angular velocity
            FVector AngularVelocity = FVector(
                FMath::RandRange(-360.0f, 360.0f),
                FMath::RandRange(-360.0f, 360.0f),
                FMath::RandRange(-360.0f, 360.0f)
            );
            MeshComp->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
        }
        
        // Store reference for cleanup
        SpawnedDebris.Add(DebrisActor);
        
        // Set debris to destroy itself after lifetime
        DebrisActor->SetLifeSpan(DebrisLifetime);
    }
}

void UCore_DestructionSystem::CleanupDebris()
{
    UE_LOG(LogDestructionSystem, Log, TEXT("Cleaning up %d debris pieces"), SpawnedDebris.Num());
    
    for (AActor* DebrisActor : SpawnedDebris)
    {
        if (IsValid(DebrisActor))
        {
            DebrisActor->Destroy();
        }
    }
    
    SpawnedDebris.Empty();
}