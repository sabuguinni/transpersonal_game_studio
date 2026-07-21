#include "Core_RagdollController.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCoreRagdollController, Log, All);

UCore_RagdollController::UCore_RagdollController()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for ragdoll updates
	
	// Initialize default settings
	RagdollBlendTime = 0.2f;
	RecoveryBlendTime = 0.5f;
	MinRagdollTime = 2.0f;
	MaxRagdollTime = 10.0f;
	StabilityThreshold = 50.0f;
	RecoveryForceThreshold = 100.0f;
	bAutoRecover = true;
	bUsePhysicsBlending = true;
	
	// Initialize state
	CurrentRagdollState = ECore_RagdollState::Normal;
	RagdollStartTime = 0.0f;
	LastStabilityCheck = 0.0f;
	StabilityCheckInterval = 0.5f;
	
	// Initialize recovery settings
	RecoverySettings.bCanRecoverFromBack = true;
	RecoverySettings.bCanRecoverFromSide = true;
	RecoverySettings.bCanRecoverFromFront = true;
	RecoverySettings.MinRecoveryTime = 1.0f;
	RecoverySettings.MaxRecoveryTime = 5.0f;
	RecoverySettings.RecoverySpeedMultiplier = 1.0f;
}

void UCore_RagdollController::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache references
	CachedWorld = GetWorld();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (OwnerCharacter)
	{
		SkeletalMeshComp = OwnerCharacter->GetMesh();
		CharacterMovement = OwnerCharacter->GetCharacterMovement();
		CapsuleComp = OwnerCharacter->GetCapsuleComponent();
		
		if (SkeletalMeshComp)
		{
			// Store original physics settings
			OriginalCollisionEnabled = SkeletalMeshComp->GetCollisionEnabled();
			OriginalSimulatePhysics = SkeletalMeshComp->IsSimulatingPhysics();
			
			// Cache original animation mode
			OriginalAnimationMode = SkeletalMeshComp->GetAnimationMode();
		}
	}
	
	UE_LOG(LogCoreRagdollController, Log, TEXT("Ragdoll Controller initialized for character: %s"), 
		OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("None"));
}

void UCore_RagdollController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (CurrentRagdollState == ECore_RagdollState::Ragdoll)
	{
		UpdateRagdollState(DeltaTime);
		
		if (bAutoRecover)
		{
			CheckForRecovery(DeltaTime);
		}
	}
	else if (CurrentRagdollState == ECore_RagdollState::Recovering)
	{
		UpdateRecoveryState(DeltaTime);
	}
}

void UCore_RagdollController::EnableRagdoll(bool bImmediate)
{
	if (!SkeletalMeshComp || !OwnerCharacter || CurrentRagdollState == ECore_RagdollState::Ragdoll)
	{
		return;
	}
	
	UE_LOG(LogCoreRagdollController, Log, TEXT("Enabling ragdoll for character: %s"), *OwnerCharacter->GetName());
	
	// Store current state
	PreviousRagdollState = CurrentRagdollState;
	CurrentRagdollState = ECore_RagdollState::Ragdoll;
	RagdollStartTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
	
	// Disable character movement
	if (CharacterMovement)
	{
		CharacterMovement->SetMovementMode(MOVE_None);
		CharacterMovement->StopMovementImmediately();
	}
	
	// Disable capsule collision
	if (CapsuleComp)
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// Enable physics simulation on skeletal mesh
	if (SkeletalMeshComp)
	{
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);
		SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		
		// Apply initial impulse if specified
		if (PendingRagdollImpulse.SizeSquared() > 0.0f)
		{
			SkeletalMeshComp->AddImpulseAtLocation(PendingRagdollImpulse, PendingRagdollImpulseLocation);
			PendingRagdollImpulse = FVector::ZeroVector;
		}
	}
	
	// Start blend timer if not immediate
	if (!bImmediate && RagdollBlendTime > 0.0f)
	{
		// Implement blend logic here if needed
	}
	
	// Broadcast event
	OnRagdollEnabled.Broadcast();
}

void UCore_RagdollController::DisableRagdoll(bool bImmediate)
{
	if (!SkeletalMeshComp || !OwnerCharacter || CurrentRagdollState == ECore_RagdollState::Normal)
	{
		return;
	}
	
	UE_LOG(LogCoreRagdollController, Log, TEXT("Disabling ragdoll for character: %s"), *OwnerCharacter->GetName());
	
	if (bImmediate)
	{
		CompleteRagdollDisable();
	}
	else
	{
		// Start recovery process
		CurrentRagdollState = ECore_RagdollState::Recovering;
		RecoveryStartTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
		
		// Calculate recovery position and rotation
		CalculateRecoveryTransform();
	}
}

