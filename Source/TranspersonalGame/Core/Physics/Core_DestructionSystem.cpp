#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentHealth = DestructionData.Health;
    bIsDestroyed = false;
    bRegenerationInProgress = false;
    RegenerationTimer = 0.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.Health;
    
    // Find the static mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        OriginalMeshComponent = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (!OriginalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: No StaticMeshComponent found on owner %s"), *Owner->GetName());
        }
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bRegenerationInProgress)
    {
        ProcessRegeneration(DeltaTime);
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    if (bIsDestroyed || DamageAmount <= 0.0f)
    {
        return;
    }
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Applied %.2f damage, health now %.2f"), DamageAmount, CurrentHealth);
    
    if (CurrentHealth <= 0.0f)
    {
        FVector ImpactDirection = ImpactNormal * -1.0f;
        TriggerDestruction(ImpactPoint, ImpactDirection);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Triggering destruction of type %d"), (int32)DestructionData.DestructionType);
    
    // Hide original mesh
    if (OriginalMeshComponent)
    {
        OriginalMeshComponent->SetVisibility(false);
        OriginalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Create fragments based on destruction type
    CreateFragments(ImpactPoint, ImpactDirection);
    
    // Trigger Blueprint event
    OnDestructionTriggered(ImpactPoint);
    
    // Start regeneration if enabled
    if (DestructionData.bCanRegenerate)
    {
        StartRegeneration();
    }
}

void UCore_DestructionSystem::CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (!GetWorld() || !OriginalMeshComponent || FragmentMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Cannot create fragments - missing requirements"));
        return;
    }
    
    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FVector OwnerBounds = OriginalMeshComponent->Bounds.BoxExtent;
    
    int32 NumFragments = FMath::Clamp((int32)DestructionData.FragmentCount, 2, 20);
    
    for (int32 i = 0; i < NumFragments; i++)
    {
        // Random position around the original object
        FVector FragmentLocation = OwnerLocation + FVector(
            FMath::RandRange(-OwnerBounds.X, OwnerBounds.X),
            FMath::RandRange(-OwnerBounds.Y, OwnerBounds.Y),
            FMath::RandRange(-OwnerBounds.Z * 0.5f, OwnerBounds.Z)
        );
        
        // Random rotation
        FRotator FragmentRotation = FRotator(
            FMath::RandRange(-180.0f, 180.0f),
            FMath::RandRange(-180.0f, 180.0f),
            FMath::RandRange(-180.0f, 180.0f)
        );
        
        // Spawn fragment actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(FragmentLocation, FragmentRotation, SpawnParams);
        
        if (Fragment)
        {
            UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
            if (FragmentMesh)
            {
                // Set random fragment mesh
                int32 MeshIndex = FMath::RandRange(0, FragmentMeshes.Num() - 1);
                FragmentMesh->SetStaticMesh(FragmentMeshes[MeshIndex]);
                
                if (FragmentMaterial)
                {
                    FragmentMesh->SetMaterial(0, FragmentMaterial);
                }
                
                // Enable physics and apply impulse
                FragmentMesh->SetSimulatePhysics(true);
                FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                
                // Calculate impulse direction
                FVector ImpulseDirection = (FragmentLocation - ImpactPoint).GetSafeNormal();
                if (ImpulseDirection.IsNearlyZero())
                {
                    ImpulseDirection = ImpactDirection;
                }
                
                // Add randomness to impulse
                ImpulseDirection += FVector(
                    FMath::RandRange(-0.3f, 0.3f),
                    FMath::RandRange(-0.3f, 0.3f),
                    FMath::RandRange(0.1f, 0.5f)
                );
                
                float ImpulseStrength = DestructionData.ExplosionForce * FMath::RandRange(0.5f, 1.5f);
                FragmentMesh->AddImpulse(ImpulseDirection * ImpulseStrength);
                
                // Add angular impulse for spinning
                FVector AngularImpulse = FVector(
                    FMath::RandRange(-1000.0f, 1000.0f),
                    FMath::RandRange(-1000.0f, 1000.0f),
                    FMath::RandRange(-1000.0f, 1000.0f)
                );
                FragmentMesh->AddAngularImpulseInDegrees(AngularImpulse);
            }
            
            SpawnedFragments.Add(Fragment);
            
            // Schedule fragment cleanup
            FTimerHandle CleanupTimer;
            GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [Fragment]()
            {
                if (IsValid(Fragment))
                {
                    Fragment->Destroy();
                }
            }, FMath::RandRange(5.0f, 15.0f), false);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Created %d fragments"), SpawnedFragments.Num());
}

void UCore_DestructionSystem::StartRegeneration()
{
    if (!DestructionData.bCanRegenerate || bRegenerationInProgress)
    {
        return;
    }
    
    bRegenerationInProgress = true;
    RegenerationTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Starting regeneration (%.2f seconds)"), DestructionData.RegenerationTime);
}

void UCore_DestructionSystem::ProcessRegeneration(float DeltaTime)
{
    if (!bRegenerationInProgress)
    {
        return;
    }
    
    RegenerationTimer += DeltaTime;
    
    if (RegenerationTimer >= DestructionData.RegenerationTime)
    {
        ResetDestruction();
        bRegenerationInProgress = false;
        RegenerationTimer = 0.0f;
        
        OnRegenerationComplete();
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Regeneration complete"));
    }
}

void UCore_DestructionSystem::ResetDestruction()
{
    // Clean up existing fragments
    for (AActor* Fragment : SpawnedFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    SpawnedFragments.Empty();
    
    // Restore original mesh
    if (OriginalMeshComponent)
    {
        OriginalMeshComponent->SetVisibility(true);
        OriginalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Reset state
    bIsDestroyed = false;
    CurrentHealth = DestructionData.Health;
    bRegenerationInProgress = false;
    RegenerationTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem: Destruction reset"));
}