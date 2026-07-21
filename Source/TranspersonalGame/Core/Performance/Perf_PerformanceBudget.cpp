#include "Perf_PerformanceBudget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"

void UPerf_PerformanceBudget::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    TargetFrameRate = 60.0f;
    bPerformanceHUDEnabled = false;
    LastMetricsUpdateTime = 0.0f;
    
    InitializeBudgets();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Budget Manager initialized - Target: %.1f fps"), TargetFrameRate);
}

void UPerf_PerformanceBudget::Deinitialize()
{
    BudgetCategories.Empty();
    Super::Deinitialize();
}

void UPerf_PerformanceBudget::InitializeBudgets()
{
    InitializeDefaultBudgets();
}

void UPerf_PerformanceBudget::InitializeDefaultBudgets()
{
    BudgetCategories.Empty();
    
    // Physics Budget
    FPerf_BudgetCategory PhysicsBudget;
    PhysicsBudget.CategoryName = TEXT("Physics");
    PhysicsBudget.MaxFrameTimeMS = 3.0f; // 18% of 16.67ms frame
    PhysicsBudget.MaxActiveObjects = 500;
    BudgetCategories.Add(TEXT("Physics"), PhysicsBudget);
    
    // Rendering Budget
    FPerf_BudgetCategory RenderingBudget;
    RenderingBudget.CategoryName = TEXT("Rendering");
    RenderingBudget.MaxFrameTimeMS = 8.0f; // 48% of frame
    RenderingBudget.MaxActiveObjects = 2000;
    BudgetCategories.Add(TEXT("Rendering"), RenderingBudget);
    
    // AI Budget
    FPerf_BudgetCategory AIBudget;
    AIBudget.CategoryName = TEXT("AI");
    AIBudget.MaxFrameTimeMS = 2.0f; // 12% of frame
    AIBudget.MaxActiveObjects = 150; // Max dinosaurs
    BudgetCategories.Add(TEXT("AI"), AIBudget);
    
    // Audio Budget
    FPerf_BudgetCategory AudioBudget;
    AudioBudget.CategoryName = TEXT("Audio");
    AudioBudget.MaxFrameTimeMS = 1.0f; // 6% of frame
    AudioBudget.MaxActiveObjects = 64; // Max audio sources
    BudgetCategories.Add(TEXT("Audio"), AudioBudget);
    
    // Gameplay Budget
    FPerf_BudgetCategory GameplayBudget;
    GameplayBudget.CategoryName = TEXT("Gameplay");
    GameplayBudget.MaxFrameTimeMS = 2.67f; // 16% of frame
    GameplayBudget.MaxActiveObjects = 1000;
    BudgetCategories.Add(TEXT("Gameplay"), GameplayBudget);
}

void UPerf_PerformanceBudget::UpdateBudgetCategory(const FString& CategoryName, float FrameTimeMS, int32 ActiveObjects)
{
    if (FPerf_BudgetCategory* Category = BudgetCategories.Find(CategoryName))
    {
        Category->CurrentFrameTimeMS = FrameTimeMS;
        Category->CurrentActiveObjects = ActiveObjects;
        Category->bIsOverBudget = (FrameTimeMS > Category->MaxFrameTimeMS) || (ActiveObjects > Category->MaxActiveObjects);
        
        if (Category->bIsOverBudget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance Budget EXCEEDED - %s: %.2fms/%.2fms, %d/%d objects"), 
                   *CategoryName, FrameTimeMS, Category->MaxFrameTimeMS, ActiveObjects, Category->MaxActiveObjects);
        }
    }
}

bool UPerf_PerformanceBudget::IsCategoryOverBudget(const FString& CategoryName) const
{
    if (const FPerf_BudgetCategory* Category = BudgetCategories.Find(CategoryName))
    {
        return Category->bIsOverBudget;
    }
    return false;
}

FPerf_BudgetCategory UPerf_PerformanceBudget::GetBudgetCategory(const FString& CategoryName) const
{
    if (const FPerf_BudgetCategory* Category = BudgetCategories.Find(CategoryName))
    {
        return *Category;
    }
    return FPerf_BudgetCategory();
}

TArray<FString> UPerf_PerformanceBudget::GetOverBudgetCategories() const
{
    TArray<FString> OverBudgetCategories;
    
    for (const auto& CategoryPair : BudgetCategories)
    {
        if (CategoryPair.Value.bIsOverBudget)
        {
            OverBudgetCategories.Add(CategoryPair.Key);
        }
    }
    
    return OverBudgetCategories;
}

