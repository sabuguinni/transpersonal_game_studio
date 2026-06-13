#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCore_DestructionComponent::UCore_DestructionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    CurrentHealth = DestructionData.Health;
    bIsDestroyed = false;
}

void UCore_DestructionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        MeshComponent = Owner->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComponent)
        {
            // Register with the destruction system
            if (UWorld* World = GetWorld())
            {
                if (UCore_DestructionSystemManager* DestructionManager = World->GetSubsystem<UCore_DestructionSystemManager>())
                {
                    DestructionManager->RegisterDestructibleActor(Owner, DestructionData);
                }
            }
        }
    }
    
    CurrentHealth = DestructionData.Health;
}

void UCore_DestructionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update destruction state if needed
    if (bIsDestroyed && MeshComponent && MeshComponent->IsValidLowLevel())
    {
        // Fade out destroyed objects over time
        if (UMaterialInstanceDynamic* DynMat = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
        {
            static float FadeTime = 0.0f;
            FadeTime += DeltaTime;
            float Alpha = FMath::Clamp(1.0f - (FadeTime / 5.0f), 0.0f, 1.0f);
            DynMat->SetScalarParameterValue(TEXT("Opacity"), Alpha);
            
            if (Alpha <= 0.0f && GetOwner())
            {
                GetOwner()->Destroy();
            }
        }
    }
}

void UCore_DestructionComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDestroyed || !DestructionData.bCanBeDestroyed)
    {
        return;
    }
    
    CurrentHealth -= DamageAmount;
    
    if (CurrentHealth <= 0.0f)
    {
        TriggerDestruction();
    }
}

void UCore_DestructionComponent::TriggerDestruction()
{
    if (bIsDestroyed || !DestructionData.bCanBeDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    // Spawn fragments
    SpawnFragments();
    
    // Hide original mesh
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Play destruction sound/effects here if needed
    UE_LOG(LogTemp, Warning, TEXT("Destruction triggered for %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

bool UCore_DestructionComponent::IsDestroyed() const
{
    return bIsDestroyed;
}

void UCore_DestructionComponent::SpawnFragments()
{
    if (!MeshComponent || !GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector OwnerScale = GetOwner()->GetActorScale3D();
    
    int32 NumFragments = FMath::RoundToInt(DestructionData.FragmentCount);
    
    for (int32 i = 0; i < NumFragments; i++)
    {
        // Generate random position around the original object
        FVector RandomOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-50.0f, 50.0f)
        ) * OwnerScale;
        
        FVector FragmentPosition = OwnerLocation + RandomOffset;
        
        // Generate random velocity for explosion effect
        FVector ExplosionDirection = RandomOffset.GetSafeNormal();
        FVector FragmentVelocity = ExplosionDirection * DestructionData.ExplosionForce;
        
        CreateFragment(FragmentPosition, FragmentVelocity);
    }
}

void UCore_DestructionComponent::CreateFragment(const FVector& Position, const FVector& Velocity)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create fragment actor
    AActor* FragmentActor = GetWorld()->SpawnActor<AActor>();
    if (!FragmentActor)
    {
        return;
    }
    
    // Add mesh component
    UStaticMeshComponent* FragmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FragmentMesh"));
    if (FragmentMesh)
    {
        FragmentActor->SetRootComponent(FragmentMesh);
        
        // Use fragment mesh if available, otherwise use scaled down original
        if (DestructionData.FragmentMeshes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, DestructionData.FragmentMeshes.Num() - 1);
            if (DestructionData.FragmentMeshes[RandomIndex])
            {
                FragmentMesh->SetStaticMesh(DestructionData.FragmentMeshes[RandomIndex]);
            }
        }
        else if (MeshComponent && MeshComponent->GetStaticMesh())
        {
            FragmentMesh->SetStaticMesh(MeshComponent->GetStaticMesh());
            FragmentMesh->SetWorldScale3D(FVector(0.3f)); // Scale down fragments
        }
        
        // Set physics properties
        FragmentMesh->SetSimulatePhysics(true);
        FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        FragmentMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Apply initial velocity
        FragmentMesh->SetPhysicsLinearVelocity(Velocity);
        FragmentMesh->SetPhysicsAngularVelocityInDegrees(FVector(
            FMath::RandRange(-360.0f, 360.0f),
            FMath::RandRange(-360.0f, 360.0f),
            FMath::RandRange(-360.0f, 360.0f)
        ));
    }
    
    FragmentActor->SetActorLocation(Position);
    
    // Register fragment with destruction system for cleanup
    if (UCore_DestructionSystemManager* DestructionManager = GetWorld()->GetSubsystem<UCore_DestructionSystemManager>())
    {
        // Fragment cleanup will be handled by the manager
    }
}

// Destruction System Manager Implementation
void UCore_DestructionSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimer,
            this,
            &UCore_DestructionSystemManager::CleanupFragments,
            CleanupInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystemManager initialized"));
}

