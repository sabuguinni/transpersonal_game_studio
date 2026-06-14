#include "Perf_CullingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

UPerf_CullingManager::UPerf_CullingManager()
{
    CullingSettings = FPerf_CullingSettings();
    CullingStats = FPerf_CullingStats();
    bShowDebugInfo = false;
    bDrawCullingBounds = false;
    LastCullingTime = 0.0;
    CullingFrameCounter = 0;
}

void UPerf_CullingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Culling Manager initialized"));
    
    // Set up periodic culling updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CullingUpdateTimer, [this]()
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    FVector ViewLocation = Pawn->GetActorLocation();
                    FVector ViewDirection = Pawn->GetActorForwardVector();
                    UpdateCulling(ViewLocation, ViewDirection, 90.0f);
                }
            }
        }, 0.1f, true); // Update every 100ms
        
        // Set up stats update timer
        World->GetTimerManager().SetTimer(StatsUpdateTimer, [this]()
        {
            CalculateMemorySavings();
        }, 1.0f, true); // Update stats every second
    }
    
    // Register all existing actors
    RegisterAllWorldActors();
}

void UPerf_CullingManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CullingUpdateTimer);
        World->GetTimerManager().ClearTimer(StatsUpdateTimer);
    }
    
    RegisteredActors.Empty();
    VisibleActors.Empty();
    CulledActors.Empty();
    
    Super::Deinitialize();
}

void UPerf_CullingManager::UpdateCulling(const FVector& ViewLocation, const FVector& ViewDirection, float FOV)
{
    double StartTime = FPlatformTime::Seconds();
    
    // Reset stats for this frame
    CullingStats.TotalActors = RegisteredActors.Num();
    CullingStats.DistanceCulledActors = 0;
    CullingStats.FrustumCulledActors = 0;
    CullingStats.OcclusionCulledActors = 0;
    
    // Clear previous frame data
    VisibleActors.Empty();
    CulledActors.Empty();
    
    // Perform culling passes
    if (CullingSettings.bEnableDistanceCulling)
    {
        PerformDistanceCulling(ViewLocation);
    }
    
    if (CullingSettings.bEnableFrustumCulling)
    {
        PerformFrustumCulling(ViewLocation, ViewDirection, FOV);
    }
    
    if (CullingSettings.bEnableOcclusionCulling)
    {
        PerformOcclusionCulling(ViewLocation);
    }
    
    // Update actor visibility
    UpdateActorVisibility();
    
    // Update stats
    CullingStats.VisibleActors = VisibleActors.Num();
    CullingStats.CullingTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to ms
    
    // Limit visible actors if needed
    if (VisibleActors.Num() > CullingSettings.MaxVisibleActors)
    {
        // Sort by distance and cull furthest ones
        VisibleActors.Sort([ViewLocation](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
        {
            if (!A.IsValid() || !B.IsValid()) return false;
            float DistA = FVector::Dist(A->GetActorLocation(), ViewLocation);
            float DistB = FVector::Dist(B->GetActorLocation(), ViewLocation);
            return DistA < DistB;
        });
        
        // Move excess actors to culled list
        for (int32 i = CullingSettings.MaxVisibleActors; i < VisibleActors.Num(); i++)
        {
            CulledActors.Add(VisibleActors[i]);
        }
        VisibleActors.SetNum(CullingSettings.MaxVisibleActors);
        CullingStats.VisibleActors = VisibleActors.Num();
    }
    
    CullingFrameCounter++;
    
    // Debug visualization
    if (bShowDebugInfo)
    {
        DrawCullingDebugInfo();
    }
}

void UPerf_CullingManager::PerformDistanceCulling(const FVector& ViewLocation)
{
    for (auto It = RegisteredActors.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
            continue;
        }
        
        AActor* Actor = It->Get();
        float Distance = FVector::Dist(Actor->GetActorLocation(), ViewLocation);
        
        if (Distance > CullingSettings.MaxDrawDistance)
        {
            CulledActors.Add(*It);
            CullingStats.DistanceCulledActors++;
        }
        else
        {
            VisibleActors.Add(*It);
        }
    }
}

void UPerf_CullingManager::PerformFrustumCulling(const FVector& ViewLocation, const FVector& ViewDirection, float FOV)
{
    TArray<TWeakObjectPtr<AActor>> TempVisible = VisibleActors;
    VisibleActors.Empty();
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TempVisible)
    {
        if (!ActorPtr.IsValid()) continue;
        
        AActor* Actor = ActorPtr.Get();
        if (IsActorInFrustum(Actor, ViewLocation, ViewDirection, FOV))
        {
            VisibleActors.Add(ActorPtr);
        }
        else
        {
            CulledActors.Add(ActorPtr);
            CullingStats.FrustumCulledActors++;
        }
    }
}