void UPerf_PerformanceBudget::UpdateSystemMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMetricsUpdateTime < 0.1f) // Update every 100ms
    {
        return;
    }
    LastMetricsUpdateTime = CurrentTime;
    
    // Get frame time
    CurrentMetrics.TotalFrameTimeMS = FApp::GetDeltaTime() * 1000.0f;
    
    // Count actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.TotalActorCount = AllActors.Num();
    
    // Count physics actors
    CurrentMetrics.PhysicsActorCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    CurrentMetrics.PhysicsActorCount++;
                }
            }
        }
    }
    
    // Count dinosaurs (actors with dinosaur-related names)
    CurrentMetrics.DinosaurCount = 0;
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
                    CurrentMetrics.DinosaurCount++;
                    break;
                }
            }
        }
    }
    
    // Update individual metrics
    UpdatePhysicsMetrics();
    UpdateRenderingMetrics();
    UpdateAIMetrics();
    UpdateMemoryMetrics();
    
    // Update budget categories
    UpdateBudgetCategory(TEXT("Physics"), CurrentMetrics.PhysicsTimeMS, CurrentMetrics.PhysicsActorCount);
    UpdateBudgetCategory(TEXT("AI"), CurrentMetrics.AITimeMS, CurrentMetrics.DinosaurCount);
    UpdateBudgetCategory(TEXT("Rendering"), CurrentMetrics.RenderingTimeMS, CurrentMetrics.TotalActorCount);
}

void UPerf_PerformanceBudget::UpdatePhysicsMetrics()
{
    // Estimate physics time based on physics actor count
    CurrentMetrics.PhysicsTimeMS = CurrentMetrics.PhysicsActorCount * 0.01f; // 0.01ms per physics actor estimate
}

void UPerf_PerformanceBudget::UpdateRenderingMetrics()
{
    // Estimate rendering time based on total actor count
    CurrentMetrics.RenderingTimeMS = CurrentMetrics.TotalActorCount * 0.003f; // 0.003ms per actor estimate
}

void UPerf_PerformanceBudget::UpdateAIMetrics()
{
    // Estimate AI time based on dinosaur count
    CurrentMetrics.AITimeMS = CurrentMetrics.DinosaurCount * 0.02f; // 0.02ms per dinosaur estimate
}

void UPerf_PerformanceBudget::UpdateMemoryMetrics()
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

FPerf_SystemMetrics UPerf_PerformanceBudget::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

float UPerf_PerformanceBudget::GetTargetFrameRate() const
{
    return TargetFrameRate;
}

void UPerf_PerformanceBudget::SetTargetFrameRate(float NewFrameRate)
{
    TargetFrameRate = FMath::Clamp(NewFrameRate, 30.0f, 120.0f);
    
    // Update budget categories based on new frame rate
    float TargetFrameTimeMS = 1000.0f / TargetFrameRate;
    
    if (FPerf_BudgetCategory* PhysicsBudget = BudgetCategories.Find(TEXT("Physics")))
    {
        PhysicsBudget->MaxFrameTimeMS = TargetFrameTimeMS * 0.18f; // 18% of frame
    }
    
    if (FPerf_BudgetCategory* RenderingBudget = BudgetCategories.Find(TEXT("Rendering")))
    {
        RenderingBudget->MaxFrameTimeMS = TargetFrameTimeMS * 0.48f; // 48% of frame
    }
    
    if (FPerf_BudgetCategory* AIBudget = BudgetCategories.Find(TEXT("AI")))
    {
        AIBudget->MaxFrameTimeMS = TargetFrameTimeMS * 0.12f; // 12% of frame
    }
    
    UE_LOG(LogTemp, Log, TEXT("Target frame rate updated to %.1f fps (%.2fms per frame)"), TargetFrameRate, TargetFrameTimeMS);
}

void UPerf_PerformanceBudget::EnforceActorLimits()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    // Enforce total actor limit (8000)
    if (AllActors.Num() > 8000)
    {
        // Remove non-essential actors
        TArray<AActor*> NonEssentialActors;
        TArray<FString> EssentialLabels = {TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
                                           TEXT("skyatmosphere"), TEXT("fog"), TEXT("player")};
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorLabel = Actor->GetActorLabel().ToLower();
                bool bIsEssential = false;
                
                for (const FString& EssentialLabel : EssentialLabels)
                {
                    if (ActorLabel.Contains(EssentialLabel))
                    {
                        bIsEssential = true;
                        break;
                    }
                }
                
                if (!bIsEssential)
                {
                    NonEssentialActors.Add(Actor);
                }
            }
        }
        
        // Destroy excess non-essential actors
        int32 ActorsToDestroy = AllActors.Num() - 7000; // Leave some buffer
        DestroyExcessActors(NonEssentialActors, NonEssentialActors.Num() - ActorsToDestroy);
        
        UE_LOG(LogTemp, Warning, TEXT("Enforced actor limit: Destroyed %d excess actors"), ActorsToDestroy);
    }
}

void UPerf_PerformanceBudget::ReduceDinosaurCount(int32 MaxDinosaurs)
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> DinosaurActors;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
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
    
    if (DinosaurActors.Num() > MaxDinosaurs)
    {
        DestroyExcessActors(DinosaurActors, MaxDinosaurs);
        UE_LOG(LogTemp, Warning, TEXT("Reduced dinosaur count from %d to %d"), DinosaurActors.Num(), MaxDinosaurs);
    }
}

