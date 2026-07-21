#include "Anim_DinosaurBehaviorController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAnim_DinosaurBehaviorController::UAnim_DinosaurBehaviorController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configuração padrão
    DinosaurSpecies = EAnim_DinosaurSpecies::TRex;
    StateTransitionCooldown = 1.0f;
    AggressionThreshold = 0.7f;
    AlertRadius = 1000.0f;
    AttackRange = 200.0f;
    MovementSpeedMultiplier = 1.0f;
    
    // Estado inicial
    BehaviorData.CurrentState = EAnim_DinosaurBehaviorState::Idle;
    BehaviorData.Species = DinosaurSpecies;
    BehaviorData.MovementSpeed = 0.0f;
    BehaviorData.AggressionLevel = 0.3f;
    BehaviorData.HealthPercentage = 1.0f;
    BehaviorData.HungerLevel = 0.5f;
    BehaviorData.bIsInCombat = false;
    BehaviorData.bIsAlerted = false;
    BehaviorData.TargetActor = nullptr;
    
    // Variáveis internas
    LastStateChangeTime = 0.0f;
    PreviousState = EAnim_DinosaurBehaviorState::Idle;
    bIsInitialized = false;
    CurrentAnimInstance = nullptr;
    SkeletalMeshComponent = nullptr;
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    FeedingTimer = 0.0f;
    AlertTimer = 0.0f;
    CombatTimer = 0.0f;
}

void UAnim_DinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_DinosaurBehaviorController::InitializeComponent()
{
    // Obter referências dos componentes
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurBehaviorController: Owner is not a Character"));
        return;
    }
    
    SkeletalMeshComponent = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurBehaviorController: No SkeletalMeshComponent found"));
        return;
    }
    
    CurrentAnimInstance = SkeletalMeshComponent->GetAnimInstance();
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    
    // Configurar animation set baseado na espécie
    if (SpeciesAnimationSets.Contains(DinosaurSpecies))
    {
        AnimationSet = SpeciesAnimationSets[DinosaurSpecies];
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorController initialized for species: %d"), (int32)DinosaurSpecies);
}

void UAnim_DinosaurBehaviorController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized || !OwnerCharacter)
    {
        return;
    }
    
    UpdateBehaviorLogic(DeltaTime);
    ProcessStateTransitions();
    UpdateAnimationParameters();
    HandleSpeciesSpecificBehavior();
}

void UAnim_DinosaurBehaviorController::UpdateBehaviorLogic(float DeltaTime)
{
    // Atualizar velocidade de movimento
    if (MovementComponent)
    {
        FVector Velocity = MovementComponent->Velocity;
        BehaviorData.MovementSpeed = Velocity.Size();
    }
    
    // Atualizar timers
    FeedingTimer = FMath::Max(0.0f, FeedingTimer - DeltaTime);
    AlertTimer = FMath::Max(0.0f, AlertTimer - DeltaTime);
    CombatTimer = FMath::Max(0.0f, CombatTimer - DeltaTime);
    
    // Lógica de estado baseada em comportamento
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    switch (BehaviorData.CurrentState)
    {
        case EAnim_DinosaurBehaviorState::Idle:
            // Transição para alerta se detectar movimento próximo
            if (BehaviorData.bIsAlerted && AlertTimer <= 0.0f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Alert);
            }
            // Transição para caminhada aleatória
            else if (CurrentTime - LastStateChangeTime > 5.0f && FMath::RandRange(0.0f, 1.0f) > 0.8f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Walking);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Walking:
            // Transição para corrida se detectar ameaça
            if (BehaviorData.bIsInCombat)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Running);
            }
            // Voltar para idle após um tempo
            else if (CurrentTime - LastStateChangeTime > 3.0f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Idle);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Alert:
            // Transição para agressivo se o nível de agressão for alto
            if (BehaviorData.AggressionLevel > AggressionThreshold)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Aggressive);
            }
            // Voltar para idle se não há mais ameaças
            else if (!BehaviorData.bIsAlerted && AlertTimer <= 0.0f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Idle);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Aggressive:
            // Transição para caça se há um alvo
            if (BehaviorData.TargetActor && BehaviorData.bIsInCombat)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Hunting);
            }
            // Voltar para alerta se a agressão diminuir
            else if (BehaviorData.AggressionLevel < AggressionThreshold * 0.5f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Alert);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Hunting:
            // Continuar caçando enquanto há alvo e está em combate
            if (!BehaviorData.TargetActor || !BehaviorData.bIsInCombat)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Alert);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Feeding:
            // Continuar se alimentando por um tempo determinado
            if (FeedingTimer <= 0.0f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Idle);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Wounded:
            // Transição para morte se a saúde for muito baixa
            if (BehaviorData.HealthPercentage <= 0.0f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Dead);
            }
            // Recuperar se a saúde melhorar
            else if (BehaviorData.HealthPercentage > 0.3f)
            {
                SetBehaviorState(EAnim_DinosaurBehaviorState::Idle);
            }
            break;
            
        case EAnim_DinosaurBehaviorState::Dead:
            // Estado final - não há transições
            break;
    }
}

