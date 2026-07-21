#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default physics settings
    PhysicsSettings = FCore_PhysicsSettings();
    ActivePhysicsObjects = 0;
    PhysicsUpdateTime = 0.0f;
    bPhysicsSystemInitialized = false;
    LastOptimizationTime = 0.0f;
    PhysicsFrameCounter = 0;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystem();
    
    // Set up optimization timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UCore_PhysicsSystemManager::OptimizePhysicsPerformance,
            5.0f, // Every 5 seconds
            true
        );
    }
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsSystemInitialized)
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Process pending registrations/unregistrations
    for (AActor* Actor : PendingRegistration)
    {
        if (IsValid(Actor))
        {
            FCore_PhysicsObjectData NewData;
            NewData.PhysicsActor = Actor;
            NewData.LastUpdateTime = GetWorld()->GetTimeSeconds();
            
            if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
            {
                NewData.Mass = PhysComp->GetMass();
                NewData.bIsSimulating = PhysComp->IsSimulatingPhysics();
                NewData.Velocity = PhysComp->GetPhysicsLinearVelocity();
                NewData.AngularVelocity = PhysComp->GetPhysicsAngularVelocityInRadians();
            }
            
            RegisteredObjects.Add(NewData);
        }
    }
    PendingRegistration.Empty();
    
    for (AActor* Actor : PendingUnregistration)
    {
        RegisteredObjects.RemoveAll([Actor](const FCore_PhysicsObjectData& Data)
        {
            return Data.PhysicsActor == Actor;
        });
    }
    PendingUnregistration.Empty();
    
    // Update physics object data
    UpdatePhysicsObjectData(DeltaTime);
    
    // Clean up invalid objects periodically
    PhysicsFrameCounter++;
    if (PhysicsFrameCounter % 300 == 0) // Every 5 seconds at 60fps
    {
        CleanupInvalidObjects();
    }
    
    PhysicsUpdateTime = FPlatformTime::Seconds() - StartTime;
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bPhysicsSystemInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Initializing physics system"));
    
    // Apply global physics settings
    if (UWorld* World = GetWorld())
    {
        if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
        {
            // Update global physics settings
            Settings->DefaultGravityZ = -980.0f * PhysicsSettings.GlobalGravityScale;
            Settings->bSubstepping = PhysicsSettings.bEnableAsyncPhysics;
            Settings->MaxSubstepDeltaTime = PhysicsSettings.PhysicsTimeStep;
            Settings->MaxSubsteps = static_cast<int32>(PhysicsSettings.MaxPhysicsSubsteps);
        }
    }
    
    ValidatePhysicsSettings();
    bPhysicsSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->DefaultGravityZ = -980.0f * PhysicsSettings.GlobalGravityScale;
        Settings->bSubstepping = PhysicsSettings.bEnableAsyncPhysics;
        Settings->MaxSubstepDeltaTime = PhysicsSettings.PhysicsTimeStep;
        Settings->MaxSubsteps = static_cast<int32>(PhysicsSettings.MaxPhysicsSubsteps);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Physics settings updated"));
}

void UCore_PhysicsSystemManager::RegisterPhysicsObject(AActor* Actor, ECore_PhysicsType PhysicsType)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    // Check if already registered
    bool bAlreadyRegistered = RegisteredObjects.ContainsByPredicate([Actor](const FCore_PhysicsObjectData& Data)
    {
        return Data.PhysicsActor == Actor;
    });
    
    if (bAlreadyRegistered)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Actor %s already registered"), *Actor->GetName());
        return;
    }
    
    PendingRegistration.AddUnique(Actor);
    
    // Configure physics component based on type
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        switch (PhysicsType)
        {
            case ECore_PhysicsType::Static:
                PhysComp->SetMobility(EComponentMobility::Static);
                PhysComp->SetSimulatePhysics(false);
                break;
                
            case ECore_PhysicsType::Kinematic:
                PhysComp->SetMobility(EComponentMobility::Movable);
                PhysComp->SetSimulatePhysics(false);
                break;
                
            case ECore_PhysicsType::Dynamic:
                PhysComp->SetMobility(EComponentMobility::Movable);
                PhysComp->SetSimulatePhysics(true);
                PhysComp->SetLinearDamping(PhysicsSettings.DefaultLinearDamping);
                PhysComp->SetAngularDamping(PhysicsSettings.DefaultAngularDamping);
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Registered actor %s with physics type %d"), 
           *Actor->GetName(), static_cast<int32>(PhysicsType));
}

