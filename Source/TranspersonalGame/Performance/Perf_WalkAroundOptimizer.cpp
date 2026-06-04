#include "Perf_WalkAroundOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UPerf_WalkAroundOptimizer::UPerf_WalkAroundOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10fps for performance monitoring
    
    OptimizationLevel = EPerf_WalkOptimizationLevel::Medium;
    MetricsUpdateRate = 1.0f;
    bEnableAutomaticOptimization = true;
    bLogPerformanceMetrics = false;
    LastMetricsUpdate = 0.0f;
    MaxFrameHistorySize = 60;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameHistorySize);
}

void UPerf_WalkAroundOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial optimization to all characters
    if (bEnableAutomaticOptimization)
    {
        OptimizeAllCharacters();
    }
    
    // Validate WALK AROUND milestone readiness
    bool bMilestoneReady = ValidateWalkAroundMilestone();
    OnMilestoneValidated.Broadcast(bMilestoneReady);
    
    UE_LOG(LogTemp, Log, TEXT("WalkAroundOptimizer initialized with level: %d"), (int32)OptimizationLevel);
}

void UPerf_WalkAroundOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame time history
    UpdateFrameTimeHistory(DeltaTime);
    
    // Update metrics periodically
    if (GetWorld()->GetTimeSeconds() - LastMetricsUpdate >= MetricsUpdateRate)
    {
        UpdatePerformanceMetrics();
        LastMetricsUpdate = GetWorld()->GetTimeSeconds();
        
        if (bLogPerformanceMetrics)
        {
            LogPerformanceData();
        }
        
        // Automatic optimization adjustment based on performance
        if (bEnableAutomaticOptimization)
        {
            if (CurrentMetrics.AverageFrameTime > CurrentMetrics.TargetFrameTime * 1.2f)
            {
                // Performance is poor, increase optimization
                if (OptimizationLevel != EPerf_WalkOptimizationLevel::Ultra)
                {
                    SetOptimizationLevel(static_cast<EPerf_WalkOptimizationLevel>((int32)OptimizationLevel + 1));
                }
            }
            else if (CurrentMetrics.AverageFrameTime < CurrentMetrics.TargetFrameTime * 0.8f)
            {
                // Performance is good, can reduce optimization for better quality
                if (OptimizationLevel != EPerf_WalkOptimizationLevel::Low)
                {
                    SetOptimizationLevel(static_cast<EPerf_WalkOptimizationLevel>((int32)OptimizationLevel - 1));
                }
            }
        }
    }
}

void UPerf_WalkAroundOptimizer::OptimizeCharacterMovement(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    ApplyOptimizationSettings(Character);
    
    // Apply LOD optimization based on distance to player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && PlayerPawn != Character)
    {
        float Distance = FVector::Dist(Character->GetActorLocation(), PlayerPawn->GetActorLocation());
        ApplyLODOptimization(Character, Distance);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized character movement for: %s"), *Character->GetName());
}

void UPerf_WalkAroundOptimizer::OptimizeAllCharacters()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), FoundCharacters);
    
    for (AActor* Actor : FoundCharacters)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            OptimizeCharacterMovement(Character);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d characters"), FoundCharacters.Num());
}

void UPerf_WalkAroundOptimizer::SetOptimizationLevel(EPerf_WalkOptimizationLevel NewLevel)
{
    if (OptimizationLevel != NewLevel)
    {
        OptimizationLevel = NewLevel;
        
        // Update settings based on optimization level
        switch (OptimizationLevel)
        {
            case EPerf_WalkOptimizationLevel::Disabled:
                OptimizationSettings.PhysicsTickRate = 120.0f;
                OptimizationSettings.MovementTickRate = 120.0f;
                OptimizationSettings.bEnableMovementCulling = false;
                OptimizationSettings.bEnablePhysicsLOD = false;
                break;
                
            case EPerf_WalkOptimizationLevel::Low:
                OptimizationSettings.PhysicsTickRate = 90.0f;
                OptimizationSettings.MovementTickRate = 90.0f;
                OptimizationSettings.bEnableMovementCulling = true;
                OptimizationSettings.bEnablePhysicsLOD = true;
                OptimizationSettings.HighDetailDistance = 1500.0f;
                break;
                
            case EPerf_WalkOptimizationLevel::Medium:
                OptimizationSettings.PhysicsTickRate = 60.0f;
                OptimizationSettings.MovementTickRate = 60.0f;
                OptimizationSettings.HighDetailDistance = 1000.0f;
                OptimizationSettings.MediumDetailDistance = 2000.0f;
                break;
                
            case EPerf_WalkOptimizationLevel::High:
                OptimizationSettings.PhysicsTickRate = 45.0f;
                OptimizationSettings.MovementTickRate = 45.0f;
                OptimizationSettings.HighDetailDistance = 800.0f;
                OptimizationSettings.MediumDetailDistance = 1500.0f;
                break;
                
            case EPerf_WalkOptimizationLevel::Ultra:
                OptimizationSettings.PhysicsTickRate = 30.0f;
                OptimizationSettings.MovementTickRate = 30.0f;
                OptimizationSettings.HighDetailDistance = 500.0f;
                OptimizationSettings.MediumDetailDistance = 1000.0f;
                OptimizationSettings.LowDetailDistance = 2000.0f;
                break;
        }
        
        // Re-optimize all characters with new settings
        OptimizeAllCharacters();
        
        OnOptimizationLevelChanged.Broadcast(OptimizationLevel);
        UE_LOG(LogTemp, Log, TEXT("Optimization level changed to: %d"), (int32)OptimizationLevel);
    }
}

