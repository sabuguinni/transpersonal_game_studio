#include "Core_PhysicsIntegrator.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsIntegrator::UCore_PhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    PhysicsTimeScale = 1.0f;
    PhysicsSubsteps = 60;
    bEnableHighQualityPhysics = true;
    bEnablePhysicsLOD = true;
    
    CurrentPhysicsFrameTime = 0.0f;
    ActivePhysicsBodies = 0;
    ActiveConstraints = 0;
    
    bSystemsInitialized = false;
    LastPerformanceUpdate = 0.0f;
    
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
}

void UCore_PhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystems();
    UpdatePhysicsQuality();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Initialized physics integration system"));
}

void UCore_PhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance monitoring
    MonitorPerformance();
    
    // Update physics quality based on performance
    if (GetWorld()->GetTimeSeconds() - LastPerformanceUpdate > 1.0f)
    {
        UpdatePhysicsPerformance();
        LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
    }
    
    // Apply physics LOD if enabled
    if (bEnablePhysicsLOD)
    {
        OptimizePhysicsSettings();
    }
}

void UCore_PhysicsIntegrator::InitializePhysicsSystems()
{
    if (bSystemsInitialized)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegrator: No world found for initialization"));
        return;
    }
    
    // Find or create physics manager components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        PhysicsManager = Owner->FindComponentByClass<UCore_PhysicsManager>();
        if (!PhysicsManager)
        {
            PhysicsManager = NewObject<UCore_PhysicsManager>(Owner);
            Owner->AddInstanceComponent(PhysicsManager);
        }
        
        CollisionSystem = Owner->FindComponentByClass<UCore_CollisionSystem>();
        if (!CollisionSystem)
        {
            CollisionSystem = NewObject<UCore_CollisionSystem>(Owner);
            Owner->AddInstanceComponent(CollisionSystem);
        }
        
        RagdollSystem = Owner->FindComponentByClass<UCore_RagdollSystem>();
        if (!RagdollSystem)
        {
            RagdollSystem = NewObject<UCore_RagdollSystem>(Owner);
            Owner->AddInstanceComponent(RagdollSystem);
        }
        
        DestructionSystem = Owner->FindComponentByClass<UCore_DestructionSystem>();
        if (!DestructionSystem)
        {
            DestructionSystem = NewObject<UCore_DestructionSystem>(Owner);
            Owner->AddInstanceComponent(DestructionSystem);
        }
    }
    
    bSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Physics systems initialized"));
}

void UCore_PhysicsIntegrator::SetPhysicsQuality(float Quality)
{
    Quality = FMath::Clamp(Quality, 0.1f, 2.0f);
    
    PhysicsTimeScale = Quality;
    PhysicsSubsteps = FMath::RoundToInt(60.0f * Quality);
    bEnableHighQualityPhysics = Quality > 0.8f;
    
    UpdatePhysicsQuality();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Physics quality set to %f"), Quality);
}

void UCore_PhysicsIntegrator::EnablePhysicsLOD(bool bEnable)
{
    bEnablePhysicsLOD = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Physics LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsIntegrator::UpdatePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Count active physics bodies and constraints
    ActivePhysicsBodies = 0;
    ActiveConstraints = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->GetBodyInstance())
            {
                if (PrimComp->GetBodyInstance()->IsInstanceSimulatingPhysics())
                {
                    ActivePhysicsBodies++;
                }
            }
        }
    }
    
    // Adjust physics quality based on performance
    if (CurrentPhysicsFrameTime > MAX_PHYSICS_FRAME_TIME)
    {
        if (PhysicsTimeScale > 0.5f)
        {
            PhysicsTimeScale *= 0.95f;
            UpdatePhysicsQuality();
        }
    }
    else if (CurrentPhysicsFrameTime < MAX_PHYSICS_FRAME_TIME * 0.5f)
    {
        if (PhysicsTimeScale < 1.0f)
        {
            PhysicsTimeScale *= 1.05f;
            PhysicsTimeScale = FMath::Min(PhysicsTimeScale, 1.0f);
            UpdatePhysicsQuality();
        }
    }
}

