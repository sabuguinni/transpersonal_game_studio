#include "Anim_SurvivalStateController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_SurvivalStateController::UAnim_SurvivalStateController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Inicializar valores padrão
    StateData = FAnim_SurvivalStateData();
    PreviousState = EAnim_SurvivalAnimState::Normal;
    
    TargetSpeedModifier = 1.0f;
    TargetPostureModifier = 0.0f;
    TargetShakeIntensity = 0.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_SurvivalStateController::BeginPlay()
{
    Super::BeginPlay();
    
    // Obter referências ao character e movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("SurvivalStateController: Inicializado para %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalStateController: Owner não é um ACharacter"));
    }
}

void UAnim_SurvivalStateController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Atualizar tempo no estado atual
    StateData.TimeInState += DeltaTime;
    
    // Aplicar transição suave entre estados
    ApplyStateTransition(DeltaTime);
}

void UAnim_SurvivalStateController::UpdateSurvivalState(float Health, float Hunger, float Thirst, float Stamina, float Fear)
{
    // Determinar novo estado baseado nos stats
    EAnim_SurvivalAnimState NewState = DetermineStateFromStats(Health, Hunger, Thirst, Stamina, Fear);
    
    // Se o estado mudou, resetar o timer
    if (NewState != StateData.CurrentState)
    {
        PreviousState = StateData.CurrentState;
        StateData.CurrentState = NewState;
        StateData.TimeInState = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalState mudou para: %d"), (int32)NewState);
    }
    
    // Calcular intensidade baseada nos stats mais críticos
    float WorstStat = FMath::Min({Health, Hunger, Thirst, Stamina});
    StateData.StateIntensity = FMath::Clamp(1.0f - WorstStat, 0.0f, 1.0f);
    
    // Adicionar componente de medo se relevante
    if (Fear > FearThreshold)
    {
        StateData.StateIntensity = FMath::Max(StateData.StateIntensity, Fear);
    }
    
    // Calcular modificadores baseados no estado atual
    CalculateStateModifiers();
}

void UAnim_SurvivalStateController::SetSurvivalState(EAnim_SurvivalAnimState NewState, float Intensity)
{
    if (NewState != StateData.CurrentState)
    {
        PreviousState = StateData.CurrentState;
        StateData.CurrentState = NewState;
        StateData.TimeInState = 0.0f;
    }
    
    StateData.StateIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    CalculateStateModifiers();
}

EAnim_SurvivalAnimState UAnim_SurvivalStateController::DetermineStateFromStats(float Health, float Hunger, float Thirst, float Stamina, float Fear)
{
    // Prioridade: Fear > Health > Stamina > Hunger > Thirst
    
    // Medo tem prioridade máxima
    if (Fear > FearThreshold)
    {
        return EAnim_SurvivalAnimState::Fearful;
    }
    
    // Saúde crítica
    if (Health < 0.1f)
    {
        return EAnim_SurvivalAnimState::Injured;
    }
    
    // Stamina muito baixa
    if (Stamina < ExhaustionThreshold)
    {
        return EAnim_SurvivalAnimState::Exhausted;
    }
    
    // Fome crítica
    if (Hunger < StarvationThreshold)
    {
        return EAnim_SurvivalAnimState::Starving;
    }
    
    // Sede crítica
    if (Thirst < DehydrationThreshold)
    {
        return EAnim_SurvivalAnimState::Dehydrated;
    }
    
    // Estado cauteloso se múltiplos stats estão baixos
    int32 LowStats = 0;
    if (Health < 0.5f) LowStats++;
    if (Hunger < 0.5f) LowStats++;
    if (Thirst < 0.5f) LowStats++;
    if (Stamina < 0.5f) LowStats++;
    
    if (LowStats >= 2)
    {
        return EAnim_SurvivalAnimState::Cautious;
    }
    
    return EAnim_SurvivalAnimState::Normal;
}

