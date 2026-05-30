#include "Core_PhysicsIntegrationManager.h"
#include "Core_PhysicsComponent.h"
#include "Core_SurvivalPhysics.h"
#include "Core_PlayerMovementComponent.h"
#include "Core_TerrainPhysics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_PhysicsIntegrationManager::UCore_PhysicsIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    IntegrationUpdateRate = 0.1f;
    bEnablePhysicsIntegration = true;
    LastIntegrationUpdate = 0.0f;
}

void UCore_PhysicsIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-discover and register physics components on the same actor
    if (AActor* Owner = GetOwner())
    {
        if (UCore_PhysicsComponent* PhysComp = Owner->FindComponentByClass<UCore_PhysicsComponent>())
        {
            RegisterPhysicsComponent(PhysComp);
        }
        
        if (UCore_SurvivalPhysics* SurvivalComp = Owner->FindComponentByClass<UCore_SurvivalPhysics>())
        {
            RegisterSurvivalComponent(SurvivalComp);
        }
        
        if (UCore_PlayerMovementComponent* MovementComp = Owner->FindComponentByClass<UCore_PlayerMovementComponent>())
        {
            RegisterMovementComponent(MovementComp);
        }
        
        if (UCore_TerrainPhysics* TerrainComp = Owner->FindComponentByClass<UCore_TerrainPhysics>())
        {
            RegisterTerrainComponent(TerrainComp);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: BeginPlay completed"));
}

void UCore_PhysicsIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsIntegration)
    {
        return;
    }
    
    LastIntegrationUpdate += DeltaTime;
    
    if (LastIntegrationUpdate >= IntegrationUpdateRate)
    {
        ValidateComponentReferences();
        UpdateMovementPhysics(DeltaTime);
        UpdateSurvivalPhysics(DeltaTime);
        UpdateTerrainInteraction(DeltaTime);
        
        LastIntegrationUpdate = 0.0f;
    }
}

void UCore_PhysicsIntegrationManager::IntegratePhysicsComponents()
{
    if (!bEnablePhysicsIntegration)
    {
        return;
    }
    
    ValidateComponentReferences();
    CalculateMovementMultipliers();
    ApplyPhysicsModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: Physics components integrated"));
}

void UCore_PhysicsIntegrationManager::UpdateMovementPhysics(float DeltaTime)
{
    if (MovementComponent.IsValid())
    {
        // Apply movement multipliers based on survival state
        if (SurvivalComponent.IsValid())
        {
            float StaminaRatio = 1.0f; // Default if no survival component
            
            // Calculate stamina-based movement modifier
            float MovementMod = FMath::Lerp(0.5f, 1.0f, StaminaRatio);
            IntegrationData.MovementMultiplier = MovementMod * (IntegrationData.bIsInWater ? 0.7f : 1.0f);
            
            // Apply terrain difficulty
            IntegrationData.MovementMultiplier *= (1.0f / FMath::Max(1.0f, IntegrationData.TerrainDifficulty));
        }
    }
}

void UCore_PhysicsIntegrationManager::UpdateSurvivalPhysics(float DeltaTime)
{
    if (SurvivalComponent.IsValid())
    {
        // Calculate stamina drain based on movement and terrain
        float BaseDrain = 1.0f;
        
        if (IntegrationData.bIsOnSteepTerrain)
        {
            BaseDrain *= 1.5f;
        }
        
        if (IntegrationData.bIsInWater)
        {
            BaseDrain *= 1.3f;
        }
        
        IntegrationData.StaminaDrainRate = BaseDrain * IntegrationData.TerrainDifficulty;
    }
}

void UCore_PhysicsIntegrationManager::UpdateTerrainInteraction(float DeltaTime)
{
    if (TerrainComponent.IsValid() && PhysicsComponent.IsValid())
    {
        // Update terrain difficulty based on current surface
        // This would typically involve raycasting or collision detection
        // For now, we'll use a simplified approach
        
        if (AActor* Owner = GetOwner())
        {
            FVector ActorLocation = Owner->GetActorLocation();
            
            // Simple terrain difficulty calculation based on height variation
            // In a real implementation, this would check surface materials, slopes, etc.
            float HeightVariation = FMath::Abs(FMath::Sin(ActorLocation.X * 0.001f) * FMath::Cos(ActorLocation.Y * 0.001f));
            IntegrationData.TerrainDifficulty = 1.0f + (HeightVariation * 0.5f);
            
            // Check for steep terrain (simplified)
            IntegrationData.bIsOnSteepTerrain = HeightVariation > 0.3f;
        }
    }
}