void UCore_PhysicsIntegrator::IntegrateCharacterPhysics(ACharacter* Character)
{
    if (!Character)
        return;
        
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
        return;
        
    // Apply physics-based movement modifications
    if (bEnableHighQualityPhysics)
    {
        MovementComp->bUseSeparateBrakingFriction = true;
        MovementComp->BrakingFriction = 2.0f;
        MovementComp->BrakingDecelerationWalking = 2048.0f;
    }
    else
    {
        MovementComp->bUseSeparateBrakingFriction = false;
        MovementComp->BrakingDecelerationWalking = 1024.0f;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Integrated physics for character %s"), *Character->GetName());
}

void UCore_PhysicsIntegrator::ApplyPhysicsImpulse(AActor* Actor, FVector Impulse, FVector Location)
{
    if (!Actor)
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->GetBodyInstance())
    {
        PrimComp->AddImpulseAtLocation(Impulse, Location);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Applied impulse to %s"), *Actor->GetName());
    }
}

void UCore_PhysicsIntegrator::EnableRagdoll(ACharacter* Character)
{
    if (!Character || !RagdollSystem)
        return;
        
    // Delegate to ragdoll system
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Enabling ragdoll for %s"), *Character->GetName());
    
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (Mesh)
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UCore_PhysicsIntegrator::DisableRagdoll(ACharacter* Character)
{
    if (!Character || !RagdollSystem)
        return;
        
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Disabling ragdoll for %s"), *Character->GetName());
    
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (Mesh)
    {
        Mesh->SetSimulatePhysics(false);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void UCore_PhysicsIntegrator::TriggerDestruction(AActor* Actor, FVector ImpactLocation, float Force)
{
    if (!Actor || !DestructionSystem)
        return;
        
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Triggering destruction for %s with force %f"), *Actor->GetName(), Force);
    
    // Create debris at impact location
    CreateDebris(ImpactLocation, FMath::RoundToInt(Force / 100.0f));
    
    // Apply destruction effects
    ApplyPhysicsImpulse(Actor, FVector(0, 0, Force), ImpactLocation);
}

void UCore_PhysicsIntegrator::CreateDebris(FVector Location, int32 DebrisCount)
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    DebrisCount = FMath::Clamp(DebrisCount, 1, 20);
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 200.0f)
        );
        
        FVector DebrisLocation = Location + RandomOffset;
        
        // In a real implementation, we would spawn debris actors here
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Created debris at %s"), *DebrisLocation.ToString());
    }
}

void UCore_PhysicsIntegrator::RegisterCollisionActor(AActor* Actor)
{
    if (!Actor || RegisteredActors.Contains(Actor))
        return;
        
    RegisteredActors.Add(Actor);
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Registered collision actor %s"), *Actor->GetName());
}

void UCore_PhysicsIntegrator::UnregisterCollisionActor(AActor* Actor)
{
    if (!Actor)
        return;
        
    RegisteredActors.Remove(Actor);
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Unregistered collision actor %s"), *Actor->GetName());
}

bool UCore_PhysicsIntegrator::CheckCollisionAtLocation(FVector Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
        
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;
    
    return World->OverlapAnyTestByChannel(Location, FQuat::Identity, ECC_WorldStatic, 
        FCollisionShape::MakeSphere(Radius), QueryParams);
}

void UCore_PhysicsIntegrator::ApplyFluidForces(AActor* Actor, FVector FluidVelocity, float Density)
{
    if (!Actor)
        return;
        
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->GetBodyInstance())
    {
        FVector FluidForce = FluidVelocity * Density * PrimComp->GetMass();
        PrimComp->AddForce(FluidForce);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Applied fluid force to %s"), *Actor->GetName());
    }
}

void UCore_PhysicsIntegrator::CreateWaterSplash(FVector Location, float Intensity)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Created water splash at %s with intensity %f"), 
        *Location.ToString(), Intensity);
    
    // In a real implementation, we would spawn particle effects here
}

void UCore_PhysicsIntegrator::ApplyVehiclePhysics(AActor* Vehicle, float Throttle, float Steering)
{
    if (!Vehicle)
        return;
        
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Applied vehicle physics to %s (Throttle: %f, Steering: %f)"), 
        *Vehicle->GetName(), Throttle, Steering);
    
    // Vehicle physics would be implemented here
}

void UCore_PhysicsIntegrator::UpdateVehicleSuspension(AActor* Vehicle)
{
    if (!Vehicle)
        return;
        
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Updated vehicle suspension for %s"), *Vehicle->GetName());
}

void UCore_PhysicsIntegrator::UpdateTerrainPhysics(FVector Location, float Radius)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsIntegrator: Updated terrain physics at %s (Radius: %f)"), 
        *Location.ToString(), Radius);
}

