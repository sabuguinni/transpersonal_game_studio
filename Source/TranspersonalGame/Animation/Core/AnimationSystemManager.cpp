#include "AnimationSystemManager.h"
#include "MotionMatching/MotionMatchingComponent.h"
#include "IK/FootIKComponent.h"
#include "Procedural/ProceduralAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    MotionMatchingBlendTime = 0.2f;
    FootIKInterpSpeed = 15.0f;
    TerrainAdaptationStrength = 1.0f;
    
    // Character personality defaults (neutral prehistoric human)
    WalkingCadence = 1.0f;
    PosturalTension = 0.5f;
    MovementWeight = 0.7f;
    GestureFrequency = 0.3f;
    
    // Initialize state
    CurrentSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInCombat = false;
    bIsInteracting = false;
    GroundNormal = FVector::UpVector;
    
    LastUpdateTime = 0.0f;
    PreviousLocation = FVector::ZeroVector;
    PreviousRotation = FRotator::ZeroRotator;
    
    MotionMatchingCost = 0.0f;
    IKUpdateCost = 0.0f;
    ProceduralAnimCost = 0.0f;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAnimationSystems();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Update core animation systems
    CalculateMovementMetrics();
    UpdateAnimationBlending(DeltaTime);
    
    // Update Motion Matching
    if (MotionMatchingComponent)
    {
        float MMStartTime = GetWorld()->GetTimeSeconds();
        UpdateMotionMatchingSearch();
        MotionMatchingCost = GetWorld()->GetTimeSeconds() - MMStartTime;
    }
    
    // Update IK Systems
    if (FootIKComponent)
    {
        float IKStartTime = GetWorld()->GetTimeSeconds();
        UpdateFootIK(DeltaTime);
        IKUpdateCost = GetWorld()->GetTimeSeconds() - IKStartTime;
    }
    
    // Update Procedural Animations
    if (ProceduralAnimComponent)
    {
        float ProcStartTime = GetWorld()->GetTimeSeconds();
        UpdateProceduralBreathing(DeltaTime);
        ProceduralAnimCost = GetWorld()->GetTimeSeconds() - ProcStartTime;
    }
    
    // Apply character personality
    ApplyPersonalityToAnimation();
    
    // Optimize performance if needed
    OptimizePerformance();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnimationSystemManager::InitializeAnimationSystems()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: No owner found"));
        return;
    }
    
    // Initialize Motion Matching Component
    if (!MotionMatchingComponent)
    {
        MotionMatchingComponent = NewObject<UMotionMatchingComponent>(Owner);
        if (MotionMatchingComponent)
        {
            Owner->AddInstanceComponent(MotionMatchingComponent);
            MotionMatchingComponent->RegisterComponent();
            UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized"));
        }
    }
    
    // Initialize Foot IK Component
    if (!FootIKComponent)
    {
        FootIKComponent = NewObject<UFootIKComponent>(Owner);
        if (FootIKComponent)
        {
            Owner->AddInstanceComponent(FootIKComponent);
            FootIKComponent->RegisterComponent();
            FootIKComponent->SetInterpSpeed(FootIKInterpSpeed);
            UE_LOG(LogTemp, Log, TEXT("Foot IK Component initialized"));
        }
    }
    
    // Initialize Procedural Animation Component
    if (!ProceduralAnimComponent)
    {
        ProceduralAnimComponent = NewObject<UProceduralAnimationComponent>(Owner);
        if (ProceduralAnimComponent)
        {
            Owner->AddInstanceComponent(ProceduralAnimComponent);
            ProceduralAnimComponent->RegisterComponent();
            UE_LOG(LogTemp, Log, TEXT("Procedural Animation Component initialized"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager fully initialized"));
}

void UAnimationSystemManager::UpdateMovementState(float Speed, float Direction, const FVector& Normal)
{
    CurrentSpeed = Speed;
    MovementDirection = Direction;
    GroundNormal = Normal;
    
    // Update terrain adaptation based on surface normal
    float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
    UpdateTerrainAdaptation(Normal, SurfaceAngle);
}

void UAnimationSystemManager::SetCombatMode(bool bInCombat)
{
    if (bIsInCombat != bInCombat)
    {
        bIsInCombat = bInCombat;
        
        // Adjust animation parameters for combat
        if (bInCombat)
        {
            MotionMatchingBlendTime = 0.1f; // Faster transitions in combat
            PosturalTension = FMath::Min(PosturalTension + 0.3f, 1.0f);
        }
        else
        {
            MotionMatchingBlendTime = 0.2f; // Normal transition speed
            PosturalTension = FMath::Max(PosturalTension - 0.3f, 0.0f);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat mode changed to: %s"), bInCombat ? TEXT("True") : TEXT("False"));
    }
}

void UAnimationSystemManager::TriggerInteraction(const FString& InteractionType)
{
    bIsInteracting = true;
    
    // Find appropriate interaction montage
    for (UAnimMontage* Montage : InteractionMontages)
    {
        if (Montage && Montage->GetName().Contains(InteractionType))
        {
            // Play the interaction montage
            if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
            {
                if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
                {
                    if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
                    {
                        AnimInstance->Montage_Play(Montage);
                        UE_LOG(LogTemp, Log, TEXT("Playing interaction montage: %s"), *Montage->GetName());
                        break;
                    }
                }
            }
        }
    }
}

void UAnimationSystemManager::UpdateMotionMatchingSearch()
{
    if (!MotionMatchingComponent || !LocomotionDatabase)
        return;
    
    // Update motion matching with current movement state
    MotionMatchingComponent->UpdateSearch(CurrentSpeed, MovementDirection, bIsInCombat);
}

void UAnimationSystemManager::SetLocomotionDatabase(UPoseSearchDatabase* NewDatabase)
{
    LocomotionDatabase = NewDatabase;
    
    if (MotionMatchingComponent)
    {
        MotionMatchingComponent->SetDatabase(NewDatabase);
    }
}

void UAnimationSystemManager::UpdateFootIK(float DeltaTime)
{
    if (!FootIKComponent)
        return;
    
    // Update foot IK with current ground normal and terrain adaptation
    FootIKComponent->UpdateIK(DeltaTime, GroundNormal, TerrainAdaptationStrength);
}

void UAnimationSystemManager::EnableFootIK(bool bEnable)
{
    if (FootIKComponent)
    {
        FootIKComponent->SetEnabled(bEnable);
    }
}

void UAnimationSystemManager::UpdateProceduralBreathing(float DeltaTime)
{
    if (!ProceduralAnimComponent)
        return;
    
    // Update breathing based on movement intensity and combat state
    float BreathingIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 600.0f), // Speed range
        FVector2D(0.5f, 2.0f),   // Breathing intensity range
        CurrentSpeed
    );
    
    if (bIsInCombat)
    {
        BreathingIntensity *= 1.5f; // Heavier breathing in combat
    }
    
    ProceduralAnimComponent->UpdateBreathing(DeltaTime, BreathingIntensity);
}

void UAnimationSystemManager::UpdateProceduralLookAt(const FVector& TargetLocation)
{
    if (!ProceduralAnimComponent)
        return;
    
    ProceduralAnimComponent->UpdateLookAt(TargetLocation);
}

void UAnimationSystemManager::SetCharacterPersonality(float Cadence, float Tension, float Weight, float GestureFreq)
{
    WalkingCadence = FMath::Clamp(Cadence, 0.5f, 2.0f);
    PosturalTension = FMath::Clamp(Tension, 0.0f, 1.0f);
    MovementWeight = FMath::Clamp(Weight, 0.1f, 2.0f);
    GestureFrequency = FMath::Clamp(GestureFreq, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Character personality updated - Cadence: %f, Tension: %f, Weight: %f, Gestures: %f"), 
           WalkingCadence, PosturalTension, MovementWeight, GestureFrequency);
}

void UAnimationSystemManager::ApplyPersonalityToAnimation()
{
    // Apply walking cadence to motion matching
    if (MotionMatchingComponent)
    {
        MotionMatchingComponent->SetPlaybackSpeed(WalkingCadence);
    }
    
    // Apply postural tension to procedural animations
    if (ProceduralAnimComponent)
    {
        ProceduralAnimComponent->SetPosturalTension(PosturalTension);
        ProceduralAnimComponent->SetGestureFrequency(GestureFrequency);
    }
}

void UAnimationSystemManager::UpdateTerrainAdaptation(const FVector& SurfaceNormal, float SurfaceAngle)
{
    // Adjust animation based on terrain slope
    float AdaptationFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 45.0f),    // Angle range
        FVector2D(1.0f, 0.3f),     // Adaptation factor range
        SurfaceAngle
    );
    
    if (FootIKComponent)
    {
        FootIKComponent->SetTerrainAdaptation(AdaptationFactor * TerrainAdaptationStrength);
    }
}

