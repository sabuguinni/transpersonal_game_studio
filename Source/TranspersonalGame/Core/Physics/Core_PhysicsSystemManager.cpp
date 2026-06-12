#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/WorldSettings.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    CurrentPhysicsMode = ECore_PhysicsMode::Standard;
    bPhysicsProfilerEnabled = false;
    PhysicsFrameTime = 0.0f;
    ActiveRigidBodies = 0;
    ProfilerUpdateInterval = 1.0f;
    ProfilerTimer = 0.0f;
    CachedWorld = nullptr;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    if (CachedWorld)
    {
        ApplyPhysicsModeSettings();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initialized with mode %d"), (int32)CurrentPhysicsMode);
    }
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bPhysicsProfilerEnabled)
    {
        ProfilerTimer += DeltaTime;
        if (ProfilerTimer >= ProfilerUpdateInterval)
        {
            UpdatePhysicsProfiler();
            ProfilerTimer = 0.0f;
        }
    }
}

void UCore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    if (CurrentPhysicsMode != NewMode)
    {
        CurrentPhysicsMode = NewMode;
        ApplyPhysicsModeSettings();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics mode changed to %d"), (int32)NewMode);
    }
}

void UCore_PhysicsSystemManager::ApplyPhysicsProfile(const FCore_PhysicsProfile& Profile)
{
    CurrentProfile = Profile;
    
    if (CachedWorld && CachedWorld->GetWorldSettings())
    {
        AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
        WorldSettings->GlobalGravityZ = -980.0f * Profile.GravityScale;
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied physics profile - Gravity Scale: %f"), Profile.GravityScale);
    }
}

void UCore_PhysicsSystemManager::EnablePhysicsProfiler(bool bEnable)
{
    bPhysicsProfilerEnabled = bEnable;
    if (bEnable)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics profiler enabled"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics profiler disabled"));
    }
}

void UCore_PhysicsSystemManager::RegisterCollisionProfile(const FName& ProfileName, const FCollisionResponseContainer& Responses)
{
    CustomCollisionProfiles.Add(ProfileName, Responses);
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Registered collision profile: %s"), *ProfileName.ToString());
}

void UCore_PhysicsSystemManager::OptimizeCollisionForPerformance()
{
    if (!CachedWorld) return;
    
    // Count and optimize collision components
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(CachedWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsCollisionEnabled())
                {
                    // Apply performance optimizations
                    if (PrimComp->GetCollisionObjectType() == ECC_WorldStatic)
                    {
                        PrimComp->SetNotifyRigidBodyCollision(false);
                        OptimizedCount++;
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Optimized %d collision components"), OptimizedCount);
}

void UCore_PhysicsSystemManager::SetWorldGravity(const FVector& NewGravity)
{
    if (CachedWorld && CachedWorld->GetWorldSettings())
    {
        AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
        WorldSettings->GlobalGravityZ = NewGravity.Z;
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: World gravity set to Z=%f"), NewGravity.Z);
    }
}

FVector UCore_PhysicsSystemManager::GetWorldGravity() const
{
    if (CachedWorld && CachedWorld->GetWorldSettings())
    {
        AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
        return FVector(0.0f, 0.0f, WorldSettings->GlobalGravityZ);
    }
    return FVector(0.0f, 0.0f, -980.0f);
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsStats()
{
    if (!CachedWorld) return;
    
    FString StatsText = FString::Printf(TEXT("Physics Stats:\nMode: %d\nFrame Time: %.2fms\nActive Bodies: %d\nGravity: %.1f"), 
        (int32)CurrentPhysicsMode, PhysicsFrameTime, ActiveRigidBodies, GetWorldGravity().Z);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, StatsText);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: %s"), *StatsText);
}

void UCore_PhysicsSystemManager::TogglePhysicsVisualization(bool bShowCollision, bool bShowConstraints, bool bShowMass)
{
    if (CachedWorld)
    {
        if (bShowCollision)
        {
            CachedWorld->Exec(CachedWorld, TEXT("showflag.collision 1"));
        }
        else
        {
            CachedWorld->Exec(CachedWorld, TEXT("showflag.collision 0"));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics visualization toggled - Collision: %s"), 
            bShowCollision ? TEXT("ON") : TEXT("OFF"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsProfiler()
{
    if (!CachedWorld) return;
    
    // Update frame time (simplified - in real implementation would use actual physics timing)
    PhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active rigid bodies
    CountActiveRigidBodies();
}

void UCore_PhysicsSystemManager::ApplyPhysicsModeSettings()
{
    if (!CachedWorld) return;
    
    switch (CurrentPhysicsMode)
    {
        case ECore_PhysicsMode::Standard:
            CurrentProfile.SolverIterations = 8;
            CurrentProfile.bEnableCCD = false;
            break;
            
        case ECore_PhysicsMode::HighPrecision:
            CurrentProfile.SolverIterations = 16;
            CurrentProfile.bEnableCCD = true;
            break;
            
        case ECore_PhysicsMode::Performance:
            CurrentProfile.SolverIterations = 4;
            CurrentProfile.bEnableCCD = false;
            break;
            
        case ECore_PhysicsMode::Cinematic:
            CurrentProfile.SolverIterations = 32;
            CurrentProfile.bEnableCCD = true;
            break;
    }
    
    ApplyPhysicsProfile(CurrentProfile);
}

void UCore_PhysicsSystemManager::CountActiveRigidBodies()
{
    if (!CachedWorld) return;
    
    ActiveRigidBodies = 0;
    
    for (TActorIterator<AActor> ActorItr(CachedWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    ActiveRigidBodies++;
                }
            }
        }
    }
}