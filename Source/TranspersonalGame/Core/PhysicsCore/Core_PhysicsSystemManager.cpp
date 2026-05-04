#include "Core_PhysicsSystemManager.h"
#include "Core_CollisionManager.h"
#include "Core_DestructionSystem.h"
#include "Core_RagdollManager.h"
#include "../BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default physics settings
    PhysicsSettings = FCore_PhysicsSettings();
    
    // Initialize subsystem pointers
    CollisionManager = nullptr;
    DestructionSystem = nullptr;
    RagdollManager = nullptr;
    
    // Initialize performance tracking
    ActivePhysicsObjectCount = 0;
    LastFramePhysicsTime = 0.0f;
    bPhysicsSystemInitialized = false;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystem();
}

void UCore_PhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownPhysicsSystem();
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bPhysicsSystemInitialized)
    {
        UpdatePhysicsPerformanceMetrics(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bPhysicsSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics system already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Core Physics System Manager"));
    
    // Initialize subsystems
    InitializeSubsystems();
    
    // Apply global physics settings
    if (UWorld* World = GetWorld())
    {
        if (UPhysicsSettings* WorldPhysicsSettings = UPhysicsSettings::Get())
        {
            // Apply our global gravity scale
            World->GetWorldSettings()->GlobalGravityZ = -980.0f * PhysicsSettings.GlobalGravityScale;
        }
    }
    
    bPhysicsSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bPhysicsSystemInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Shutting down Core Physics System Manager"));
    
    // Clear all registered physics objects
    RegisteredPhysicsObjects.Empty();
    
    // Cleanup subsystems
    CleanupSubsystems();
    
    bPhysicsSystemInitialized = false;
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager shutdown complete"));
}

bool UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor, float Mass)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot enable physics on null actor"));
        return false;
    }
    
    UPrimitiveComponent* PhysicsComponent = GetPrimaryPhysicsComponent(Actor);
    if (!PhysicsComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor %s has no physics component"), *Actor->GetName());
        return false;
    }
    
    // Enable physics simulation
    PhysicsComponent->SetSimulatePhysics(true);
    PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Set mass
    if (PhysicsComponent->GetBodyInstance())
    {
        PhysicsComponent->GetBodyInstance()->SetMassOverride(Mass, true);
    }
    
    // Apply default physics properties
    PhysicsComponent->SetPhysicsMaterialOverride(nullptr); // Use default physics material
    
    // Register the physics object
    FCore_PhysicsObjectData PhysicsData;
    PhysicsData.PhysicsActor = Actor;
    PhysicsData.PhysicsComponent = PhysicsComponent;
    PhysicsData.Mass = Mass;
    PhysicsData.bSimulatePhysics = true;
    
    RegisterPhysicsObject(Actor, PhysicsData);
    
    UE_LOG(LogTemp, Log, TEXT("Enabled physics on actor %s with mass %.2f"), *Actor->GetName(), Mass);
    return true;
}

