#include "AnimationStateManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"

UAnimationStateManager::UAnimationStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Estado inicial
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    
    // Inicializar dados de movimento
    MovementData = FAnim_MovementData();
    
    // Timer de transição
    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    PendingState = EAnim_MovementState::Idle;
    
    OwnerMeshComponent = nullptr;
    MovementBlendSpace = nullptr;
}

void UAnimationStateManager::BeginPlay()
{
    Super::BeginPlay();

    // Encontrar o componente de mesh do owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMeshComponent)
        {
            // Se for um Character, usar o mesh do Character
            ACharacter* Character = Cast<ACharacter>(Owner);
            if (Character)
            {
                OwnerMeshComponent = Character->GetMesh();
            }
        }
    }

    // Inicializar transições padrão
    InitializeDefaultTransitions();

    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager initialized for %s"), 
           Owner ? *Owner->GetName() : TEXT("Unknown"));
}

void UAnimationStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Processar transições de estado
    if (bIsTransitioning)
    {
        ProcessStateTransition(DeltaTime);
    }

    // Actualizar dados de movimento automaticamente se for um Character
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetCharacterMovement())
    {
        UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
        
        // Calcular velocidade e direcção
        FVector Velocity = Movement->Velocity;
        float Speed = Velocity.Size();
        float Direction = 0.0f;
        
        if (Speed > 0.1f)
        {
            FVector Forward = Character->GetActorForwardVector();
            Direction = FMath::RadiansToDegrees(FMath::Atan2(
                FVector::CrossProduct(Forward, Velocity.GetSafeNormal()).Z,
                FVector::DotProduct(Forward, Velocity.GetSafeNormal())
            ));
        }

        // Actualizar dados de movimento
        UpdateMovementData(
            Speed,
            Direction,
            Movement->IsFalling(),
            Movement->IsCrouching()
        );

        // Auto-transição baseada na velocidade
        EAnim_MovementState NewState = CurrentMovementState;
        
        if (Movement->IsFalling())
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else if (Movement->IsCrouching())
        {
            NewState = EAnim_MovementState::Crouching;
        }
        else if (Speed > 400.0f) // Running threshold
        {
            NewState = EAnim_MovementState::Running;
        }
        else if (Speed > 50.0f) // Walking threshold
        {
            NewState = EAnim_MovementState::Walking;
        }
        else
        {
            NewState = EAnim_MovementState::Idle;
        }

        if (NewState != CurrentMovementState)
        {
            SetMovementState(NewState);
        }
    }

    // Actualizar instância de animação
    UpdateAnimationInstance();
}

void UAnimationStateManager::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState == CurrentMovementState || bIsTransitioning)
    {
        return;
    }

    if (CanTransitionTo(NewState))
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Transitioning from %d to %d"), 
               (int32)CurrentMovementState, (int32)NewState);

        PendingState = NewState;
        bIsTransitioning = true;
        TransitionTimer = GetTransitionDuration(CurrentMovementState, NewState);
    }
}

void UAnimationStateManager::SetActionState(EAnim_ActionState NewState)
{
    if (NewState != CurrentActionState)
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Action state changed to %d"), (int32)NewState);
        CurrentActionState = NewState;

        if (NewState != EAnim_ActionState::None)
        {
            PlayActionMontage(NewState);
        }
    }
}

void UAnimationStateManager::PlayActionMontage(EAnim_ActionState ActionType)
{
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetAnimInstance())
    {
        return;
    }

    UAnimMontage** FoundMontage = ActionMontages.Find(ActionType);
    if (FoundMontage && *FoundMontage)
    {
        UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
        AnimInstance->Montage_Play(*FoundMontage);
        
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Playing action montage for state %d"), (int32)ActionType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationStateManager: No montage found for action state %d"), (int32)ActionType);
    }
}

void UAnimationStateManager::StopCurrentAction()
{
    if (OwnerMeshComponent && OwnerMeshComponent->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
        AnimInstance->Montage_StopGroupByName(0.2f, TEXT("DefaultGroup"));
    }

    SetActionState(EAnim_ActionState::None);
}

