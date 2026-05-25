#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS tick rate
    
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    DefaultLinearDamping = 0.1f;
    DefaultAngularDamping = 0.1f;
    bEnablePhysicsSimulation = true;
    
    // Collision settings
    bEnableComplexCollision = true;
    CollisionTolerance = 0.1f;
    MaxCollisionIterations = 8;
    
    // Ragdoll settings
    bEnableRagdollPhysics = true;
    RagdollBlendWeight = 1.0f;
    RagdollLifetime = 30.0f;
    
    // Destruction settings
    bEnableDestruction = true;
    DestructionThreshold = 1000.0f;
    MaxDestructionParticles = 100;
    
    // Internal state
    LastPhysicsUpdateTime = 0.0f;
    PhysicsObjectCount = 0;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics System Manager initialized"));
    
    InitializePhysicsSystem();
    
    // Bind to physics events
    if (AActor* Owner = GetOwner())
    {
        Owner->OnActorHit.AddDynamic(this, &UCore_PhysicsSystemManager::OnActorHit);
        Owner->OnActorBeginOverlap.AddDynamic(this, &UCore_PhysicsSystemManager::OnActorBeginOverlap);
        Owner->OnActorEndOverlap.AddDynamic(this, &UCore_PhysicsSystemManager::OnActorEndOverlap);
    }
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsSimulation)
    {
        return;
    }
    
    // Update tracked actors
    UpdateTrackedActors();
    
    // Process ragdoll characters
    ProcessRagdollCharacters(DeltaTime);
    
    // Monitor performance
    MonitorPerformance();
    
    // Cleanup destroyed actors
    CleanupDestroyedActors();
    
    LastPhysicsUpdateTime = GetWorld()->GetTimeSeconds();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Initializing physics system with gravity scale: %f"), GlobalGravityScale);
    
    if (UWorld* World = GetWorld())
    {
        // Set global physics settings
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
            PhysicsSettings->DefaultTerminalVelocity = 4000.0f;
            PhysicsSettings->DefaultFluidFriction = 0.3f;
        }
        
        // Initialize physics scene
        World->GetPhysicsScene()->SetGravityZ(-980.0f * GlobalGravityScale);
    }
    
    TrackedPhysicsActors.Empty();
    RagdollCharacters.Empty();
    PhysicsObjectCount = 0;
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetGravityZ(-980.0f * GlobalGravityScale);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Updated physics settings - Gravity: %f"), GlobalGravityScale);
    }
}

void UCore_PhysicsSystemManager::SetGlobalGravity(float NewGravity)
{
    GlobalGravityScale = NewGravity;
    UpdatePhysicsSettings();
}

void UCore_PhysicsSystemManager::EnablePhysicsSimulation(bool bEnable)
{
    bEnablePhysicsSimulation = bEnable;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics simulation %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UCore_PhysicsSystemManager::CheckCollisionAtLocation(FVector Location, float Radius)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bEnableComplexCollision;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->OverlapBlockingTestByChannel(
        Location,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
}

TArray<AActor*> UCore_PhysicsSystemManager::GetOverlappingActors(FVector Location, float Radius)
{
    TArray<AActor*> OverlappingActors;
    
    if (!GetWorld())
    {
        return OverlappingActors;
    }
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = bEnableComplexCollision;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Location,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor())
            {
                OverlappingActors.Add(Result.GetActor());
            }
        }
    }
    
    return OverlappingActors;
}

void UCore_PhysicsSystemManager::SetCollisionResponseForActor(AActor* Actor, ECollisionResponse Response)
{
    if (!Actor)
    {
        return;
    }
    
    if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        StaticMesh->SetCollisionResponseToAllChannels(Response);
    }
    
    if (USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetCollisionResponseToAllChannels(Response);
    }
}

void UCore_PhysicsSystemManager::EnableRagdollForCharacter(ACharacter* Character)
{
    if (!Character || !bEnableRagdollPhysics)
    {
        return;
    }
    
    if (USkeletalMeshComponent* Mesh = Character->GetMesh())
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        RagdollCharacters.AddUnique(Character);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Enabled ragdoll for character: %s"), 
               *Character->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    if (USkeletalMeshComponent* Mesh = Character->GetMesh())
    {
        Mesh->SetSimulatePhysics(false);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        RagdollCharacters.Remove(Character);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Disabled ragdoll for character: %s"), 
               *Character->GetName());
    }
}

void UCore_PhysicsSystemManager::SetRagdollBlendWeight(ACharacter* Character, float Weight)
{
    if (!Character)
    {
        return;
    }
    
    RagdollBlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    
    if (USkeletalMeshComponent* Mesh = Character->GetMesh())
    {
        // Apply blend weight to ragdoll simulation
        Mesh->SetAllBodiesPhysicsBlendWeight(RagdollBlendWeight);
    }
}

