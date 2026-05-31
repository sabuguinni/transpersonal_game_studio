#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Math/UnrealMathUtility.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default destruction data
    DestructionData.DestructionType = ECore_DestructionType::Fracture;
    DestructionData.DestructionThreshold = 100.0f;
    DestructionData.FragmentLifetime = 10.0f;
    DestructionData.MaxFragments = 20;
    DestructionData.bAutoCleanup = true;
    
    bCanBeDestroyed = true;
    CurrentDamage = 0.0f;
    bIsDestroyed = false;
    TimeSinceDestruction = 0.0f;
    bInitialized = false;
    FragmentCleanupTimer = 0.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDestruction();
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDestroyed)
    {
        TimeSinceDestruction += DeltaTime;
        UpdateFragmentCleanup(DeltaTime);
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    if (!bCanBeDestroyed || bIsDestroyed || DamageAmount <= 0.0f)
    {
        return;
    }
    
    CurrentDamage += DamageAmount;
    
    if (CurrentDamage >= DestructionData.DestructionThreshold)
    {
        FVector ImpactForce = ImpactNormal * DamageAmount * 10.0f; // Scale force based on damage
        TriggerDestruction(ImpactPoint, ImpactForce);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactForce)
{
    if (!bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    TimeSinceDestruction = 0.0f;
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(GetOwner(), ImpactPoint);
    
    // Spawn fragments based on destruction type
    SpawnFragments(ImpactPoint, ImpactForce);
    
    // Hide or disable the original actor
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorHiddenInGame(true);
        
        // Disable collision on all primitive components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
    
    OnDestructionComplete.Broadcast();
}

void UCore_DestructionSystem::SpawnFragments(const FVector& ImpactPoint, const FVector& ImpactForce)
{
    if (!GetWorld() || FragmentMeshes.Num() == 0)
    {
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector OwnerLocation = Owner->GetActorLocation();
    FVector OwnerBounds = Owner->GetComponentsBoundingBox().GetSize();
    
    int32 FragmentsToSpawn = FMath::Min(DestructionData.MaxFragments, FragmentMeshes.Num());
    
    for (int32 i = 0; i < FragmentsToSpawn; i++)
    {
        if (!FragmentMeshes[i])
        {
            continue;
        }
        
        // Calculate fragment spawn location with some randomization
        FVector FragmentLocation = OwnerLocation + FVector(
            FMath::RandRange(-OwnerBounds.X * 0.5f, OwnerBounds.X * 0.5f),
            FMath::RandRange(-OwnerBounds.Y * 0.5f, OwnerBounds.Y * 0.5f),
            FMath::RandRange(-OwnerBounds.Z * 0.5f, OwnerBounds.Z * 0.5f)
        );
        
        FRotator FragmentRotation = FRotator(
            FMath::RandRange(-180.0f, 180.0f),
            FMath::RandRange(-180.0f, 180.0f),
            FMath::RandRange(-180.0f, 180.0f)
        );
        
        FVector FragmentVelocity = CalculateFragmentVelocity(ImpactPoint, FragmentLocation, ImpactForce);
        
        AActor* Fragment = CreateFragment(FragmentMeshes[i], FragmentLocation, FragmentRotation, FragmentVelocity);
        if (Fragment)
        {
            SpawnedFragments.Add(Fragment);
            OnFragmentSpawned.Broadcast(Fragment);
        }
    }
}

AActor* UCore_DestructionSystem::CreateFragment(UStaticMesh* FragmentMesh, const FVector& Location, const FRotator& Rotation, const FVector& Velocity)
{
    if (!GetWorld() || !FragmentMesh)
    {
        return nullptr;
    }
    
    // Spawn static mesh actor for fragment
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* FragmentActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParams);
    if (!FragmentActor)
    {
        return nullptr;
    }
    
    // Configure the static mesh component
    UStaticMeshComponent* MeshComp = FragmentActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetStaticMesh(FragmentMesh);
        
        if (FragmentMaterial)
        {
            MeshComp->SetMaterial(0, FragmentMaterial);
        }
        
        // Enable physics simulation
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        
        // Apply initial velocity
        SetupFragmentPhysics(FragmentActor, Velocity);
    }
    
    // Set fragment lifetime
    if (DestructionData.bAutoCleanup && DestructionData.FragmentLifetime > 0.0f)
    {
        FragmentActor->SetLifeSpan(DestructionData.FragmentLifetime);
    }
    
    return FragmentActor;
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

bool UCore_DestructionSystem::CanBeDestroyed() const
{
    return bCanBeDestroyed && !bIsDestroyed;
}

float UCore_DestructionSystem::GetDestructionProgress() const
{
    if (DestructionData.DestructionThreshold <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(CurrentDamage / DestructionData.DestructionThreshold, 0.0f, 1.0f);
}

bool UCore_DestructionSystem::ValidateDestructionSetup() const
{
    if (DestructionData.DestructionThreshold <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: DestructionThreshold must be greater than 0"));
        return false;
    }
    
    if (DestructionData.MaxFragments <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: MaxFragments must be greater than 0"));
        return false;
    }
    
    if (FragmentMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionSystem: No fragment meshes assigned"));
        return false;
    }
    
    return true;
}

void UCore_DestructionSystem::InitializeDestruction()
{
    if (bInitialized)
    {
        return;
    }
    
    // Validate setup
    if (!ValidateDestructionSetup())
    {
        UE_LOG(LogTemp, Error, TEXT("DestructionSystem: Failed validation, destruction disabled"));
        bCanBeDestroyed = false;
        return;
    }
    
    bInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DestructionSystem: Initialized successfully"));
}

void UCore_DestructionSystem::UpdateFragmentCleanup(float DeltaTime)
{
    if (!DestructionData.bAutoCleanup)
    {
        return;
    }
    
    FragmentCleanupTimer += DeltaTime;
    
    // Clean up invalid fragments
    for (int32 i = SpawnedFragments.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(SpawnedFragments[i]))
        {
            SpawnedFragments.RemoveAt(i);
        }
    }
    
    // Force cleanup after fragment lifetime expires
    if (FragmentCleanupTimer >= DestructionData.FragmentLifetime && DestructionData.FragmentLifetime > 0.0f)
    {
        CleanupFragments();
        FragmentCleanupTimer = 0.0f;
    }
}

FVector UCore_DestructionSystem::CalculateFragmentVelocity(const FVector& ImpactPoint, const FVector& FragmentLocation, const FVector& ImpactForce) const
{
    // Calculate direction from impact point to fragment
    FVector Direction = (FragmentLocation - ImpactPoint).GetSafeNormal();
    
    // Add some randomization
    Direction += FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(0.1f, 0.5f)  // Bias upward
    );
    Direction.Normalize();
    
    // Scale by impact force magnitude
    float ForceScale = FMath::Clamp(ImpactForce.Size() * 0.01f, 100.0f, 1000.0f);
    
    return Direction * ForceScale;
}

void UCore_DestructionSystem::SetupFragmentPhysics(AActor* Fragment, const FVector& Velocity) const
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
    
    // Apply initial velocity
    MeshComp->SetPhysicsLinearVelocity(Velocity);
    
    // Add some angular velocity for more realistic tumbling
    FVector AngularVelocity = FVector(
        FMath::RandRange(-360.0f, 360.0f),
        FMath::RandRange(-360.0f, 360.0f),
        FMath::RandRange(-360.0f, 360.0f)
    );
    MeshComp->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
    
    // Set physics properties for realistic fragment behavior
    if (FBodyInstance* BodyInstance = MeshComp->GetBodyInstance())
    {
        BodyInstance->SetMassOverride(FMath::RandRange(1.0f, 10.0f));
        BodyInstance->bOverrideMass = true;
        BodyInstance->LinearDamping = 0.1f;
        BodyInstance->AngularDamping = 0.1f;
    }
}