#include "Perf_CharacterPerformanceOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

UPerf_CharacterPerformanceOptimizer::UPerf_CharacterPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    TargetFrameRate = 60.0f;
    PerformanceTolerancePercent = 10.0f;
    bEnableAutomaticOptimization = true;
    OptimizationCheckInterval = 1.0f;
    CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::High;
    
    // Initialize survival system settings
    SurvivalUpdateFrequency = 1.0f;
    bOptimizeSurvivalSystems = true;
    
    // Initialize movement settings
    MovementLODDistance = 5000.0f;
    bOptimizeMovement = true;
    
    // Initialize physics settings
    bOptimizePhysics = true;
    PhysicsComplexity = EPerf_CharacterOptimizationLevel::High;
    
    // Initialize performance data
    CurrentPerformanceData = FPerf_CharacterPerformanceData();
    OwnerCharacter = nullptr;
    LastOptimizationCheck = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
}

void UPerf_CharacterPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to owner character
    OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwner());
    }
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Character Performance Optimizer initialized for: %s"), *OwnerCharacter->GetName());
        
        // Apply initial optimizations
        if (bEnableAutomaticOptimization)
        {
            ApplyOptimizations();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Character Performance Optimizer: Owner is not a TranspersonalCharacter"));
    }
}

void UPerf_CharacterPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if we need to run optimization
    LastOptimizationCheck += DeltaTime;
    if (LastOptimizationCheck >= OptimizationCheckInterval)
    {
        LastOptimizationCheck = 0.0f;
        
        if (bEnableAutomaticOptimization)
        {
            CheckPerformanceThresholds();
        }
    }
}

FPerf_CharacterPerformanceData UPerf_CharacterPerformanceOptimizer::GetPerformanceData() const
{
    return CurrentPerformanceData;
}

void UPerf_CharacterPerformanceOptimizer::OptimizeCharacterPerformance(EPerf_CharacterOptimizationLevel OptimizationLevel)
{
    CurrentOptimizationLevel = OptimizationLevel;
    ApplyOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Applied optimization level %d"), (int32)OptimizationLevel);
}

void UPerf_CharacterPerformanceOptimizer::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Target frame rate set to %.1f FPS"), TargetFrameRate);
}

bool UPerf_CharacterPerformanceOptimizer::IsPerformanceWithinTarget() const
{
    if (FrameCount == 0)
    {
        return true; // No data yet
    }
    
    float AverageFrameTime = AccumulatedFrameTime / FrameCount;
    float CurrentFPS = 1.0f / AverageFrameTime;
    float TargetFrameTime = 1.0f / TargetFrameRate;
    float ToleranceFrameTime = TargetFrameTime * (1.0f + PerformanceTolerancePercent / 100.0f);
    
    return AverageFrameTime <= ToleranceFrameTime;
}