void UPerf_CullingManager::PerformOcclusionCulling(const FVector& ViewLocation)
{
    if (!CullingSettings.bEnableOcclusionCulling) return;
    
    TArray<TWeakObjectPtr<AActor>> TempVisible = VisibleActors;
    VisibleActors.Empty();
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TempVisible)
    {
        if (!ActorPtr.IsValid()) continue;
        
        AActor* Actor = ActorPtr.Get();
        FVector ActorLocation = Actor->GetActorLocation();
        
        // Simple line trace for occlusion
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Actor);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            ViewLocation,
            ActorLocation,
            ECC_Visibility,
            QueryParams
        );
        
        if (!bHit || HitResult.GetActor() == Actor)
        {
            VisibleActors.Add(ActorPtr);
        }
        else
        {
            CulledActors.Add(ActorPtr);
            CullingStats.OcclusionCulledActors++;
        }
    }
}

void UPerf_CullingManager::UpdateActorVisibility()
{
    // Set visible actors to be rendered
    for (const TWeakObjectPtr<AActor>& ActorPtr : VisibleActors)
    {
        if (!ActorPtr.IsValid()) continue;
        
        AActor* Actor = ActorPtr.Get();
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetVisibility(true);
            PrimComp->SetHiddenInGame(false);
        }
    }
    
    // Hide culled actors
    for (const TWeakObjectPtr<AActor>& ActorPtr : CulledActors)
    {
        if (!ActorPtr.IsValid()) continue;
        
        AActor* Actor = ActorPtr.Get();
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetVisibility(false);
            PrimComp->SetHiddenInGame(true);
        }
    }
}

bool UPerf_CullingManager::IsActorInFrustum(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection, float FOV) const
{
    if (!Actor) return false;
    
    FVector ActorLocation = Actor->GetActorLocation();
    FVector ToActor = (ActorLocation - ViewLocation).GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ViewDirection, ToActor);
    float HalfFOV = FMath::DegreesToRadians(FOV * 0.5f);
    
    // Add margin for culling
    float CullingAngle = FMath::Cos(HalfFOV + FMath::DegreesToRadians(CullingSettings.FrustumCullingMargin / 100.0f));
    
    return DotProduct > CullingAngle;
}

void UPerf_CullingManager::CalculateMemorySavings()
{
    float TotalMemorySaved = 0.0f;
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : CulledActors)
    {
        if (ActorPtr.IsValid())
        {
            TotalMemorySaved += CalculateActorMemoryUsage(ActorPtr.Get());
        }
    }
    
    CullingStats.MemorySavedMB = TotalMemorySaved / (1024.0f * 1024.0f);
}

float UPerf_CullingManager::CalculateActorMemoryUsage(AActor* Actor) const
{
    if (!Actor) return 0.0f;
    
    float MemoryUsage = 0.0f;
    
    // Estimate memory usage based on components
    TArray<UActorComponent*> Components = Actor->GetComponents().Array();
    for (UActorComponent* Component : Components)
    {
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component))
        {
            if (UStaticMesh* Mesh = MeshComp->GetStaticMesh())
            {
                // Rough estimate: 1KB per triangle
                MemoryUsage += Mesh->GetNumTriangles(0) * 1024.0f;
            }
        }
        else if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(Component))
        {
            if (USkeletalMesh* SkelMesh = SkelComp->GetSkeletalMeshAsset())
            {
                // Rough estimate: 2KB per triangle for skeletal meshes
                MemoryUsage += SkelMesh->GetResourceForRendering()->LODRenderData[0].GetNumTriangles() * 2048.0f;
            }
        }
    }
    
    return MemoryUsage;
}

void UPerf_CullingManager::SetCullingSettings(const FPerf_CullingSettings& NewSettings)
{
    CullingSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Culling settings updated"));
}

void UPerf_CullingManager::RegisterActor(AActor* Actor)
{
    if (!Actor) return;
    
    TWeakObjectPtr<AActor> ActorPtr(Actor);
    if (!RegisteredActors.Contains(ActorPtr))
    {
        RegisteredActors.Add(ActorPtr);
    }
}

void UPerf_CullingManager::UnregisterActor(AActor* Actor)
{
    if (!Actor) return;
    
    TWeakObjectPtr<AActor> ActorPtr(Actor);
    RegisteredActors.Remove(ActorPtr);
    VisibleActors.Remove(ActorPtr);
    CulledActors.Remove(ActorPtr);
}

