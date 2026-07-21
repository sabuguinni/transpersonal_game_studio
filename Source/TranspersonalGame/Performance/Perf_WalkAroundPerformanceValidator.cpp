#include "Perf_WalkAroundPerformanceValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"

UPerf_WalkAroundPerformanceValidator::UPerf_WalkAroundPerformanceValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS tick rate for performance monitoring
    
    TargetFPS = PC_TARGET_FPS;
    ValidationInterval = 5.0f;
    bAutoOptimize = true;
    bLogPerformanceWarnings = true;
    ValidationTimer = 0.0f;
    bIsValidating = false;
    
    FrameTimeHistory.Reserve(FRAME_HISTORY_SIZE);
}

void UPerf_WalkAroundPerformanceValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_WalkAroundPerformanceValidator: Starting performance validation for WALK AROUND milestone"));
    
    // Auto-start validation
    StartPerformanceValidation();
    
    // Initial validation
    ValidateWalkAroundMilestone();
}

void UPerf_WalkAroundPerformanceValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsValidating)
        return;
    
    UpdatePerformanceMetrics();
    
    ValidationTimer += DeltaTime;
    if (ValidationTimer >= ValidationInterval)
    {
        ValidateRequirements();
        
        if (bAutoOptimize && !CurrentMetrics.bMeetsTargetFPS)
        {
            OptimizeForWalkAround();
        }
        
        if (bLogPerformanceWarnings)
        {
            LogPerformanceData();
        }
        
        ValidationTimer = 0.0f;
    }
}

void UPerf_WalkAroundPerformanceValidator::StartPerformanceValidation()
{
    bIsValidating = true;
    ValidationTimer = 0.0f;
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_WalkAroundPerformanceValidator: Performance validation started"));
}

void UPerf_WalkAroundPerformanceValidator::StopPerformanceValidation()
{
    bIsValidating = false;
    UE_LOG(LogTemp, Log, TEXT("Perf_WalkAroundPerformanceValidator: Performance validation stopped"));
}

FPerf_WalkAroundMetrics UPerf_WalkAroundPerformanceValidator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

FPerf_WalkAroundRequirements UPerf_WalkAroundPerformanceValidator::GetRequirementStatus() const
{
    return RequirementStatus;
}

bool UPerf_WalkAroundPerformanceValidator::ValidateWalkAroundMilestone()
{
    ValidateRequirements();
    
    // Calculate milestone progress
    int32 RequirementsMet = 0;
    int32 TotalRequirements = 5;
    
    if (RequirementStatus.bHasThirdPersonCharacter) RequirementsMet++;
    if (RequirementStatus.bHasLandscapeTerrain) RequirementsMet++;
    if (RequirementStatus.bHasDirectionalLight) RequirementsMet++;
    if (RequirementStatus.bHasSkyAtmosphere) RequirementsMet++;
    if (RequirementStatus.bHasDinosaurMeshes && RequirementStatus.DinosaurMeshCount >= MIN_DINOSAUR_COUNT) RequirementsMet++;
    
    RequirementStatus.MilestoneProgress = (float)RequirementsMet / (float)TotalRequirements;
    
    bool bMilestoneComplete = RequirementsMet == TotalRequirements && CurrentMetrics.bMeetsTargetFPS;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_WalkAroundPerformanceValidator: WALK AROUND milestone progress: %.1f%% (%d/%d requirements met)"), 
           RequirementStatus.MilestoneProgress * 100.0f, RequirementsMet, TotalRequirements);
    
    return bMilestoneComplete;
}

void UPerf_WalkAroundPerformanceValidator::OptimizeForWalkAround()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_WalkAroundPerformanceValidator: Applying WALK AROUND optimizations - Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    
    ApplyWalkAroundOptimizations();
}

void UPerf_WalkAroundPerformanceValidator::RunPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_WalkAroundPerformanceValidator: Running performance test"));
    
    ValidateWalkAroundMilestone();
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Test Results:"));
    UE_LOG(LogTemp, Log, TEXT("- Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("- Frame Time: %.3fms"), CurrentMetrics.AverageFrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("- Active Characters: %d"), CurrentMetrics.ActiveCharacters);
    UE_LOG(LogTemp, Log, TEXT("- Visible Meshes: %d"), CurrentMetrics.VisibleMeshes);
    UE_LOG(LogTemp, Log, TEXT("- Milestone Progress: %.1f%%"), RequirementStatus.MilestoneProgress * 100.0f);
}

void UPerf_WalkAroundPerformanceValidator::UpdatePerformanceMetrics()
{
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    CurrentMetrics.CurrentFPS = 1.0f / CurrentFrameTime;
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > FRAME_HISTORY_SIZE)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average, min, max frame times
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        CurrentMetrics.MinFrameTime = FrameTimeHistory[0];
        CurrentMetrics.MaxFrameTime = FrameTimeHistory[0];
        
        for (float FrameTime : FrameTimeHistory)
        {
            Sum += FrameTime;
            CurrentMetrics.MinFrameTime = FMath::Min(CurrentMetrics.MinFrameTime, FrameTime);
            CurrentMetrics.MaxFrameTime = FMath::Max(CurrentMetrics.MaxFrameTime, FrameTime);
        }
        
        CurrentMetrics.AverageFrameTime = Sum / FrameTimeHistory.Num();
    }
    
    // Check if meets target FPS
    CurrentMetrics.bMeetsTargetFPS = CurrentMetrics.CurrentFPS >= TargetFPS;
    
    // Count active characters and visible meshes
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.ActiveCharacters = 0;
        CurrentMetrics.VisibleMeshes = 0;
        
        for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
        {
            ACharacter* Character = *ActorItr;
            if (Character && !Character->IsPendingKill())
            {
                CurrentMetrics.ActiveCharacters++;
            }
        }
        
        for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AStaticMeshActor* MeshActor = *ActorItr;
            if (MeshActor && !MeshActor->IsPendingKill())
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp && MeshComp->IsVisible())
                {
                    CurrentMetrics.VisibleMeshes++;
                }
            }
        }
    }
    
    // Estimate thread times (simplified)
    CurrentMetrics.GameThreadTime = CurrentFrameTime * 0.6f; // Estimate 60% game thread
    CurrentMetrics.RenderThreadTime = CurrentFrameTime * 0.3f; // Estimate 30% render thread
    CurrentMetrics.PhysicsTickTime = CurrentFrameTime * 0.1f; // Estimate 10% physics
}

