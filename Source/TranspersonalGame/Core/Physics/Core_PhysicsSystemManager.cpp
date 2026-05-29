#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default physics settings
    PhysicsSettings.Gravity = -980.0f;
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.05f;
    PhysicsSettings.MaxPhysicsStepDelta = 0.05f;
    PhysicsSettings.bEnableAsyncPhysics = true;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initialized with gravity=%f"), PhysicsSettings.Gravity);
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePhysicsLayers();
    ProcessRagdollActors(DeltaTime);
}

void UCore_PhysicsSystemManager::InitializePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No world found"));
        return;
    }

    // Apply gravity settings
    World->GetPhysicsScene()->GetPxScene()->setGravity(FVector(0.0f, 0.0f, PhysicsSettings.Gravity).ToPxVec3());
    
    ValidatePhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics settings applied"));
}

void UCore_PhysicsSystemManager::SetGravityScale(float NewGravity)
{
    PhysicsSettings.Gravity = NewGravity;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetPhysicsScene()->GetPxScene()->setGravity(FVector(0.0f, 0.0f, NewGravity).ToPxVec3());
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Gravity set to %f"), NewGravity);
    }
}

void UCore_PhysicsSystemManager::EnableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: EnableRagdoll - Actor is null"));
        return;
    }

    USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkelMesh)
    {
        SkelMesh->SetSimulatePhysics(true);
        SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        if (!RagdollActors.Contains(Actor))
        {
            RagdollActors.Add(Actor);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll enabled for %s"), *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No skeletal mesh found on %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkelMesh)
    {
        SkelMesh->SetSimulatePhysics(false);
        SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        RagdollActors.Remove(Actor);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Ragdoll disabled for %s"), *Actor->GetName());
    }
}

bool UCore_PhysicsSystemManager::IsActorInPhysicsLayer(AActor* Actor, ECore_PhysicsLayer Layer)
{
    if (!Actor)
    {
        return false;
    }

    if (LayeredActors.Contains(Layer))
    {
        return LayeredActors[Layer].Contains(Actor);
    }
    
    return false;
}

void UCore_PhysicsSystemManager::SetActorPhysicsLayer(AActor* Actor, ECore_PhysicsLayer Layer)
{
    if (!Actor)
    {
        return;
    }

    // Remove from all other layers first
    for (auto& LayerPair : LayeredActors)
    {
        LayerPair.Value.Remove(Actor);
    }

    // Add to new layer
    if (!LayeredActors.Contains(Layer))
    {
        LayeredActors.Add(Layer, TArray<AActor*>());
    }
    
    LayeredActors[Layer].AddUnique(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Actor %s assigned to layer %d"), *Actor->GetName(), (int32)Layer);
}

TArray<AActor*> UCore_PhysicsSystemManager::GetActorsInPhysicsLayer(ECore_PhysicsLayer Layer)
{
    if (LayeredActors.Contains(Layer))
    {
        return LayeredActors[Layer];
    }
    
    return TArray<AActor*>();
}

void UCore_PhysicsSystemManager::UpdatePhysicsLayers()
{
    // Clean up null actors from layers
    for (auto& LayerPair : LayeredActors)
    {
        LayerPair.Value.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
    }
    
    // Clean up null actors from ragdoll list
    RagdollActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

void UCore_PhysicsSystemManager::ProcessRagdollActors(float DeltaTime)
{
    for (AActor* Actor : RagdollActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMesh && SkelMesh->IsSimulatingPhysics())
        {
            // Apply damping to prevent excessive movement
            SkelMesh->SetLinearDamping(PhysicsSettings.LinearDamping);
            SkelMesh->SetAngularDamping(PhysicsSettings.AngularDamping);
        }
    }
}

void UCore_PhysicsSystemManager::ValidatePhysicsSettings()
{
    if (PhysicsSettings.Gravity > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Positive gravity detected, this may cause objects to fall upward"));
    }
    
    if (PhysicsSettings.MaxPhysicsStepDelta <= 0.0f)
    {
        PhysicsSettings.MaxPhysicsStepDelta = 0.05f;
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Invalid MaxPhysicsStepDelta, reset to 0.05"));
    }
}