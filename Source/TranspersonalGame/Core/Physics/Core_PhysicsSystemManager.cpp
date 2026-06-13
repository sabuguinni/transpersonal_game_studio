#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    PhysicsSettings = FCore_PhysicsSettings();
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics system"));
    
    InitializePhysicsSettings();
    SetupTimers();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing physics system"));
    
    CleanupTimers();
    
    // Clean up active ragdolls
    for (ACharacter* Character : ActiveRagdolls)
    {
        if (IsValid(Character))
        {
            DeactivateRagdoll(Character);
        }
    }
    ActiveRagdolls.Empty();
    
    // Clean up destruction fragments
    CleanupDestructionFragments();
    
    // Clear physics states
    ActorPhysicsStates.Empty();
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSettings()
{
    // Load settings from config if available
    PhysicsSettings.GravityScale = 1.0f;
    PhysicsSettings.AirControl = 0.2f;
    PhysicsSettings.GroundFriction = 8.0f;
    PhysicsSettings.BrakingFriction = 2.0f;
    PhysicsSettings.CollisionResponseMultiplier = 1.0f;
    PhysicsSettings.ImpactDamageThreshold = 500.0f;
    PhysicsSettings.MaxImpactDamage = 100.0f;
    PhysicsSettings.RagdollActivationForce = 1000.0f;
    PhysicsSettings.RagdollDuration = 3.0f;
    PhysicsSettings.bAutoRecoverFromRagdoll = true;
    PhysicsSettings.DestructionThreshold = 2000.0f;
    PhysicsSettings.FragmentLifetime = 10.0f;
    PhysicsSettings.MaxFragments = 20;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics settings initialized"));
}

void UCore_PhysicsSystemManager::SetupTimers()
{
    if (UWorld* World = GetWorld())
    {
        // Set up ragdoll update timer
        World->GetTimerManager().SetTimer(
            RagdollUpdateTimer,
            [this]()
            {
                for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
                {
                    if (IsValid(ActiveRagdolls[i]))
                    {
                        UpdateRagdollState(ActiveRagdolls[i], 0.1f);
                    }
                    else
                    {
                        ActiveRagdolls.RemoveAt(i);
                    }
                }
            },
            0.1f, // Update every 100ms
            true
        );
        
        // Set up fragment cleanup timer
        World->GetTimerManager().SetTimer(
            FragmentCleanupTimer,
            [this]()
            {
                CleanupDestructionFragments();
            },
            5.0f, // Cleanup every 5 seconds
            true
        );
    }
}

void UCore_PhysicsSystemManager::CleanupTimers()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RagdollUpdateTimer);
        World->GetTimerManager().ClearTimer(FragmentCleanupTimer);
    }
}

void UCore_PhysicsSystemManager::SetPhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Apply new settings to all tracked actors
    for (auto& Pair : ActorPhysicsStates)
    {
        if (IsValid(Pair.Key))
        {
            if (ACharacter* Character = Cast<ACharacter>(Pair.Key))
            {
                ApplyPhysicsToCharacter(Character);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics settings updated"));
}

void UCore_PhysicsSystemManager::ResetToDefaultSettings()
{
    PhysicsSettings = FCore_PhysicsSettings();
    SetPhysicsSettings(PhysicsSettings);
}

void UCore_PhysicsSystemManager::ApplyPhysicsToCharacter(ACharacter* Character)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Character has no movement component"));
        return;
    }
    
    // Apply movement physics settings
    MovementComp->GravityScale = PhysicsSettings.GravityScale;
    MovementComp->AirControl = PhysicsSettings.AirControl;
    MovementComp->GroundFriction = PhysicsSettings.GroundFriction;
    MovementComp->BrakingFriction = PhysicsSettings.BrakingFriction;
    
    // Initialize physics state for this actor
    FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Character);
    PhysicsState.CurrentGravityScale = PhysicsSettings.GravityScale;
    PhysicsState.CurrentFriction = PhysicsSettings.GroundFriction;
    
    LogPhysicsEvent(TEXT("ApplyPhysics"), Character, TEXT("Physics settings applied to character"));
}