bool UCore_PhysicsSystemManager::DisablePhysicsOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    UPrimitiveComponent* PhysicsComponent = GetPrimaryPhysicsComponent(Actor);
    if (PhysicsComponent)
    {
        PhysicsComponent->SetSimulatePhysics(false);
        PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    
    UnregisterPhysicsObject(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Disabled physics on actor %s"), *Actor->GetName());
    return true;
}

void UCore_PhysicsSystemManager::RegisterPhysicsObject(AActor* Actor, const FCore_PhysicsObjectData& PhysicsData)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if already registered
    for (int32 i = 0; i < RegisteredPhysicsObjects.Num(); i++)
    {
        if (RegisteredPhysicsObjects[i].PhysicsActor == Actor)
        {
            // Update existing entry
            RegisteredPhysicsObjects[i] = PhysicsData;
            return;
        }
    }
    
    // Add new entry
    RegisteredPhysicsObjects.Add(PhysicsData);
    ActivePhysicsObjectCount = RegisteredPhysicsObjects.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Registered physics object: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::UnregisterPhysicsObject(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    for (int32 i = RegisteredPhysicsObjects.Num() - 1; i >= 0; i--)
    {
        if (RegisteredPhysicsObjects[i].PhysicsActor == Actor)
        {
            RegisteredPhysicsObjects.RemoveAt(i);
            break;
        }
    }
    
    ActivePhysicsObjectCount = RegisteredPhysicsObjects.Num();
    UE_LOG(LogTemp, Log, TEXT("Unregistered physics object: %s"), *Actor->GetName());
}

TArray<FCore_PhysicsObjectData> UCore_PhysicsSystemManager::GetAllPhysicsObjects() const
{
    return RegisteredPhysicsObjects;
}

void UCore_PhysicsSystemManager::ApplyBiomePhysicsProperties(AActor* Actor, ECore_BiomeType BiomeType)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PhysicsComponent = GetPrimaryPhysicsComponent(Actor);
    if (!PhysicsComponent)
    {
        return;
    }
    
    // Apply biome-specific physics properties
    switch (BiomeType)
    {
        case ECore_BiomeType::Swamp:
            // Swamp: higher drag, lower friction
            if (PhysicsComponent->GetBodyInstance())
            {
                PhysicsComponent->GetBodyInstance()->LinearDamping = 2.0f;
                PhysicsComponent->GetBodyInstance()->AngularDamping = 2.0f;
            }
            break;
            
        case ECore_BiomeType::Desert:
            // Desert: lower friction (sand), normal drag
            if (PhysicsComponent->GetBodyInstance())
            {
                PhysicsComponent->GetBodyInstance()->LinearDamping = 0.5f;
                PhysicsComponent->GetBodyInstance()->AngularDamping = 0.5f;
            }
            break;
            
        case ECore_BiomeType::Mountain:
            // Mountain: higher friction (rock), normal drag
            if (PhysicsComponent->GetBodyInstance())
            {
                PhysicsComponent->GetBodyInstance()->LinearDamping = 1.0f;
                PhysicsComponent->GetBodyInstance()->AngularDamping = 1.0f;
            }
            break;
            
        case ECore_BiomeType::Forest:
        case ECore_BiomeType::Savanna:
        default:
            // Default properties
            if (PhysicsComponent->GetBodyInstance())
            {
                PhysicsComponent->GetBodyInstance()->LinearDamping = 1.0f;
                PhysicsComponent->GetBodyInstance()->AngularDamping = 1.0f;
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied biome physics properties for %s in biome %d"), *Actor->GetName(), (int32)BiomeType);
}

void UCore_PhysicsSystemManager::ApplyEnvironmentalForces(AActor* Actor, const FVector& Force, const FVector& Location)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PhysicsComponent = GetPrimaryPhysicsComponent(Actor);
    if (PhysicsComponent && PhysicsComponent->IsSimulatingPhysics())
    {
        PhysicsComponent->AddForceAtLocation(Force, Location);
        UE_LOG(LogTemp, Log, TEXT("Applied force %.2f,%.2f,%.2f to %s at location %.2f,%.2f,%.2f"), 
               Force.X, Force.Y, Force.Z, *Actor->GetName(), Location.X, Location.Y, Location.Z);
    }
}

void UCore_PhysicsSystemManager::SetupCollisionForActor(AActor* Actor, ECore_CollisionType CollisionType)
{
    if (!Actor || !CollisionManager)
    {
        return;
    }
    
    // Delegate to collision manager
    CollisionManager->SetupCollisionForActor(Actor, CollisionType);
}

bool UCore_PhysicsSystemManager::CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB)
{
    if (!ActorA || !ActorB || !CollisionManager)
    {
        return false;
    }
    
    return CollisionManager->CheckCollisionBetweenActors(ActorA, ActorB);
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float ImpactForce)
{
    if (!Actor || !DestructionSystem)
    {
        return;
    }
    
    DestructionSystem->TriggerDestruction(Actor, ImpactPoint, ImpactForce);
}