void UCore_PhysicsIntegrationManager::RegisterPhysicsComponent(UCore_PhysicsComponent* Component)
{
    if (Component)
    {
        PhysicsComponent = Component;
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: Physics component registered"));
    }
}

void UCore_PhysicsIntegrationManager::RegisterSurvivalComponent(UCore_SurvivalPhysics* Component)
{
    if (Component)
    {
        SurvivalComponent = Component;
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: Survival component registered"));
    }
}

void UCore_PhysicsIntegrationManager::RegisterMovementComponent(UCore_PlayerMovementComponent* Component)
{
    if (Component)
    {
        MovementComponent = Component;
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: Movement component registered"));
    }
}

void UCore_PhysicsIntegrationManager::RegisterTerrainComponent(UCore_TerrainPhysics* Component)
{
    if (Component)
    {
        TerrainComponent = Component;
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationManager: Terrain component registered"));
    }
}

void UCore_PhysicsIntegrationManager::SetMovementMultiplier(float Multiplier)
{
    IntegrationData.MovementMultiplier = FMath::Clamp(Multiplier, 0.1f, 2.0f);
}

void UCore_PhysicsIntegrationManager::SetStaminaDrainRate(float Rate)
{
    IntegrationData.StaminaDrainRate = FMath::Clamp(Rate, 0.1f, 5.0f);
}

void UCore_PhysicsIntegrationManager::SetTerrainDifficulty(float Difficulty)
{
    IntegrationData.TerrainDifficulty = FMath::Clamp(Difficulty, 0.5f, 3.0f);
}

void UCore_PhysicsIntegrationManager::SetWaterState(bool bInWater)
{
    IntegrationData.bIsInWater = bInWater;
}

void UCore_PhysicsIntegrationManager::SetSteepTerrainState(bool bOnSteepTerrain)
{
    IntegrationData.bIsOnSteepTerrain = bOnSteepTerrain;
}

void UCore_PhysicsIntegrationManager::CalculateMovementMultipliers()
{
    // Base calculation for movement multipliers
    float BaseMultiplier = 1.0f;
    
    // Apply survival state modifiers
    if (SurvivalComponent.IsValid())
    {
        // Reduce movement based on low stamina, hunger, etc.
        BaseMultiplier *= 0.9f; // Simplified for now
    }
    
    // Apply terrain modifiers
    if (IntegrationData.bIsInWater)
    {
        BaseMultiplier *= 0.7f;
    }
    
    if (IntegrationData.bIsOnSteepTerrain)
    {
        BaseMultiplier *= 0.8f;
    }
    
    IntegrationData.MovementMultiplier = BaseMultiplier;
}

void UCore_PhysicsIntegrationManager::ApplyPhysicsModifiers()
{
    // Apply calculated modifiers to registered components
    if (MovementComponent.IsValid())
    {
        // Apply movement multiplier to movement component
        // This would be implemented in the movement component itself
    }
    
    if (SurvivalComponent.IsValid())
    {
        // Apply stamina drain rate to survival component
        // This would be implemented in the survival component itself
    }
}

void UCore_PhysicsIntegrationManager::ValidateComponentReferences()
{
    // Check if any component references have become invalid
    if (PhysicsComponent.IsValid() && !IsValid(PhysicsComponent.Get()))
    {
        PhysicsComponent.Reset();
    }
    
    if (SurvivalComponent.IsValid() && !IsValid(SurvivalComponent.Get()))
    {
        SurvivalComponent.Reset();
    }
    
    if (MovementComponent.IsValid() && !IsValid(MovementComponent.Get()))
    {
        MovementComponent.Reset();
    }
    
    if (TerrainComponent.IsValid() && !IsValid(TerrainComponent.Get()))
    {
        TerrainComponent.Reset();
    }
}