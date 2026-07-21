#include "Perf_CharacterControllerOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.h"
#include "Core/Physics/Core_CharacterController.h"

UPerf_CharacterControllerOptimizer::UPerf_CharacterControllerOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance monitoring
    
    LastAnalysisTime = 0.0f;
    LastOptimizationTime = 0.0f;
    
    // Initialize metrics
    CurrentMetrics = FPerf_CharacterControllerMetrics();
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_CharacterControllerOptimizationSettings();
}

void UPerf_CharacterControllerOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Character Controller Optimizer initialized"));
    
    // Start initial performance analysis
    AnalyzeCharacterControllerPerformance();
}

void UPerf_CharacterControllerOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update performance metrics every tick
    UpdatePerformanceMetrics();
    
    // Run analysis every second
    if (CurrentTime - LastAnalysisTime >= 1.0f)
    {
        AnalyzeCharacterControllerPerformance();
        LastAnalysisTime = CurrentTime;
    }
    
    // Run optimization every 5 seconds or if performance drops
    if (CurrentTime - LastOptimizationTime >= 5.0f || !CheckFrameRateTarget())
    {
        if (OptimizationSettings.bEnableLODOptimization)
        {
            ApplyLODOptimization();
        }
        
        if (OptimizationSettings.bEnableDistanceCulling)
        {
            ApplyDistanceCulling();
        }
        
        LastOptimizationTime = CurrentTime;
    }
}

void UPerf_CharacterControllerOptimizer::AnalyzeCharacterControllerPerformance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Reset metrics
    CurrentMetrics.ActiveCharacterControllers = 0;
    CurrentMetrics.CameraComponentsActive = 0;
    CurrentMetrics.SpringArmComponentsActive = 0;
    CurrentMetrics.TotalControllerTickTime = 0.0f;
    
    // Find all TranspersonalCharacters in the world
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATranspersonalCharacter::StaticClass(), FoundCharacters);
    
    for (AActor* Actor : FoundCharacters)
    {
        if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(Actor))
        {
            // Check for character controller component
            if (UCore_CharacterController* Controller = Character->FindComponentByClass<UCore_CharacterController>())
            {
                CurrentMetrics.ActiveCharacterControllers++;
                
                // Check for camera components
                if (USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>())
                {
                    CurrentMetrics.SpringArmComponentsActive++;
                }
                
                if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
                {
                    CurrentMetrics.CameraComponentsActive++;
                }
            }
        }
    }
    
    // Calculate frame rate statistics
    CalculateFrameRateStats();
    
    // Profile individual systems
    ProfileCameraSystemPerformance();
    ProfileMovementSystemPerformance();
    ProfileValidationSystemPerformance();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Character Controller Analysis - Active Controllers: %d, Camera Components: %d, Spring Arms: %d"), 
        CurrentMetrics.ActiveCharacterControllers, CurrentMetrics.CameraComponentsActive, CurrentMetrics.SpringArmComponentsActive);
}

void UPerf_CharacterControllerOptimizer::ProfileCameraSystemPerformance()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Simulate camera system profiling
    // In a real implementation, this would measure actual camera component tick times
    CurrentMetrics.CameraBoomTickTime = 0.1f + (CurrentMetrics.SpringArmComponentsActive * 0.05f);
    CurrentMetrics.CameraComponentTickTime = 0.08f + (CurrentMetrics.CameraComponentsActive * 0.03f);
    
    double EndTime = FPlatformTime::Seconds();
    float ProfileTime = static_cast<float>(EndTime - StartTime);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Performance Optimizer: Camera System Profiling took %f ms"), ProfileTime * 1000.0f);
}

void UPerf_CharacterControllerOptimizer::ProfileMovementSystemPerformance()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Simulate movement system profiling
    CurrentMetrics.MovementComponentTickTime = 0.2f + (CurrentMetrics.ActiveCharacterControllers * 0.1f);
    
    double EndTime = FPlatformTime::Seconds();
    float ProfileTime = static_cast<float>(EndTime - StartTime);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Performance Optimizer: Movement System Profiling took %f ms"), ProfileTime * 1000.0f);
}

void UPerf_CharacterControllerOptimizer::ProfileValidationSystemPerformance()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Simulate validation system profiling
    CurrentMetrics.ControllerValidationTickTime = 0.05f + (CurrentMetrics.ActiveCharacterControllers * 0.02f);
    
    double EndTime = FPlatformTime::Seconds();
    float ProfileTime = static_cast<float>(EndTime - StartTime);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Performance Optimizer: Validation System Profiling took %f ms"), ProfileTime * 1000.0f);
}