void UPerf_WalkAroundPerformanceValidator::ValidateRequirements()
{
    CheckCharacterSystem();
    CheckTerrainSystem();
    CheckLightingSystem();
    CheckDinosaurMeshes();
}

void UPerf_WalkAroundPerformanceValidator::CheckCharacterSystem()
{
    UWorld* World = GetWorld();
    RequirementStatus.bHasThirdPersonCharacter = false;
    
    if (World)
    {
        for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
        {
            ACharacter* Character = *ActorItr;
            if (Character && !Character->IsPendingKill())
            {
                // Check if it's a player character with movement
                if (Character->GetCharacterMovement())
                {
                    RequirementStatus.bHasThirdPersonCharacter = true;
                    break;
                }
            }
        }
    }
}

void UPerf_WalkAroundPerformanceValidator::CheckTerrainSystem()
{
    UWorld* World = GetWorld();
    RequirementStatus.bHasLandscapeTerrain = false;
    
    if (World)
    {
        for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
        {
            ALandscape* Landscape = *ActorItr;
            if (Landscape && !Landscape->IsPendingKill())
            {
                RequirementStatus.bHasLandscapeTerrain = true;
                break;
            }
        }
    }
}

void UPerf_WalkAroundPerformanceValidator::CheckLightingSystem()
{
    UWorld* World = GetWorld();
    RequirementStatus.bHasDirectionalLight = false;
    RequirementStatus.bHasSkyAtmosphere = false;
    
    if (World)
    {
        // Check for directional light
        for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            ADirectionalLight* Light = *ActorItr;
            if (Light && !Light->IsPendingKill())
            {
                RequirementStatus.bHasDirectionalLight = true;
                break;
            }
        }
        
        // Check for sky atmosphere
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsPendingKill())
            {
                USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>();
                if (SkyComp)
                {
                    RequirementStatus.bHasSkyAtmosphere = true;
                    break;
                }
            }
        }
    }
}

void UPerf_WalkAroundPerformanceValidator::CheckDinosaurMeshes()
{
    UWorld* World = GetWorld();
    RequirementStatus.DinosaurMeshCount = 0;
    RequirementStatus.bHasDinosaurMeshes = false;
    
    if (World)
    {
        for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AStaticMeshActor* MeshActor = *ActorItr;
            if (MeshActor && !MeshActor->IsPendingKill())
            {
                FString ActorName = MeshActor->GetName();
                // Check for dinosaur-related names
                if (ActorName.Contains(TEXT("Dinosaur")) || 
                    ActorName.Contains(TEXT("TRex")) || 
                    ActorName.Contains(TEXT("Raptor")) || 
                    ActorName.Contains(TEXT("Brachio")))
                {
                    RequirementStatus.DinosaurMeshCount++;
                }
            }
        }
        
        RequirementStatus.bHasDinosaurMeshes = RequirementStatus.DinosaurMeshCount >= MIN_DINOSAUR_COUNT;
    }
}

void UPerf_WalkAroundPerformanceValidator::ApplyWalkAroundOptimizations()
{
    // Basic optimizations for WALK AROUND milestone
    if (GEngine)
    {
        // Reduce view distance if FPS is too low
        if (CurrentMetrics.CurrentFPS < TargetFPS * 0.8f)
        {
            GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
            UE_LOG(LogTemp, Warning, TEXT("Applied view distance optimization"));
        }
        
        // Reduce shadow quality if needed
        if (CurrentMetrics.CurrentFPS < TargetFPS * 0.6f)
        {
            GEngine->Exec(GetWorld(), TEXT("r.Shadow.MaxResolution 1024"));
            UE_LOG(LogTemp, Warning, TEXT("Applied shadow quality optimization"));
        }
        
        // Reduce foliage density if needed
        if (CurrentMetrics.CurrentFPS < TargetFPS * 0.4f)
        {
            GEngine->Exec(GetWorld(), TEXT("foliage.DensityScale 0.5"));
            UE_LOG(LogTemp, Warning, TEXT("Applied foliage density optimization"));
        }
    }
}

void UPerf_WalkAroundPerformanceValidator::LogPerformanceData()
{
    if (CurrentMetrics.CurrentFPS < TargetFPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_WalkAroundPerformanceValidator: Performance below target! FPS: %.1f (Target: %.1f)"), 
               CurrentMetrics.CurrentFPS, TargetFPS);
    }
    
    if (CurrentMetrics.AverageFrameTime > WARNING_FRAME_TIME)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_WalkAroundPerformanceValidator: High frame time detected: %.3fms"), 
               CurrentMetrics.AverageFrameTime * 1000.0f);
    }
}