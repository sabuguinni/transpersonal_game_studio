#include "MotionMatchingComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

UMotionMatchingComponent::UMotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = EMotionMatchingState::Locomotion;
    StateTransitionTime = 0.3f;
    bDebugMotionMatching = false;
    
    // Estados de sobrevivência
    bStealthMode = false;
    bPanicMode = false;
    bInjuredMode = false;
    InjurySeverity = 0.0f;
    StaminaLevel = 1.0f;
    FearLevel = 0.0f;
    
    StateTransitionTimer = 0.0f;
    PreviousState = EMotionMatchingState::Locomotion;
}

void UMotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultConfigurations();
    
    UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent initialized for: %s"), 
           *GetOwner()->GetName());
}

void UMotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateSurvivalStates(DeltaTime);
    UpdateStateBasedOnConditions();
    
    if (StateTransitionTimer > 0.0f)
    {
        StateTransitionTimer -= DeltaTime;
    }
}

void UMotionMatchingComponent::InitializeDefaultConfigurations()
{
    // Configuração para Locomotion
    FMotionMatchingConfig LocomotionConfig;
    LocomotionConfig.BlendTime = 0.2f;
    LocomotionConfig.SearchThreshold = 0.1f;
    LocomotionConfig.bUseTrajectoryPrediction = true;
    StateConfigurations.Add(EMotionMatchingState::Locomotion, LocomotionConfig);
    
    // Configuração para Stealth
    FMotionMatchingConfig StealthConfig;
    StealthConfig.BlendTime = 0.4f; // Transições mais lentas e suaves
    StealthConfig.SearchThreshold = 0.05f; // Mais preciso
    StealthConfig.bUseTrajectoryPrediction = true;
    StateConfigurations.Add(EMotionMatchingState::Stealth, StealthConfig);
    
    // Configuração para Panic
    FMotionMatchingConfig PanicConfig;
    PanicConfig.BlendTime = 0.1f; // Transições rápidas
    PanicConfig.SearchThreshold = 0.2f; // Menos preciso, mais reativo
    PanicConfig.bUseTrajectoryPrediction = false; // Movimentos erráticos
    StateConfigurations.Add(EMotionMatchingState::Panic, PanicConfig);
    
    // Configuração para Injured
    FMotionMatchingConfig InjuredConfig;
    InjuredConfig.BlendTime = 0.5f; // Transições lentas
    InjuredConfig.SearchThreshold = 0.15f;
    InjuredConfig.bUseTrajectoryPrediction = false; // Movimentos limitados
    StateConfigurations.Add(EMotionMatchingState::Injured, InjuredConfig);
    
    // Configuração para Combat
    FMotionMatchingConfig CombatConfig;
    CombatConfig.BlendTime = 0.15f;
    CombatConfig.SearchThreshold = 0.08f;
    CombatConfig.bUseTrajectoryPrediction = true;
    StateConfigurations.Add(EMotionMatchingState::Combat, CombatConfig);
    
    // Configuração para Interaction
    FMotionMatchingConfig InteractionConfig;
    InteractionConfig.BlendTime = 0.3f;
    InteractionConfig.SearchThreshold = 0.05f;
    InteractionConfig.bUseTrajectoryPrediction = false;
    StateConfigurations.Add(EMotionMatchingState::Interaction, InteractionConfig);
}

void UMotionMatchingComponent::SetMotionMatchingState(EMotionMatchingState NewState)
{
    if (CurrentState != NewState)
    {
        HandleStateTransition(NewState);
    }
}

void UMotionMatchingComponent::HandleStateTransition(EMotionMatchingState NewState)
{
    PreviousState = CurrentState;
    CurrentState = NewState;
    StateTransitionTimer = StateTransitionTime;
    
    if (bDebugMotionMatching)
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching state changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentState);
    }
}

