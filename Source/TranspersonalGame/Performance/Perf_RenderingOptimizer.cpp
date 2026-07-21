#include "Perf_RenderingOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

UPerf_RenderingOptimizer::UPerf_RenderingOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    RenderingSettings = FPerf_RenderingSettings();
    TargetFPS = 60;
    bAutoOptimize = true;
    OptimizationInterval = 2.0f;
    CurrentFPS = 0.0f;
    AverageFrameTime = 0.0f;
    VisibleActors = 0;
    CulledActors = 0;
    LastOptimizationTime = 0.0f;
    MaxFrameHistorySize = 60;
}

void UPerf_RenderingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameHistorySize);
    
    // Apply initial rendering settings
    ApplyRenderingOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Rendering Optimizer initialized - Target FPS: %d"), TargetFPS);
}

void UPerf_RenderingOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceStats();
    
    if (bAutoOptimize)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
        {
            OptimizeRendering();
            LastOptimizationTime = CurrentTime;
        }
    }
}

void UPerf_RenderingOptimizer::UpdatePerformanceStats()
{
    if (!GetWorld()) return;
    
    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
        
        // Update frame time history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > MaxFrameHistorySize)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average frame time
        float TotalFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalFrameTime += FrameTime;
        }
        AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }
    
    // Count visible actors (simplified)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    VisibleActors = AllActors.Num();
    
    // Estimate culled actors (actors beyond max draw distance)
    CulledActors = 0;
    if (AActor* Owner = GetOwner())
    {
        FVector OwnerLocation = Owner->GetActorLocation();
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor != Owner)
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance > RenderingSettings.MaxDrawDistance)
                {
                    CulledActors++;
                }
            }
        }
    }
}

void UPerf_RenderingOptimizer::OptimizeRendering()
{
    if (!IsPerformanceGood())
    {
        // Performance is below target - reduce quality
        if (RenderingSettings.QualityLevel == EPerf_RenderQuality::Ultra)
        {
            SetRenderQuality(EPerf_RenderQuality::High);
        }
        else if (RenderingSettings.QualityLevel == EPerf_RenderQuality::High)
        {
            SetRenderQuality(EPerf_RenderQuality::Medium);
        }
        else if (RenderingSettings.QualityLevel == EPerf_RenderQuality::Medium)
        {
            SetRenderQuality(EPerf_RenderQuality::Low);
        }
        
        // Also reduce render scale if needed
        if (CurrentFPS < (TargetFPS * 0.7f))
        {
            float NewScale = FMath::Max(0.5f, RenderingSettings.RenderScale - 0.1f);
            SetRenderScale(NewScale);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance below target (%d FPS) - Current: %.1f FPS - Reducing quality"), TargetFPS, CurrentFPS);
    }
    else if (CurrentFPS > (TargetFPS * 1.2f))
    {
        // Performance is above target - can increase quality
        if (RenderingSettings.RenderScale < 1.0f)
        {
            float NewScale = FMath::Min(1.0f, RenderingSettings.RenderScale + 0.05f);
            SetRenderScale(NewScale);
        }
        else if (RenderingSettings.QualityLevel == EPerf_RenderQuality::Low)
        {
            SetRenderQuality(EPerf_RenderQuality::Medium);
        }
        else if (RenderingSettings.QualityLevel == EPerf_RenderQuality::Medium)
        {
            SetRenderQuality(EPerf_RenderQuality::High);
        }
        else if (RenderingSettings.QualityLevel == EPerf_RenderQuality::High)
        {
            SetRenderQuality(EPerf_RenderQuality::Ultra);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance above target - Current: %.1f FPS - Increasing quality"), CurrentFPS);
    }
    
    ApplyRenderingOptimizations();
}

void UPerf_RenderingOptimizer::SetRenderQuality(EPerf_RenderQuality NewQuality)
{
    RenderingSettings.QualityLevel = NewQuality;
    
    // Adjust settings based on quality level
    switch (NewQuality)
    {
        case EPerf_RenderQuality::Low:
            RenderingSettings.MaxDrawDistance = 2000.0f;
            RenderingSettings.bEnableDynamicLOD = true;
            RenderingSettings.bEnableOcclusion = true;
            break;
        case EPerf_RenderQuality::Medium:
            RenderingSettings.MaxDrawDistance = 3500.0f;
            RenderingSettings.bEnableDynamicLOD = true;
            RenderingSettings.bEnableOcclusion = true;
            break;
        case EPerf_RenderQuality::High:
            RenderingSettings.MaxDrawDistance = 5000.0f;
            RenderingSettings.bEnableDynamicLOD = true;
            RenderingSettings.bEnableOcclusion = true;
            break;
        case EPerf_RenderQuality::Ultra:
            RenderingSettings.MaxDrawDistance = 7500.0f;
            RenderingSettings.bEnableDynamicLOD = false;
            RenderingSettings.bEnableOcclusion = true;
            break;
    }
}

void UPerf_RenderingOptimizer::SetRenderScale(float NewScale)
{
    RenderingSettings.RenderScale = FMath::Clamp(NewScale, 0.1f, 2.0f);
    
    // Apply render scale to engine
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %f"), RenderingSettings.RenderScale * 100.0f));
    }
}

void UPerf_RenderingOptimizer::EnableLODOptimization(bool bEnable)
{
    RenderingSettings.bEnableDynamicLOD = bEnable;
    UpdateLODSettings();
}

void UPerf_RenderingOptimizer::SetMaxDrawDistance(float Distance)
{
    RenderingSettings.MaxDrawDistance = FMath::Max(500.0f, Distance);
    CullDistantObjects();
}

void UPerf_RenderingOptimizer::ApplyRenderingOptimizations()
{
    if (!GetWorld()) return;
    
    // Apply render scale
    SetRenderScale(RenderingSettings.RenderScale);
    
    // Update LOD settings
    UpdateLODSettings();
    
    // Cull distant objects
    CullDistantObjects();
    
    // Optimize mesh components
    OptimizeMeshComponents();
}

void UPerf_RenderingOptimizer::OptimizeMeshComponents()
{
    if (!GetWorld() || !GetOwner()) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        // Get all mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp)
            {
                // Adjust LOD bias based on distance and quality
                int32 LODBias = 0;
                if (RenderingSettings.bEnableDynamicLOD)
                {
                    if (Distance > RenderingSettings.MaxDrawDistance * 0.8f)
                    {
                        LODBias = 2;
                    }
                    else if (Distance > RenderingSettings.MaxDrawDistance * 0.6f)
                    {
                        LODBias = 1;
                    }
                }
                
                // Apply visibility based on distance
                bool bShouldBeVisible = Distance <= RenderingSettings.MaxDrawDistance;
                MeshComp->SetVisibility(bShouldBeVisible);
            }
        }
    }
}

void UPerf_RenderingOptimizer::UpdateLODSettings()
{
    if (!GetWorld()) return;
    
    // Apply global LOD settings
    if (GEngine)
    {
        if (RenderingSettings.bEnableDynamicLOD)
        {
            GEngine->Exec(GetWorld(), TEXT("r.ForceLOD -1"));
        }
        else
        {
            GEngine->Exec(GetWorld(), TEXT("r.ForceLOD 0"));
        }
    }
}

void UPerf_RenderingOptimizer::CullDistantObjects()
{
    if (!GetWorld() || !GetOwner()) return;
    
    // This is handled in OptimizeMeshComponents for now
    // Could be expanded to handle other types of culling
}