void UCore_RagdollController::CompleteRagdollDisable()
{
	if (!SkeletalMeshComp || !OwnerCharacter)
	{
		return;
	}
	
	// Restore physics settings
	SkeletalMeshComp->SetAllBodiesSimulatePhysics(false);
	SkeletalMeshComp->SetCollisionEnabled(OriginalCollisionEnabled);
	SkeletalMeshComp->SetAnimationMode(OriginalAnimationMode);
	
	// Re-enable capsule collision
	if (CapsuleComp)
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	// Re-enable character movement
	if (CharacterMovement)
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
	}
	
	// Position character correctly
	if (CachedWorld)
	{
		FVector NewLocation = CalculateStandingPosition();
		FRotator NewRotation = CalculateStandingRotation();
		
		OwnerCharacter->SetActorLocationAndRotation(NewLocation, NewRotation);
	}
	
	// Update state
	CurrentRagdollState = ECore_RagdollState::Normal;
	
	// Broadcast event
	OnRagdollDisabled.Broadcast();
}

void UCore_RagdollController::ApplyImpulseToRagdoll(const FVector& Impulse, const FVector& Location, FName BoneName)
{
	if (!SkeletalMeshComp)
	{
		return;
	}
	
	if (CurrentRagdollState == ECore_RagdollState::Ragdoll)
	{
		// Apply impulse immediately
		if (BoneName != NAME_None)
		{
			SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location, BoneName);
		}
		else
		{
			SkeletalMeshComp->AddImpulseAtLocation(Impulse, Location);
		}
	}
	else
	{
		// Store impulse for when ragdoll is enabled
		PendingRagdollImpulse = Impulse;
		PendingRagdollImpulseLocation = Location;
	}
}

void UCore_RagdollController::SetRagdollSettings(const FCore_RagdollSettings& NewSettings)
{
	RagdollBlendTime = NewSettings.BlendTime;
	RecoveryBlendTime = NewSettings.RecoveryBlendTime;
	MinRagdollTime = NewSettings.MinRagdollTime;
	MaxRagdollTime = NewSettings.MaxRagdollTime;
	StabilityThreshold = NewSettings.StabilityThreshold;
	bAutoRecover = NewSettings.bAutoRecover;
	bUsePhysicsBlending = NewSettings.bUsePhysicsBlending;
}

void UCore_RagdollController::SetRecoverySettings(const FCore_RecoverySettings& NewSettings)
{
	RecoverySettings = NewSettings;
}

bool UCore_RagdollController::IsRagdollActive() const
{
	return CurrentRagdollState == ECore_RagdollState::Ragdoll;
}

bool UCore_RagdollController::IsRecovering() const
{
	return CurrentRagdollState == ECore_RagdollState::Recovering;
}

float UCore_RagdollController::GetRagdollTime() const
{
	if (CurrentRagdollState != ECore_RagdollState::Ragdoll || !CachedWorld)
	{
		return 0.0f;
	}
	
	return CachedWorld->GetTimeSeconds() - RagdollStartTime;
}

void UCore_RagdollController::UpdateRagdollState(float DeltaTime)
{
	if (!SkeletalMeshComp || !CachedWorld)
	{
		return;
	}
	
	float CurrentTime = CachedWorld->GetTimeSeconds();
	float RagdollTime = CurrentTime - RagdollStartTime;
	
	// Check if maximum ragdoll time exceeded
	if (RagdollTime > MaxRagdollTime)
	{
		UE_LOG(LogCoreRagdollController, Warning, TEXT("Ragdoll time exceeded maximum, forcing recovery"));
		DisableRagdoll(false);
		return;
	}
	
	// Update stability tracking
	if (CurrentTime - LastStabilityCheck > StabilityCheckInterval)
	{
		UpdateStabilityTracking();
		LastStabilityCheck = CurrentTime;
	}
}

void UCore_RagdollController::CheckForRecovery(float DeltaTime)
{
	if (!SkeletalMeshComp || !CachedWorld)
	{
		return;
	}
	
	float RagdollTime = GetRagdollTime();
	
	// Don't recover too quickly
	if (RagdollTime < MinRagdollTime)
	{
		return;
	}
	
	// Check if character is stable enough to recover
	if (IsCharacterStable())
	{
		ECore_RecoveryType RecoveryType = DetermineRecoveryType();
		
		if (CanRecoverFromPosition(RecoveryType))
		{
			UE_LOG(LogCoreRagdollController, Log, TEXT("Auto-recovery triggered after %f seconds"), RagdollTime);
			DisableRagdoll(false);
		}
	}
}

