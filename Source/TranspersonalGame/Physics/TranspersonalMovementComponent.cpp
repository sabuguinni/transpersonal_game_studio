#include "TranspersonalMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsManager.h"
#include "../Core/ConsciousnessSystem.h"

UTranspersonalMovementComponent::UTranspersonalMovementComponent()
{
    // Initialize default movement values
    BaseWalkSpeed = 600.0f;
    MaxSpeedMultiplier = 2.0f;
    MinSpeedMultiplier = 0.5f;
    ConsciousnessFloatSpeed = 400.0f;
    ConsciousnessAirControl = 1.0f;
    ConsciousnessGravityScale = 1.0f;
    bConsciousnessFloatingEnabled = false;
    PhaseWalkCooldown = 2.0f;
    MinPhaseWalkConsciousness = 0.7f;
    
    // Initialize runtime values
    CurrentConsciousnessLevel = 0.5f;
    CurrentSpeedMultiplier = 1.0f;
    TimeSinceLastPhaseWalk = 0.0f;
    bIsPhaseWalking = false;
    PhaseWalkDuration = 0.5f;
    RemainingPhaseWalkTime = 0.0f;
    LastFrameTime = 0.0f;
    
    // Set default UE5 movement values
    MaxWalkSpeed = BaseWalkSpeed;
    JumpZVelocity = 420.0f;
    AirControl = 0.2f;
    GravityScale = 1.0f;
    
    // Enable tick for consciousness updates
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UTranspersonalMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache system references
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            ConsciousnessSystem = GameInstance->GetSubsystem<UConsciousnessSystem>();
        }
        
        // Find physics manager in the world
        if (AActor* Owner = GetOwner())
        {
            PhysicsManager = Owner->FindComponentByClass<UPhysicsManager>();
        }
    }
    
    // Validate system references
    if (!ConsciousnessSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalMovementComponent: ConsciousnessSystem not found"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalMovementComponent: Initialized with base speed %.2f"), BaseWalkSpeed);
}

void UTranspersonalMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // Performance tracking start
    double StartTime = FPlatformTime::Seconds();
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update consciousness level
    if (ConsciousnessSystem)
    {
        float NewConsciousnessLevel = ConsciousnessSystem->GetCurrentConsciousnessLevel();
        if (FMath::Abs(NewConsciousnessLevel - CurrentConsciousnessLevel) > 0.01f)
        {
            CurrentConsciousnessLevel = NewConsciousnessLevel;
            UpdateMovementFromConsciousness(CurrentConsciousnessLevel);
        }
    }
    
    // Process special movement states
    ProcessConsciousnessFloating(DeltaTime);
    ProcessPhaseWalking(DeltaTime);
    
    // Update cooldowns
    TimeSinceLastPhaseWalk += DeltaTime;
    
    // Performance tracking end
    double EndTime = FPlatformTime::Seconds();
    LastFrameTime = (EndTime - StartTime) * 1000.0f;
    ValidatePerformance(DeltaTime);
}

void UTranspersonalMovementComponent::UpdateMovementFromConsciousness(float ConsciousnessLevel)
{
    // Clamp input
    ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.0f, 1.0f);
    
    // Update all movement parameters
    UpdateSpeed(ConsciousnessLevel);
    UpdateAirControl(ConsciousnessLevel);
    UpdateGravityEffects(ConsciousnessLevel);
}

void UTranspersonalMovementComponent::SetConsciousnessFloating(bool bEnable)
{
    if (bEnable && CurrentConsciousnessLevel < 0.6f)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalMovementComponent: Insufficient consciousness for floating (%.2f < 0.6)"), 
               CurrentConsciousnessLevel);
        return;
    }
    
    bConsciousnessFloatingEnabled = bEnable;
    
    if (bEnable)
    {
        // Enable flying movement mode
        SetMovementMode(MOVE_Flying);
        UE_LOG(LogTemp, Log, TEXT("TranspersonalMovementComponent: Consciousness floating enabled"));
    }
    else
    {
        // Return to walking mode
        SetMovementMode(MOVE_Walking);
        UE_LOG(LogTemp, Log, TEXT("TranspersonalMovementComponent: Consciousness floating disabled"));
    }
}

bool UTranspersonalMovementComponent::AttemptPhaseWalk(FVector Direction)
{
    // Check prerequisites
    if (TimeSinceLastPhaseWalk < PhaseWalkCooldown)
    {
        return false;
    }
    
    if (CurrentConsciousnessLevel < MinPhaseWalkConsciousness)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalMovementComponent: Insufficient consciousness for phase walk (%.2f < %.2f)"), 
               CurrentConsciousnessLevel, MinPhaseWalkConsciousness);
        return false;
    }
    
    if (bIsPhaseWalking)
    {
        return false;
    }
    
    // Start phase walking
    bIsPhaseWalking = true;
    RemainingPhaseWalkTime = PhaseWalkDuration;
    TimeSinceLastPhaseWalk = 0.0f;
    
    // Disable collision temporarily
    if (AActor* Owner = GetOwner())
    {
        if (UCapsuleComponent* CapsuleComp = Owner->FindComponentByClass<UCapsuleComponent>())
        {
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Apply phase walk impulse
    FVector NormalizedDirection = Direction.GetSafeNormal();
    FVector PhaseImpulse = NormalizedDirection * 1000.0f * CurrentConsciousnessLevel;
    AddImpulse(PhaseImpulse, true);
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalMovementComponent: Phase walk initiated"));
    return true;
}

float UTranspersonalMovementComponent::GetConsciousnessSpeedMultiplier() const
{
    return CurrentSpeedMultiplier;
}

bool UTranspersonalMovementComponent::IsConsciousnessFloating() const
{
    return bConsciousnessFloatingEnabled && MovementMode == MOVE_Flying;
}

void UTranspersonalMovementComponent::ApplyConsciousnessImpulse(FVector Direction, float Intensity)
{
    // Clamp intensity
    Intensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    // Scale impulse by consciousness level
    float EffectiveIntensity = Intensity * CurrentConsciousnessLevel;
    FVector NormalizedDirection = Direction.GetSafeNormal();
    FVector FinalImpulse = NormalizedDirection * EffectiveIntensity * 500.0f;
    
    // Apply the impulse
    AddImpulse(FinalImpulse, true);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("TranspersonalMovementComponent: Applied consciousness impulse %.2f"), 
           EffectiveIntensity);
}

