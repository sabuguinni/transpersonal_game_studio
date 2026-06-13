#include "Core_SurvivalPhysicsComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UCore_SurvivalPhysicsComponent::UCore_SurvivalPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    // Initialize default physics state
    CurrentPhysicsState = FCore_SurvivalPhysicsState();
    
    // Set default thresholds
    ColdTemperatureThreshold = 10.0f;
    HotTemperatureThreshold = 35.0f;
    CriticalHungerThreshold = 20.0f;
    LowHungerThreshold = 50.0f;
    CriticalThirstThreshold = 15.0f;
    LowThirstThreshold = 40.0f;
    HighFearThreshold = 70.0f;
    PanicFearThreshold = 90.0f;
    LowStaminaThreshold = 25.0f;
    CriticalStaminaThreshold = 10.0f;
    
    PhysicsUpdateTimer = 0.0f;
    PhysicsUpdateInterval = 0.1f;
}

void UCore_SurvivalPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics state
    ResetPhysicsMultipliers();
}

void UCore_SurvivalPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    PhysicsUpdateTimer += DeltaTime;
    
    // Update physics at specified intervals for performance
    if (PhysicsUpdateTimer >= PhysicsUpdateInterval)
    {
        PhysicsUpdateTimer = 0.0f;
        
        // Apply current physics multipliers to owner
        if (AActor* Owner = GetOwner())
        {
            if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->GetBodyInstance())
                {
                    // Apply overall physics multiplier to mass and damping
                    float OverallMultiplier = GetOverallPhysicsMultiplier();
                    
                    // Modify linear damping based on survival state
                    float BaseDamping = 0.01f;
                    float ModifiedDamping = BaseDamping * (2.0f - OverallMultiplier);
                    PrimComp->GetBodyInstance()->LinearDamping = FMath::Clamp(ModifiedDamping, 0.0f, 10.0f);
                    
                    // Modify angular damping based on survival state
                    float BaseAngularDamping = 0.01f;
                    float ModifiedAngularDamping = BaseAngularDamping * (2.0f - OverallMultiplier);
                    PrimComp->GetBodyInstance()->AngularDamping = FMath::Clamp(ModifiedAngularDamping, 0.0f, 10.0f);
                }
            }
        }
    }
}

void UCore_SurvivalPhysicsComponent::UpdateSurvivalPhysics(float Temperature, float Hunger, float Thirst, float Fear, float Stamina)
{
    // Apply individual survival factor physics
    ApplyTemperaturePhysics(Temperature);
    ApplyHungerPhysics(Hunger);
    ApplyThirstPhysics(Thirst);
    ApplyFearPhysics(Fear);
    ApplyStaminaPhysics(Stamina);
}

float UCore_SurvivalPhysicsComponent::GetMovementPhysicsMultiplier() const
{
    return CurrentPhysicsState.TemperatureMovementMultiplier * 
           CurrentPhysicsState.StaminaPhysicsMultiplier * 
           CurrentPhysicsState.FearPhysicsMultiplier;
}

float UCore_SurvivalPhysicsComponent::GetCollisionPhysicsMultiplier() const
{
    return CurrentPhysicsState.ThirstCollisionMultiplier * 
           CurrentPhysicsState.HungerPhysicsMultiplier;
}

float UCore_SurvivalPhysicsComponent::GetOverallPhysicsMultiplier() const
{
    return (CurrentPhysicsState.TemperatureMovementMultiplier + 
            CurrentPhysicsState.HungerPhysicsMultiplier + 
            CurrentPhysicsState.ThirstCollisionMultiplier + 
            CurrentPhysicsState.FearPhysicsMultiplier + 
            CurrentPhysicsState.StaminaPhysicsMultiplier) / 5.0f;
}

void UCore_SurvivalPhysicsComponent::ApplyTemperaturePhysics(float Temperature)
{
    CurrentPhysicsState.TemperatureMovementMultiplier = CalculateTemperatureEffect(Temperature);
}

void UCore_SurvivalPhysicsComponent::ApplyHungerPhysics(float Hunger)
{
    CurrentPhysicsState.HungerPhysicsMultiplier = CalculateHungerEffect(Hunger);
}

