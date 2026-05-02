#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    GlobalPhysicsTimeDilation = 1.0f;
    bEnableAdvancedPhysics = true;
    bEnableRagdollPhysics = true;
    bEnableDestructiblePhysics = false; // Disabled by default for performance
    
    MaxPhysicsSubsteps = 6;
    MaxSubstepDeltaTime = 0.016667f; // 60 FPS
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core_PhysicsSystemManager initialized"));
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystem();
    RegisterPhysicsEvents();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System started"));
}

void UCore_PhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterPhysicsEvents();
    CleanupPhysicsSystem();
    
    Super::EndPlay(EndPlayReason);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System stopped"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableAdvancedPhysics)
    {
        UpdatePhysicsSimulation(DeltaTime);
        MonitorPhysicsPerformance(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Failed to get world for physics initialization"));
        return;
    }
    
    // Configure global physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->MaxSubstepDeltaTime = MaxSubstepDeltaTime;
        PhysicsSettings->MaxSubsteps = MaxPhysicsSubsteps;
    }
    
    // Initialize physics tracking
    ActiveRigidBodies.Empty();
    RagdollActors.Empty();
    DestructibleActors.Empty();
    
    // Scan for existing physics actors
    ScanForPhysicsActors();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics system initialized with %d rigid bodies"), ActiveRigidBodies.Num());
}

void UCore_PhysicsSystemManager::CleanupPhysicsSystem()
{
    // Clean up tracking arrays
    ActiveRigidBodies.Empty();
    RagdollActors.Empty();
    DestructibleActors.Empty();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics system cleaned up"));
}

void UCore_PhysicsSystemManager::RegisterPhysicsEvents()
{
    // Register for physics collision events if needed
    UE_LOG(LogCorePhysics, Log, TEXT("Physics events registered"));
}

void UCore_PhysicsSystemManager::UnregisterPhysicsEvents()
{
    // Unregister physics events
    UE_LOG(LogCorePhysics, Log, TEXT("Physics events unregistered"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Update physics time dilation
    UWorld* World = GetWorld();
    if (World && World->GetWorldSettings())
    {
        World->GetWorldSettings()->TimeDilation = GlobalPhysicsTimeDilation;
    }
    
    // Update ragdoll physics
    if (bEnableRagdollPhysics)
    {
        UpdateRagdollPhysics(DeltaTime);
    }
    
    // Update destructible physics
    if (bEnableDestructiblePhysics)
    {
        UpdateDestructiblePhysics(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::MonitorPhysicsPerformance(float DeltaTime)
{
    // Simple performance monitoring
    static float AccumulatedTime = 0.0f;
    static int32 FrameCount = 0;
    
    AccumulatedTime += DeltaTime;
    FrameCount++;
    
    // Log performance every 5 seconds
    if (AccumulatedTime >= 5.0f)
    {
        float AvgFrameTime = AccumulatedTime / FrameCount;
        float AvgFPS = 1.0f / AvgFrameTime;
        
        UE_LOG(LogCorePhysics, Log, TEXT("Physics Performance - Avg FPS: %.1f, Active Bodies: %d, Ragdolls: %d"), 
               AvgFPS, ActiveRigidBodies.Num(), RagdollActors.Num());
        
        AccumulatedTime = 0.0f;
        FrameCount = 0;
    }
}

void UCore_PhysicsSystemManager::ScanForPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Scan for actors with physics components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        // Check for static mesh components with physics
        if (UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            if (StaticMeshComp->IsSimulatingPhysics())
            {
                RegisterRigidBody(StaticMeshComp);
            }
        }
        
        // Check for skeletal mesh components (potential ragdolls)
        if (USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (SkeletalMeshComp->IsSimulatingPhysics())
            {
                RegisterRagdollActor(Actor);
            }
        }
    }
}

void UCore_PhysicsSystemManager::RegisterRigidBody(UPrimitiveComponent* Component)
{
    if (Component && !ActiveRigidBodies.Contains(Component))
    {
        ActiveRigidBodies.Add(Component);
        UE_LOG(LogCorePhysics, Log, TEXT("Registered rigid body: %s"), *Component->GetName());
    }
}

void UCore_PhysicsSystemManager::UnregisterRigidBody(UPrimitiveComponent* Component)
{
    if (Component)
    {
        ActiveRigidBodies.Remove(Component);
        UE_LOG(LogCorePhysics, Log, TEXT("Unregistered rigid body: %s"), *Component->GetName());
    }
}

void UCore_PhysicsSystemManager::RegisterRagdollActor(AActor* Actor)
{
    if (Actor && !RagdollActors.Contains(Actor))
    {
        RagdollActors.Add(Actor);
        UE_LOG(LogCorePhysics, Log, TEXT("Registered ragdoll actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::UnregisterRagdollActor(AActor* Actor)
{
    if (Actor)
    {
        RagdollActors.Remove(Actor);
        UE_LOG(LogCorePhysics, Log, TEXT("Unregistered ragdoll actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableRagdollPhysics(AActor* Actor)
{
    if (!Actor || !bEnableRagdollPhysics)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMeshComp->SetSimulatePhysics(true);
        SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        RegisterRagdollActor(Actor);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Enabled ragdoll physics for actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdollPhysics(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMeshComp->SetSimulatePhysics(false);
        SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        UnregisterRagdollActor(Actor);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Disabled ragdoll physics for actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::UpdateRagdollPhysics(float DeltaTime)
{
    // Update ragdoll actors
    for (int32 i = RagdollActors.Num() - 1; i >= 0; i--)
    {
        if (!RagdollActors[i] || !IsValid(RagdollActors[i]))
        {
            RagdollActors.RemoveAt(i);
            continue;
        }
        
        // Additional ragdoll-specific updates can be added here
    }
}

void UCore_PhysicsSystemManager::UpdateDestructiblePhysics(float DeltaTime)
{
    // Update destructible actors
    for (int32 i = DestructibleActors.Num() - 1; i >= 0; i--)
    {
        if (!DestructibleActors[i] || !IsValid(DestructibleActors[i]))
        {
            DestructibleActors.RemoveAt(i);
            continue;
        }
        
        // Additional destructible-specific updates can be added here
    }
}

void UCore_PhysicsSystemManager::SetGlobalGravityScale(float NewGravityScale)
{
    GlobalGravityScale = FMath::Clamp(NewGravityScale, 0.1f, 10.0f);
    
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Global gravity scale set to: %f"), GlobalGravityScale);
}

void UCore_PhysicsSystemManager::SetPhysicsTimeDilation(float NewTimeDilation)
{
    GlobalPhysicsTimeDilation = FMath::Clamp(NewTimeDilation, 0.1f, 2.0f);
    
    UWorld* World = GetWorld();
    if (World && World->GetWorldSettings())
    {
        World->GetWorldSettings()->TimeDilation = GlobalPhysicsTimeDilation;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics time dilation set to: %f"), GlobalPhysicsTimeDilation);
}

int32 UCore_PhysicsSystemManager::GetActiveRigidBodyCount() const
{
    return ActiveRigidBodies.Num();
}

int32 UCore_PhysicsSystemManager::GetActiveRagdollCount() const
{
    return RagdollActors.Num();
}

bool UCore_PhysicsSystemManager::IsPhysicsSystemEnabled() const
{
    return bEnableAdvancedPhysics;
}

void UCore_PhysicsSystemManager::SetPhysicsSystemEnabled(bool bEnabled)
{
    bEnableAdvancedPhysics = bEnabled;
    UE_LOG(LogCorePhysics, Log, TEXT("Physics system enabled: %s"), bEnabled ? TEXT("true") : TEXT("false"));
}