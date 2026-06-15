#include "Core_DestructionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

ACore_DestructionManager::ACore_DestructionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default destruction parameters
    MinDestructionForce = 500.0f;
    MaxDebrisCount = 8;
    DebrisLifetime = 30.0f;
    DebrisScale = 0.3f;
    
    // Initialize world time tracking
    CurrentWorldTime = 0.0f;
}

void ACore_DestructionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionManager: System initialized"));
    
    // Find and register existing destructible objects in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
    
    int32 RegisteredCount = 0;
    for (AActor* Actor : FoundActors)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Boulder")) || ActorName.Contains(TEXT("Log")) || 
            ActorName.Contains(TEXT("Destructible")) || ActorName.Contains(TEXT("Breakable")))
        {
            RegisterDestructible(Actor, MinDestructionForce);
            RegisteredCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionManager: Registered %d destructible objects"), RegisteredCount);
}

void ACore_DestructionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update world time for debris cleanup
    CurrentWorldTime += DeltaTime;
    
    // Clean up old debris every 5 seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f)
    {
        CleanupOldDebris();
        CleanupTimer = 0.0f;
    }
}

void ACore_DestructionManager::RegisterDestructible(AActor* DestructibleActor, float DestructionThreshold)
{
    if (!DestructibleActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_DestructionManager: Cannot register null actor"));
        return;
    }
    
    // Add to registered destructibles
    DestructibleObjects.Add(DestructibleActor, DestructionThreshold);
    
    // Bind collision events
    BindDestructibleEvents(DestructibleActor);
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionManager: Registered %s with threshold %.1f"), 
           *DestructibleActor->GetName(), DestructionThreshold);
}

void ACore_DestructionManager::TriggerDestruction(AActor* TargetActor, FVector ImpactLocation, float ImpactForce)
{
    if (!TargetActor || !CanDestroy(TargetActor, ImpactForce))
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionManager: Destroying %s with force %.1f"), 
           *TargetActor->GetName(), ImpactForce);
    
    // Calculate debris count based on impact force
    int32 DebrisCount = FMath::Clamp(
        FMath::RoundToInt(ImpactForce / 200.0f), 
        3, 
        MaxDebrisCount
    );
    
    // Spawn debris before destroying original
    SpawnDebris(TargetActor, ImpactLocation, DebrisCount);
    
    // Trigger blueprint event
    OnObjectDestroyed(TargetActor, ImpactLocation);
    
    // Remove from tracking and destroy
    DestructibleObjects.Remove(TargetActor);
    TargetActor->Destroy();
}

void ACore_DestructionManager::SpawnDebris(AActor* OriginalActor, FVector ImpactLocation, int32 DebrisCount)
{
    if (!OriginalActor)
    {
        return;
    }
    
    TArray<AActor*> NewDebris;
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Calculate random spawn location around impact point
        FVector RandomOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 50.0f)
        );
        
        FVector SpawnLocation = ImpactLocation + RandomOffset;
        
        // Calculate random velocity away from impact
        FVector Velocity = (SpawnLocation - ImpactLocation).GetSafeNormal() * FMath::RandRange(200.0f, 600.0f);
        Velocity.Z += FMath::RandRange(100.0f, 300.0f); // Add upward component
        
        // Create debris piece
        AActor* DebrisPiece = CreateDebrisPiece(OriginalActor, SpawnLocation, Velocity);
        if (DebrisPiece)
        {
            NewDebris.Add(DebrisPiece);
            ActiveDebris.Add(DebrisPiece);
            DebrisSpawnTimes.Add(CurrentWorldTime);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionManager: Spawned %d debris pieces"), NewDebris.Num());
    
    // Trigger blueprint event
    OnDebrisSpawned(NewDebris);
}

