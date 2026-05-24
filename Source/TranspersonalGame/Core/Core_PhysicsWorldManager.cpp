#include "Core_PhysicsWorldManager.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"

UCore_PhysicsWorldManager::UCore_PhysicsWorldManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default physics world settings
    WorldGravityScale = 1.0f;
    GlobalPhysicsTimeStep = 0.016667f; // 60 FPS
    MaxPhysicsSubSteps = 6;
    bEnableAsyncPhysics = true;

    // Default environmental settings
    WindStrength = 0.0f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    AirDensity = 1.225f; // kg/m³ at sea level
    WaterDensity = 1000.0f; // kg/m³

    // Default optimization settings
    PhysicsLODDistance = 5000.0f;
    MaxActiveRigidBodies = 1000;
    bEnablePhysicsCulling = true;

    // Internal state
    bPhysicsWorldInitialized = false;
    LastPhysicsUpdateTime = 0.0f;
}

void UCore_PhysicsWorldManager::BeginPlay()
{
    Super::BeginPlay();
    InitializePhysicsWorld();
}

void UCore_PhysicsWorldManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bPhysicsWorldInitialized)
    {
        LastPhysicsUpdateTime += DeltaTime;

        // Update physics LOD every 0.1 seconds
        if (LastPhysicsUpdateTime >= 0.1f)
        {
            UpdatePhysicsLOD();
            LastPhysicsUpdateTime = 0.0f;
        }

        // Optimize performance if enabled
        if (bEnablePhysicsCulling)
        {
            CullDistantPhysicsBodies();
        }
    }
}

void UCore_PhysicsWorldManager::InitializePhysicsWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: No valid world found"));
        return;
    }

    // Set global physics settings
    UpdatePhysicsSettings();

    // Initialize physics actor tracking
    ActivePhysicsActors.Empty();
    CulledPhysicsActors.Empty();

    // Find all actors with physics components
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                ActivePhysicsActors.Add(Actor);
            }
        }
    }

    bPhysicsWorldInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics world initialized with %d active physics actors"), ActivePhysicsActors.Num());
}

void UCore_PhysicsWorldManager::UpdatePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Update world gravity
    FVector NewGravity = FVector(0.0f, 0.0f, -980.0f * WorldGravityScale);
    World->GetPhysicsScene()->SetGravity(NewGravity);

    // Update physics time step
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->MaxSubstepDeltaTime = GlobalPhysicsTimeStep;
        PhysicsSettings->MaxSubsteps = MaxPhysicsSubSteps;
        PhysicsSettings->bEnableAsyncScene = bEnableAsyncPhysics;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics settings updated - Gravity Scale: %f, Time Step: %f"), 
           WorldGravityScale, GlobalPhysicsTimeStep);
}

void UCore_PhysicsWorldManager::SetGlobalGravity(float NewGravity)
{
    WorldGravityScale = NewGravity;
    UpdatePhysicsSettings();
}

void UCore_PhysicsWorldManager::SetPhysicsTimeStep(float NewTimeStep)
{
    GlobalPhysicsTimeStep = FMath::Clamp(NewTimeStep, 0.008333f, 0.033333f); // 30-120 FPS range
    UpdatePhysicsSettings();
}

void UCore_PhysicsWorldManager::ApplyWindForce(AActor* TargetActor, float ForceMultiplier)
{
    if (!TargetActor || WindStrength <= 0.0f)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        FVector WindForce = WindDirection.GetSafeNormal() * WindStrength * ForceMultiplier * AirDensity;
        PrimComp->AddForce(WindForce, NAME_None, true);
    }
}

void UCore_PhysicsWorldManager::SetEnvironmentalConditions(float Wind, FVector WindDir, float AirDens, float WaterDens)
{
    WindStrength = FMath::Max(0.0f, Wind);
    WindDirection = WindDir.GetSafeNormal();
    AirDensity = FMath::Max(0.1f, AirDens);
    WaterDensity = FMath::Max(100.0f, WaterDens);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Environmental conditions updated - Wind: %f, Air Density: %f"), 
           WindStrength, AirDensity);
}

