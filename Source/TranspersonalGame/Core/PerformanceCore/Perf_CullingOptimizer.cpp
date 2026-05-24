#include "Perf_CullingOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

APerf_CullingOptimizer::APerf_CullingOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    // Default culling settings
    MaxCullingDistance = 15000.0f;
    BiomeCullingDistance = 8000.0f;
    bEnableDistanceCulling = true;
    bEnableFrustumCulling = true;
    bEnableOcclusionCulling = true;
    CullingUpdateFrequency = 0.5f;
    
    // Performance thresholds
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    bAdaptiveCulling = true;
    
    // Biome-specific settings
    BiomeCullingMultipliers.Add(TEXT("Pantano"), 0.8f);
    BiomeCullingMultipliers.Add(TEXT("Floresta"), 0.7f);
    BiomeCullingMultipliers.Add(TEXT("Savana"), 1.0f);
    BiomeCullingMultipliers.Add(TEXT("Deserto"), 1.2f);
    BiomeCullingMultipliers.Add(TEXT("Montanha"), 1.1f);
    
    LastCullingUpdate = 0.0f;
    CurrentFrameRate = 60.0f;
}

void APerf_CullingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Starting performance-based culling system"));
    
    // Initialize culling system
    InitializeCullingSystem();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!CachedPlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("CullingOptimizer: Failed to get player controller"));
    }
}

void APerf_CullingOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableDistanceCulling && !bEnableFrustumCulling && !bEnableOcclusionCulling)
    {
        return;
    }
    
    LastCullingUpdate += DeltaTime;
    
    if (LastCullingUpdate >= CullingUpdateFrequency)
    {
        UpdateCullingSystem();
        LastCullingUpdate = 0.0f;
    }
    
    // Update frame rate tracking
    UpdateFrameRateTracking(DeltaTime);
    
    // Adaptive culling based on performance
    if (bAdaptiveCulling)
    {
        UpdateAdaptiveCulling();
    }
}

void APerf_CullingOptimizer::InitializeCullingSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CullingOptimizer: No world found"));
        return;
    }
    
    // Get all static mesh actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
    
    CullableActors.Empty();
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && IsValid(Actor))
        {
            AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
            if (MeshActor)
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp && MeshComp->GetStaticMesh())
                {
                    FCullableActorInfo ActorInfo;
                    ActorInfo.Actor = MeshActor;
                    ActorInfo.OriginalVisibility = MeshComp->IsVisible();
                    ActorInfo.LastCullDistance = 0.0f;
                    ActorInfo.BiomeType = DetermineBiomeType(MeshActor->GetActorLocation());
                    
                    CullableActors.Add(ActorInfo);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Initialized with %d cullable actors"), CullableActors.Num());
}

void APerf_CullingOptimizer::UpdateCullingSystem()
{
    if (!CachedPlayerController)
    {
        return;
    }
    
    APawn* PlayerPawn = CachedPlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FRotator PlayerRotation = CachedPlayerController->GetControlRotation();
    
    int32 CulledActors = 0;
    int32 VisibleActors = 0;
    
    for (FCullableActorInfo& ActorInfo : CullableActors)
    {
        if (!ActorInfo.Actor || !IsValid(ActorInfo.Actor))
        {
            continue;
        }
        
        AStaticMeshActor* MeshActor = ActorInfo.Actor;
        UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
        
        if (!MeshComp)
        {
            continue;
        }
        
        FVector ActorLocation = MeshActor->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, ActorLocation);
        
        bool bShouldBeCulled = false;
        
        // Distance culling
        if (bEnableDistanceCulling)
        {
            float EffectiveCullingDistance = GetEffectiveCullingDistance(ActorInfo.BiomeType);
            if (Distance > EffectiveCullingDistance)
            {
                bShouldBeCulled = true;
            }
        }
        
        // Frustum culling (simplified)
        if (bEnableFrustumCulling && !bShouldBeCulled)
        {
            FVector DirectionToActor = (ActorLocation - PlayerLocation).GetSafeNormal();
            FVector PlayerForward = PlayerRotation.Vector();
            float DotProduct = FVector::DotProduct(PlayerForward, DirectionToActor);
            
            // If actor is behind player (dot product < 0) and far away, cull it
            if (DotProduct < -0.2f && Distance > 2000.0f)
            {
                bShouldBeCulled = true;
            }
        }
        
        // Apply culling
        if (bShouldBeCulled)
        {
            if (MeshComp->IsVisible())
            {
                MeshComp->SetVisibility(false);
                CulledActors++;
            }
        }
        else
        {
            if (!MeshComp->IsVisible())
            {
                MeshComp->SetVisibility(true);
            }
            VisibleActors++;
        }
        
        ActorInfo.LastCullDistance = Distance;
    }
    
    // Log culling statistics periodically
    static float LogTimer = 0.0f;
    LogTimer += CullingUpdateFrequency;
    
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("CullingOptimizer: Visible: %d, Culled: %d, FPS: %.1f"), 
               VisibleActors, CulledActors, CurrentFrameRate);
        LogTimer = 0.0f;
    }
}