void UAnim_SurvivalStateController::CalculateStateModifiers()
{
    float Intensity = StateData.StateIntensity;
    
    switch (StateData.CurrentState)
    {
        case EAnim_SurvivalAnimState::Normal:
            TargetSpeedModifier = 1.0f;
            TargetPostureModifier = 0.0f;
            TargetShakeIntensity = 0.0f;
            break;
            
        case EAnim_SurvivalAnimState::Exhausted:
            TargetSpeedModifier = FMath::Lerp(1.0f, 0.6f, Intensity);
            TargetPostureModifier = FMath::Lerp(0.0f, 0.3f, Intensity); // Curvado para frente
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.2f, Intensity);
            break;
            
        case EAnim_SurvivalAnimState::Injured:
            TargetSpeedModifier = FMath::Lerp(1.0f, 0.4f, Intensity);
            TargetPostureModifier = FMath::Lerp(0.0f, 0.5f, Intensity);
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.3f, Intensity);
            break;
            
        case EAnim_SurvivalAnimState::Starving:
            TargetSpeedModifier = FMath::Lerp(1.0f, 0.7f, Intensity);
            TargetPostureModifier = FMath::Lerp(0.0f, 0.2f, Intensity);
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.4f, Intensity); // Tremor de fome
            break;
            
        case EAnim_SurvivalAnimState::Dehydrated:
            TargetSpeedModifier = FMath::Lerp(1.0f, 0.8f, Intensity);
            TargetPostureModifier = FMath::Lerp(0.0f, 0.1f, Intensity);
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.25f, Intensity);
            break;
            
        case EAnim_SurvivalAnimState::Fearful:
            TargetSpeedModifier = FMath::Lerp(1.0f, 1.3f, Intensity); // Mais rápido quando com medo
            TargetPostureModifier = FMath::Lerp(0.0f, -0.2f, Intensity); // Mais ereto, alerta
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.6f, Intensity); // Tremor de medo
            break;
            
        case EAnim_SurvivalAnimState::Cautious:
            TargetSpeedModifier = FMath::Lerp(1.0f, 0.85f, Intensity);
            TargetPostureModifier = FMath::Lerp(0.0f, -0.1f, Intensity); // Ligeiramente mais ereto
            TargetShakeIntensity = FMath::Lerp(0.0f, 0.1f, Intensity);
            break;
            
        case EAnim_SurvivalAnimState::Hunting:
            TargetSpeedModifier = 0.9f; // Movimento mais controlado
            TargetPostureModifier = -0.15f; // Postura de caça
            TargetShakeIntensity = 0.0f;
            break;
            
        case EAnim_SurvivalAnimState::Gathering:
            TargetSpeedModifier = 0.8f; // Movimento mais lento e cuidadoso
            TargetPostureModifier = 0.1f; // Ligeiramente curvado
            TargetShakeIntensity = 0.0f;
            break;
    }
}

void UAnim_SurvivalStateController::ApplyStateTransition(float DeltaTime)
{
    float TransitionAlpha = StateTransitionSpeed * DeltaTime;
    
    // Interpolar suavemente para os valores alvo
    StateData.SpeedModifier = FMath::FInterpTo(StateData.SpeedModifier, TargetSpeedModifier, DeltaTime, StateTransitionSpeed);
    StateData.PostureModifier = FMath::FInterpTo(StateData.PostureModifier, TargetPostureModifier, DeltaTime, StateTransitionSpeed);
    StateData.ShakeIntensity = FMath::FInterpTo(StateData.ShakeIntensity, TargetShakeIntensity, DeltaTime, StateTransitionSpeed);
    
    // Aplicar modificador de velocidade ao movement component se disponível
    if (MovementComponent && StateData.SpeedModifier != 1.0f)
    {
        float BaseWalkSpeed = 600.0f; // Velocidade base padrão do UE5
        MovementComponent->MaxWalkSpeed = BaseWalkSpeed * StateData.SpeedModifier;
    }
}