void UTranspersonalMovementComponent::UpdateSpeed(float ConsciousnessLevel)
{
    // Calculate speed multiplier based on consciousness
    // Higher consciousness = faster movement (transcendent speed)
    CurrentSpeedMultiplier = FMath::Lerp(MinSpeedMultiplier, MaxSpeedMultiplier, ConsciousnessLevel);
    
    // Apply to movement speeds
    MaxWalkSpeed = BaseWalkSpeed * CurrentSpeedMultiplier;
    MaxFlySpeed = ConsciousnessFloatSpeed * CurrentSpeedMultiplier;
    
    // Also affect acceleration for responsive feel
    MaxAcceleration = 2048.0f * CurrentSpeedMultiplier;
    BrakingDecelerationWalking = 2048.0f * CurrentSpeedMultiplier;
}

void UTranspersonalMovementComponent::UpdateAirControl(float ConsciousnessLevel)
{
    // Higher consciousness = better air control (defying physics)
    ConsciousnessAirControl = FMath::Lerp(0.05f, 0.8f, ConsciousnessLevel);
    AirControl = ConsciousnessAirControl;
    
    // Also affect air control boost
    AirControlBoostMultiplier = 1.0f + (ConsciousnessLevel * 2.0f);
}

void UTranspersonalMovementComponent::UpdateGravityEffects(float ConsciousnessLevel)
{
    // Higher consciousness = reduced gravity effect
    ConsciousnessGravityScale = FMath::Lerp(1.2f, 0.3f, ConsciousnessLevel);
    GravityScale = ConsciousnessGravityScale;
    
    // Affect jump velocity for consciousness-enhanced jumping
    JumpZVelocity = 420.0f * (1.0f + ConsciousnessLevel);
}

void UTranspersonalMovementComponent::ProcessConsciousnessFloating(float DeltaTime)
{
    if (!bConsciousnessFloatingEnabled || MovementMode != MOVE_Flying)
    {
        return;
    }
    
    // Apply consciousness-based floating forces
    if (PhysicsManager)
    {
        PhysicsManager->ApplyConsciousnessForce(GetOwner(), CurrentConsciousnessLevel);
    }
    
    // Add subtle floating oscillation for organic feel
    static float FloatTime = 0.0f;
    FloatTime += DeltaTime;
    
    float OscillationForce = FMath::Sin(FloatTime * 2.0f) * 50.0f * CurrentConsciousnessLevel;
    FVector FloatOscillation = FVector(0.0f, 0.0f, OscillationForce);
    
    AddForce(FloatOscillation);
}

void UTranspersonalMovementComponent::ProcessPhaseWalking(float DeltaTime)
{
    if (!bIsPhaseWalking)
    {
        return;
    }
    
    // Update phase walk timer
    RemainingPhaseWalkTime -= DeltaTime;
    
    // End phase walk when time expires
    if (RemainingPhaseWalkTime <= 0.0f)
    {
        bIsPhaseWalking = false;
        
        // Re-enable collision
        if (AActor* Owner = GetOwner())
        {
            if (UCapsuleComponent* CapsuleComp = Owner->FindComponentByClass<UCapsuleComponent>())
            {
                CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("TranspersonalMovementComponent: Phase walk ended"));
    }
}

void UTranspersonalMovementComponent::ValidatePerformance(float DeltaTime)
{
    const float MaxBudgetMs = 0.5f;
    const float TargetBudgetMs = 0.3f;
    
    if (LastFrameTime > MaxBudgetMs)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalMovementComponent: Performance budget exceeded! %.3fms (Max: %.3fms)"), 
               LastFrameTime, MaxBudgetMs);
    }
}

// Override movement functions for consciousness integration
void UTranspersonalMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    
    // Add consciousness-specific flag processing here if needed
}

float UTranspersonalMovementComponent::GetMaxSpeed() const
{
    float BaseSpeed = Super::GetMaxSpeed();
    return BaseSpeed * CurrentSpeedMultiplier;
}

float UTranspersonalMovementComponent::GetMaxAcceleration() const
{
    float BaseAcceleration = Super::GetMaxAcceleration();
    return BaseAcceleration * CurrentSpeedMultiplier;
}

void UTranspersonalMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
    // Apply consciousness effects before standard walking physics
    if (CurrentConsciousnessLevel > 0.8f)
    {
        // High consciousness allows brief hovering while walking
        FVector HoverForce = FVector(0.0f, 0.0f, 100.0f) * CurrentConsciousnessLevel;
        AddForce(HoverForce);
    }
    
    Super::PhysWalking(deltaTime, Iterations);
}

void UTranspersonalMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
    // Apply consciousness effects during falling
    if (CurrentConsciousnessLevel > 0.6f)
    {
        // High consciousness reduces fall speed
        Velocity.Z = FMath::Max(Velocity.Z, -800.0f * (1.0f - CurrentConsciousnessLevel * 0.5f));
    }
    
    Super::PhysFalling(deltaTime, Iterations);
}