void APerf_CullingOptimizer::UpdateFrameRateTracking(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        float InstantFPS = 1.0f / DeltaTime;
        
        // Smooth the frame rate using exponential moving average
        CurrentFrameRate = CurrentFrameRate * 0.9f + InstantFPS * 0.1f;
    }
}

void APerf_CullingOptimizer::UpdateAdaptiveCulling()
{
    if (CurrentFrameRate < MinFrameRate)
    {
        // Performance is poor, increase culling aggressiveness
        MaxCullingDistance *= 0.95f;
        BiomeCullingDistance *= 0.95f;
        
        // Clamp to reasonable minimums
        MaxCullingDistance = FMath::Max(MaxCullingDistance, 5000.0f);
        BiomeCullingDistance = FMath::Max(BiomeCullingDistance, 3000.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Poor performance detected (%.1f FPS), increasing culling"), CurrentFrameRate);
    }
    else if (CurrentFrameRate > TargetFrameRate * 1.1f)
    {
        // Performance is good, reduce culling aggressiveness
        MaxCullingDistance *= 1.02f;
        BiomeCullingDistance *= 1.02f;
        
        // Clamp to reasonable maximums
        MaxCullingDistance = FMath::Min(MaxCullingDistance, 20000.0f);
        BiomeCullingDistance = FMath::Min(BiomeCullingDistance, 15000.0f);
    }
}

float APerf_CullingOptimizer::GetEffectiveCullingDistance(const FString& BiomeType) const
{
    const float* Multiplier = BiomeCullingMultipliers.Find(BiomeType);
    float BaseDistance = BiomeCullingDistance;
    
    if (Multiplier)
    {
        BaseDistance *= (*Multiplier);
    }
    
    return FMath::Min(BaseDistance, MaxCullingDistance);
}

FString APerf_CullingOptimizer::DetermineBiomeType(const FVector& Location) const
{
    // Biome determination based on world coordinates
    // Pantano (sudoeste): X(-77500 a -25000), Y(-76500 a -15000)
    if (Location.X >= -77500 && Location.X <= -25000 && Location.Y >= -76500 && Location.Y <= -15000)
    {
        return TEXT("Pantano");
    }
    // Floresta (noroeste): X(-77500 a -15000), Y(15000 a 76500)
    else if (Location.X >= -77500 && Location.X <= -15000 && Location.Y >= 15000 && Location.Y <= 76500)
    {
        return TEXT("Floresta");
    }
    // Savana (centro): X(-20000 a 20000), Y(-20000 a 20000)
    else if (Location.X >= -20000 && Location.X <= 20000 && Location.Y >= -20000 && Location.Y <= 20000)
    {
        return TEXT("Savana");
    }
    // Deserto (leste): X(25000 a 79500), Y(-30000 a 30000)
    else if (Location.X >= 25000 && Location.X <= 79500 && Location.Y >= -30000 && Location.Y <= 30000)
    {
        return TEXT("Deserto");
    }
    // Montanha (nordeste): X(15000 a 79500), Y(20000 a 76500)
    else if (Location.X >= 15000 && Location.X <= 79500 && Location.Y >= 20000 && Location.Y <= 76500)
    {
        return TEXT("Montanha");
    }
    
    return TEXT("Unknown");
}

void APerf_CullingOptimizer::SetCullingEnabled(bool bEnabled)
{
    bEnableDistanceCulling = bEnabled;
    
    if (!bEnabled)
    {
        // Restore visibility to all actors
        for (FCullableActorInfo& ActorInfo : CullableActors)
        {
            if (ActorInfo.Actor && IsValid(ActorInfo.Actor))
            {
                UStaticMeshComponent* MeshComp = ActorInfo.Actor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    MeshComp->SetVisibility(ActorInfo.OriginalVisibility);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Culling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_CullingOptimizer::SetCullingDistance(float NewDistance)
{
    MaxCullingDistance = FMath::Max(NewDistance, 1000.0f);
    BiomeCullingDistance = MaxCullingDistance * 0.8f;
    
    UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Culling distance set to %.0f"), MaxCullingDistance);
}

void APerf_CullingOptimizer::RefreshCullableActors()
{
    UE_LOG(LogTemp, Warning, TEXT("CullingOptimizer: Refreshing cullable actors list"));
    InitializeCullingSystem();
}