void UMotionMatchingComponent::UpdateStateBasedOnConditions()
{
    // Prioridade dos estados (do mais alto para o mais baixo)
    
    // 1. Injured tem prioridade máxima
    if (bInjuredMode && CurrentState != EMotionMatchingState::Injured)
    {
        SetMotionMatchingState(EMotionMatchingState::Injured);
        return;
    }
    
    // 2. Panic tem segunda prioridade
    if (bPanicMode && CurrentState != EMotionMatchingState::Panic)
    {
        SetMotionMatchingState(EMotionMatchingState::Panic);
        return;
    }
    
    // 3. Stealth quando ativado
    if (bStealthMode && CurrentState != EMotionMatchingState::Stealth)
    {
        SetMotionMatchingState(EMotionMatchingState::Stealth);
        return;
    }
    
    // 4. Retorna para Locomotion se nenhum estado especial estiver ativo
    if (!bInjuredMode && !bPanicMode && !bStealthMode && 
        CurrentState != EMotionMatchingState::Locomotion &&
        CurrentState != EMotionMatchingState::Combat &&
        CurrentState != EMotionMatchingState::Interaction)
    {
        SetMotionMatchingState(EMotionMatchingState::Locomotion);
    }
}

void UMotionMatchingComponent::UpdateSurvivalStates(float DeltaTime)
{
    // Atualização do nível de medo baseado no ambiente
    // Isso seria conectado com o sistema de detecção de dinossauros
    
    // Recuperação gradual da stamina
    if (StaminaLevel < 1.0f && !bPanicMode)
    {
        StaminaLevel = FMath::Clamp(StaminaLevel + (DeltaTime * 0.1f), 0.0f, 1.0f);
    }
    
    // Redução gradual do medo se não há ameaças
    if (FearLevel > 0.0f && !bPanicMode)
    {
        FearLevel = FMath::Clamp(FearLevel - (DeltaTime * 0.2f), 0.0f, 1.0f);
    }
}

void UMotionMatchingComponent::SetDatabase(EMotionMatchingState State, UPoseSearchDatabase* Database)
{
    if (StateConfigurations.Contains(State))
    {
        StateConfigurations[State].Database = Database;
        
        if (bDebugMotionMatching)
        {
            UE_LOG(LogTemp, Warning, TEXT("Database set for state %d"), (int32)State);
        }
    }
}

UPoseSearchDatabase* UMotionMatchingComponent::GetCurrentDatabase() const
{
    if (StateConfigurations.Contains(CurrentState))
    {
        return StateConfigurations[CurrentState].Database;
    }
    return nullptr;
}

void UMotionMatchingComponent::UpdateTrajectory(const FVector& DesiredVelocity, const FVector& DesiredFacing)
{
    // Esta função será chamada pelo Animation Blueprint
    // para atualizar a trajetória do Motion Matching
    
    if (bDebugMotionMatching)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Trajectory updated: Velocity=%s, Facing=%s"), 
               *DesiredVelocity.ToString(), *DesiredFacing.ToString());
    }
}

void UMotionMatchingComponent::SetStealthMode(bool bEnabled)
{
    bStealthMode = bEnabled;
    
    if (bDebugMotionMatching)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stealth mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UMotionMatchingComponent::SetPanicMode(bool bEnabled)
{
    bPanicMode = bEnabled;
    
    if (bEnabled)
    {
        FearLevel = 1.0f;
        StaminaLevel = FMath::Clamp(StaminaLevel - 0.3f, 0.0f, 1.0f);
    }
    
    if (bDebugMotionMatching)
    {
        UE_LOG(LogTemp, Warning, TEXT("Panic mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UMotionMatchingComponent::SetInjuredMode(bool bEnabled, float InjurySeverity)
{
    bInjuredMode = bEnabled;
    this->InjurySeverity = FMath::Clamp(InjurySeverity, 0.0f, 1.0f);
    
    if (bDebugMotionMatching)
    {
        UE_LOG(LogTemp, Warning, TEXT("Injured mode %s with severity %.2f"), 
               bEnabled ? TEXT("enabled") : TEXT("disabled"), this->InjurySeverity);
    }
}