void UCore_PhysicsSystemManager::UpdateCharacterPhysics(ACharacter* Character, float DeltaTime)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    FCore_PhysicsState* PhysicsState = ActorPhysicsStates.Find(Character);
    if (!PhysicsState)
    {
        // Initialize physics state if not found
        ApplyPhysicsToCharacter(Character);
        PhysicsState = ActorPhysicsStates.Find(Character);
    }
    
    if (PhysicsState && PhysicsState->bIsRagdollActive)
    {
        UpdateRagdollState(Character, DeltaTime);
    }
}

void UCore_PhysicsSystemManager::ModifyPhysicsForSurvivalState(ACharacter* Character, float HealthPercent, float StaminaPercent, float FearLevel)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Calculate survival-based physics modifiers
    float SurvivalModifier = CalculateSurvivalPhysicsModifier(HealthPercent, StaminaPercent, FearLevel);
    
    // Apply modified physics settings
    float ModifiedGravityScale = PhysicsSettings.GravityScale * SurvivalModifier;
    float ModifiedFriction = PhysicsSettings.GroundFriction * SurvivalModifier;
    
    MovementComp->GravityScale = ModifiedGravityScale;
    MovementComp->GroundFriction = ModifiedFriction;
    
    // Update physics state
    FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Character);
    PhysicsState.CurrentGravityScale = ModifiedGravityScale;
    PhysicsState.CurrentFriction = ModifiedFriction;
    
    LogPhysicsEvent(TEXT("SurvivalModify"), Character, 
        FString::Printf(TEXT("Health: %.2f, Stamina: %.2f, Fear: %.2f, Modifier: %.2f"), 
            HealthPercent, StaminaPercent, FearLevel, SurvivalModifier));
}

float UCore_PhysicsSystemManager::CalculateSurvivalPhysicsModifier(float HealthPercent, float StaminaPercent, float FearLevel) const
{
    // Base modifier starts at 1.0 (normal)
    float Modifier = 1.0f;
    
    // Health affects overall physics stability
    if (HealthPercent < 0.5f)
    {
        Modifier *= (0.7f + (HealthPercent * 0.6f)); // Reduced stability when injured
    }
    
    // Stamina affects movement responsiveness
    if (StaminaPercent < 0.3f)
    {
        Modifier *= (0.8f + (StaminaPercent * 0.67f)); // Sluggish when exhausted
    }
    
    // Fear affects control precision
    if (FearLevel > 0.5f)
    {
        Modifier *= (1.2f - (FearLevel * 0.4f)); // Jittery when scared
    }
    
    return FMath::Clamp(Modifier, 0.3f, 1.5f);
}

void UCore_PhysicsSystemManager::HandleCollisionImpact(AActor* Actor, const FVector& ImpactPoint, const FVector& ImpactNormal, float ImpactForce)
{
    if (!ValidateActor(Actor))
    {
        return;
    }
    
    // Update physics state with impact info
    FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Actor);
    PhysicsState.LastImpactForce = ImpactForce;
    
    // Check if impact should trigger ragdoll
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (ShouldActivateRagdoll(ImpactForce))
        {
            ActivateRagdoll(Character, ImpactNormal * ImpactForce);
        }
    }
    
    // Check if impact should trigger destruction
    if (ImpactForce > PhysicsSettings.DestructionThreshold)
    {
        TriggerDestruction(Actor, ImpactPoint, ImpactForce);
    }
    
    LogPhysicsEvent(TEXT("CollisionImpact"), Actor, 
        FString::Printf(TEXT("Force: %.2f, Point: %s"), ImpactForce, *ImpactPoint.ToString()));
}

float UCore_PhysicsSystemManager::CalculateImpactDamage(float ImpactForce) const
{
    if (ImpactForce < PhysicsSettings.ImpactDamageThreshold)
    {
        return 0.0f;
    }
    
    float ExcessForce = ImpactForce - PhysicsSettings.ImpactDamageThreshold;
    float DamageRatio = ExcessForce / PhysicsSettings.ImpactDamageThreshold;
    
    return FMath::Clamp(DamageRatio * PhysicsSettings.MaxImpactDamage, 0.0f, PhysicsSettings.MaxImpactDamage);
}