float UCore_PhysicsIntegrator::GetTerrainStability(FVector Location)
{
    // Simple terrain stability calculation
    UWorld* World = GetWorld();
    if (!World)
        return 1.0f;
        
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
    {
        FVector Normal = HitResult.Normal;
        float Stability = FMath::Abs(FVector::DotProduct(Normal, FVector::UpVector));
        return Stability;
    }
    
    return 0.5f; // Default stability
}

void UCore_PhysicsIntegrator::DebugDrawPhysicsInfo()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    FVector PlayerLocation = FVector::ZeroVector;
    if (World->GetFirstPlayerController() && World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = World->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    }
    
    // Draw physics info near player
    DrawDebugString(World, PlayerLocation + FVector(0, 0, 200), 
        FString::Printf(TEXT("Physics Bodies: %d"), ActivePhysicsBodies), 
        nullptr, FColor::Green, 0.0f);
        
    DrawDebugString(World, PlayerLocation + FVector(0, 0, 180), 
        FString::Printf(TEXT("Frame Time: %.2fms"), CurrentPhysicsFrameTime), 
        nullptr, FColor::Yellow, 0.0f);
        
    DrawDebugString(World, PlayerLocation + FVector(0, 0, 160), 
        FString::Printf(TEXT("Quality: %.2f"), PhysicsTimeScale), 
        nullptr, FColor::Blue, 0.0f);
}

void UCore_PhysicsIntegrator::TogglePhysicsDebugDraw()
{
    // Toggle physics debug visualization
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrator: Toggled physics debug draw"));
}

void UCore_PhysicsIntegrator::LogPhysicsStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS INTEGRATOR STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Physics Bodies: %d"), ActivePhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("Active Constraints: %d"), ActiveConstraints);
    UE_LOG(LogTemp, Log, TEXT("Current Frame Time: %.2fms"), CurrentPhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Physics Time Scale: %.2f"), PhysicsTimeScale);
    UE_LOG(LogTemp, Log, TEXT("Physics Substeps: %d"), PhysicsSubsteps);
    UE_LOG(LogTemp, Log, TEXT("High Quality Physics: %s"), bEnableHighQualityPhysics ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Physics LOD: %s"), bEnablePhysicsLOD ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Registered Actors: %d"), RegisteredActors.Num());
}

void UCore_PhysicsIntegrator::UpdatePhysicsQuality()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Apply physics settings to world
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Update physics settings based on quality
        PhysicsSettings->DefaultGravityZ = -980.0f * PhysicsTimeScale;
    }
}

void UCore_PhysicsIntegrator::MonitorPerformance()
{
    // Simple frame time monitoring
    CurrentPhysicsFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f;
}

void UCore_PhysicsIntegrator::OptimizePhysicsSettings()
{
    if (!bEnablePhysicsLOD)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (World->GetFirstPlayerController() && World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = World->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    }
    
    // Apply LOD to registered actors
    for (AActor* Actor : RegisteredActors)
    {
        if (Actor && ShouldUsePhysicsLOD(Actor))
        {
            float Distance = CalculateDistanceToPlayer(Actor);
            
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp)
            {
                if (Distance > 5000.0f)
                {
                    // Disable physics for distant objects
                    PrimComp->SetSimulatePhysics(false);
                }
                else if (Distance > 2000.0f)
                {
                    // Reduce physics quality for medium distance objects
                    PrimComp->SetSimulatePhysics(true);
                    // Could reduce collision complexity here
                }
                else
                {
                    // Full physics for close objects
                    PrimComp->SetSimulatePhysics(true);
                }
            }
        }
    }
}

bool UCore_PhysicsIntegrator::ShouldUsePhysicsLOD(AActor* Actor)
{
    if (!Actor)
        return false;
        
    // Don't apply LOD to characters or important objects
    if (Cast<ACharacter>(Actor))
        return false;
        
    return true;
}

float UCore_PhysicsIntegrator::CalculateDistanceToPlayer(AActor* Actor)
{
    if (!Actor)
        return 0.0f;
        
    UWorld* World = GetWorld();
    if (!World || !World->GetFirstPlayerController() || !World->GetFirstPlayerController()->GetPawn())
        return 0.0f;
        
    FVector PlayerLocation = World->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    FVector ActorLocation = Actor->GetActorLocation();
    
    return FVector::Dist(PlayerLocation, ActorLocation);
}