void UAnim_DinosaurBehaviorController::ProcessStateTransitions()
{
    // Verificar se é possível fazer transição de estado
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateChangeTime < StateTransitionCooldown)
    {
        return;
    }
    
    // Transições baseadas em saúde
    if (BehaviorData.HealthPercentage <= 0.0f && BehaviorData.CurrentState != EAnim_DinosaurBehaviorState::Dead)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Dead);
        return;
    }
    
    if (BehaviorData.HealthPercentage <= 0.2f && BehaviorData.CurrentState != EAnim_DinosaurBehaviorState::Wounded)
    {
        SetBehaviorState(EAnim_DinosaurBehaviorState::Wounded);
        return;
    }
}

void UAnim_DinosaurBehaviorController::UpdateAnimationParameters()
{
    if (!CurrentAnimInstance)
    {
        return;
    }
    
    // Atualizar parâmetros de animação baseados no estado atual
    // Nota: Estes parâmetros devem corresponder aos nomes no Animation Blueprint
    
    // Velocidade de movimento
    if (CurrentAnimInstance->GetClass()->FindPropertyByName(TEXT("MovementSpeed")))
    {
        CurrentAnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
    }
    
    // Estado de combate
    if (CurrentAnimInstance->GetClass()->FindPropertyByName(TEXT("bIsInCombat")))
    {
        // Definir variável de animação para combate
    }
    
    // Nível de agressão
    if (CurrentAnimInstance->GetClass()->FindPropertyByName(TEXT("AggressionLevel")))
    {
        // Definir variável de animação para agressão
    }
}

void UAnim_DinosaurBehaviorController::HandleSpeciesSpecificBehavior()
{
    // Comportamentos específicos por espécie
    switch (DinosaurSpecies)
    {
        case EAnim_DinosaurSpecies::TRex:
            // T-Rex: Mais agressivo, movimentos pesados
            if (BehaviorData.CurrentState == EAnim_DinosaurBehaviorState::Idle)
            {
                BehaviorData.AggressionLevel = FMath::Clamp(BehaviorData.AggressionLevel + 0.001f, 0.0f, 1.0f);
            }
            break;
            
        case EAnim_DinosaurSpecies::Raptor:
            // Raptor: Mais ágil, caça em grupo
            MovementSpeedMultiplier = 1.3f;
            break;
            
        case EAnim_DinosaurSpecies::Brachiosaurus:
            // Brachiosaurus: Pacífico, movimentos lentos
            MovementSpeedMultiplier = 0.7f;
            BehaviorData.AggressionLevel = FMath::Max(0.0f, BehaviorData.AggressionLevel - 0.001f);
            break;
            
        case EAnim_DinosaurSpecies::Triceratops:
            // Triceratops: Defensivo, territorial
            if (BehaviorData.TargetActor && BehaviorData.bIsAlerted)
            {
                BehaviorData.AggressionLevel = FMath::Clamp(BehaviorData.AggressionLevel + 0.002f, 0.0f, 1.0f);
            }
            break;
            
        default:
            break;
    }
}