bool UCore_PhysicsSystemManager::ShouldActivateRagdoll(float ImpactForce) const
{
    return ImpactForce >= PhysicsSettings.RagdollActivationForce;
}

void UCore_PhysicsSystemManager::ActivateRagdoll(ACharacter* Character, const FVector& ImpactForce)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Activate ragdoll physics
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Apply impact force
    MeshComp->AddImpulse(ImpactForce, NAME_None, true);
    
    // Update physics state
    FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Character);
    PhysicsState.bIsRagdollActive = true;
    PhysicsState.RagdollTimeRemaining = PhysicsSettings.RagdollDuration;
    
    // Add to active ragdolls list
    ActiveRagdolls.AddUnique(Character);
    
    LogPhysicsEvent(TEXT("RagdollActivate"), Character, 
        FString::Printf(TEXT("Force: %s, Duration: %.2f"), *ImpactForce.ToString(), PhysicsSettings.RagdollDuration));
}

void UCore_PhysicsSystemManager::DeactivateRagdoll(ACharacter* Character)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Deactivate ragdoll physics
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Update physics state
    FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Character);
    PhysicsState.bIsRagdollActive = false;
    PhysicsState.RagdollTimeRemaining = 0.0f;
    
    // Remove from active ragdolls list
    ActiveRagdolls.Remove(Character);
    
    LogPhysicsEvent(TEXT("RagdollDeactivate"), Character, TEXT("Ragdoll physics disabled"));
}

void UCore_PhysicsSystemManager::UpdateRagdollState(ACharacter* Character, float DeltaTime)
{
    if (!ValidateActor(Character))
    {
        return;
    }
    
    FCore_PhysicsState* PhysicsState = ActorPhysicsStates.Find(Character);
    if (!PhysicsState || !PhysicsState->bIsRagdollActive)
    {
        return;
    }
    
    // Update ragdoll timer
    PhysicsState->RagdollTimeRemaining -= DeltaTime;
    
    // Check if ragdoll should be deactivated
    if (PhysicsSettings.bAutoRecoverFromRagdoll && PhysicsState->RagdollTimeRemaining <= 0.0f)
    {
        DeactivateRagdoll(Character);
    }
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float DestructionForce)
{
    if (!ValidateActor(Actor))
    {
        return;
    }
    
    // Calculate number of fragments based on destruction force
    int32 NumFragments = FMath::Clamp(
        FMath::RoundToInt(DestructionForce / PhysicsSettings.DestructionThreshold * 5.0f),
        3,
        PhysicsSettings.MaxFragments
    );
    
    CreateDestructionFragments(Actor, ImpactPoint, NumFragments);
    
    LogPhysicsEvent(TEXT("Destruction"), Actor, 
        FString::Printf(TEXT("Force: %.2f, Fragments: %d"), DestructionForce, NumFragments));
}

void UCore_PhysicsSystemManager::CreateDestructionFragments(AActor* Actor, const FVector& ImpactPoint, int32 NumFragments)
{
    if (!ValidateActor(Actor) || NumFragments <= 0)
    {
        return;
    }
    
    UWorld* World = Actor->GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get actor bounds for fragment distribution
    FVector Origin, BoxExtent;
    Actor->GetActorBounds(false, Origin, BoxExtent);
    
    // Create fragments
    for (int32 i = 0; i < NumFragments; i++)
    {
        // Random position within actor bounds
        FVector FragmentLocation = Origin + FVector(
            FMath::RandRange(-BoxExtent.X, BoxExtent.X),
            FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
            FMath::RandRange(-BoxExtent.Z, BoxExtent.Z)
        );
        
        // Create fragment actor (simple cube for now)
        AStaticMeshActor* Fragment = World->SpawnActor<AStaticMeshActor>(FragmentLocation, FRotator::ZeroRotator);
        if (Fragment)
        {
            // Set up fragment physics
            UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
            if (FragmentMesh)
            {
                FragmentMesh->SetSimulatePhysics(true);
                FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                
                // Apply random impulse
                FVector RandomImpulse = FVector(
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(200.0f, 800.0f)
                );
                FragmentMesh->AddImpulse(RandomImpulse);
            }
            
            // Track fragment for cleanup
            DestructionFragments.Add(Fragment);
            
            // Update physics state
            FCore_PhysicsState& PhysicsState = ActorPhysicsStates.FindOrAdd(Actor);
            PhysicsState.ActiveFragments++;
        }
    }
}