void UCore_PhysicsSystemManager::UnregisterPhysicsObject(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    PendingUnregistration.AddUnique(Actor);
    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Unregistered actor %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::SetGlobalGravity(float NewGravityScale)
{
    PhysicsSettings.GlobalGravityScale = NewGravityScale;
    
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->DefaultGravityZ = -980.0f * NewGravityScale;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Global gravity set to %f"), NewGravityScale);
}

void UCore_PhysicsSystemManager::EnablePhysicsSimulation(AActor* Actor, bool bEnable)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->SetSimulatePhysics(bEnable);
        
        // Update registered object data
        for (FCore_PhysicsObjectData& Data : RegisteredObjects)
        {
            if (Data.PhysicsActor == Actor)
            {
                Data.bIsSimulating = bEnable;
                break;
            }
        }
    }
}

void UCore_PhysicsSystemManager::ApplyImpulse(AActor* Actor, const FVector& Impulse, const FVector& Location)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        if (Location.IsZero())
        {
            PhysComp->AddImpulse(Impulse);
        }
        else
        {
            PhysComp->AddImpulseAtLocation(Impulse, Location);
        }
    }
}

void UCore_PhysicsSystemManager::ApplyForce(AActor* Actor, const FVector& Force)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->AddForce(Force);
    }
}

void UCore_PhysicsSystemManager::SetObjectMass(AActor* Actor, float NewMass)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->SetMassOverrideInKg(NAME_None, NewMass, true);
        
        // Update registered object data
        for (FCore_PhysicsObjectData& Data : RegisteredObjects)
        {
            if (Data.PhysicsActor == Actor)
            {
                Data.Mass = NewMass;
                break;
            }
        }
    }
}

void UCore_PhysicsSystemManager::SetLinearDamping(AActor* Actor, float Damping)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->SetLinearDamping(Damping);
    }
}

void UCore_PhysicsSystemManager::SetAngularDamping(AActor* Actor, float Damping)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->SetAngularDamping(Damping);
    }
}

void UCore_PhysicsSystemManager::FreezeObject(AActor* Actor, bool bFreezePosition, bool bFreezeRotation)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        if (bFreezePosition)
        {
            PhysComp->SetConstraintMode(EDOFMode::SixDOF);
        }
        
        if (bFreezeRotation)
        {
            PhysComp->SetConstraintMode(EDOFMode::SixDOF);
        }
    }
}

void UCore_PhysicsSystemManager::WakeUpObject(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->WakeAllRigidBodies();
    }
}

void UCore_PhysicsSystemManager::PutObjectToSleep(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        PhysComp->PutAllRigidBodiesToSleep();
    }
}

bool UCore_PhysicsSystemManager::IsObjectSimulating(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        return PhysComp->IsSimulatingPhysics();
    }
    
    return false;
}

FVector UCore_PhysicsSystemManager::GetObjectVelocity(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return FVector::ZeroVector;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        return PhysComp->GetPhysicsLinearVelocity();
    }
    
    return FVector::ZeroVector;
}

float UCore_PhysicsSystemManager::GetObjectMass(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return 0.0f;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        return PhysComp->GetMass();
    }
    
    return 0.0f;
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    return ActivePhysicsObjects;
}

TArray<AActor*> UCore_PhysicsSystemManager::GetPhysicsObjects() const
{
    TArray<AActor*> Result;
    for (const FCore_PhysicsObjectData& Data : RegisteredObjects)
    {
        if (IsValid(Data.PhysicsActor))
        {
            Result.Add(Data.PhysicsActor);
        }
    }
    return Result;
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bPhysicsSystemInitialized)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Skip if optimization was run recently
    if (CurrentTime - LastOptimizationTime < 5.0f)
    {
        return;
    }
    
    int32 SimulatingObjects = 0;
    int32 SleepingObjects = 0;
    
    for (const FCore_PhysicsObjectData& Data : RegisteredObjects)
    {
        if (IsValid(Data.PhysicsActor))
        {
            if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Data.PhysicsActor))
            {
                if (PhysComp->IsSimulatingPhysics())
                {
                    SimulatingObjects++;
                    
                    // Put slow-moving objects to sleep
                    FVector Velocity = PhysComp->GetPhysicsLinearVelocity();
                    if (Velocity.SizeSquared() < 1.0f) // Very slow movement
                    {
                        PhysComp->PutAllRigidBodiesToSleep();
                        SleepingObjects++;
                    }
                }
            }
        }
    }
    
    ActivePhysicsObjects = SimulatingObjects;
    LastOptimizationTime = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Optimization complete - %d simulating, %d sleeping"), 
           SimulatingObjects, SleepingObjects);
}

void UCore_PhysicsSystemManager::SetPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    ApplyPhysicsLOD(Actor, LODLevel);
}