void UPerf_PerformanceBudget::DestroyExcessActors(const TArray<AActor*>& Actors, int32 MaxCount)
{
    if (Actors.Num() <= MaxCount)
    {
        return;
    }
    
    TArray<AActor*> ActorsToDestroy = Actors;
    
    // Shuffle to randomly select which actors to destroy
    for (int32 i = ActorsToDestroy.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ActorsToDestroy.Swap(i, j);
    }
    
    // Destroy excess actors
    for (int32 i = MaxCount; i < ActorsToDestroy.Num(); i++)
    {
        if (ActorsToDestroy[i] && IsValid(ActorsToDestroy[i]))
        {
            ActorsToDestroy[i]->Destroy();
        }
    }
}

void UPerf_PerformanceBudget::CullDistantActors(float CullDistance)
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);
    
    int32 CulledCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && !Actor->IsA<APawn>())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance > CullDistance)
            {
                Actor->SetActorHiddenInGame(true);
                CulledCount++;
            }
            else
            {
                Actor->SetActorHiddenInGame(false);
            }
        }
    }
    
    if (CulledCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Culled %d distant actors beyond %.0f units"), CulledCount, CullDistance);
    }
}

void UPerf_PerformanceBudget::OptimizePhysicsObjects()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 OptimizedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    // Disable physics for very small objects
                    FVector ActorScale = Actor->GetActorScale3D();
                    if (ActorScale.GetMax() < 0.1f)
                    {
                        PrimComp->SetSimulatePhysics(false);
                        OptimizedCount++;
                    }
                }
            }
        }
    }
    
    if (OptimizedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Optimized %d small physics objects"), OptimizedCount);
    }
}

void UPerf_PerformanceBudget::AdjustQualityBasedOnPerformance()
{
    UpdateSystemMetrics();
    
    float CurrentFrameRate = 1000.0f / CurrentMetrics.TotalFrameTimeMS;
    
    if (CurrentFrameRate < TargetFrameRate * 0.8f) // If below 80% of target
    {
        // Reduce quality
        SetLODDistanceScale(0.8f);
        SetShadowQuality(2);
        CullDistantActors(8000.0f);
        UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f fps) - reducing quality"), CurrentFrameRate);
    }
    else if (CurrentFrameRate > TargetFrameRate * 1.1f) // If above 110% of target
    {
        // Increase quality
        SetLODDistanceScale(1.2f);
        SetShadowQuality(3);
        CullDistantActors(12000.0f);
        UE_LOG(LogTemp, Log, TEXT("Performance above target (%.1f fps) - increasing quality"), CurrentFrameRate);
    }
}

void UPerf_PerformanceBudget::SetLODDistanceScale(float Scale)
{
    if (GetWorld())
    {
        // Apply LOD distance scaling through console command
        FString Command = FString::Printf(TEXT("r.LODDistanceScale %.2f"), Scale);
        GetWorld()->Exec(GetWorld(), *Command);
    }
}

void UPerf_PerformanceBudget::SetShadowQuality(int32 Quality)
{
    if (GetWorld())
    {
        // Apply shadow quality through console command
        FString Command = FString::Printf(TEXT("r.ShadowQuality %d"), FMath::Clamp(Quality, 0, 4));
        GetWorld()->Exec(GetWorld(), *Command);
    }
}

void UPerf_PerformanceBudget::LogPerformanceReport()
{
    UpdateSystemMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms (Target: %.2fms)"), 
           CurrentMetrics.TotalFrameTimeMS, 1000.0f / TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Physics Actors: %d"), CurrentMetrics.PhysicsActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUDGET STATUS ==="));
    for (const auto& CategoryPair : BudgetCategories)
    {
        const FPerf_BudgetCategory& Category = CategoryPair.Value;
        FString Status = Category.bIsOverBudget ? TEXT("OVER BUDGET") : TEXT("OK");
        UE_LOG(LogTemp, Warning, TEXT("%s: %.2f/%.2fms, %d/%d objects [%s]"),
               *Category.CategoryName, Category.CurrentFrameTimeMS, Category.MaxFrameTimeMS,
               Category.CurrentActiveObjects, Category.MaxActiveObjects, *Status);
    }
}

void UPerf_PerformanceBudget::EnablePerformanceHUD(bool bEnable)
{
    bPerformanceHUDEnabled = bEnable;
    
    if (GetWorld())
    {
        if (bEnable)
        {
            GetWorld()->Exec(GetWorld(), TEXT("stat fps"));
            GetWorld()->Exec(GetWorld(), TEXT("stat unit"));
        }
        else
        {
            GetWorld()->Exec(GetWorld(), TEXT("stat none"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance HUD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceBudget::TestPerformanceBudgets()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Performance Budget System..."));
    
    InitializeBudgets();
    UpdateSystemMetrics();
    LogPerformanceReport();
    
    // Test enforcement
    EnforceActorLimits();
    ReduceDinosaurCount(150);
    OptimizePhysicsObjects();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Budget test completed"));
}