void UPerf_CharacterPerformanceOptimizer::OptimizeSurvivalSystems(bool bEnableOptimization)
{
    bOptimizeSurvivalSystems = bEnableOptimization;
    
    if (bEnableOptimization)
    {
        OptimizeSurvivalSystemsInternal();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Survival system optimization %s"), 
           bEnableOptimization ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_CharacterPerformanceOptimizer::SetSurvivalUpdateFrequency(float UpdateFrequency)
{
    SurvivalUpdateFrequency = FMath::Clamp(UpdateFrequency, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Survival update frequency set to %.2f Hz"), SurvivalUpdateFrequency);
}

void UPerf_CharacterPerformanceOptimizer::OptimizeMovementSystem(bool bEnableOptimization)
{
    bOptimizeMovement = bEnableOptimization;
    
    if (bEnableOptimization)
    {
        OptimizeMovementSystemInternal();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Movement optimization %s"), 
           bEnableOptimization ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_CharacterPerformanceOptimizer::SetMovementLOD(float Distance)
{
    MovementLODDistance = FMath::Clamp(Distance, 1000.0f, 20000.0f);
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Movement LOD distance set to %.1f"), MovementLODDistance);
}

void UPerf_CharacterPerformanceOptimizer::OptimizePhysicsSystem(bool bEnableOptimization)
{
    bOptimizePhysics = bEnableOptimization;
    
    if (bEnableOptimization)
    {
        OptimizePhysicsSystemInternal();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Physics optimization %s"), 
           bEnableOptimization ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_CharacterPerformanceOptimizer::SetPhysicsComplexity(EPerf_CharacterOptimizationLevel Complexity)
{
    PhysicsComplexity = Complexity;
    
    if (bOptimizePhysics)
    {
        OptimizePhysicsSystemInternal();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Physics complexity set to %d"), (int32)Complexity);
}

void UPerf_CharacterPerformanceOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Accumulate frame time data
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    // Calculate individual system costs
    CurrentPerformanceData.MovementCost = CalculateMovementCost();
    CurrentPerformanceData.SurvivalSystemCost = CalculateSurvivalCost();
    CurrentPerformanceData.PhysicsCost = CalculatePhysicsCost();
    CurrentPerformanceData.AnimationCost = CalculateAnimationCost();
    
    // Calculate total frame cost
    CurrentPerformanceData.TotalFrameCost = 
        CurrentPerformanceData.MovementCost +
        CurrentPerformanceData.SurvivalSystemCost +
        CurrentPerformanceData.PhysicsCost +
        CurrentPerformanceData.AnimationCost;
    
    // Reset accumulation every 60 frames for rolling average
    if (FrameCount >= 60)
    {
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
    }
}

void UPerf_CharacterPerformanceOptimizer::CheckPerformanceThresholds()
{
    if (!IsPerformanceWithinTarget())
    {
        // Performance is below target, increase optimization level
        switch (CurrentOptimizationLevel)
        {
            case EPerf_CharacterOptimizationLevel::High:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::Medium;
                break;
            case EPerf_CharacterOptimizationLevel::Medium:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::Low;
                break;
            case EPerf_CharacterOptimizationLevel::Low:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::Minimal;
                break;
            case EPerf_CharacterOptimizationLevel::Minimal:
                // Already at minimum, can't optimize further
                break;
        }
        
        ApplyOptimizations();
        UE_LOG(LogTemp, Warning, TEXT("Character Performance: Degraded to optimization level %d"), (int32)CurrentOptimizationLevel);
    }
    else if (CurrentOptimizationLevel != EPerf_CharacterOptimizationLevel::High)
    {
        // Performance is good, try to increase quality
        switch (CurrentOptimizationLevel)
        {
            case EPerf_CharacterOptimizationLevel::Minimal:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::Low;
                break;
            case EPerf_CharacterOptimizationLevel::Low:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::Medium;
                break;
            case EPerf_CharacterOptimizationLevel::Medium:
                CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::High;
                break;
            case EPerf_CharacterOptimizationLevel::High:
                // Already at maximum quality
                break;
        }
        
        ApplyOptimizations();
        UE_LOG(LogTemp, Log, TEXT("Character Performance: Improved to optimization level %d"), (int32)CurrentOptimizationLevel);
    }
}

void UPerf_CharacterPerformanceOptimizer::ApplyOptimizations()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    if (bOptimizeSurvivalSystems)
    {
        OptimizeSurvivalSystemsInternal();
    }
    
    if (bOptimizeMovement)
    {
        OptimizeMovementSystemInternal();
    }
    
    if (bOptimizePhysics)
    {
        OptimizePhysicsSystemInternal();
    }
    
    OptimizeAnimationSystemInternal();
}

void UPerf_CharacterPerformanceOptimizer::OptimizeSurvivalSystemsInternal()
{
    // Adjust survival system update frequency based on optimization level
    float OptimizedFrequency = SurvivalUpdateFrequency;
    
    switch (CurrentOptimizationLevel)
    {
        case EPerf_CharacterOptimizationLevel::High:
            OptimizedFrequency = SurvivalUpdateFrequency;
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            OptimizedFrequency = SurvivalUpdateFrequency * 0.75f;
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            OptimizedFrequency = SurvivalUpdateFrequency * 0.5f;
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            OptimizedFrequency = SurvivalUpdateFrequency * 0.25f;
            break;
    }
    
    // Apply the optimized frequency (this would be implemented in the survival system)
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Survival system frequency optimized to %.2f Hz"), OptimizedFrequency);
}

void UPerf_CharacterPerformanceOptimizer::OptimizeMovementSystemInternal()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Adjust movement component settings based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_CharacterOptimizationLevel::High:
            MovementComp->SetNetworkMoveDataContainer(MovementComp->GetNetworkMoveDataContainer());
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            // Reduce movement precision slightly
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            // Further reduce movement calculations
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            // Minimal movement processing
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Movement system optimized for level %d"), (int32)CurrentOptimizationLevel);
}

void UPerf_CharacterPerformanceOptimizer::OptimizePhysicsSystemInternal()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return;
    }
    
    // Adjust physics complexity based on optimization level
    switch (PhysicsComplexity)
    {
        case EPerf_CharacterOptimizationLevel::High:
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Physics system optimized for complexity %d"), (int32)PhysicsComplexity);
}

void UPerf_CharacterPerformanceOptimizer::OptimizeAnimationSystemInternal()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Adjust animation quality based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_CharacterOptimizationLevel::High:
            MeshComp->SetUpdateAnimationInEditor(true);
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            // Reduce animation update frequency
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            // Further reduce animation processing
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            // Minimal animation processing
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Performance: Animation system optimized for level %d"), (int32)CurrentOptimizationLevel);
}

float UPerf_CharacterPerformanceOptimizer::CalculateMovementCost() const
{
    // Estimate movement system cost based on current activity
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return 0.0f;
    }
    
    float MovementCost = 0.1f; // Base cost
    
    // Add cost based on movement state
    if (MovementComp->IsMovingOnGround())
    {
        MovementCost += 0.2f;
    }
    
    if (MovementComp->IsFalling())
    {
        MovementCost += 0.15f;
    }
    
    // Add cost based on velocity
    float VelocityMagnitude = MovementComp->Velocity.Size();
    MovementCost += VelocityMagnitude / 1000.0f; // Normalize to reasonable range
    
    return FMath::Clamp(MovementCost, 0.0f, 2.0f);
}

float UPerf_CharacterPerformanceOptimizer::CalculateSurvivalCost() const
{
    // Estimate survival system cost
    // This would be based on actual survival system implementation
    float SurvivalCost = 0.05f; // Base cost for survival calculations
    
    // Add cost based on update frequency
    SurvivalCost += (SurvivalUpdateFrequency / 10.0f) * 0.1f;
    
    return FMath::Clamp(SurvivalCost, 0.0f, 1.0f);
}

float UPerf_CharacterPerformanceOptimizer::CalculatePhysicsCost() const
{
    // Estimate physics system cost
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return 0.0f;
    }
    
    float PhysicsCost = 0.1f; // Base cost
    
    // Add cost based on collision complexity
    switch (PhysicsComplexity)
    {
        case EPerf_CharacterOptimizationLevel::High:
            PhysicsCost += 0.3f;
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            PhysicsCost += 0.2f;
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            PhysicsCost += 0.1f;
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            PhysicsCost += 0.05f;
            break;
    }
    
    return FMath::Clamp(PhysicsCost, 0.0f, 1.0f);
}

float UPerf_CharacterPerformanceOptimizer::CalculateAnimationCost() const
{
    // Estimate animation system cost
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return 0.0f;
    }
    
    float AnimationCost = 0.15f; // Base cost for skeletal mesh animation
    
    // Add cost based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_CharacterOptimizationLevel::High:
            AnimationCost += 0.25f;
            break;
        case EPerf_CharacterOptimizationLevel::Medium:
            AnimationCost += 0.2f;
            break;
        case EPerf_CharacterOptimizationLevel::Low:
            AnimationCost += 0.15f;
            break;
        case EPerf_CharacterOptimizationLevel::Minimal:
            AnimationCost += 0.1f;
            break;
    }
    
    return FMath::Clamp(AnimationCost, 0.0f, 1.0f);
}