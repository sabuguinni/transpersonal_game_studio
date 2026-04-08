#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configuração inicial
    SetComponentTickEnabled(true);
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicialização do sistema
    CurrentAnimationState = "Idle";
    TargetStressLevel = StressLevel;
    CurrentStressTransition = StressLevel;
    
    // Log de inicialização
    UE_LOG(LogTemp, Log, TEXT("Animation System Manager initialized for %s"), *GetOwner()->GetName());
    
    // Aplicar variação inicial se temos um seed
    if (MovementVariationSeed != 0)
    {
        ApplyMovementVariation(MovementVariationSeed);
    }
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Actualizar transição de stress
    UpdateStressTransition(DeltaTime);
    
    // Processar Motion Matching query se necessário
    LastMotionMatchingQuery += DeltaTime;
    if (LastMotionMatchingQuery >= MotionMatchingQueryInterval)
    {
        ProcessMotionMatchingQuery();
        LastMotionMatchingQuery = 0.0f;
    }
}

void UAnimationSystemManager::UpdateStressLevel(float NewStressLevel, float TransitionSpeed)
{
    // Clamp do valor
    NewStressLevel = FMath::Clamp(NewStressLevel, 0.0f, 1.0f);
    
    // Se mudança significativa, triggerar evento
    if (FMath::Abs(TargetStressLevel - NewStressLevel) > 0.1f)
    {
        FName NewState = "Calm";
        if (NewStressLevel > 0.7f)
        {
            NewState = "Panic";
        }
        else if (NewStressLevel > 0.4f)
        {
            NewState = "Alert";
        }
        else if (NewStressLevel > 0.1f)
        {
            NewState = "Cautious";
        }
        
        // Triggerar evento se estado mudou
        if (NewState != CurrentAnimationState)
        {
            PreviousAnimationState = CurrentAnimationState;
            CurrentAnimationState = NewState;
            OnAnimationStateChanged.Broadcast(CurrentAnimationState, PreviousAnimationState);
        }
    }
    
    TargetStressLevel = NewStressLevel;
}

void UAnimationSystemManager::UpdateStressTransition(float DeltaTime)
{
    if (FMath::Abs(CurrentStressTransition - TargetStressLevel) > 0.01f)
    {
        CurrentStressTransition = FMath::FInterpTo(CurrentStressTransition, TargetStressLevel, DeltaTime, 2.0f);
        StressLevel = CurrentStressTransition;
    }
}

void UAnimationSystemManager::ProcessMotionMatchingQuery()
{
    // Simular qualidade do match baseado no estado atual
    float MatchQuality = 0.8f; // Base quality
    
    // Ajustar qualidade baseado em stress
    if (StressLevel > 0.5f)
    {
        MatchQuality *= (1.0f - (StressLevel - 0.5f) * 0.3f); // Reduz qualidade com stress alto
    }
    
    // Ajustar por fadiga
    if (FatigueLevel > 0.3f)
    {
        MatchQuality *= (1.0f - FatigueLevel * 0.2f);
    }
    
    // Broadcast do resultado
    OnMotionMatchingQueryComplete.Broadcast(MatchQuality);
}

void UAnimationSystemManager::ApplyMovementVariation(int32 NewSeed)
{
    MovementVariationSeed = NewSeed;
    
    // Usar seed para gerar variações consistentes
    FRandomStream RandomStream(NewSeed);
    
    // Variação na velocidade de interpolação do IK (±20%)
    float IKVariation = RandomStream.FRandRange(0.8f, 1.2f);
    IKInterpolationSpeed *= IKVariation;
    
    // Variação no intervalo de query do Motion Matching (±15%)
    float QueryVariation = RandomStream.FRandRange(0.85f, 1.15f);
    MotionMatchingQueryInterval *= QueryVariation;
    
    // Log da variação aplicada
    UE_LOG(LogTemp, Log, TEXT("Applied movement variation with seed %d: IK Speed %.2f, Query Interval %.3f"), 
           NewSeed, IKInterpolationSpeed, MotionMatchingQueryInterval);
}

UPoseSearchDatabase* UAnimationSystemManager::GetCurrentDatabase() const
{
    return SelectDatabaseByState();
}

UPoseSearchDatabase* UAnimationSystemManager::SelectDatabaseByState() const
{
    // Seleccionar database baseado no estado de stress
    if (StressLevel > 0.6f && StressStateDatabase)
    {
        return StressStateDatabase;
    }
    
    // Se temos interações ambientais activas, usar essa database
    if (EnvironmentInteractionDatabase)
    {
        // TODO: Adicionar lógica para detectar interações ambientais
    }
    
    // Default para locomotion
    return PrimaryLocomotionDatabase;
}

void UAnimationSystemManager::ForceTransitionToState(FName StateName)
{
    if (StateName != CurrentAnimationState)
    {
        PreviousAnimationState = CurrentAnimationState;
        CurrentAnimationState = StateName;
        OnAnimationStateChanged.Broadcast(CurrentAnimationState, PreviousAnimationState);
        
        UE_LOG(LogTemp, Log, TEXT("Forced transition from %s to %s"), 
               *PreviousAnimationState.ToString(), *CurrentAnimationState.ToString());
    }
}