void UAnim_DinosaurBehaviorController::SetBehaviorState(EAnim_DinosaurBehaviorState NewState)
{
    if (BehaviorData.CurrentState == NewState)
    {
        return;
    }
    
    if (!ValidateStateTransition(BehaviorData.CurrentState, NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state transition from %d to %d"), (int32)BehaviorData.CurrentState, (int32)NewState);
        return;
    }
    
    EAnim_DinosaurBehaviorState OldState = BehaviorData.CurrentState;
    PreviousState = OldState;
    BehaviorData.CurrentState = NewState;
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Reproduzir animação para o novo estado
    PlayMontageForState(NewState);
    
    // Callback de mudança de estado
    OnStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorController: State changed from %d to %d"), (int32)OldState, (int32)NewState);
}

void UAnim_DinosaurBehaviorController::SetDinosaurSpecies(EAnim_DinosaurSpecies NewSpecies)
{
    DinosaurSpecies = NewSpecies;
    BehaviorData.Species = NewSpecies;
    
    // Atualizar animation set baseado na nova espécie
    if (SpeciesAnimationSets.Contains(NewSpecies))
    {
        AnimationSet = SpeciesAnimationSets[NewSpecies];
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorController: Species changed to %d"), (int32)NewSpecies);
}

void UAnim_DinosaurBehaviorController::UpdateMovementSpeed(float Speed)
{
    BehaviorData.MovementSpeed = Speed * MovementSpeedMultiplier;
}

void UAnim_DinosaurBehaviorController::SetTarget(AActor* NewTarget)
{
    BehaviorData.TargetActor = NewTarget;
    
    if (NewTarget)
    {
        BehaviorData.bIsAlerted = true;
        AlertTimer = 5.0f;
        
        // Aumentar agressão se há um alvo
        BehaviorData.AggressionLevel = FMath::Clamp(BehaviorData.AggressionLevel + 0.2f, 0.0f, 1.0f);
    }
}

void UAnim_DinosaurBehaviorController::TriggerAttackAnimation()
{
    if (AnimationSet.AttackMontage && CurrentAnimInstance)
    {
        CurrentAnimInstance->Montage_Play(AnimationSet.AttackMontage);
        BehaviorData.bIsInCombat = true;
        CombatTimer = 3.0f;
    }
}

void UAnim_DinosaurBehaviorController::TriggerFeedingAnimation()
{
    if (AnimationSet.FeedingMontage && CurrentAnimInstance)
    {
        CurrentAnimInstance->Montage_Play(AnimationSet.FeedingMontage);
        SetBehaviorState(EAnim_DinosaurBehaviorState::Feeding);
        FeedingTimer = 10.0f;
        
        // Reduzir fome
        BehaviorData.HungerLevel = FMath::Max(0.0f, BehaviorData.HungerLevel - 0.3f);
    }
}

void UAnim_DinosaurBehaviorController::TriggerDeathAnimation()
{
    if (AnimationSet.DeathMontage && CurrentAnimInstance)
    {
        CurrentAnimInstance->Montage_Play(AnimationSet.DeathMontage);
        SetBehaviorState(EAnim_DinosaurBehaviorState::Dead);
        BehaviorData.HealthPercentage = 0.0f;
    }
}

bool UAnim_DinosaurBehaviorController::IsInState(EAnim_DinosaurBehaviorState State) const
{
    return BehaviorData.CurrentState == State;
}

float UAnim_DinosaurBehaviorController::GetCurrentAnimationLength() const
{
    if (CurrentAnimInstance)
    {
        UAnimMontage* CurrentMontage = CurrentAnimInstance->GetCurrentActiveMontage();
        if (CurrentMontage)
        {
            return CurrentMontage->GetPlayLength();
        }
    }
    return 0.0f;
}

EAnim_DinosaurBehaviorState UAnim_DinosaurBehaviorController::GetCurrentBehaviorState() const
{
    return BehaviorData.CurrentState;
}

EAnim_DinosaurSpecies UAnim_DinosaurBehaviorController::GetDinosaurSpecies() const
{
    return DinosaurSpecies;
}

bool UAnim_DinosaurBehaviorController::CanTransitionToState(EAnim_DinosaurBehaviorState TargetState) const
{
    return ValidateStateTransition(BehaviorData.CurrentState, TargetState);
}

void UAnim_DinosaurBehaviorController::PlayMontageForState(EAnim_DinosaurBehaviorState State)
{
    if (!CurrentAnimInstance)
    {
        return;
    }
    
    UAnimMontage* MontageToPlay = GetMontageForState(State);
    if (MontageToPlay)
    {
        CurrentAnimInstance->Montage_Play(MontageToPlay);
    }
}

UAnimMontage* UAnim_DinosaurBehaviorController::GetMontageForState(EAnim_DinosaurBehaviorState State) const
{
    switch (State)
    {
        case EAnim_DinosaurBehaviorState::Idle:
            return AnimationSet.IdleMontage;
        case EAnim_DinosaurBehaviorState::Walking:
            return AnimationSet.WalkMontage;
        case EAnim_DinosaurBehaviorState::Running:
            return AnimationSet.RunMontage;
        case EAnim_DinosaurBehaviorState::Hunting:
        case EAnim_DinosaurBehaviorState::Aggressive:
            return AnimationSet.AttackMontage;
        case EAnim_DinosaurBehaviorState::Feeding:
            return AnimationSet.FeedingMontage;
        case EAnim_DinosaurBehaviorState::Sleeping:
            return AnimationSet.SleepMontage;
        case EAnim_DinosaurBehaviorState::Dead:
            return AnimationSet.DeathMontage;
        default:
            return AnimationSet.IdleMontage;
    }
}

bool UAnim_DinosaurBehaviorController::ValidateStateTransition(EAnim_DinosaurBehaviorState FromState, EAnim_DinosaurBehaviorState ToState) const
{
    // Estado morto é final
    if (FromState == EAnim_DinosaurBehaviorState::Dead)
    {
        return false;
    }
    
    // Sempre pode morrer
    if (ToState == EAnim_DinosaurBehaviorState::Dead)
    {
        return true;
    }
    
    // Transições específicas baseadas em lógica de comportamento
    switch (FromState)
    {
        case EAnim_DinosaurBehaviorState::Sleeping:
            // Só pode acordar para idle ou alerta
            return (ToState == EAnim_DinosaurBehaviorState::Idle || ToState == EAnim_DinosaurBehaviorState::Alert);
            
        case EAnim_DinosaurBehaviorState::Feeding:
            // Pode ser interrompido por alerta ou agressão
            return (ToState == EAnim_DinosaurBehaviorState::Idle || 
                    ToState == EAnim_DinosaurBehaviorState::Alert ||
                    ToState == EAnim_DinosaurBehaviorState::Aggressive);
            
        default:
            return true; // Outras transições são geralmente permitidas
    }
}

void UAnim_DinosaurBehaviorController::OnStateChanged(EAnim_DinosaurBehaviorState OldState, EAnim_DinosaurBehaviorState NewState)
{
    // Lógica específica para mudanças de estado
    switch (NewState)
    {
        case EAnim_DinosaurBehaviorState::Alert:
            AlertTimer = 5.0f;
            break;
            
        case EAnim_DinosaurBehaviorState::Aggressive:
            BehaviorData.bIsInCombat = true;
            break;
            
        case EAnim_DinosaurBehaviorState::Idle:
            BehaviorData.bIsInCombat = false;
            BehaviorData.bIsAlerted = false;
            break;
            
        case EAnim_DinosaurBehaviorState::Dead:
            // Desabilitar movimento e colisão
            if (MovementComponent)
            {
                MovementComponent->SetMovementMode(MOVE_None);
            }
            break;
            
        default:
            break;
    }
}