void UPerf_WalkAroundOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active characters
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), FoundCharacters);
    CurrentMetrics.ActiveCharacters = FoundCharacters.Num();
    
    // Count moving characters
    CurrentMetrics.MovingCharacters = 0;
    for (AActor* Actor : FoundCharacters)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            if (Character->GetVelocity().Size() > 10.0f)
            {
                CurrentMetrics.MovingCharacters++;
            }
        }
    }
    
    // Calculate average frame time
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    
    // Validate milestone readiness
    CurrentMetrics.bMilestoneReady = ValidateWalkAroundMilestone();
    
    // Update tick times (simplified for now)
    CurrentMetrics.MovementTickTime = CurrentMetrics.AverageFrameTime * 0.3f; // Estimate
    CurrentMetrics.PhysicsTickTime = CurrentMetrics.AverageFrameTime * 0.2f; // Estimate
}

bool UPerf_WalkAroundOptimizer::ValidateWalkAroundMilestone()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for player character with movement component
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone: No player pawn found"));
        return false;
    }
    
    ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone: Player pawn is not a Character"));
        return false;
    }
    
    UCharacterMovementComponent* MovementComp = PlayerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone: No movement component found"));
        return false;
    }
    
    // Check if performance is acceptable
    if (CurrentMetrics.AverageFrameTime > CurrentMetrics.TargetFrameTime * 1.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone: Performance too poor (%.2fms > %.2fms)"), 
               CurrentMetrics.AverageFrameTime, CurrentMetrics.TargetFrameTime * 1.5f);
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WALK AROUND Milestone: READY - Character with movement, performance OK"));
    return true;
}

void UPerf_WalkAroundOptimizer::ApplyLODOptimization(ACharacter* Character, float Distance)
{
    if (!Character || !OptimizationSettings.bEnablePhysicsLOD)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Apply different optimization levels based on distance
    if (Distance <= OptimizationSettings.HighDetailDistance)
    {
        // High detail - full optimization
        MovementComp->SetComponentTickEnabled(true);
        MovementComp->SetComponentTickInterval(1.0f / OptimizationSettings.MovementTickRate);
    }
    else if (Distance <= OptimizationSettings.MediumDetailDistance)
    {
        // Medium detail - reduced tick rate
        MovementComp->SetComponentTickEnabled(true);
        MovementComp->SetComponentTickInterval(1.0f / (OptimizationSettings.MovementTickRate * 0.5f));
    }
    else if (Distance <= OptimizationSettings.LowDetailDistance)
    {
        // Low detail - very low tick rate
        MovementComp->SetComponentTickEnabled(true);
        MovementComp->SetComponentTickInterval(1.0f / (OptimizationSettings.MovementTickRate * 0.25f));
    }
    else
    {
        // Very far - disable movement updates if culling enabled
        if (OptimizationSettings.bEnableMovementCulling)
        {
            MovementComp->SetComponentTickEnabled(false);
        }
    }
}

void UPerf_WalkAroundOptimizer::EnableMovementCulling(bool bEnable)
{
    OptimizationSettings.bEnableMovementCulling = bEnable;
    OptimizeAllCharacters(); // Re-apply optimization
}

void UPerf_WalkAroundOptimizer::SetTargetFrameRate(float TargetFPS)
{
    CurrentMetrics.TargetFrameTime = 1000.0f / TargetFPS; // Convert to milliseconds
    UE_LOG(LogTemp, Log, TEXT("Target frame rate set to %.1f FPS (%.2fms)"), TargetFPS, CurrentMetrics.TargetFrameTime);
}

void UPerf_WalkAroundOptimizer::ApplyOptimizationSettings(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Apply movement settings
    MovementComp->MaxWalkSpeed = OptimizationSettings.MaxWalkSpeed;
    MovementComp->MaxAcceleration = OptimizationSettings.MaxAcceleration;
    MovementComp->BrakingDecelerationWalking = OptimizationSettings.BrakingDeceleration;
    MovementComp->GroundFriction = OptimizationSettings.GroundFriction;
    
    // Set tick rate
    MovementComp->SetComponentTickInterval(1.0f / OptimizationSettings.MovementTickRate);
}

void UPerf_WalkAroundOptimizer::UpdateFrameTimeHistory(float DeltaTime)
{
    float FrameTimeMs = DeltaTime * 1000.0f; // Convert to milliseconds
    
    FrameTimeHistory.Add(FrameTimeMs);
    
    // Keep history size limited
    if (FrameTimeHistory.Num() > MaxFrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UPerf_WalkAroundOptimizer::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 16.67f; // Default to 60fps
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UPerf_WalkAroundOptimizer::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("Performance Metrics - Avg Frame: %.2fms, Target: %.2fms, Characters: %d/%d, Level: %d"),
           CurrentMetrics.AverageFrameTime,
           CurrentMetrics.TargetFrameTime,
           CurrentMetrics.MovingCharacters,
           CurrentMetrics.ActiveCharacters,
           (int32)OptimizationLevel);
}