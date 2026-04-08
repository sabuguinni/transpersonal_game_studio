#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize animation state
    CurrentTensionLevel = 0.0f;
    CurrentFearLevel = 0.0f;
    bIsBeingHunted = false;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeAnimationDatabases();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAnimationState(DeltaTime);
}

void UAnimationSystemManager::InitializeAnimationDatabases()
{
    // Load Motion Matching databases for different emotional states
    // These will be created by the Animation Blueprint system
    
    // Player databases represent different psychological states
    // Locomotion: Normal, confident movement
    // Caution: Heightened awareness, slower movements
    // Fear: Rapid, jerky movements, frequent looking around
    
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager: Initializing databases for vulnerability-based animation"));
}

void UAnimationSystemManager::UpdateAnimationState(float DeltaTime)
{
    // Calculate tension based on movement and environment
    float NewTension = CalculateMovementTension();
    float NewFear = CalculateEnvironmentalFear();
    
    // Smooth transitions between states
    CurrentTensionLevel = FMath::FInterpTo(CurrentTensionLevel, NewTension, DeltaTime, 2.0f);
    CurrentFearLevel = FMath::FInterpTo(CurrentFearLevel, NewFear, DeltaTime, 1.5f);
    
    // Log state changes for debugging
    if (FMath::Abs(CurrentTensionLevel - NewTension) > 0.1f || FMath::Abs(CurrentFearLevel - NewFear) > 0.1f)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Animation State Update - Tension: %.2f, Fear: %.2f"), CurrentTensionLevel, CurrentFearLevel);
    }
}

float UAnimationSystemManager::CalculateMovementTension() const
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return 0.0f;
    }
    
    // Higher tension when moving faster or changing direction frequently
    float Speed = MovementComp->Velocity.Size();
    float MaxSpeed = MovementComp->MaxWalkSpeed;
    float SpeedRatio = FMath::Clamp(Speed / MaxSpeed, 0.0f, 1.0f);
    
    // Add tension based on acceleration changes (direction changes)
    FVector Acceleration = MovementComp->GetCurrentAcceleration();
    float AccelerationMagnitude = Acceleration.Size() / MovementComp->MaxAcceleration;
    
    return FMath::Clamp(SpeedRatio * 0.7f + AccelerationMagnitude * 0.3f, 0.0f, 1.0f);
}

float UAnimationSystemManager::CalculateEnvironmentalFear() const
{
    // This will be expanded to check for nearby predators, sounds, etc.
    // For now, return base fear level
    float BaseFear = bIsBeingHunted ? 0.8f : 0.2f;
    
    // TODO: Add proximity checks for dinosaurs
    // TODO: Add audio-based fear triggers
    // TODO: Add visibility-based fear (being watched)
    
    return FMath::Clamp(BaseFear, 0.0f, 1.0f);
}

void UAnimationSystemManager::UpdateTensionLevel(float NewTension)
{
    CurrentTensionLevel = FMath::Clamp(NewTension, 0.0f, 1.0f);
}

void UAnimationSystemManager::UpdateFearLevel(float NewFear)
{
    CurrentFearLevel = FMath::Clamp(NewFear, 0.0f, 1.0f);
}

void UAnimationSystemManager::SetHuntedState(bool bHunted)
{
    bIsBeingHunted = bHunted;
    UE_LOG(LogTemp, Log, TEXT("Animation System: Player hunted state changed to %s"), bHunted ? TEXT("TRUE") : TEXT("FALSE"));
}

UPoseSearchDatabase* UAnimationSystemManager::GetActivePlayerDatabase() const
{
    // Select database based on current emotional state
    if (CurrentFearLevel > 0.7f)
    {
        return PlayerFearDatabase;
    }
    else if (CurrentTensionLevel > 0.5f)
    {
        return PlayerCautionDatabase;
    }
    else
    {
        return PlayerLocomotionDatabase;
    }
}

void UAnimationSystemManager::RegisterDinosaurAnimationSet(const FString& SpeciesName, UPoseSearchDatabase* Database, UIKRigDefinition* IKRig)
{
    if (Database)
    {
        DinosaurDatabases.Add(SpeciesName, Database);
    }
    
    if (IKRig)
    {
        DinosaurIKRigs.Add(SpeciesName, IKRig);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Registered animation set for species: %s"), *SpeciesName);
}