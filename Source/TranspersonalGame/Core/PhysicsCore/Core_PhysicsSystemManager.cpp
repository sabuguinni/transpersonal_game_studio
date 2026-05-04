#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"

ACore_PhysicsSystemManager::ACore_PhysicsSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default physics settings
    InitializePhysicsSettings();
}

void ACore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: BeginPlay - Initializing physics system"));
    
    // Apply initial physics mode
    SetPhysicsMode(CurrentPhysicsMode);
    
    // Start performance monitoring
    LastPerformanceCheck = GetWorld()->GetTimeSeconds();
}

void ACore_PhysicsSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor physics performance
    MonitorPhysicsPerformance();
}

void ACore_PhysicsSystemManager::InitializePhysicsSettings()
{
    // Realistic physics settings (default for survival game)
    RealisticSettings.GravityScale = 1.0f;
    RealisticSettings.LinearDamping = 0.01f;
    RealisticSettings.AngularDamping = 0.05f;
    RealisticSettings.MaxAngularVelocity = 3600.0f;
    RealisticSettings.bEnableGravity = true;
    RealisticSettings.bSimulatePhysics = true;

    // Arcade physics settings (more responsive)
    ArcadeSettings.GravityScale = 0.8f;
    ArcadeSettings.LinearDamping = 0.1f;
    ArcadeSettings.AngularDamping = 0.2f;
    ArcadeSettings.MaxAngularVelocity = 1800.0f;
    ArcadeSettings.bEnableGravity = true;
    ArcadeSettings.bSimulatePhysics = true;

    // Survival physics settings (heavier, more realistic)
    SurvivalSettings.GravityScale = 1.2f;
    SurvivalSettings.LinearDamping = 0.02f;
    SurvivalSettings.AngularDamping = 0.1f;
    SurvivalSettings.MaxAngularVelocity = 2400.0f;
    SurvivalSettings.bEnableGravity = true;
    SurvivalSettings.bSimulatePhysics = true;

    // Cinematic physics settings (dramatic, slower)
    CinematicSettings.GravityScale = 0.6f;
    CinematicSettings.LinearDamping = 0.05f;
    CinematicSettings.AngularDamping = 0.3f;
    CinematicSettings.MaxAngularVelocity = 1200.0f;
    CinematicSettings.bEnableGravity = true;
    CinematicSettings.bSimulatePhysics = true;
}

void ACore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    CurrentPhysicsMode = NewMode;
    
    FCore_PhysicsSettings SettingsToApply;
    
    switch (CurrentPhysicsMode)
    {
        case ECore_PhysicsMode::Realistic:
            SettingsToApply = RealisticSettings;
            UE_LOG(LogTemp, Warning, TEXT("Physics Mode: REALISTIC"));
            break;
        case ECore_PhysicsMode::Arcade:
            SettingsToApply = ArcadeSettings;
            UE_LOG(LogTemp, Warning, TEXT("Physics Mode: ARCADE"));
            break;
        case ECore_PhysicsMode::Survival:
            SettingsToApply = SurvivalSettings;
            UE_LOG(LogTemp, Warning, TEXT("Physics Mode: SURVIVAL"));
            break;
        case ECore_PhysicsMode::Cinematic:
            SettingsToApply = CinematicSettings;
            UE_LOG(LogTemp, Warning, TEXT("Physics Mode: CINEMATIC"));
            break;
    }
    
    ApplyGlobalPhysicsSettings(SettingsToApply);
}

void ACore_PhysicsSystemManager::ApplyGlobalPhysicsSettings(const FCore_PhysicsSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: No valid world found"));
        return;
    }

    // Apply settings to world physics
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Update global gravity
        World->GetPhysicsScene()->SetGravityZ(-980.0f * Settings.GravityScale);
        UE_LOG(LogTemp, Warning, TEXT("Applied gravity scale: %f"), Settings.GravityScale);
    }

    // Apply settings to all physics objects in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != this) // Don't apply to self
        {
            ApplyPhysicsSettingsToActor(Actor, Settings);
        }
    }
}

void ACore_PhysicsSystemManager::ApplyPhysicsSettingsToActor(AActor* Actor, const FCore_PhysicsSettings& Settings)
{
    if (!Actor)
    {
        return;
    }

    // Apply settings to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            Component->SetLinearDamping(Settings.LinearDamping);
            Component->SetAngularDamping(Settings.AngularDamping);
            Component->SetEnableGravity(Settings.bEnableGravity);
            
            // Set max angular velocity if the component supports it
            if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component))
            {
                MeshComp->SetPhysicsMaxAngularVelocityInRadians(FMath::DegreesToRadians(Settings.MaxAngularVelocity));
            }
        }
    }
}

void ACore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 PhysicsObjectCount = 0;
    
    // Count and potentially optimize physics objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    PhysicsObjectCount++;
                    
                    // If we're over the limit, disable physics on distant objects
                    if (PhysicsObjectCount > MaxPhysicsObjects)
                    {
                        float Distance = FVector::Dist(Actor->GetActorLocation(), GetActorLocation());
                        if (Distance > 5000.0f) // 50 meters
                        {
                            Component->SetSimulatePhysics(false);
                            UE_LOG(LogTemp, Warning, TEXT("Disabled physics on distant object: %s"), *Actor->GetName());
                        }
                    }
                }
            }
        }
    }
    
    ActivePhysicsObjects = PhysicsObjectCount;
    UE_LOG(LogTemp, Warning, TEXT("Physics optimization complete. Active objects: %d"), ActivePhysicsObjects);
}

FCore_PhysicsSettings ACore_PhysicsSystemManager::GetCurrentPhysicsSettings() const
{
    switch (CurrentPhysicsMode)
    {
        case ECore_PhysicsMode::Realistic:
            return RealisticSettings;
        case ECore_PhysicsMode::Arcade:
            return ArcadeSettings;
        case ECore_PhysicsMode::Survival:
            return SurvivalSettings;
        case ECore_PhysicsMode::Cinematic:
            return CinematicSettings;
        default:
            return RealisticSettings;
    }
}

void ACore_PhysicsSystemManager::UpdatePhysicsStatistics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 PhysicsObjectCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    PhysicsObjectCount++;
                }
            }
        }
    }
    
    ActivePhysicsObjects = PhysicsObjectCount;
}

void ACore_PhysicsSystemManager::MonitorPhysicsPerformance()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastPerformanceCheck >= PerformanceCheckInterval)
    {
        // Update statistics
        UpdatePhysicsStatistics();
        
        // Calculate frame time (simplified)
        PhysicsFrameTime = GetWorld()->GetDeltaSeconds();
        
        // Auto-optimize if performance is poor
        if (PhysicsFrameTime > 0.033f && ActivePhysicsObjects > MaxPhysicsObjects * 0.8f)
        {
            OptimizePhysicsPerformance();
        }
        
        LastPerformanceCheck = CurrentTime;
    }
}

void ACore_PhysicsSystemManager::TestPhysicsSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM TEST ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Mode: %d"), (int32)CurrentPhysicsMode);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Objects: %d"), ActivePhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Physics Frame Time: %f ms"), PhysicsFrameTime * 1000.0f);
    
    // Test physics settings application
    SetPhysicsMode(ECore_PhysicsMode::Survival);
    
    UE_LOG(LogTemp, Warning, TEXT("Physics system test complete"));
}

void ACore_PhysicsSystemManager::ResetPhysicsSettings()
{
    InitializePhysicsSettings();
    SetPhysicsMode(ECore_PhysicsMode::Realistic);
    UE_LOG(LogTemp, Warning, TEXT("Physics settings reset to defaults"));
}