void UCore_DestructionSystemManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimer);
    }
    
    RegisteredActors.Empty();
    FragmentActors.Empty();
    
    Super::Deinitialize();
}

void UCore_DestructionSystemManager::RegisterDestructibleActor(AActor* Actor, const FCore_DestructionData& DestructionData)
{
    if (Actor)
    {
        RegisteredActors.Add(Actor, DestructionData);
        UE_LOG(LogTemp, Warning, TEXT("Registered destructible actor: %s"), *Actor->GetName());
    }
}

void UCore_DestructionSystemManager::UnregisterDestructibleActor(AActor* Actor)
{
    if (Actor)
    {
        RegisteredActors.Remove(Actor);
    }
}

void UCore_DestructionSystemManager::ProcessAreaDestruction(const FVector& Center, float Radius, float Damage)
{
    TArray<AActor*> ActorsInRange = GetDestructibleActorsInRadius(Center, Radius);
    
    for (AActor* Actor : ActorsInRange)
    {
        if (UCore_DestructionComponent* DestructionComp = Actor->FindComponentByClass<UCore_DestructionComponent>())
        {
            // Calculate distance-based damage falloff
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            float DamageFalloff = FMath::Clamp(1.0f - (Distance / Radius), 0.1f, 1.0f);
            float FinalDamage = Damage * DamageFalloff;
            
            DestructionComp->ApplyDamage(FinalDamage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Area destruction processed: %d actors affected"), ActorsInRange.Num());
}

TArray<AActor*> UCore_DestructionSystemManager::GetDestructibleActorsInRadius(const FVector& Center, float Radius)
{
    TArray<AActor*> Result;
    
    for (auto& Pair : RegisteredActors)
    {
        AActor* Actor = Pair.Key;
        if (Actor && IsValid(Actor))
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

void UCore_DestructionSystemManager::CleanupFragments()
{
    // Remove invalid fragments
    FragmentActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    // If we have too many fragments, remove the oldest ones
    if (FragmentActors.Num() > MaxFragments)
    {
        int32 NumToRemove = FragmentActors.Num() - MaxFragments;
        for (int32 i = 0; i < NumToRemove; i++)
        {
            if (FragmentActors[i] && IsValid(FragmentActors[i]))
            {
                FragmentActors[i]->Destroy();
            }
        }
        FragmentActors.RemoveAt(0, NumToRemove);
    }
    
    // Clean up registered actors that are no longer valid
    TArray<AActor*> ActorsToRemove;
    for (auto& Pair : RegisteredActors)
    {
        if (!IsValid(Pair.Key))
        {
            ActorsToRemove.Add(Pair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        RegisteredActors.Remove(Actor);
    }
}

void UCore_DestructionSystemManager::UpdateDestructionStates()
{
    // Update any global destruction state logic here
    // This could include environmental effects, sound management, etc.
}