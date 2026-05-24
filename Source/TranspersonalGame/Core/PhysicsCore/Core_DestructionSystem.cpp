#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize destruction data
    DestructionData = FCore_DestructionData();
    CurrentHealth = DestructionData.HealthPoints;
    bIsDestroyed = false;
    bDebugDestruction = false;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.HealthPoints;
    bIsDestroyed = false;
    SpawnedFragments.Empty();
    
    // Bind to hit events if owner has primitive component
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->OnComponentHit.AddDynamic(this, &UCore_DestructionSystem::OnComponentHit);
            
            if (bDebugDestruction)
            {
                UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Bound to hit events for %s"), *Owner->GetName());
            }
        }
    }
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up expired fragments
    CleanupFragments();
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (!DestructionData.bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }
    
    CurrentHealth -= DamageAmount;
    
    if (bDebugDestruction)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Applied %.2f damage, health now %.2f"), DamageAmount, CurrentHealth);
    }
    
    // Apply impact force
    ApplyImpactForce(ImpactLocation, ImpactDirection, DamageAmount * 10.0f);
    
    // Check if object should be destroyed
    if (CurrentHealth <= 0.0f)
    {
        DestroyObject();
    }
}

void UCore_DestructionSystem::DestroyObject()
{
    if (bIsDestroyed || !DestructionData.bCanBeDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    if (AActor* Owner = GetOwner())
    {
        FVector DestructionLocation = Owner->GetActorLocation();
        
        // Spawn destruction fragments
        SpawnDestructionFragments(DestructionLocation);
        
        if (bDebugDestruction)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Destroyed %s at location %s"), 
                   *Owner->GetName(), *DestructionLocation.ToString());
        }
        
        // Hide or destroy the original actor
        Owner->SetActorHiddenInGame(true);
        Owner->SetActorEnableCollision(false);
        
        // Schedule destruction after fragment lifetime
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
        {
            if (AActor* Owner = GetOwner())
            {
                Owner->Destroy();
            }
        }, DestructionData.FragmentLifetime + 1.0f, false);
    }
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return DestructionData.bCanBeDestroyed && !bIsDestroyed;
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
    if (!DestructionData.bCanBeDestroyed || bIsDestroyed || !OtherActor)
    {
        return;
    }
    
    // Calculate impact force magnitude
    float ImpactForce = NormalImpulse.Size();
    
    if (ImpactForce >= DestructionData.ImpactThreshold)
    {
        FVector ImpactDirection = NormalImpulse.GetSafeNormal();
        float DamageAmount = FMath::Max(0.0f, (ImpactForce - DestructionData.ImpactThreshold) * 0.5f);
        
        if (bDebugDestruction)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Impact from %s, force %.2f, damage %.2f"), 
                   *OtherActor->GetName(), ImpactForce, DamageAmount);
        }
        
        ApplyDamage(DamageAmount, Hit.Location, ImpactDirection);
    }
}

void UCore_DestructionSystem::SpawnDestructionFragments(const FVector& ImpactLocation)
{
    if (DestructionData.DestroyedMeshPieces.Num() == 0)
    {
        // Create default fragments if no custom meshes provided
        int32 NumFragments = FMath::RandRange(3, 8);
        
        for (int32 i = 0; i < NumFragments; i++)
        {
            FVector FragmentLocation = ImpactLocation + FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(0.0f, 50.0f)
            );
            
            FRotator FragmentRotation = FRotator(
                FMath::RandRange(-180.0f, 180.0f),
                FMath::RandRange(-180.0f, 180.0f),
                FMath::RandRange(-180.0f, 180.0f)
            );
            
            // Spawn a basic cube as fragment
            if (UWorld* World = GetWorld())
            {
                AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>(FragmentLocation, FragmentRotation);
                if (Fragment)
                {
                    // Set up basic cube mesh
                    if (UStaticMeshComponent* MeshComp = Fragment->GetStaticMeshComponent())
                    {
                        // Load default cube mesh
                        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
                        if (CubeMesh)
                        {
                            MeshComp->SetStaticMesh(CubeMesh);
                            MeshComp->SetWorldScale3D(FVector(FMath::RandRange(0.1f, 0.5f)));
                            MeshComp->SetSimulatePhysics(true);
                            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                            
                            // Apply random impulse
                            FVector RandomImpulse = FVector(
                                FMath::RandRange(-500.0f, 500.0f),
                                FMath::RandRange(-500.0f, 500.0f),
                                FMath::RandRange(200.0f, 800.0f)
                            );
                            MeshComp->AddImpulse(RandomImpulse, NAME_None, true);
                        }
                    }
                    
                    SpawnedFragments.Add(Fragment);
                    
                    // Schedule fragment destruction
                    FTimerHandle FragmentTimer;
                    World->GetTimerManager().SetTimer(FragmentTimer, [Fragment]()
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
    else
    {
        // Use custom mesh pieces
        for (UStaticMesh* MeshPiece : DestructionData.DestroyedMeshPieces)
        {
            if (MeshPiece)
            {
                FVector FragmentLocation = ImpactLocation + FVector(
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(0.0f, 50.0f)
                );
                
                FRotator FragmentRotation = FRotator(
                    FMath::RandRange(-180.0f, 180.0f),
                    FMath::RandRange(-180.0f, 180.0f),
                    FMath::RandRange(-180.0f, 180.0f)
                );
                
                if (UWorld* World = GetWorld())
                {
                    AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>(FragmentLocation, FragmentRotation);
                    if (Fragment && Fragment->GetStaticMeshComponent())
                    {
                        Fragment->GetStaticMeshComponent()->SetStaticMesh(MeshPiece);
                        Fragment->GetStaticMeshComponent()->SetSimulatePhysics(true);
                        Fragment->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                        
                        SpawnedFragments.Add(Fragment);
                    }
                }
            }
        }
    }
}

void UCore_DestructionSystem::CleanupFragments()
{
    // Remove invalid fragments from tracking array
    SpawnedFragments.RemoveAll([](AActor* Fragment)
    {
        return !IsValid(Fragment);
    });
}

void UCore_DestructionSystem::ApplyImpactForce(const FVector& ImpactLocation, const FVector& ImpactDirection, float Force)
{
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                FVector ImpulseVector = ImpactDirection * Force;
                PrimComp->AddImpulseAtLocation(ImpulseVector, ImpactLocation);
                
                if (bDebugDestruction)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Applied impulse %s at location %s"), 
                           *ImpulseVector.ToString(), *ImpactLocation.ToString());
                }
            }
        }
    }
}