void UCore_SurvivalPhysicsComponent::ApplyThirstPhysics(float Thirst)
{
    CurrentPhysicsState.ThirstCollisionMultiplier = CalculateThirstEffect(Thirst);
}

void UCore_SurvivalPhysicsComponent::ApplyFearPhysics(float Fear)
{
    CurrentPhysicsState.FearPhysicsMultiplier = CalculateFearEffect(Fear);
}

void UCore_SurvivalPhysicsComponent::ApplyStaminaPhysics(float Stamina)
{
    CurrentPhysicsState.StaminaPhysicsMultiplier = CalculateStaminaEffect(Stamina);
}

void UCore_SurvivalPhysicsComponent::ResetPhysicsMultipliers()
{
    CurrentPhysicsState.TemperatureMovementMultiplier = 1.0f;
    CurrentPhysicsState.HungerPhysicsMultiplier = 1.0f;
    CurrentPhysicsState.ThirstCollisionMultiplier = 1.0f;
    CurrentPhysicsState.FearPhysicsMultiplier = 1.0f;
    CurrentPhysicsState.StaminaPhysicsMultiplier = 1.0f;
}

float UCore_SurvivalPhysicsComponent::CalculateTemperatureEffect(float Temperature)
{
    if (Temperature < ColdTemperatureThreshold)
    {
        // Cold slows movement (hypothermia effect)
        float ColdSeverity = (ColdTemperatureThreshold - Temperature) / ColdTemperatureThreshold;
        return FMath::Clamp(1.0f - (ColdSeverity * 0.5f), 0.3f, 1.0f);
    }
    else if (Temperature > HotTemperatureThreshold)
    {
        // Heat reduces physics performance (heat exhaustion)
        float HeatSeverity = (Temperature - HotTemperatureThreshold) / (50.0f - HotTemperatureThreshold);
        return FMath::Clamp(1.0f - (HeatSeverity * 0.4f), 0.4f, 1.0f);
    }
    
    return 1.0f; // Normal temperature range
}

float UCore_SurvivalPhysicsComponent::CalculateHungerEffect(float Hunger)
{
    if (Hunger < CriticalHungerThreshold)
    {
        // Critical hunger severely affects physics
        return 0.3f;
    }
    else if (Hunger < LowHungerThreshold)
    {
        // Low hunger moderately affects physics
        float HungerRatio = Hunger / LowHungerThreshold;
        return FMath::Clamp(0.5f + (HungerRatio * 0.5f), 0.3f, 1.0f);
    }
    
    return 1.0f; // Well-fed
}

float UCore_SurvivalPhysicsComponent::CalculateThirstEffect(float Thirst)
{
    if (Thirst < CriticalThirstThreshold)
    {
        // Critical thirst affects collision detection (dehydration confusion)
        return 0.4f;
    }
    else if (Thirst < LowThirstThreshold)
    {
        // Low thirst moderately affects collision detection
        float ThirstRatio = Thirst / LowThirstThreshold;
        return FMath::Clamp(0.6f + (ThirstRatio * 0.4f), 0.4f, 1.0f);
    }
    
    return 1.0f; // Well-hydrated
}

float UCore_SurvivalPhysicsComponent::CalculateFearEffect(float Fear)
{
    if (Fear > PanicFearThreshold)
    {
        // Panic increases erratic physics (adrenaline but uncontrolled)
        return 1.3f;
    }
    else if (Fear > HighFearThreshold)
    {
        // High fear slightly increases physics response (adrenaline)
        float FearRatio = (Fear - HighFearThreshold) / (PanicFearThreshold - HighFearThreshold);
        return FMath::Clamp(1.0f + (FearRatio * 0.3f), 1.0f, 1.3f);
    }
    
    return 1.0f; // Normal fear levels
}

float UCore_SurvivalPhysicsComponent::CalculateStaminaEffect(float Stamina)
{
    if (Stamina < CriticalStaminaThreshold)
    {
        // Critical stamina severely reduces physics performance
        return 0.2f;
    }
    else if (Stamina < LowStaminaThreshold)
    {
        // Low stamina moderately reduces physics performance
        float StaminaRatio = Stamina / LowStaminaThreshold;
        return FMath::Clamp(0.4f + (StaminaRatio * 0.6f), 0.2f, 1.0f);
    }
    
    return 1.0f; // High stamina
}