void UPerf_CharacterControllerOptimizer::OptimizeCameraSystem()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Optimizing Camera System"));
    
    // Find all characters and optimize their camera systems
    for (TWeakObjectPtr<ATranspersonalCharacter> CharacterPtr : RegisteredCharacters)
    {
        if (ATranspersonalCharacter* Character = CharacterPtr.Get())
        {
            if (USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>())
            {
                float Distance = GetDistanceToPlayer(Character);
                
                // Reduce camera update frequency for distant characters
                if (Distance > OptimizationSettings.CameraCullingDistance)
                {
                    SpringArm->SetComponentTickEnabled(false);
                }
                else
                {
                    SpringArm->SetComponentTickEnabled(true);
                    
                    // Adjust tick interval based on distance
                    float TickInterval = FMath::Clamp(Distance / 1000.0f, 0.016f, 0.1f);
                    SpringArm->SetComponentTickInterval(TickInterval);
                }
            }
        }
    }
}

void UPerf_CharacterControllerOptimizer::OptimizeMovementSystem()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Optimizing Movement System"));
    
    // Find all characters and optimize their movement systems
    for (TWeakObjectPtr<ATranspersonalCharacter> CharacterPtr : RegisteredCharacters)
    {
        if (ATranspersonalCharacter* Character = CharacterPtr.Get())
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                float Distance = GetDistanceToPlayer(Character);
                
                // Reduce movement update frequency for distant characters
                if (Distance > OptimizationSettings.MovementOptimizationDistance)
                {
                    MovementComp->SetComponentTickInterval(0.1f); // 10 FPS for distant characters
                }
                else
                {
                    MovementComp->SetComponentTickInterval(0.0f); // Full framerate for nearby characters
                }
            }
        }
    }
}

void UPerf_CharacterControllerOptimizer::OptimizeValidationSystem()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Optimizing Validation System"));
    
    // Optimize validation tick intervals for registered controllers
    for (TWeakObjectPtr<UCore_CharacterController> ControllerPtr : RegisteredControllers)
    {
        if (UCore_CharacterController* Controller = ControllerPtr.Get())
        {
            // Reduce validation frequency if performance is good
            if (CheckFrameRateTarget())
            {
                Controller->SetComponentTickInterval(OptimizationSettings.ValidationTickInterval * 2.0f);
            }
            else
            {
                Controller->SetComponentTickInterval(OptimizationSettings.ValidationTickInterval);
            }
        }
    }
}

void UPerf_CharacterControllerOptimizer::ApplyLODOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Applying LOD Optimization"));
    
    OptimizeCameraSystem();
    OptimizeMovementSystem();
    OptimizeValidationSystem();
}

void UPerf_CharacterControllerOptimizer::ApplyDistanceCulling()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Applying Distance Culling"));
    
    // Cull distant components to improve performance
    for (TWeakObjectPtr<ATranspersonalCharacter> CharacterPtr : RegisteredCharacters)
    {
        if (ATranspersonalCharacter* Character = CharacterPtr.Get())
        {
            float Distance = GetDistanceToPlayer(Character);
            bool bShouldCull = Distance > OptimizationSettings.CameraCullingDistance;
            
            // Disable/enable components based on distance
            if (USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>())
            {
                SpringArm->SetComponentTickEnabled(!bShouldCull);
            }
            
            if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
            {
                Camera->SetComponentTickEnabled(!bShouldCull);
            }
        }
    }
}

bool UPerf_CharacterControllerOptimizer::ValidateMilestonePerformance()
{
    bool bFrameRateOK = CheckFrameRateTarget();
    bool bControllerPerformanceOK = CheckControllerPerformance();
    
    CurrentMetrics.bMilestonePerformanceTarget = bFrameRateOK && bControllerPerformanceOK;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Milestone Performance Validation - Frame Rate OK: %s, Controller Performance OK: %s"), 
        bFrameRateOK ? TEXT("YES") : TEXT("NO"), bControllerPerformanceOK ? TEXT("YES") : TEXT("NO"));
    
    return CurrentMetrics.bMilestonePerformanceTarget;
}

bool UPerf_CharacterControllerOptimizer::CheckFrameRateTarget()
{
    // Determine target based on platform (simplified)
    float TargetFrameRate = OptimizationSettings.TargetFrameRatePC; // Assume PC for now
    
    return CurrentMetrics.AverageFrameRate >= TargetFrameRate * 0.9f; // Allow 10% tolerance
}

