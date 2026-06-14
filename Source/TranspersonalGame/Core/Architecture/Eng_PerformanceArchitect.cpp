#include "Eng_PerformanceArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

void UEng_PerformanceArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Set default performance budget
    PerformanceBudget.TargetFPS = 60.0f;
    PerformanceBudget.MaxActors = 8000;
    PerformanceBudget.MaxDinosaurs = 150;
    PerformanceBudget.MaxMemoryMB = 4096.0f;

    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UEng_PerformanceArchitect::UpdateMetrics,
            1.0f,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("PerformanceArchitect: Initialized with FPS target %.1f"), PerformanceBudget.TargetFPS);
}

void UEng_PerformanceArchitect::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    Super::Deinitialize();
}

FEng_PerformanceMetrics UEng_PerformanceArchitect::GetCurrentMetrics() const
{
    return CachedMetrics;
}

bool UEng_PerformanceArchitect::IsPerformanceWithinBudget() const
{
    return CachedMetrics.CurrentFPS >= (PerformanceBudget.TargetFPS * 0.9f) &&
           CachedMetrics.ActiveActors <= PerformanceBudget.MaxActors &&
           CachedMetrics.MemoryUsageMB <= PerformanceBudget.MaxMemoryMB;
}

void UEng_PerformanceArchitect::EnforcePerformanceBudgets()
{
    UpdateMetrics();
    
    if (CachedMetrics.ActiveActors > PerformanceBudget.MaxActors)
    {
        EnforceActorCap();
    }
    
    EnforceDinosaurCap();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceArchitect: Budgets enforced - FPS: %.1f, Actors: %d"), 
           CachedMetrics.CurrentFPS, CachedMetrics.ActiveActors);
}

void UEng_PerformanceArchitect::SetPerformanceBudget(const FEng_PerformanceBudget& NewBudget)
{
    PerformanceBudget = NewBudget;
    UE_LOG(LogTemp, Warning, TEXT("PerformanceArchitect: Budget updated - Target FPS: %.1f, Max Actors: %d"), 
           PerformanceBudget.TargetFPS, PerformanceBudget.MaxActors);
}

void UEng_PerformanceArchitect::ValidateCurrentPerformance()
{
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), CachedMetrics.CurrentFPS, PerformanceBudget.TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d (Max: %d)"), CachedMetrics.ActiveActors, PerformanceBudget.MaxActors);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB (Max: %.1f MB)"), CachedMetrics.MemoryUsageMB, PerformanceBudget.MaxMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Within Budget: %s"), IsPerformanceWithinBudget() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

void UEng_PerformanceArchitect::UpdateMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Get all actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CachedMetrics.ActiveActors = AllActors.Num();

        // Calculate FPS (simplified)
        CachedMetrics.FrameTime = FApp::GetDeltaTime();
        CachedMetrics.CurrentFPS = CachedMetrics.FrameTime > 0.0f ? (1.0f / CachedMetrics.FrameTime) : 0.0f;

        // Estimate memory usage (simplified)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CachedMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

        // Count visible primitives (simplified)
        CachedMetrics.VisiblePrimitives = AllActors.Num() * 2; // Rough estimate
    }
}

void UEng_PerformanceArchitect::EnforceDinosaurCap()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        TArray<AActor*> DinosaurActors;
        TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                         TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorLabel = Actor->GetActorLabel().ToLower();
                for (const FString& DinoLabel : DinosaurLabels)
                {
                    if (ActorLabel.Contains(DinoLabel))
                    {
                        DinosaurActors.Add(Actor);
                        break;
                    }
                }
            }
        }
        
        if (DinosaurActors.Num() > PerformanceBudget.MaxDinosaurs)
        {
            int32 ToRemove = DinosaurActors.Num() - PerformanceBudget.MaxDinosaurs;
            for (int32 i = 0; i < ToRemove && i < DinosaurActors.Num(); i++)
            {
                if (DinosaurActors[i])
                {
                    DinosaurActors[i]->Destroy();
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("PerformanceArchitect: Removed %d dinosaurs to enforce cap"), ToRemove);
        }
    }
}

void UEng_PerformanceArchitect::EnforceActorCap()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        if (AllActors.Num() > PerformanceBudget.MaxActors)
        {
            // Remove non-essential actors first
            TArray<FString> EssentialLabels = {TEXT("playerstart"), TEXT("directionallight"), 
                                              TEXT("skylight"), TEXT("skyatmosphere"), TEXT("fog")};
            
            TArray<AActor*> NonEssentialActors;
            for (AActor* Actor : AllActors)
            {
                if (Actor)
                {
                    FString ActorLabel = Actor->GetActorLabel().ToLower();
                    bool IsEssential = false;
                    for (const FString& EssentialLabel : EssentialLabels)
                    {
                        if (ActorLabel.Contains(EssentialLabel))
                        {
                            IsEssential = true;
                            break;
                        }
                    }
                    if (!IsEssential)
                    {
                        NonEssentialActors.Add(Actor);
                    }
                }
            }
            
            int32 ToRemove = AllActors.Num() - PerformanceBudget.MaxActors;
            for (int32 i = 0; i < ToRemove && i < NonEssentialActors.Num(); i++)
            {
                if (NonEssentialActors[i])
                {
                    NonEssentialActors[i]->Destroy();
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("PerformanceArchitect: Removed %d actors to enforce cap"), ToRemove);
        }
    }
}