// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsIntegrationComponent.h"
#include "ChaosPhysicsManager.h"
#include "CollisionSystem.h"
#include "PhysicsSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsIntegration, Log, All);

// Static LOD distance thresholds
const TArray<float> UPhysicsIntegrationComponent::LODDistances = {500.0f, 1500.0f, 5000.0f, 15000.0f};

UPhysicsIntegrationComponent::UPhysicsIntegrationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    bWantsInitializeComponent = true;
}

void UPhysicsIntegrationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("PhysicsIntegrationComponent BeginPlay for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    InitializePhysicsIntegration();
}

void UPhysicsIntegrationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromPhysicsSystems();
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LODUpdateTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPhysicsIntegrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics LOD based on distance to player
    if (bPhysicsEnabled && bRegisteredWithPhysics)
    {
        UpdatePhysicsLOD();
    }
}

void UPhysicsIntegrationComponent::InitializePhysicsIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsIntegration, Error, TEXT("Failed to get world for physics integration"));
        return;
    }
    
    // Get references to physics subsystems
    ChaosPhysicsManager = UChaosPhysicsManager::Get(this);
    PhysicsSystemManager = UPhysicsSystemManager::Get(this);
    
    // Get collision system component from owner
    if (AActor* Owner = GetOwner())
    {
        CollisionSystem = Owner->FindComponentByClass<UCollisionSystem>();
    }
    
    if (!ChaosPhysicsManager)
    {
        UE_LOG(LogPhysicsIntegration, Warning, TEXT("ChaosPhysicsManager not found"));
    }
    
    if (!PhysicsSystemManager)
    {
        UE_LOG(LogPhysicsIntegration, Warning, TEXT("PhysicsSystemManager not found"));
    }
    
    if (!CollisionSystem)
    {
        UE_LOG(LogPhysicsIntegration, Log, TEXT("No CollisionSystem component found on owner"));
    }
    
    // Register with physics systems
    RegisterWithPhysicsSystems(PhysicsType);
    
    // Setup LOD update timer
    World->GetTimerManager().SetTimer(
        LODUpdateTimer,
        this,
        &UPhysicsIntegrationComponent::UpdatePhysicsLOD,
        LODUpdateFrequency,
        true
    );
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Physics integration initialized successfully"));
}

void UPhysicsIntegrationComponent::RegisterWithPhysicsSystems(const FString& PhysicsType)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogPhysicsIntegration, Error, TEXT("No owner actor for physics registration"));
        return;
    }
    
    this->PhysicsType = PhysicsType;
    
    // Register with Physics System Manager
    if (PhysicsSystemManager)
    {
        PhysicsSystemManager->RegisterPhysicsActor(Owner, PhysicsType);
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Registered actor %s with PhysicsSystemManager as type: %s"), 
               *Owner->GetName(), *PhysicsType);
    }
    
    // Apply appropriate physics profile
    if (ChaosPhysicsManager)
    {
        FString ProfileName = PhysicsType.IsEmpty() ? "Default" : PhysicsType;
        ChaosPhysicsManager->ApplyPhysicsProfile(Owner, ProfileName);
        AppliedPhysicsProfile = ProfileName;
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Applied physics profile %s to actor %s"), 
               *ProfileName, *Owner->GetName());
    }
    
    // Initialize collision system if available
    if (CollisionSystem)
    {
        CollisionSystem->InitializeCollisionSystem();
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Initialized collision system for actor %s"), 
               *Owner->GetName());
    }
    
    bRegisteredWithPhysics = true;
}

void UPhysicsIntegrationComponent::UnregisterFromPhysicsSystems()
{
    AActor* Owner = GetOwner();
    if (!Owner || !bRegisteredWithPhysics)
    {
        return;
    }
    
    // Unregister from Physics System Manager
    if (PhysicsSystemManager)
    {
        PhysicsSystemManager->UnregisterPhysicsActor(Owner);
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Unregistered actor %s from PhysicsSystemManager"), 
               *Owner->GetName());
    }
    
    bRegisteredWithPhysics = false;
}

void UPhysicsIntegrationComponent::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Enable/disable physics on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            Component->SetSimulatePhysics(bEnabled);
        }
    }
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Physics %s for actor %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), *Owner->GetName());
}

void UPhysicsIntegrationComponent::ApplyPhysicsProfile(const FString& ProfileName)
{
    if (ChaosPhysicsManager && GetOwner())
    {
        ChaosPhysicsManager->ApplyPhysicsProfile(GetOwner(), ProfileName);
        AppliedPhysicsProfile = ProfileName;
        
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Applied physics profile %s to actor %s"), 
               *ProfileName, *GetOwner()->GetName());
    }
}

bool UPhysicsIntegrationComponent::IsPhysicsActive() const
{
    if (!bPhysicsEnabled || !GetOwner())
    {
        return false;
    }
    
    // Check if any primitive component is simulating physics
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    GetOwner()->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (const UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            return true;
        }
    }
    
    return false;
}

void UPhysicsIntegrationComponent::ForcePhysicsUpdate()
{
    if (ChaosPhysicsManager)
    {
        ChaosPhysicsManager->ForcePhysicsUpdate();
    }
    
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Wake up physics bodies
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->IsSimulatingPhysics())
            {
                Component->WakeAllRigidBodies();
            }
        }
    }
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Forced physics update for actor %s"), 
           Owner ? *Owner->GetName() : TEXT("Unknown"));
}

void UPhysicsIntegrationComponent::SetPhysicsLOD(int32 LODLevel)
{
    CurrentPhysicsLOD = FMath::Clamp(LODLevel, 0, LODDistances.Num() - 1);
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Adjust physics complexity based on LOD level
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            switch (CurrentPhysicsLOD)
            {
                case 0: // Highest detail
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    break;
                case 1: // High detail
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    break;
                case 2: // Medium detail
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    break;
                case 3: // Low detail
                    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    break;
                default:
                    break;
            }
        }
    }
    
    UE_LOG(LogPhysicsIntegration, VeryVerbose, TEXT("Set physics LOD %d for actor %s"), 
           CurrentPhysicsLOD, *Owner->GetName());
}

float UPhysicsIntegrationComponent::GetDistanceToNearestPlayer() const
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    
    if (!Owner || !World)
    {
        return MAX_FLT;
    }
    
    float MinDistance = MAX_FLT;
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Find nearest player
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return MinDistance;
}

void UPhysicsIntegrationComponent::UpdatePhysicsLOD()
{
    float DistanceToPlayer = GetDistanceToNearestPlayer();
    int32 NewLODLevel = GetLODLevelForDistance(DistanceToPlayer);
    
    if (NewLODLevel != CurrentPhysicsLOD)
    {
        SetPhysicsLOD(NewLODLevel);
    }
}

int32 UPhysicsIntegrationComponent::GetLODLevelForDistance(float Distance) const
{
    for (int32 i = 0; i < LODDistances.Num(); ++i)
    {
        if (Distance <= LODDistances[i])
        {
            return i;
        }
    }
    
    return LODDistances.Num() - 1; // Return highest LOD level for very far distances
}