void UCore_PhysicsSystemManager::EnablePhysicsMultithreading(bool bEnable)
{
    PhysicsSettings.bEnableAsyncPhysics = bEnable;
    
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->bSubstepping = bEnable;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System: Physics multithreading %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsObjects()
{
    if (!GetWorld())
    {
        return;
    }
    
    for (const FCore_PhysicsObjectData& Data : RegisteredObjects)
    {
        if (IsValid(Data.PhysicsActor))
        {
            FVector Location = Data.PhysicsActor->GetActorLocation();
            FColor Color = Data.bIsSimulating ? FColor::Green : FColor::Red;
            
            DrawDebugSphere(GetWorld(), Location, 50.0f, 12, Color, false, 1.0f);
            
            // Draw velocity vector
            if (Data.bIsSimulating && !Data.Velocity.IsZero())
            {
                DrawDebugDirectionalArrow(GetWorld(), Location, Location + Data.Velocity, 100.0f, FColor::Blue, false, 1.0f);
            }
        }
    }
}

void UCore_PhysicsSystemManager::LogPhysicsSystemStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Core Physics System Stats ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Objects: %d"), RegisteredObjects.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Objects: %d"), ActivePhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Physics Update Time: %f ms"), PhysicsUpdateTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Global Gravity Scale: %f"), PhysicsSettings.GlobalGravityScale);
    UE_LOG(LogTemp, Warning, TEXT("Physics Initialized: %s"), bPhysicsSystemInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("====================================="));
}

void UCore_PhysicsSystemManager::UpdatePhysicsObjectData(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FCore_PhysicsObjectData& Data : RegisteredObjects)
    {
        if (IsValid(Data.PhysicsActor))
        {
            if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Data.PhysicsActor))
            {
                Data.bIsSimulating = PhysComp->IsSimulatingPhysics();
                Data.Velocity = PhysComp->GetPhysicsLinearVelocity();
                Data.AngularVelocity = PhysComp->GetPhysicsAngularVelocityInRadians();
                Data.Mass = PhysComp->GetMass();
                Data.LastUpdateTime = CurrentTime;
            }
        }
    }
}

void UCore_PhysicsSystemManager::CleanupInvalidObjects()
{
    RegisteredObjects.RemoveAll([](const FCore_PhysicsObjectData& Data)
    {
        return !IsValid(Data.PhysicsActor);
    });
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System: Cleaned up invalid objects, %d remaining"), RegisteredObjects.Num());
}

UPrimitiveComponent* UCore_PhysicsSystemManager::GetPhysicsComponent(AActor* Actor) const
{
    if (!IsValid(Actor))
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
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        return PrimComp;
    }
    
    return nullptr;
}

void UCore_PhysicsSystemManager::ApplyPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    if (UPrimitiveComponent* PhysComp = GetPhysicsComponent(Actor))
    {
        switch (LODLevel)
        {
            case 0: // High detail
                PhysComp->SetSimulatePhysics(true);
                PhysComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
                
            case 1: // Medium detail
                PhysComp->SetSimulatePhysics(true);
                PhysComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                break;
                
            case 2: // Low detail
                PhysComp->SetSimulatePhysics(false);
                PhysComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                break;
                
            case 3: // No physics
                PhysComp->SetSimulatePhysics(false);
                PhysComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                break;
        }
    }
}

void UCore_PhysicsSystemManager::ValidatePhysicsSettings()
{
    // Clamp values to safe ranges
    PhysicsSettings.GlobalGravityScale = FMath::Clamp(PhysicsSettings.GlobalGravityScale, 0.1f, 5.0f);
    PhysicsSettings.DefaultLinearDamping = FMath::Clamp(PhysicsSettings.DefaultLinearDamping, 0.0f, 10.0f);
    PhysicsSettings.DefaultAngularDamping = FMath::Clamp(PhysicsSettings.DefaultAngularDamping, 0.0f, 10.0f);
    PhysicsSettings.MaxPhysicsSubsteps = FMath::Clamp(PhysicsSettings.MaxPhysicsSubsteps, 1.0f, 20.0f);
    PhysicsSettings.PhysicsTimeStep = FMath::Clamp(PhysicsSettings.PhysicsTimeStep, 0.008333f, 0.033333f);
    PhysicsSettings.CollisionTolerance = FMath::Clamp(PhysicsSettings.CollisionTolerance, 0.01f, 1.0f);
    PhysicsSettings.MaxPhysicsObjects = FMath::Clamp(PhysicsSettings.MaxPhysicsObjects, 100, 50000);
}