void UAnimationStateManager::UpdateMovementData(float Speed, float Direction, bool bInAir, bool bCrouching)
{
    MovementData.Speed = Speed;
    MovementData.Direction = Direction;
    MovementData.bIsInAir = bInAir;
    MovementData.bIsCrouching = bCrouching;

    // Calcular distância ao chão (simplificado)
    if (GetOwner())
    {
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = Start - FVector(0, 0, 200.0f);
        
        FHitResult HitResult;
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic))
        {
            MovementData.GroundDistance = HitResult.Distance;
        }
        else
        {
            MovementData.GroundDistance = 200.0f;
        }
    }
}

bool UAnimationStateManager::CanTransitionTo(EAnim_MovementState TargetState) const
{
    // Regras básicas de transição
    if (TargetState == CurrentMovementState)
    {
        return false;
    }

    // Verificar se existe uma transição configurada
    return FindTransition(CurrentMovementState, TargetState) != nullptr;
}

float UAnimationStateManager::GetTransitionDuration(EAnim_MovementState FromState, EAnim_MovementState ToState) const
{
    const FAnim_StateTransition* Transition = FindTransition(FromState, ToState);
    return Transition ? Transition->TransitionDuration : 0.2f; // Duração padrão
}

void UAnimationStateManager::InitializeDefaultTransitions()
{
    StateTransitions.Empty();

    // Transições básicas de movimento
    TArray<EAnim_MovementState> AllStates = {
        EAnim_MovementState::Idle,
        EAnim_MovementState::Walking,
        EAnim_MovementState::Running,
        EAnim_MovementState::Jumping,
        EAnim_MovementState::Crouching
    };

    // Criar transições bidirecionais entre todos os estados
    for (EAnim_MovementState FromState : AllStates)
    {
        for (EAnim_MovementState ToState : AllStates)
        {
            if (FromState != ToState)
            {
                FAnim_StateTransition Transition;
                Transition.FromState = FromState;
                Transition.ToState = ToState;
                
                // Durações específicas para certas transições
                if (FromState == EAnim_MovementState::Jumping || ToState == EAnim_MovementState::Jumping)
                {
                    Transition.TransitionDuration = 0.1f; // Transições rápidas para salto
                }
                else if (FromState == EAnim_MovementState::Running || ToState == EAnim_MovementState::Running)
                {
                    Transition.TransitionDuration = 0.3f; // Transições mais lentas para corrida
                }
                else
                {
                    Transition.TransitionDuration = 0.2f; // Duração padrão
                }

                StateTransitions.Add(Transition);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Initialized %d default transitions"), StateTransitions.Num());
}

void UAnimationStateManager::ProcessStateTransition(float DeltaTime)
{
    TransitionTimer -= DeltaTime;
    
    if (TransitionTimer <= 0.0f)
    {
        // Completar transição
        CurrentMovementState = PendingState;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AnimationStateManager: Transition completed to state %d"), (int32)CurrentMovementState);
    }
}

void UAnimationStateManager::UpdateAnimationInstance()
{
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetAnimInstance())
    {
        return;
    }

    UAnimInstance* AnimInstance = OwnerMeshComponent->GetAnimInstance();
    
    // Aqui podemos definir variáveis no Animation Blueprint
    // Por agora, apenas registamos os valores
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationStateManager: Speed=%.1f, Direction=%.1f, State=%d"), 
           MovementData.Speed, MovementData.Direction, (int32)CurrentMovementState);
}

FAnim_StateTransition* UAnimationStateManager::FindTransition(EAnim_MovementState From, EAnim_MovementState To)
{
    for (FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == From && Transition.ToState == To)
        {
            return &Transition;
        }
    }
    return nullptr;
}

const FAnim_StateTransition* UAnimationStateManager::FindTransition(EAnim_MovementState From, EAnim_MovementState To) const
{
    for (const FAnim_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == From && Transition.ToState == To)
        {
            return &Transition;
        }
    }
    return nullptr;
}