void UCore_PhysicsSystemManager::CleanupDestructionFragments()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (int32 i = DestructionFragments.Num() - 1; i >= 0; i--)
    {
        AActor* Fragment = DestructionFragments[i];
        if (!IsValid(Fragment))
        {
            DestructionFragments.RemoveAt(i);
            continue;
        }
        
        // Check if fragment has exceeded lifetime
        float FragmentAge = CurrentTime - Fragment->GetActorTimestamp();
        if (FragmentAge > PhysicsSettings.FragmentLifetime)
        {
            Fragment->Destroy();
            DestructionFragments.RemoveAt(i);
        }
    }
}

FCore_PhysicsState UCore_PhysicsSystemManager::GetPhysicsState(AActor* Actor) const
{
    if (const FCore_PhysicsState* State = ActorPhysicsStates.Find(Actor))
    {
        return *State;
    }
    
    return FCore_PhysicsState();
}

bool UCore_PhysicsSystemManager::IsActorInRagdoll(ACharacter* Character) const
{
    if (const FCore_PhysicsState* State = ActorPhysicsStates.Find(Character))
    {
        return State->bIsRagdollActive;
    }
    
    return false;
}

float UCore_PhysicsSystemManager::GetCurrentGravityScale(AActor* Actor) const
{
    if (const FCore_PhysicsState* State = ActorPhysicsStates.Find(Actor))
    {
        return State->CurrentGravityScale;
    }
    
    return PhysicsSettings.GravityScale;
}

void UCore_PhysicsSystemManager::ValidatePhysicsSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Validating physics settings..."));
    
    bool bValid = true;
    
    if (PhysicsSettings.GravityScale <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid gravity scale: %.2f"), PhysicsSettings.GravityScale);
        bValid = false;
    }
    
    if (PhysicsSettings.RagdollDuration <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ragdoll duration: %.2f"), PhysicsSettings.RagdollDuration);
        bValid = false;
    }
    
    if (PhysicsSettings.MaxFragments <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid max fragments: %d"), PhysicsSettings.MaxFragments);
        bValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics settings validation: %s"), bValid ? TEXT("PASSED") : TEXT("FAILED"));
}

void UCore_PhysicsSystemManager::DebugPhysicsState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Tracked actors: %d"), ActorPhysicsStates.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active ragdolls: %d"), ActiveRagdolls.Num());
    UE_LOG(LogTemp, Warning, TEXT("Destruction fragments: %d"), DestructionFragments.Num());
    
    for (const auto& Pair : ActorPhysicsStates)
    {
        if (IsValid(Pair.Key))
        {
            const FCore_PhysicsState& State = Pair.Value;
            UE_LOG(LogTemp, Warning, TEXT("Actor: %s, Ragdoll: %s, Gravity: %.2f, Friction: %.2f"),
                *Pair.Key->GetName(),
                State.bIsRagdollActive ? TEXT("YES") : TEXT("NO"),
                State.CurrentGravityScale,
                State.CurrentFriction);
        }
    }
}

FString UCore_PhysicsSystemManager::GetPhysicsSystemStatus() const
{
    return FString::Printf(TEXT("Physics System - Actors: %d, Ragdolls: %d, Fragments: %d"),
        ActorPhysicsStates.Num(),
        ActiveRagdolls.Num(),
        DestructionFragments.Num());
}

bool UCore_PhysicsSystemManager::ValidateActor(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Invalid actor reference"));
        return false;
    }
    
    return true;
}

void UCore_PhysicsSystemManager::LogPhysicsEvent(const FString& EventName, AActor* Actor, const FString& Details) const
{
    if (IsValid(Actor))
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager [%s]: %s - %s"), 
            *EventName, *Actor->GetName(), *Details);
    }
}