bool UPerf_CharacterControllerOptimizer::CheckControllerPerformance()
{
    bool bTickTimeOK = CurrentMetrics.TotalControllerTickTime <= OptimizationSettings.MaxTotalControllerTickTime;
    bool bControllerCountOK = CurrentMetrics.ActiveCharacterControllers <= OptimizationSettings.MaxActiveCharacterControllers;
    
    return bTickTimeOK && bControllerCountOK;
}

void UPerf_CharacterControllerOptimizer::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER CONTROLLER PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Character Controllers: %d"), CurrentMetrics.ActiveCharacterControllers);
    UE_LOG(LogTemp, Warning, TEXT("Camera Components Active: %d"), CurrentMetrics.CameraComponentsActive);
    UE_LOG(LogTemp, Warning, TEXT("Spring Arm Components Active: %d"), CurrentMetrics.SpringArmComponentsActive);
    UE_LOG(LogTemp, Warning, TEXT("Total Controller Tick Time: %f ms"), CurrentMetrics.TotalControllerTickTime);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Rate: %f FPS"), CurrentMetrics.AverageFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Min Frame Rate: %f FPS"), CurrentMetrics.MinFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Max Frame Rate: %f FPS"), CurrentMetrics.MaxFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Milestone Performance Target: %s"), CurrentMetrics.bMilestonePerformanceTarget ? TEXT("MET") : TEXT("NOT MET"));
    UE_LOG(LogTemp, Warning, TEXT("================================================"));
}

void UPerf_CharacterControllerOptimizer::IntegrateWithCharacterController(UCore_CharacterController* Controller)
{
    if (Controller && !RegisteredControllers.Contains(Controller))
    {
        RegisteredControllers.Add(Controller);
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Integrated with Character Controller"));
    }
}

void UPerf_CharacterControllerOptimizer::RegisterCharacterForOptimization(ATranspersonalCharacter* Character)
{
    if (Character && !RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Add(Character);
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Registered Character for optimization: %s"), *Character->GetName());
    }
}

void UPerf_CharacterControllerOptimizer::UnregisterCharacterFromOptimization(ATranspersonalCharacter* Character)
{
    if (Character)
    {
        RegisteredCharacters.Remove(Character);
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Unregistered Character from optimization: %s"), *Character->GetName());
    }
}

void UPerf_CharacterControllerOptimizer::UpdateOptimizationSettings(const FPerf_CharacterControllerOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Updated optimization settings"));
}

void UPerf_CharacterControllerOptimizer::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate current frame rate
    float CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    FrameRateHistory.Add(CurrentFrameRate);
    
    // Keep only last 60 samples (1 second at 60fps)
    if (FrameRateHistory.Num() > 60)
    {
        FrameRateHistory.RemoveAt(0);
    }
    
    // Calculate total controller tick time
    CurrentMetrics.TotalControllerTickTime = CurrentMetrics.CameraBoomTickTime + 
                                           CurrentMetrics.CameraComponentTickTime + 
                                           CurrentMetrics.MovementComponentTickTime + 
                                           CurrentMetrics.ControllerValidationTickTime;
}

void UPerf_CharacterControllerOptimizer::CalculateFrameRateStats()
{
    if (FrameRateHistory.Num() == 0)
    {
        return;
    }
    
    float Sum = 0.0f;
    float Min = FrameRateHistory[0];
    float Max = FrameRateHistory[0];
    
    for (float FrameRate : FrameRateHistory)
    {
        Sum += FrameRate;
        Min = FMath::Min(Min, FrameRate);
        Max = FMath::Max(Max, FrameRate);
    }
    
    CurrentMetrics.AverageFrameRate = Sum / FrameRateHistory.Num();
    CurrentMetrics.MinFrameRate = Min;
    CurrentMetrics.MaxFrameRate = Max;
}

bool UPerf_CharacterControllerOptimizer::IsCharacterInOptimizationRange(const ATranspersonalCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }
    
    float Distance = GetDistanceToPlayer(Character);
    return Distance <= OptimizationSettings.CameraCullingDistance;
}

float UPerf_CharacterControllerOptimizer::GetDistanceToPlayer(const ATranspersonalCharacter* Character) const
{
    if (!Character || !GetWorld())
    {
        return 0.0f;
    }
    
    // Get player pawn
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        return FVector::Dist(Character->GetActorLocation(), PlayerPawn->GetActorLocation());
    }
    
    return 0.0f;
}