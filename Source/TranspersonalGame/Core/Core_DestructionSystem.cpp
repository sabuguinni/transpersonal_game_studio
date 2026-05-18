#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for destruction checks
    
    // Initialize destruction data
    DestructionData = FCore_DestructionData();
    CurrentHealth = DestructionData.HealthPoints;
    FragmentLifetime = 10.0f;
    bUsePhysicsFragments = true;
    bIsDestroyed = false;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = DestructionData.HealthPoints;
    InitializeFragmentMeshes();
    
    UE_LOG(LogTemp, Warning, TEXT("DestructionSystem initialized for %s - Health: %.1f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), CurrentHealth);
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for fragment cleanup
    if (SpawnedFragments.Num() > 0)
    {
        // Remove null fragments (destroyed by GC)
        SpawnedFragments.RemoveAll([](AActor* Fragment) {
            return !IsValid(Fragment);
        });
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactDirection)
{
    if (bIsDestroyed || !CanBeDestroyed())
    {
        return;
    }
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    
    // Broadcast damage event
    OnDamageApplied.Broadcast(GetOwner(), DamageAmount, CurrentHealth);
    
    UE_LOG(LogTemp, Warning, TEXT("Damage applied to %s: %.1f damage, %.1f health remaining"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), DamageAmount, CurrentHealth);
    
    // Check if destruction threshold reached
    if (CurrentHealth <= 0.0f || DamageAmount >= DestructionData.DamageThreshold)
    {
        TriggerDestruction(ImpactPoint);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ExplosionCenter)
{
    if (bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    // Create fragments
    CreateFragments(ExplosionCenter);
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(GetOwner(), ExplosionCenter);
    
    UE_LOG(LogTemp, Warning, TEXT("Destruction triggered for %s at location %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), *ExplosionCenter.ToString());
    
    // Hide original actor
    if (GetOwner())
    {
        GetOwner()->SetActorHiddenInGame(true);
        GetOwner()->SetActorEnableCollision(false);
        
        // Schedule cleanup
        GetWorld()->GetTimerManager().SetTimer(FragmentCleanupTimer, 
            this, &UCore_DestructionSystem::CleanupFragments, FragmentLifetime, false);
    }
}

void UCore_DestructionSystem::CreateFragments(const FVector& ExplosionCenter)
{
    if (FragmentMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No fragment meshes available for destruction"));
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector OwnerLocation = Owner->GetActorLocation();
    FVector OwnerBounds = Owner->GetActorBounds(false).BoxExtent;
    
    // Create fragments in a pattern around the explosion center
    int32 FragmentsToCreate = FMath::Min(DestructionData.FragmentCount, FragmentMeshes.Num() * 4);
    
    for (int32 i = 0; i < FragmentsToCreate; i++)
    {
        // Select random fragment mesh
        UStaticMesh* FragmentMesh = FragmentMeshes[i % FragmentMeshes.Num()];
        if (!FragmentMesh)
        {
            continue;
        }
        
        // Calculate fragment spawn location
        FVector RandomOffset = FVector(
            FMath::RandRange(-OwnerBounds.X, OwnerBounds.X),
            FMath::RandRange(-OwnerBounds.Y, OwnerBounds.Y),
            FMath::RandRange(-OwnerBounds.Z, OwnerBounds.Z)
        );
        FVector FragmentLocation = OwnerLocation + RandomOffset;
        
        // Calculate explosion velocity
        FVector ExplosionDirection = (FragmentLocation - ExplosionCenter).GetSafeNormal();
        FVector ExplosionVelocity = ExplosionDirection * FMath::RandRange(200.0f, DestructionData.ExplosionForce);
        
        // Create fragment
        AActor* Fragment = CreateFragment(FragmentMesh, FragmentLocation, ExplosionVelocity);
        if (Fragment)
        {
            SpawnedFragments.Add(Fragment);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d fragments for destruction"), SpawnedFragments.Num());
}

AActor* UCore_DestructionSystem::CreateFragment(UStaticMesh* FragmentMesh, const FVector& Location, const FVector& Velocity)
{
    if (!FragmentMesh || !GetWorld())
    {
        return nullptr;
    }
    
    // Spawn static mesh actor for fragment
    AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!Fragment)
    {
        return nullptr;
    }
    
    // Configure fragment
    Fragment->SetActorLabel(FString::Printf(TEXT("Fragment_%s_%d"), 
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), SpawnedFragments.Num()));
    
    UStaticMeshComponent* MeshComp = Fragment->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetStaticMesh(FragmentMesh);
        
        if (bUsePhysicsFragments)
        {
            ApplyFragmentPhysics(Fragment, Location - Velocity.GetSafeNormal() * 100.0f);
            
            // Apply initial velocity
            MeshComp->SetPhysicsLinearVelocity(Velocity);
            MeshComp->SetPhysicsAngularVelocityInRadians(FVector(
                FMath::RandRange(-10.0f, 10.0f),
                FMath::RandRange(-10.0f, 10.0f),
                FMath::RandRange(-10.0f, 10.0f)
            ));
        }
    }
    
    return Fragment;
}

void UCore_DestructionSystem::ApplyFragmentPhysics(AActor* Fragment, const FVector& ExplosionCenter)
{
    if (!Fragment)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Fragment->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        
        // Set physics properties
        if (MeshComp->GetBodyInstance())
        {
            MeshComp->GetBodyInstance()->SetMassOverride(FMath::RandRange(10.0f, 50.0f));
            MeshComp->GetBodyInstance()->SetLinearDamping(0.5f);
            MeshComp->GetBodyInstance()->SetAngularDamping(0.5f);
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
    
    // Destroy the original actor
    if (GetOwner())
    {
        GetOwner()->Destroy();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Destruction cleanup completed"));
}

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return DestructionData.bCanBeDestroyed && !bIsDestroyed;
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    if (DestructionData.HealthPoints <= 0.0f)
    {
        return 0.0f;
    }
    
    return CurrentHealth / DestructionData.HealthPoints;
}

void UCore_DestructionSystem::InitializeFragmentMeshes()
{
    // If no fragment meshes specified, try to use default cube fragments
    if (FragmentMeshes.Num() == 0)
    {
        // Try to load default engine cube mesh for fragments
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            FragmentMeshes.Add(CubeMesh);
            UE_LOG(LogTemp, Warning, TEXT("Using default cube mesh for destruction fragments"));
        }
    }
}