void UPerf_CullingManager::RegisterAllWorldActors()
{
    if (!GetWorld()) return;
    
    RegisteredActors.Empty();
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            // Only register actors with renderable components
            RegisterActor(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d actors for culling"), RegisteredActors.Num());
}

void UPerf_CullingManager::EnableDistanceCulling(bool bEnable)
{
    CullingSettings.bEnableDistanceCulling = bEnable;
}

void UPerf_CullingManager::EnableFrustumCulling(bool bEnable)
{
    CullingSettings.bEnableFrustumCulling = bEnable;
}

void UPerf_CullingManager::EnableOcclusionCulling(bool bEnable)
{
    CullingSettings.bEnableOcclusionCulling = bEnable;
}

void UPerf_CullingManager::SetMaxDrawDistance(float Distance)
{
    CullingSettings.MaxDrawDistance = FMath::Max(Distance, 100.0f);
}

void UPerf_CullingManager::SetMaxVisibleActors(int32 MaxActors)
{
    CullingSettings.MaxVisibleActors = FMath::Max(MaxActors, 10);
}

void UPerf_CullingManager::AnalyzeCullingPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CULLING PERFORMANCE ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CullingStats.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Visible Actors: %d"), CullingStats.VisibleActors);
    UE_LOG(LogTemp, Warning, TEXT("Distance Culled: %d"), CullingStats.DistanceCulledActors);
    UE_LOG(LogTemp, Warning, TEXT("Frustum Culled: %d"), CullingStats.FrustumCulledActors);
    UE_LOG(LogTemp, Warning, TEXT("Occlusion Culled: %d"), CullingStats.OcclusionCulledActors);
    UE_LOG(LogTemp, Warning, TEXT("Culling Time: %.2f ms"), CullingStats.CullingTime);
    UE_LOG(LogTemp, Warning, TEXT("Memory Saved: %.2f MB"), CullingStats.MemorySavedMB);
    UE_LOG(LogTemp, Warning, TEXT("Culling Efficiency: %.1f%%"), 
        CullingStats.TotalActors > 0 ? (float)(CullingStats.TotalActors - CullingStats.VisibleActors) / CullingStats.TotalActors * 100.0f : 0.0f);
}

void UPerf_CullingManager::OptimizeCullingSettings()
{
    // Auto-optimize based on current performance
    if (CullingStats.CullingTime > 5.0f) // If culling takes more than 5ms
    {
        // Reduce max visible actors
        CullingSettings.MaxVisibleActors = FMath::Max(CullingSettings.MaxVisibleActors - 100, 500);
        UE_LOG(LogTemp, Warning, TEXT("Reduced max visible actors to %d for performance"), CullingSettings.MaxVisibleActors);
    }
    
    if (CullingStats.VisibleActors > CullingSettings.MaxVisibleActors * 0.9f)
    {
        // Reduce draw distance if we're hitting the actor limit
        CullingSettings.MaxDrawDistance *= 0.9f;
        UE_LOG(LogTemp, Warning, TEXT("Reduced draw distance to %.0f for performance"), CullingSettings.MaxDrawDistance);
    }
}

void UPerf_CullingManager::ResetCullingStats()
{
    CullingStats = FPerf_CullingStats();
    CullingFrameCounter = 0;
    UE_LOG(LogTemp, Log, TEXT("Culling stats reset"));
}

void UPerf_CullingManager::ToggleDebugVisualization()
{
    bShowDebugInfo = !bShowDebugInfo;
    bDrawCullingBounds = !bDrawCullingBounds;
}

void UPerf_CullingManager::DrawCullingDebugInfo()
{
    if (!GetWorld()) return;
    
    // Draw visible actors in green
    for (const TWeakObjectPtr<AActor>& ActorPtr : VisibleActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 50.0f, 8, FColor::Green, false, 0.1f);
        }
    }
    
    // Draw culled actors in red
    for (const TWeakObjectPtr<AActor>& ActorPtr : CulledActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 30.0f, 8, FColor::Red, false, 0.1f);
        }
    }
    
    // Draw culling bounds
    if (bDrawCullingBounds)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                FVector ViewLocation = Pawn->GetActorLocation();
                DrawDebugSphere(GetWorld(), ViewLocation, CullingSettings.MaxDrawDistance, 32, FColor::Yellow, false, 0.1f, 0, 10.0f);
            }
        }
    }
}