void UCore_RagdollController::UpdateRecoveryState(float DeltaTime)
{
	if (!CachedWorld)
	{
		return;
	}
	
	float RecoveryTime = CachedWorld->GetTimeSeconds() - RecoveryStartTime;
	float RecoveryAlpha = FMath::Clamp(RecoveryTime / RecoveryBlendTime, 0.0f, 1.0f);
	
	if (RecoveryAlpha >= 1.0f)
	{
		// Recovery complete
		CompleteRagdollDisable();
	}
	else
	{
		// Blend between ragdoll and standing position
		BlendToStandingPosition(RecoveryAlpha);
	}
}

bool UCore_RagdollController::IsCharacterStable() const
{
	if (!SkeletalMeshComp)
	{
		return false;
	}
	
	// Check if the character's velocity is low enough
	FVector Velocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
	float Speed = Velocity.Size();
	
	return Speed < StabilityThreshold;
}

ECore_RecoveryType UCore_RagdollController::DetermineRecoveryType() const
{
	if (!SkeletalMeshComp)
	{
		return ECore_RecoveryType::Front;
	}
	
	// Get the character's current orientation
	FVector UpVector = SkeletalMeshComp->GetUpVector();
	FVector WorldUp = FVector::UpVector;
	
	float DotProduct = FVector::DotProduct(UpVector, WorldUp);
	
	if (DotProduct > 0.5f)
	{
		return ECore_RecoveryType::Front; // Character is mostly upright
	}
	else if (DotProduct < -0.5f)
	{
		return ECore_RecoveryType::Back; // Character is upside down
	}
	else
	{
		return ECore_RecoveryType::Side; // Character is on their side
	}
}

bool UCore_RagdollController::CanRecoverFromPosition(ECore_RecoveryType RecoveryType) const
{
	switch (RecoveryType)
	{
		case ECore_RecoveryType::Front:
			return RecoverySettings.bCanRecoverFromFront;
		case ECore_RecoveryType::Back:
			return RecoverySettings.bCanRecoverFromBack;
		case ECore_RecoveryType::Side:
			return RecoverySettings.bCanRecoverFromSide;
		default:
			return false;
	}
}

void UCore_RagdollController::CalculateRecoveryTransform()
{
	if (!SkeletalMeshComp || !OwnerCharacter)
	{
		return;
	}
	
	// Calculate target position and rotation for recovery
	TargetRecoveryLocation = CalculateStandingPosition();
	TargetRecoveryRotation = CalculateStandingRotation();
	
	// Store starting transform for blending
	StartRecoveryLocation = OwnerCharacter->GetActorLocation();
	StartRecoveryRotation = OwnerCharacter->GetActorRotation();
}

FVector UCore_RagdollController::CalculateStandingPosition() const
{
	if (!SkeletalMeshComp || !OwnerCharacter)
	{
		return FVector::ZeroVector;
	}
	
	// Get the pelvis bone location as a reference
	FVector PelvisLocation = SkeletalMeshComp->GetBoneLocation(TEXT("pelvis"));
	
	// Project to ground level
	FVector GroundLocation = PelvisLocation;
	GroundLocation.Z = OwnerCharacter->GetActorLocation().Z;
	
	return GroundLocation;
}

FRotator UCore_RagdollController::CalculateStandingRotation() const
{
	if (!SkeletalMeshComp)
	{
		return FRotator::ZeroRotator;
	}
	
	// Get the character's current forward direction
	FVector ForwardVector = SkeletalMeshComp->GetForwardVector();
	ForwardVector.Z = 0.0f; // Remove pitch
	ForwardVector.Normalize();
	
	return ForwardVector.Rotation();
}

void UCore_RagdollController::BlendToStandingPosition(float Alpha)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Blend location
	FVector BlendedLocation = FMath::Lerp(StartRecoveryLocation, TargetRecoveryLocation, Alpha);
	
	// Blend rotation
	FRotator BlendedRotation = FMath::Lerp(StartRecoveryRotation, TargetRecoveryRotation, Alpha);
	
	// Apply blended transform
	OwnerCharacter->SetActorLocationAndRotation(BlendedLocation, BlendedRotation);
}

void UCore_RagdollController::UpdateStabilityTracking()
{
	if (!SkeletalMeshComp)
	{
		return;
	}
	
	// Track velocity over time for stability analysis
	FVector CurrentVelocity = SkeletalMeshComp->GetPhysicsLinearVelocity();
	VelocityHistory.Add(CurrentVelocity);
	
	// Keep only recent history
	const int32 MaxHistorySize = 10;
	if (VelocityHistory.Num() > MaxHistorySize)
	{
		VelocityHistory.RemoveAt(0);
	}
}

void UCore_RagdollController::ForceRecovery()
{
	if (CurrentRagdollState == ECore_RagdollState::Ragdoll)
	{
		UE_LOG(LogCoreRagdollController, Log, TEXT("Forcing ragdoll recovery"));
		DisableRagdoll(false);
	}
}