void UCore_PhysicsWorldManager::OptimizePhysicsPerformance()
{
    // Clean up invalid actor references
    ActivePhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });

    CulledPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });

    // Limit active physics bodies
    if (ActivePhysicsActors.Num() > MaxActiveRigidBodies)
    {
        int32 ExcessBodies = ActivePhysicsActors.Num() - MaxActiveRigidBodies;
        for (int32 i = 0; i < ExcessBodies; i++)
        {
            if (ActivePhysicsActors.IsValidIndex(i))
            {
                AActor* Actor = ActivePhysicsActors[i].Get();
                if (Actor)
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp)
                    {
                        PrimComp->SetSimulatePhysics(false);
                        CulledPhysicsActors.Add(Actor);
                    }
                }
                ActivePhysicsActors.RemoveAt(i);
                i--; // Adjust index after removal
                ExcessBodies--;
            }
        }
    }

    ManagePhysicsMemory();
}

int32 UCore_PhysicsWorldManager::GetActivePhysicsBodiesCount()
{
    return ActivePhysicsActors.Num();
}

void UCore_PhysicsWorldManager::DebugPhysicsWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Core_PhysicsWorldManager Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physics World Initialized: %s"), bPhysicsWorldInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Bodies: %d"), ActivePhysicsActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Culled Physics Bodies: %d"), CulledPhysicsActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Gravity Scale: %f"), WorldGravityScale);
    UE_LOG(LogTemp, Warning, TEXT("Physics Time Step: %f"), GlobalPhysicsTimeStep);
    UE_LOG(LogTemp, Warning, TEXT("Wind Strength: %f"), WindStrength);
    UE_LOG(LogTemp, Warning, TEXT("Air Density: %f"), AirDensity);
    UE_LOG(LogTemp, Warning, TEXT("==========================================="));
}

void UCore_PhysicsWorldManager::UpdatePhysicsLOD()
{
    if (!GetOwner())
    {
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Update LOD for active physics actors
    for (int32 i = ActivePhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (ActivePhysicsActors.IsValidIndex(i))
        {
            AActor* Actor = ActivePhysicsActors[i].Get();
            if (Actor)
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance > PhysicsLODDistance)
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp)
                    {
                        PrimComp->SetSimulatePhysics(false);
                        CulledPhysicsActors.Add(Actor);
                        ActivePhysicsActors.RemoveAt(i);
                    }
                }
            }
            else
            {
                ActivePhysicsActors.RemoveAt(i);
            }
        }
    }

    // Re-enable physics for culled actors that are now close
    for (int32 i = CulledPhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (CulledPhysicsActors.IsValidIndex(i))
        {
            AActor* Actor = CulledPhysicsActors[i].Get();
            if (Actor)
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= PhysicsLODDistance)
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp)
                    {
                        PrimComp->SetSimulatePhysics(true);
                        ActivePhysicsActors.Add(Actor);
                        CulledPhysicsActors.RemoveAt(i);
                    }
                }
            }
            else
            {
                CulledPhysicsActors.RemoveAt(i);
            }
        }
    }
}

void UCore_PhysicsWorldManager::CullDistantPhysicsBodies()
{
    // This is called from UpdatePhysicsLOD, so no additional work needed here
    // Could be extended for more sophisticated culling strategies
}

void UCore_PhysicsWorldManager::ManagePhysicsMemory()
{
    // Force garbage collection if we have too many physics objects
    if (ActivePhysicsActors.Num() + CulledPhysicsActors.Num() > MaxActiveRigidBodies * 2)
    {
        // Clean up invalid references
        ActivePhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
        {
            return !ActorPtr.IsValid();
        });

        CulledPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
        {
            return !ActorPtr.IsValid();
        });

        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics memory cleanup completed"));
    }
}