void UCore_PhysicsSystemManager::DestroyActor(AActor* Actor, FVector ImpactLocation, float Force)
{
    if (!Actor || !bEnableDestruction)
    {
        return;
    }
    
    // Create destruction effect
    CreateDestructionEffect(ImpactLocation, Force / DestructionThreshold);
    
    // Apply impulse before destruction
    if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        FVector ImpulseDirection = (Actor->GetActorLocation() - ImpactLocation).GetSafeNormal();
        StaticMesh->AddImpulseAtLocation(ImpulseDirection * Force, ImpactLocation);
    }
    
    // Schedule destruction
    Actor->SetLifeSpan(0.1f);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Destroyed actor: %s with force: %f"), 
           *Actor->GetName(), Force);
}

void UCore_PhysicsSystemManager::CreateDestructionEffect(FVector Location, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create particle effect for destruction
    // Note: This would typically spawn a Niagara system
    UE_LOG(LogCorePhysics, Log, TEXT("Created destruction effect at location: %s with intensity: %f"), 
           *Location.ToString(), Intensity);
    
    // Draw debug sphere for visualization
    DrawDebugSphere(GetWorld(), Location, 100.0f * Intensity, 12, FColor::Orange, false, 2.0f);
}

bool UCore_PhysicsSystemManager::CanActorBeDestroyed(AActor* Actor)
{
    if (!Actor || !bEnableDestruction)
    {
        return false;
    }
    
    // Check if actor has destructible components
    return Actor->FindComponentByClass<UStaticMeshComponent>() != nullptr;
}

float UCore_PhysicsSystemManager::GetPhysicsFrameTime() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetDeltaSeconds();
    }
    return 0.0f;
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsObjects() const
{
    return PhysicsObjectCount;
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    // Remove invalid actors from tracking
    TrackedPhysicsActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    // Limit ragdoll characters
    if (RagdollCharacters.Num() > 10)
    {
        // Remove oldest ragdoll characters
        for (int32 i = 0; i < RagdollCharacters.Num() - 10; i++)
        {
            if (ACharacter* Character = RagdollCharacters[i])
            {
                DisableRagdollForCharacter(Character);
            }
        }
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Optimized physics performance - Active objects: %d"), 
           PhysicsObjectCount);
}

void UCore_PhysicsSystemManager::UpdateTrackedActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count active physics objects
    PhysicsObjectCount = 0;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (IsValid(Actor))
        {
            if (UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                if (StaticMesh->IsSimulatingPhysics())
                {
                    PhysicsObjectCount++;
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::ProcessRagdollCharacters(float DeltaTime)
{
    for (int32 i = RagdollCharacters.Num() - 1; i >= 0; i--)
    {
        ACharacter* Character = RagdollCharacters[i];
        if (!IsValid(Character))
        {
            RagdollCharacters.RemoveAt(i);
            continue;
        }
        
        // Check ragdoll lifetime
        float TimeSinceRagdoll = GetWorld()->GetTimeSeconds() - LastPhysicsUpdateTime;
        if (TimeSinceRagdoll > RagdollLifetime)
        {
            DisableRagdollForCharacter(Character);
        }
    }
}

void UCore_PhysicsSystemManager::MonitorPerformance()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float FrameTime = CurrentTime - LastPhysicsUpdateTime;
    
    // If frame time is too high, optimize
    if (FrameTime > 0.033f) // 30 FPS threshold
    {
        OptimizePhysicsPerformance();
    }
}

void UCore_PhysicsSystemManager::CleanupDestroyedActors()
{
    TrackedPhysicsActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor) || Actor->IsPendingKill();
    });
    
    RagdollCharacters.RemoveAll([](ACharacter* Character) {
        return !IsValid(Character) || Character->IsPendingKill();
    });
}

void UCore_PhysicsSystemManager::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }
    
    float ImpactForce = NormalImpulse.Size();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Actor hit: %s hit %s with force: %f"), 
           *SelfActor->GetName(), *OtherActor->GetName(), ImpactForce);
    
    // Check for destruction
    if (ImpactForce > DestructionThreshold && CanActorBeDestroyed(OtherActor))
    {
        DestroyActor(OtherActor, Hit.Location, ImpactForce);
    }
}

void UCore_PhysicsSystemManager::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OverlappedActor || !OtherActor)
    {
        return;
    }
    
    // Track physics actors
    if (UStaticMeshComponent* StaticMesh = OtherActor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (StaticMesh->IsSimulatingPhysics())
        {
            TrackedPhysicsActors.AddUnique(OtherActor);
        }
    }
}

void UCore_PhysicsSystemManager::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OverlappedActor || !OtherActor)
    {
        return;
    }
    
    // Remove from tracking
    TrackedPhysicsActors.Remove(OtherActor);
}