void UCore_PhysicsSystemManager::CreateDestructionFragments(AActor* Actor, int32 FragmentCount)
{
    if (!Actor || !DestructionSystem)
    {
        return;
    }
    
    DestructionSystem->CreateFragments(Actor, FragmentCount);
}

void UCore_PhysicsSystemManager::EnableRagdollPhysics(AActor* Character)
{
    if (!Character || !RagdollManager)
    {
        return;
    }
    
    RagdollManager->EnableRagdoll(Character);
}

void UCore_PhysicsSystemManager::DisableRagdollPhysics(AActor* Character)
{
    if (!Character || !RagdollManager)
    {
        return;
    }
    
    RagdollManager->DisableRagdoll(Character);
}

void UCore_PhysicsSystemManager::SetGlobalPhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Apply to world
    if (UWorld* World = GetWorld())
    {
        World->GetWorldSettings()->GlobalGravityZ = -980.0f * PhysicsSettings.GlobalGravityScale;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated global physics settings"));
}

FCore_PhysicsSettings UCore_PhysicsSystemManager::GetGlobalPhysicsSettings() const
{
    return PhysicsSettings;
}

void UCore_PhysicsSystemManager::PausePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(true);
        UE_LOG(LogTemp, Log, TEXT("Physics simulation paused"));
    }
}

void UCore_PhysicsSystemManager::ResumePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(false);
        UE_LOG(LogTemp, Log, TEXT("Physics simulation resumed"));
    }
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    return ActivePhysicsObjectCount;
}

float UCore_PhysicsSystemManager::GetPhysicsPerformanceMetrics() const
{
    return LastFramePhysicsTime;
}

void UCore_PhysicsSystemManager::ValidatePhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bPhysicsSystemInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Objects: %d"), ActivePhysicsObjectCount);
    UE_LOG(LogTemp, Warning, TEXT("Collision Manager: %s"), CollisionManager ? TEXT("VALID") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Destruction System: %s"), DestructionSystem ? TEXT("VALID") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll Manager: %s"), RagdollManager ? TEXT("VALID") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Last Frame Physics Time: %.4f ms"), LastFramePhysicsTime);
}

void UCore_PhysicsSystemManager::InitializeSubsystems()
{
    // Create collision manager
    CollisionManager = NewObject<UCore_CollisionManager>(this);
    if (CollisionManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Collision Manager initialized"));
    }
    
    // Create destruction system
    DestructionSystem = NewObject<UCore_DestructionSystem>(this);
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Destruction System initialized"));
    }
    
    // Create ragdoll manager
    RagdollManager = NewObject<UCore_RagdollManager>(this);
    if (RagdollManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Ragdoll Manager initialized"));
    }
}

void UCore_PhysicsSystemManager::CleanupSubsystems()
{
    if (CollisionManager)
    {
        CollisionManager = nullptr;
    }
    
    if (DestructionSystem)
    {
        DestructionSystem = nullptr;
    }
    
    if (RagdollManager)
    {
        RagdollManager = nullptr;
    }
}

UPrimitiveComponent* UCore_PhysicsSystemManager::GetPrimaryPhysicsComponent(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }
    
    // Try static mesh component first
    if (UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        return StaticMeshComp;
    }
    
    // Try skeletal mesh component
    if (USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        return SkeletalMeshComp;
    }
    
    // Try any primitive component
    if (UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        return PrimitiveComp;
    }
    
    return nullptr;
}

void UCore_PhysicsSystemManager::UpdatePhysicsPerformanceMetrics(float DeltaTime)
{
    // Simple performance tracking
    LastFramePhysicsTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update active object count
    ActivePhysicsObjectCount = RegisteredPhysicsObjects.Num();
    
    // Log performance warnings if needed
    if (LastFramePhysicsTime > 16.67f) // More than 60 FPS frame time
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics frame time high: %.2f ms with %d objects"), 
               LastFramePhysicsTime, ActivePhysicsObjectCount);
    }
}