void UAnimationSystemManager::DebugDrawAnimationState()
{
    if (!GetWorld())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Draw speed indicator
    FColor SpeedColor = FColor::Green;
    if (CurrentSpeed > 300.0f) SpeedColor = FColor::Yellow;
    if (CurrentSpeed > 500.0f) SpeedColor = FColor::Red;
    
    DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 200), 
                   FString::Printf(TEXT("Speed: %.1f"), CurrentSpeed), 
                   nullptr, SpeedColor, 0.0f);
    
    // Draw movement direction
    FVector DirectionVector = FVector(
        FMath::Cos(FMath::DegreesToRadians(MovementDirection)),
        FMath::Sin(FMath::DegreesToRadians(MovementDirection)),
        0
    ) * 100.0f;
    
    DrawDebugDirectionalArrow(GetWorld(), OwnerLocation, OwnerLocation + DirectionVector, 
                             50.0f, FColor::Blue, false, 0.0f, 0, 3.0f);
    
    // Draw ground normal
    DrawDebugDirectionalArrow(GetWorld(), OwnerLocation, OwnerLocation + (GroundNormal * 100.0f), 
                             30.0f, FColor::Green, false, 0.0f, 0, 2.0f);
    
    // Draw state indicators
    if (bIsInCombat)
    {
        DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 180), 
                       TEXT("COMBAT"), nullptr, FColor::Red, 0.0f);
    }
    
    if (bIsInteracting)
    {
        DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 160), 
                       TEXT("INTERACTING"), nullptr, FColor::Orange, 0.0f);
    }
}

