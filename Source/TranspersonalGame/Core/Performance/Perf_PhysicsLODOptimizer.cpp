#include "Perf_PhysicsLODOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UPerf_PhysicsLODOptimizer::UPerf_PhysicsLODOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LODSettings = FPerf_PhysicsLODSettings();
    UpdateInterval = 0.1f;
    MaxObjectsPerFrame = 50;
    bDebugVisualization = false;
    
    OptimizedObjectCount = 0;
    CurrentFrameRate = 60.0f;
    AverageFrameTime = 16.67f;
    
    FrameTimeHistory.Reserve(FrameHistorySize);
}

void UPerf_PhysicsLODOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize frame time history
    FrameTimeHistory.Empty();
    for (int32 i = 0; i < FrameHistorySize; ++i)
    {
        FrameTimeHistory.Add(16.67f); // Default to 60 FPS
    }
    
    // Auto-initialize physics LOD system
    InitializePhysicsLOD();
    
    UE_LOG(LogTemp, Log, TEXT("Physics LOD Optimizer initialized"));
}

void UPerf_PhysicsLODOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame rate tracking
    UpdateFrameRateTracking(DeltaTime);
    
    // Check if it's time to update physics LOD
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        LastUpdateTime = CurrentTime;
        
        // Get viewer location (player camera or pawn)
        FVector ViewerLocation = FVector::ZeroVector;
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    ViewerLocation = PlayerPawn->GetActorLocation();
                }
            }
        }
        
        // Update physics LOD for tracked objects
        UpdatePhysicsLOD(ViewerLocation);
        
        // Clean up invalid objects
        CleanupInvalidObjects();
        
        // Debug visualization
        if (bDebugVisualization)
        {
            DrawDebugInfo(ViewerLocation);
        }
    }
}

void UPerf_PhysicsLODOptimizer::InitializePhysicsLOD()
{
    TrackedObjects.Empty();
    OptimizedObjectCount = 0;
    
    // Find all physics-enabled actors in the world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (IsValid(Actor))
            {
                // Check if actor has physics components
                if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
                {
                    if (MeshComp->IsSimulatingPhysics() || MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                    {
                        RegisterPhysicsObject(Actor);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics LOD initialized with %d tracked objects"), TrackedObjects.Num());
}

void UPerf_PhysicsLODOptimizer::UpdatePhysicsLOD(const FVector& ViewerLocation)
{
    if (TrackedObjects.Num() == 0)
    {
        return;
    }
    
    // Process objects in batches to spread load across frames
    int32 BatchSize = FMath::Min(MaxObjectsPerFrame, TrackedObjects.Num());
    ProcessObjectBatch(ViewerLocation, BatchSize);
}

void UPerf_PhysicsLODOptimizer::ProcessObjectBatch(const FVector& ViewerLocation, int32 BatchSize)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 ProcessedCount = 0;
    
    for (int32 i = 0; i < TrackedObjects.Num() && ProcessedCount < BatchSize; ++i)
    {
        int32 Index = (CurrentUpdateIndex + i) % TrackedObjects.Num();
        FPerf_PhysicsObjectData& ObjectData = TrackedObjects[Index];
        
        // Skip if object should not be updated this frame
        if (!ShouldUpdateObject(ObjectData, CurrentTime))
        {
            continue;
        }
        
        // Check if actor is still valid
        if (!ObjectData.Actor.IsValid())
        {
            continue;
        }
        
        AActor* Actor = ObjectData.Actor.Get();
        float Distance = FVector::Dist(ViewerLocation, Actor->GetActorLocation());
        
        // Determine optimal LOD level
        EPerf_PhysicsLODLevel OptimalLOD = GetOptimalLODLevel(Distance, CurrentFrameRate);
        
        // Apply LOD if changed
        if (OptimalLOD != ObjectData.CurrentLODLevel)
        {
            ApplyPhysicsLOD(Actor, OptimalLOD);
            ObjectData.CurrentLODLevel = OptimalLOD;
            OptimizedObjectCount++;
        }
        
        // Update object data
        ObjectData.LastUpdateDistance = Distance;
        ObjectData.LastUpdateTime = CurrentTime;
        
        ProcessedCount++;
    }
    
    // Update batch index for next frame
    CurrentUpdateIndex = (CurrentUpdateIndex + ProcessedCount) % FMath::Max(1, TrackedObjects.Num());
}

void UPerf_PhysicsLODOptimizer::RegisterPhysicsObject(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    // Check if already registered
    for (const FPerf_PhysicsObjectData& ObjectData : TrackedObjects)
    {
        if (ObjectData.Actor == Actor)
        {
            return; // Already registered
        }
    }
    
    // Create new object data
    FPerf_PhysicsObjectData NewObjectData;
    NewObjectData.Actor = Actor;
    NewObjectData.CurrentLODLevel = EPerf_PhysicsLODLevel::FullPhysics;
    NewObjectData.LastUpdateDistance = 0.0f;
    NewObjectData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Check initial physics state
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        NewObjectData.bWasPhysicsEnabled = MeshComp->IsSimulatingPhysics();
    }
    
    TrackedObjects.Add(NewObjectData);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Registered physics object: %s"), *Actor->GetName());
}

void UPerf_PhysicsLODOptimizer::UnregisterPhysicsObject(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    for (int32 i = TrackedObjects.Num() - 1; i >= 0; --i)
    {
        if (TrackedObjects[i].Actor == Actor)
        {
            TrackedObjects.RemoveAt(i);
            UE_LOG(LogTemp, VeryVerbose, TEXT("Unregistered physics object: %s"), *Actor->GetName());
            break;
        }
    }
}

EPerf_PhysicsLODLevel UPerf_PhysicsLODOptimizer::GetOptimalLODLevel(float Distance, float FrameRate) const
{
    // Distance-based LOD
    if (LODSettings.bEnableDistanceCulling && Distance > LODSettings.CullingDistance)
    {
        return EPerf_PhysicsLODLevel::Disabled;
    }
    
    if (Distance > LODSettings.LOD2Distance)
    {
        return EPerf_PhysicsLODLevel::StaticOnly;
    }
    
    if (Distance > LODSettings.LOD1Distance)
    {
        return EPerf_PhysicsLODLevel::ReducedPhysics;
    }
    
    if (Distance > LODSettings.LOD0Distance)
    {
        // Frame rate adaptive LOD
        if (LODSettings.bEnableFrameRateAdaptive && FrameRate < LODSettings.TargetFrameRate * 0.8f)
        {
            return EPerf_PhysicsLODLevel::ReducedPhysics;
        }
        return EPerf_PhysicsLODLevel::FullPhysics;
    }
    
    return EPerf_PhysicsLODLevel::FullPhysics;
}

void UPerf_PhysicsLODOptimizer::ApplyPhysicsLOD(AActor* Actor, EPerf_PhysicsLODLevel LODLevel)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EPerf_PhysicsLODLevel::FullPhysics:
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case EPerf_PhysicsLODLevel::ReducedPhysics:
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case EPerf_PhysicsLODLevel::StaticOnly:
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
            
        case EPerf_PhysicsLODLevel::Disabled:
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Applied LOD %d to actor %s"), 
           static_cast<int32>(LODLevel), *Actor->GetName());
}

