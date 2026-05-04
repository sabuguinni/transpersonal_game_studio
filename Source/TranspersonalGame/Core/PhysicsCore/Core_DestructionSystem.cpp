#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentHealth = 100.0f;
    bIsDestroyed = false;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.HealthPoints;
    bIsDestroyed = false;
    
    // Bind to hit events if owner has a mesh component
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->OnComponentHit.AddDynamic(this, &UCore_DestructionSystem::OnComponentHit);
            
            if (bDebugDestruction)
            {
                UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Bound to mesh component on %s"), *Owner->GetName());
            }
        }
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up old fragments
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
    if (bIsDestroyed || !DestructionData.bCanBeDestroyed)
    {
        return;
    }
    
    CurrentHealth -= DamageAmount;
    
    if (bDebugDestruction)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Applied %.1f damage, health now %.1f"), DamageAmount, CurrentHealth);
    }
    
    // Apply visual impact force
    ApplyImpactForce(ImpactLocation, ImpactDirection, DamageAmount * 10.0f);
    
    // Check if object should be destroyed
    if (CurrentHealth <= 0.0f)
    {
        DestroyObject();
    }
}

void UCore_DestructionSystem::DestroyObject()
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    if (bDebugDestruction)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Destroying object %s"), *GetOwner()->GetName());
    }
    
    // Spawn destruction fragments
    if (AActor* Owner = GetOwner())
    {
        SpawnDestructionFragments(Owner->GetActorLocation());
        
        // Hide or destroy the original object
        Owner->SetActorHiddenInGame(true);
        Owner->SetActorEnableCollision(false);
        
        // Schedule destruction after fragments settle
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
        {
            if (AActor* Owner = GetOwner())
            {
                Owner->Destroy();
            }
        }, DestructionData.FragmentLifetime + 2.0f, false);
    }
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return DestructionData.bCanBeDestroyed && !bIsDestroyed && CurrentHealth > 0.0f;
}

float UCore_DestructionSystem::GetCurrentHealth() const
{
    return CurrentHealth;
}

void UCore_DestructionSystem::SetDestructionEnabled(bool bEnabled)
{
    DestructionData.bCanBeDestroyed = bEnabled;
}

void UCore_DestructionSystem::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!DestructionData.bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }
    
    // Calculate impact force
    float ImpactForce = NormalImpulse.Size();
    
    if (ImpactForce > DestructionData.ImpactThreshold)
    {
        // Convert impact force to damage
        float Damage = (ImpactForce - DestructionData.ImpactThreshold) * 0.1f;
        
        FVector ImpactDirection = NormalImpulse.GetSafeNormal();
        ApplyDamage(Damage, Hit.Location, ImpactDirection);
        
        if (bDebugDestruction)
        {
            UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: Impact from %s, force %.1f, damage %.1f"), 
                OtherActor ? *OtherActor->GetName() : TEXT("Unknown"), ImpactForce, Damage);
        }
    }
}

void UCore_DestructionSystem::SpawnDestructionFragments(const FVector& ImpactLocation)
{
    if (DestructionData.DestroyedMeshPieces.Num() == 0)
    {
        // Create default fragments using basic shapes
        for (int32 i = 0; i < 5; i++)
        {
            FVector FragmentLocation = ImpactLocation + FVector(
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(50.0f, 150.0f)
            );
            
            FRotator FragmentRotation = FRotator(
                FMath::RandRange(-45.0f, 45.0f),
                FMath::RandRange(0.0f, 360.0f),
                FMath::RandRange(-45.0f, 45.0f)
            );
            
            if (UWorld* World = GetWorld())
            {
                AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>(FragmentLocation, FragmentRotation);
                if (Fragment)
                {
                    Fragment->SetActorLabel(FString::Printf(TEXT("Fragment_%d"), i));
                    
                    // Configure fragment physics
                    if (UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent())
                    {
                        FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                        FragmentMesh->SetSimulatePhysics(true);
                        FragmentMesh->SetMassOverrideInKg(NAME_None, FMath::RandRange(5.0f, 20.0f));
                        
                        // Apply random impulse
                        FVector RandomImpulse = FVector(
                            FMath::RandRange(-500.0f, 500.0f),
                            FMath::RandRange(-500.0f, 500.0f),
                            FMath::RandRange(200.0f, 800.0f)
                        );
                        FragmentMesh->AddImpulse(RandomImpulse);
                    }
                    
                    SpawnedFragments.Add(Fragment);
                    
                    // Schedule fragment cleanup
                    FTimerHandle CleanupTimer;
                    World->GetTimerManager().SetTimer(CleanupTimer, [Fragment]()
                    {
                        if (IsValid(Fragment))
                        {
                            Fragment->Destroy();
                        }
                    }, DestructionData.FragmentLifetime, false);
                }
            }
        }
    }
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
}

void UCore_DestructionSystem::ApplyImpactForce(const FVector& ImpactLocation, const FVector& ImpactDirection, float Force)
{
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            if (MeshComp->IsSimulatingPhysics())
            {
                FVector ForceVector = ImpactDirection * Force;
                MeshComp->AddImpulseAtLocation(ForceVector, ImpactLocation);
            }
        }
    }
}