void UAnimationSystemManager::LogAnimationPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Animation Performance - MM: %.4fms, IK: %.4fms, Proc: %.4fms"), 
           MotionMatchingCost * 1000.0f, IKUpdateCost * 1000.0f, ProceduralAnimCost * 1000.0f);
}

void UAnimationSystemManager::CalculateMovementMetrics()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector CurrentLocation = Character->GetActorLocation();
            FRotator CurrentRotation = Character->GetActorRotation();
            
            // Calculate speed
            FVector Velocity = MovementComp->Velocity;
            CurrentSpeed = Velocity.Size();
            
            // Calculate movement direction relative to character facing
            if (CurrentSpeed > 1.0f)
            {
                FVector ForwardVector = Character->GetActorForwardVector();
                FVector VelocityDirection = Velocity.GetSafeNormal();
                
                float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
                float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
                
                MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
            }
            
            // Update previous values for next frame
            PreviousLocation = CurrentLocation;
            PreviousRotation = CurrentRotation;
        }
    }
}

void UAnimationSystemManager::UpdateAnimationBlending(float DeltaTime)
{
    // Update blend space parameters if available
    if (LocomotionBlendSpace)
    {
        // Speed parameter (0-3 range: idle, walk, run, sprint)
        float SpeedParameter = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 600.0f),
            FVector2D(0.0f, 3.0f),
            CurrentSpeed
        );
        
        // Direction parameter (-180 to 180)
        float DirectionParameter = MovementDirection;
        
        // Apply personality modulation
        SpeedParameter *= WalkingCadence;
        
        // Store parameters for use in Animation Blueprint
        // These would typically be accessed via the Animation Blueprint
    }
}

void UAnimationSystemManager::OptimizePerformance()
{
    float TotalCost = MotionMatchingCost + IKUpdateCost + ProceduralAnimCost;
    
    // If animation systems are taking too long, reduce quality
    if (TotalCost > 0.005f) // 5ms budget
    {
        // Reduce update frequency for expensive systems
        if (MotionMatchingComponent && MotionMatchingCost > 0.002f)
        {
            MotionMatchingComponent->SetUpdateFrequency(0.5f); // Update every other frame
        }
        
        if (FootIKComponent && IKUpdateCost > 0.002f)
        {
            FootIKComponent->SetLOD(1); // Reduce IK precision
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Animation performance optimization triggered - Total cost: %.4fms"), TotalCost * 1000.0f);
    }
}