void UPerf_PhysicsLODOptimizer::ForceRefreshAllObjects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FPerf_PhysicsObjectData& ObjectData : TrackedObjects)
    {
        ObjectData.LastUpdateTime = 0.0f; // Force update
    }
    
    UE_LOG(LogTemp, Log, TEXT("Forced refresh of all %d physics objects"), TrackedObjects.Num());
}

void UPerf_PhysicsLODOptimizer::SetLODSettings(const FPerf_PhysicsLODSettings& NewSettings)
{
    LODSettings = NewSettings;
    ForceRefreshAllObjects();
    
    UE_LOG(LogTemp, Log, TEXT("Updated LOD settings - LOD0: %.0f, LOD1: %.0f, LOD2: %.0f, Culling: %.0f"), 
           LODSettings.LOD0Distance, LODSettings.LOD1Distance, 
           LODSettings.LOD2Distance, LODSettings.CullingDistance);
}

float UPerf_PhysicsLODOptimizer::GetCurrentFrameRate() const
{
    return CurrentFrameRate;
}

int32 UPerf_PhysicsLODOptimizer::GetActivePhysicsObjectCount() const
{
    int32 ActiveCount = 0;
    
    for (const FPerf_PhysicsObjectData& ObjectData : TrackedObjects)
    {
        if (ObjectData.Actor.IsValid() && ObjectData.CurrentLODLevel == EPerf_PhysicsLODLevel::FullPhysics)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

int32 UPerf_PhysicsLODOptimizer::GetTotalTrackedObjectCount() const
{
    return TrackedObjects.Num();
}

FString UPerf_PhysicsLODOptimizer::GetPerformanceStats() const
{
    int32 FullPhysics = 0, ReducedPhysics = 0, StaticOnly = 0, Disabled = 0;
    
    for (const FPerf_PhysicsObjectData& ObjectData : TrackedObjects)
    {
        if (ObjectData.Actor.IsValid())
        {
            switch (ObjectData.CurrentLODLevel)
            {
                case EPerf_PhysicsLODLevel::FullPhysics: FullPhysics++; break;
                case EPerf_PhysicsLODLevel::ReducedPhysics: ReducedPhysics++; break;
                case EPerf_PhysicsLODLevel::StaticOnly: StaticOnly++; break;
                case EPerf_PhysicsLODLevel::Disabled: Disabled++; break;
            }
        }
    }
    
    return FString::Printf(TEXT("Physics LOD Stats - FPS: %.1f | Full: %d | Reduced: %d | Static: %d | Disabled: %d | Optimized: %d"), 
                          CurrentFrameRate, FullPhysics, ReducedPhysics, StaticOnly, Disabled, OptimizedObjectCount);
}

void UPerf_PhysicsLODOptimizer::ToggleDebugVisualization()
{
    bDebugVisualization = !bDebugVisualization;
    UE_LOG(LogTemp, Log, TEXT("Physics LOD debug visualization: %s"), 
           bDebugVisualization ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UPerf_PhysicsLODOptimizer::DrawDebugInfo(const FVector& ViewerLocation)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw LOD distance rings
    DrawDebugCircle(GetWorld(), ViewerLocation, LODSettings.LOD0Distance, 32, FColor::Green, false, UpdateInterval);
    DrawDebugCircle(GetWorld(), ViewerLocation, LODSettings.LOD1Distance, 32, FColor::Yellow, false, UpdateInterval);
    DrawDebugCircle(GetWorld(), ViewerLocation, LODSettings.LOD2Distance, 32, FColor::Orange, false, UpdateInterval);
    DrawDebugCircle(GetWorld(), ViewerLocation, LODSettings.CullingDistance, 32, FColor::Red, false, UpdateInterval);
    
    // Draw object LOD states
    for (const FPerf_PhysicsObjectData& ObjectData : TrackedObjects)
    {
        if (ObjectData.Actor.IsValid())
        {
            FColor LODColor = FColor::White;
            switch (ObjectData.CurrentLODLevel)
            {
                case EPerf_PhysicsLODLevel::FullPhysics: LODColor = FColor::Green; break;
                case EPerf_PhysicsLODLevel::ReducedPhysics: LODColor = FColor::Yellow; break;
                case EPerf_PhysicsLODLevel::StaticOnly: LODColor = FColor::Orange; break;
                case EPerf_PhysicsLODLevel::Disabled: LODColor = FColor::Red; break;
            }
            
            FVector ActorLocation = ObjectData.Actor->GetActorLocation();
            DrawDebugSphere(GetWorld(), ActorLocation, 50.0f, 8, LODColor, false, UpdateInterval);
        }
    }
    
    // Display performance stats on screen
    if (GEngine)
    {
        FString StatsText = GetPerformanceStats();
        GEngine->AddOnScreenDebugMessage(-1, UpdateInterval, FColor::Cyan, StatsText);
    }
}

void UPerf_PhysicsLODOptimizer::UpdateFrameRateTracking(float DeltaTime)
{
    // Add current frame time to history
    float FrameTimeMs = DeltaTime * 1000.0f;
    FrameTimeHistory[CurrentUpdateIndex % FrameHistorySize] = FrameTimeMs;
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    
    AverageFrameTime = TotalFrameTime / FrameHistorySize;
    CurrentFrameRate = 1000.0f / FMath::Max(AverageFrameTime, 0.1f);
}

bool UPerf_PhysicsLODOptimizer::ShouldUpdateObject(const FPerf_PhysicsObjectData& ObjectData, float CurrentTime) const
{
    // Always update if never updated
    if (ObjectData.LastUpdateTime <= 0.0f)
    {
        return true;
    }
    
    // Update based on distance (closer objects update more frequently)
    float UpdateFrequency = UpdateInterval;
    if (ObjectData.LastUpdateDistance > LODSettings.LOD1Distance)
    {
        UpdateFrequency *= 2.0f; // Update distant objects less frequently
    }
    
    return (CurrentTime - ObjectData.LastUpdateTime) >= UpdateFrequency;
}

void UPerf_PhysicsLODOptimizer::CleanupInvalidObjects()
{
    for (int32 i = TrackedObjects.Num() - 1; i >= 0; --i)
    {
        if (!TrackedObjects[i].Actor.IsValid())
        {
            TrackedObjects.RemoveAt(i);
        }
    }
}