void ACore_DestructionManager::CleanupOldDebris()
{
    int32 CleanedCount = 0;
    
    for (int32 i = ActiveDebris.Num() - 1; i >= 0; i--)
    {
        if (i >= DebrisSpawnTimes.Num())
        {
            continue;
        }
        
        float DebrisAge = CurrentWorldTime - DebrisSpawnTimes[i];
        if (DebrisAge >= DebrisLifetime)
        {
            // Clean up old debris
            if (ActiveDebris[i] && IsValid(ActiveDebris[i]))
            {
                ActiveDebris[i]->Destroy();
                CleanedCount++;
            }
            
            // Remove from tracking arrays
            ActiveDebris.RemoveAt(i);
            DebrisSpawnTimes.RemoveAt(i);
        }
    }
    
    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_DestructionManager: Cleaned up %d old debris pieces"), CleanedCount);
    }
}

bool ACore_DestructionManager::CanDestroy(AActor* TargetActor, float ImpactForce) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    const float* Threshold = DestructibleObjects.Find(TargetActor);
    if (!Threshold)
    {
        return false; // Not registered as destructible
    }
    
    return ImpactForce >= *Threshold;
}

void ACore_DestructionManager::OnDestructibleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                                                UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                                                const FHitResult& Hit)
{
    if (!HitComp || !OtherActor)
    {
        return;
    }
    
    AActor* HitActor = HitComp->GetOwner();
    if (!HitActor || !DestructibleObjects.Contains(HitActor))
    {
        return;
    }
    
    // Calculate impact force
    float ObjectMass = 100.0f; // Default mass
    if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HitComp))
    {
        ObjectMass = MeshComp->GetMass();
    }
    
    float ImpactForce = CalculateImpactForce(NormalImpulse, ObjectMass);
    
    // Trigger destruction if force is sufficient
    if (CanDestroy(HitActor, ImpactForce))
    {
        TriggerDestruction(HitActor, Hit.Location, ImpactForce);
    }
}

float ACore_DestructionManager::CalculateImpactForce(const FVector& NormalImpulse, float ObjectMass) const
{
    // Convert impulse to force estimate
    float ImpulseMagnitude = NormalImpulse.Size();
    float EstimatedForce = ImpulseMagnitude * 10.0f; // Scale factor for gameplay
    
    // Factor in object mass
    EstimatedForce *= FMath::Sqrt(ObjectMass / 100.0f);
    
    return EstimatedForce;
}

AActor* ACore_DestructionManager::CreateDebrisPiece(AActor* OriginalActor, FVector SpawnLocation, FVector Velocity)
{
    if (!OriginalActor)
    {
        return nullptr;
    }
    
    // Spawn new static mesh actor for debris
    AStaticMeshActor* DebrisActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        SpawnLocation,
        FRotator::ZeroRotator
    );
    
    if (!DebrisActor)
    {
        return nullptr;
    }
    
    // Copy mesh from original (simplified)
    if (AStaticMeshActor* OriginalMeshActor = Cast<AStaticMeshActor>(OriginalActor))
    {
        UStaticMeshComponent* OriginalMesh = OriginalMeshActor->GetStaticMeshComponent();
        UStaticMeshComponent* DebrisMesh = DebrisActor->GetStaticMeshComponent();
        
        if (OriginalMesh && DebrisMesh)
        {
            DebrisMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
            
            // Scale down debris
            DebrisActor->SetActorScale3D(OriginalActor->GetActorScale3D() * DebrisScale);
            
            // Enable physics and apply velocity
            DebrisMesh->SetSimulatePhysics(true);
            DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            DebrisMesh->AddImpulse(Velocity, NAME_None, true);
        }
    }
    
    // Set debris name
    DebrisActor->SetActorLabel(FString::Printf(TEXT("Debris_%s_%d"), 
                                              *OriginalActor->GetName(), 
                                              FMath::RandRange(100, 999)));
    
    return DebrisActor;
}

void ACore_DestructionManager::BindDestructibleEvents(AActor* DestructibleActor)
{
    if (!DestructibleActor)
    {
        return;
    }
    
    // Find static mesh component and bind hit events
    if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(DestructibleActor))
    {
        UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Enable collision events
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetSimulatePhysics(true);
            
            // Bind hit event
            MeshComp->OnComponentHit.AddDynamic(this, &ACore_DestructionManager::OnDestructibleHit);
        }
    }
}