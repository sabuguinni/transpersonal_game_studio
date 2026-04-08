// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "DinosaurLODSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY(LogDinosaurLOD);

UDinosaurLODSystem::UDinosaurLODSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms by default
    PrimaryComponentTick.bTickEvenWhenPaused = false;
    
    // Initialize default values
    CurrentLODLevel = EDinosaurLODLevel::LOD0_HighDetail;
    PreviousLODLevel = EDinosaurLODLevel::LOD0_HighDetail;
    DistanceToPlayer = 0.0f;
    LastLODUpdate = 0.0f;
    LODUpdateInterval = 0.1f;
    bEnableAdaptiveLOD = true;
    
    // Initialize default LOD settings
    LODSettings = FDinosaurLODSettings();
}

void UDinosaurLODSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    if (AActor* Owner = GetOwner())
    {
        DinosaurMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!DinosaurMesh)
        {
            UE_LOG(LogDinosaurLOD, Warning, TEXT("DinosaurLODSystem: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
    
    // Initialize LOD state
    UpdateLODLevel();
    
    UE_LOG(LogDinosaurLOD, Log, TEXT("DinosaurLODSystem initialized on %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UDinosaurLODSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAdaptiveLOD)
    {
        return;
    }
    
    LastLODUpdate += DeltaTime;
    
    // Update LOD at specified intervals to reduce performance overhead
    if (LastLODUpdate >= LODUpdateInterval)
    {
        LastLODUpdate = 0.0f;
        UpdateLODLevel();
    }
}

void UDinosaurLODSystem::UpdateLODLevel()
{
    // Calculate distance to nearest player
    DistanceToPlayer = GetDistanceToNearestPlayer();
    
    // Determine appropriate LOD level
    EDinosaurLODLevel NewLODLevel = CalculateLODLevel(DistanceToPlayer);
    
    // Apply LOD changes if level has changed
    if (NewLODLevel != CurrentLODLevel)
    {
        PreviousLODLevel = CurrentLODLevel;
        CurrentLODLevel = NewLODLevel;
        
        ApplyLODLevel(NewLODLevel);
        
        // Broadcast LOD change event
        OnLODLevelChanged(PreviousLODLevel, CurrentLODLevel);
        
        UE_LOG(LogDinosaurLOD, Verbose, TEXT("%s LOD changed from %d to %d (Distance: %.1f)"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               static_cast<int32>(PreviousLODLevel),
               static_cast<int32>(CurrentLODLevel),
               DistanceToPlayer);
    }
}

EDinosaurLODLevel UDinosaurLODSystem::CalculateLODLevel(float Distance) const
{
    // Apply distance scale multiplier
    float ScaledDistance = Distance * LODSettings.DistanceScaleMultiplier;
    
    // Determine LOD level based on distance thresholds
    if (ScaledDistance <= LODSettings.LOD0_Distance)
    {
        return EDinosaurLODLevel::LOD0_HighDetail;
    }
    else if (ScaledDistance <= LODSettings.LOD1_Distance)
    {
        return EDinosaurLODLevel::LOD1_MediumDetail;
    }
    else if (ScaledDistance <= LODSettings.LOD2_Distance)
    {
        return EDinosaurLODLevel::LOD2_LowDetail;
    }
    else
    {
        return EDinosaurLODLevel::LOD3_Impostor;
    }
}

void UDinosaurLODSystem::ApplyLODLevel(EDinosaurLODLevel NewLODLevel)
{
    // Apply mesh LOD adjustments
    AdjustMeshLOD(NewLODLevel);
    
    // Apply animation quality adjustments
    AdjustAnimationQuality(NewLODLevel);
    
    // Apply AI complexity adjustments
    AdjustAIComplexity(NewLODLevel);
    
    // Apply physics simulation adjustments
    AdjustPhysicsSimulation(NewLODLevel);
}

void UDinosaurLODSystem::AdjustMeshLOD(EDinosaurLODLevel LODLevel)
{
    if (!DinosaurMesh)
    {
        return;
    }
    
    // Set forced LOD level on the skeletal mesh
    int32 ForcedLOD = static_cast<int32>(LODLevel);
    DinosaurMesh->SetForcedLOD(ForcedLOD + 1); // UE uses 1-based LOD indexing for forced LOD
    
    // Adjust mesh component settings based on LOD
    switch (LODLevel)
    {
        case EDinosaurLODLevel::LOD0_HighDetail:
            DinosaurMesh->SetVisibility(true);
            DinosaurMesh->SetCastShadow(true);
            break;
            
        case EDinosaurLODLevel::LOD1_MediumDetail:
            DinosaurMesh->SetVisibility(true);
            DinosaurMesh->SetCastShadow(true);
            break;
            
        case EDinosaurLODLevel::LOD2_LowDetail:
            DinosaurMesh->SetVisibility(true);
            DinosaurMesh->SetCastShadow(false); // Disable shadows for distant objects
            break;
            
        case EDinosaurLODLevel::LOD3_Impostor:
            DinosaurMesh->SetVisibility(true);
            DinosaurMesh->SetCastShadow(false);
            break;
    }
}

void UDinosaurLODSystem::AdjustAnimationQuality(EDinosaurLODLevel LODLevel)
{
    if (!DinosaurMesh)
    {
        return;
    }
    
    // Get animation instance
    UAnimInstance* AnimInstance = DinosaurMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }
    
    // Adjust animation update rate based on LOD level
    float AnimationUpdateRate = 1.0f;
    
    switch (LODLevel)
    {
        case EDinosaurLODLevel::LOD0_HighDetail:
            AnimationUpdateRate = 1.0f; // Full rate
            DinosaurMesh->bEnableUpdateRateOptimizations = false;
            break;
            
        case EDinosaurLODLevel::LOD1_MediumDetail:
            AnimationUpdateRate = 0.5f; // Half rate (30fps)
            DinosaurMesh->bEnableUpdateRateOptimizations = true;
            break;
            
        case EDinosaurLODLevel::LOD2_LowDetail:
            AnimationUpdateRate = 0.25f; // Quarter rate (15fps)
            DinosaurMesh->bEnableUpdateRateOptimizations = true;
            break;
            
        case EDinosaurLODLevel::LOD3_Impostor:
            AnimationUpdateRate = 0.1f; // Very low rate (6fps)
            DinosaurMesh->bEnableUpdateRateOptimizations = true;
            break;
    }
    
    // Apply animation rate optimization
    DinosaurMesh->SetAnimationUpdateRate(AnimationUpdateRate);
}

void UDinosaurLODSystem::AdjustAIComplexity(EDinosaurLODLevel LODLevel)
{
    bool bAIEnabled = false;
    
    // Determine AI state based on LOD level and settings
    switch (LODLevel)
    {
        case EDinosaurLODLevel::LOD0_HighDetail:
            bAIEnabled = LODSettings.bEnableFullAI_LOD0;
            break;
            
        case EDinosaurLODLevel::LOD1_MediumDetail:
            bAIEnabled = LODSettings.bEnableSimplifiedAI_LOD1;
            break;
            
        case EDinosaurLODLevel::LOD2_LowDetail:
            bAIEnabled = LODSettings.bEnableBasicAI_LOD2;
            break;
            
        case EDinosaurLODLevel::LOD3_Impostor:
            bAIEnabled = !LODSettings.bDisableAI_LOD3;
            break;
    }
    
    // Broadcast AI complexity change event
    OnAIComplexityChanged(LODLevel, bAIEnabled);
    
    // Note: Actual AI enabling/disabling should be handled by the AI system
    // This component just provides the information about what should be done
}

void UDinosaurLODSystem::AdjustPhysicsSimulation(EDinosaurLODLevel LODLevel)
{
    if (!DinosaurMesh)
    {
        return;
    }
    
    // Adjust physics simulation complexity based on LOD
    switch (LODLevel)
    {
        case EDinosaurLODLevel::LOD0_HighDetail:
            // Full physics simulation
            DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            DinosaurMesh->SetNotifyRigidBodyCollision(true);
            break;
            
        case EDinosaurLODLevel::LOD1_MediumDetail:
            // Reduced physics simulation
            DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DinosaurMesh->SetNotifyRigidBodyCollision(false);
            break;
            
        case EDinosaurLODLevel::LOD2_LowDetail:
            // Minimal physics simulation
            DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DinosaurMesh->SetNotifyRigidBodyCollision(false);
            break;
            
        case EDinosaurLODLevel::LOD3_Impostor:
            // No physics simulation for impostors
            DinosaurMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            DinosaurMesh->SetNotifyRigidBodyCollision(false);
            break;
    }
}

float UDinosaurLODSystem::GetDistanceToNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 0.0f;
    }
    
    float MinDistance = FLT_MAX;
    
    // Find all player controllers and calculate distance to nearest player
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(Owner->GetActorLocation(), PC->GetPawn()->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
            }
        }
    }
    
    return (MinDistance == FLT_MAX) ? 0.0f : MinDistance;
}

void UDinosaurLODSystem::SetLODSettings(const FDinosaurLODSettings& NewSettings)
{
    LODSettings = NewSettings;
    
    // Force immediate LOD update with new settings
    if (bEnableAdaptiveLOD)
    {
        ForceUpdateLOD();
    }
    
    UE_LOG(LogDinosaurLOD, Log, TEXT("LOD settings updated for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UDinosaurLODSystem::ForceUpdateLOD()
{
    UpdateLODLevel();
}

void UDinosaurLODSystem::SetAdaptiveLODEnabled(bool bEnabled)
{
    bEnableAdaptiveLOD = bEnabled;
    
    if (!bEnabled)
    {
        // Reset to highest LOD when adaptive LOD is disabled
        CurrentLODLevel = EDinosaurLODLevel::LOD0_HighDetail;
        ApplyLODLevel(CurrentLODLevel);
    }
    
    UE_LOG(LogDinosaurLOD, Log, TEXT("Adaptive LOD %s for %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"),
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}