#include "Anim_TribalCharacterSystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UAnim_TribalCharacterSystem::UAnim_TribalCharacterSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TribalType = EAnim_TribalType::Hunter;
    CurrentState = EAnim_TribalState::Idle;
    PreviousState = EAnim_TribalState::Idle;
    
    MovementSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInCombat = false;
    bIsCrafting = false;
    bIsPerformingRitual = false;
    
    CharacterAnimInstance = nullptr;
    CurrentMontage = nullptr;
    
    DefaultBlendTime = 0.25f;
    RitualDuration = 5.0f;
    CraftingCycleDuration = 3.0f;
    
    StateTransitionTime = 0.0f;
    bIsTransitioning = false;
}

void UAnim_TribalCharacterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to character and anim instance
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            CharacterAnimInstance = MeshComp->GetAnimInstance();
        }
    }
    
    // Initialize tribal animation sets
    InitializeTribalAnimationSets();
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Character Animation System initialized for type: %d"), (int32)TribalType);
}

void UAnim_TribalCharacterSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update animation state based on character movement
    UpdateAnimationState();
    
    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTime += DeltaTime;
        if (StateTransitionTime >= DefaultBlendTime)
        {
            bIsTransitioning = false;
            StateTransitionTime = 0.0f;
        }
    }
}

void UAnim_TribalCharacterSystem::SetTribalType(EAnim_TribalType NewType)
{
    if (TribalType != NewType)
    {
        TribalType = NewType;
        UE_LOG(LogTemp, Log, TEXT("Tribal type changed to: %d"), (int32)TribalType);
        
        // Reinitialize animation sets for new type
        InitializeTribalAnimationSets();
    }
}

void UAnim_TribalCharacterSystem::SetTribalState(EAnim_TribalState NewState)
{
    if (CurrentState != NewState)
    {
        HandleStateTransition(NewState);
    }
}

void UAnim_TribalCharacterSystem::PlayTribalMontage(EAnim_TribalState StateType)
{
    if (!CharacterAnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("No anim instance found for tribal montage"));
        return;
    }
    
    FAnim_TribalAnimationSet* AnimSet = TribalAnimationSets.Find(TribalType);
    if (!AnimSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("No animation set found for tribal type: %d"), (int32)TribalType);
        return;
    }
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (StateType)
    {
        case EAnim_TribalState::Idle:
            MontageToPlay = AnimSet->IdleMontage;
            break;
        case EAnim_TribalState::Walking:
            MontageToPlay = AnimSet->WalkMontage;
            break;
        case EAnim_TribalState::Running:
            MontageToPlay = AnimSet->RunMontage;
            break;
        case EAnim_TribalState::Combat:
            MontageToPlay = AnimSet->CombatMontage;
            bIsInCombat = true;
            break;
        case EAnim_TribalState::Crafting:
            MontageToPlay = AnimSet->CraftingMontage;
            bIsCrafting = true;
            break;
        case EAnim_TribalState::Ritual:
            MontageToPlay = AnimSet->RitualMontage;
            bIsPerformingRitual = true;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        CurrentMontage = MontageToPlay;
        CharacterAnimInstance->Montage_Play(MontageToPlay);
        UE_LOG(LogTemp, Log, TEXT("Playing tribal montage for state: %d"), (int32)StateType);
    }
}

void UAnim_TribalCharacterSystem::StopCurrentMontage()
{
    if (CharacterAnimInstance && CurrentMontage)
    {
        CharacterAnimInstance->Montage_Stop(DefaultBlendTime, CurrentMontage);
        CurrentMontage = nullptr;
        
        // Reset activity flags
        bIsInCombat = false;
        bIsCrafting = false;
        bIsPerformingRitual = false;
        
        UE_LOG(LogTemp, Log, TEXT("Stopped current tribal montage"));
    }
}

FAnim_TribalAnimationSet UAnim_TribalCharacterSystem::GetCurrentAnimationSet() const
{
    const FAnim_TribalAnimationSet* AnimSet = TribalAnimationSets.Find(TribalType);
    return AnimSet ? *AnimSet : FAnim_TribalAnimationSet();
}

void UAnim_TribalCharacterSystem::PlayElderWisdomGesture()
{
    if (TribalType == EAnim_TribalType::Elder)
    {
        SetTribalState(EAnim_TribalState::Ritual);
        PlayTribalMontage(EAnim_TribalState::Ritual);
        UE_LOG(LogTemp, Log, TEXT("Elder performing wisdom gesture"));
    }
}

void UAnim_TribalCharacterSystem::PlayWarriorBattleCry()
{
    if (TribalType == EAnim_TribalType::Warrior)
    {
        SetTribalState(EAnim_TribalState::Combat);
        PlayTribalMontage(EAnim_TribalState::Combat);
        UE_LOG(LogTemp, Log, TEXT("Warrior performing battle cry"));
    }
}

void UAnim_TribalCharacterSystem::PlayHealerRitual()
{
    if (TribalType == EAnim_TribalType::Healer)
    {
        SetTribalState(EAnim_TribalState::Ritual);
        PlayTribalMontage(EAnim_TribalState::Ritual);
        UE_LOG(LogTemp, Log, TEXT("Healer performing healing ritual"));
    }
}

void UAnim_TribalCharacterSystem::PlayScoutAlert()
{
    if (TribalType == EAnim_TribalType::Scout)
    {
        SetTribalState(EAnim_TribalState::Hunting);
        PlayTribalMontage(EAnim_TribalState::Combat); // Use combat montage for alert stance
        UE_LOG(LogTemp, Log, TEXT("Scout performing alert scan"));
    }
}

void UAnim_TribalCharacterSystem::PlayChildCuriosity()
{
    if (TribalType == EAnim_TribalType::Child)
    {
        SetTribalState(EAnim_TribalState::Gathering);
        PlayTribalMontage(EAnim_TribalState::Idle); // Use idle montage for curious behavior
        UE_LOG(LogTemp, Log, TEXT("Child showing curiosity"));
    }
}

void UAnim_TribalCharacterSystem::UpdateLocomotionBlending(float Speed, float Direction)
{
    MovementSpeed = Speed;
    MovementDirection = Direction;
    
    // Update animation state based on speed
    if (Speed < 1.0f)
    {
        SetTribalState(EAnim_TribalState::Idle);
    }
    else if (Speed < 300.0f)
    {
        SetTribalState(EAnim_TribalState::Walking);
    }
    else
    {
        SetTribalState(EAnim_TribalState::Running);
    }
}

void UAnim_TribalCharacterSystem::BlendToState(EAnim_TribalState TargetState, float BlendTime)
{
    if (CurrentState != TargetState)
    {
        DefaultBlendTime = BlendTime;
        SetTribalState(TargetState);
    }
}

void UAnim_TribalCharacterSystem::InitializeTribalAnimationSets()
{
    // Initialize animation sets for each tribal type
    // In a full implementation, these would load actual animation assets
    
    for (int32 TypeIndex = 0; TypeIndex < (int32)EAnim_TribalType::Crafter + 1; TypeIndex++)
    {
        EAnim_TribalType Type = (EAnim_TribalType)TypeIndex;
        FAnim_TribalAnimationSet AnimSet;
        
        // Set default values - in production these would be loaded from assets
        AnimSet.IdleMontage = nullptr;
        AnimSet.WalkMontage = nullptr;
        AnimSet.RunMontage = nullptr;
        AnimSet.CombatMontage = nullptr;
        AnimSet.CraftingMontage = nullptr;
        AnimSet.RitualMontage = nullptr;
        AnimSet.LocomotionBlendSpace = nullptr;
        
        TribalAnimationSets.Add(Type, AnimSet);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized tribal animation sets for %d types"), TribalAnimationSets.Num());
}

void UAnim_TribalCharacterSystem::UpdateAnimationState()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float CurrentSpeed = MovementComp->Velocity.Size();
            UpdateLocomotionBlending(CurrentSpeed, 0.0f); // Direction calculation would be more complex
        }
    }
}

void UAnim_TribalCharacterSystem::HandleStateTransition(EAnim_TribalState NewState)
{
    PreviousState = CurrentState;
    CurrentState = NewState;
    bIsTransitioning = true;
    StateTransitionTime = 0.0f;
    
    // Play appropriate montage for new state
    PlayTribalMontage(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Tribal state transition: %d -> %d"), (int32)PreviousState, (int32)CurrentState);
}

// UAnim_TribalCharacterAnimInstance Implementation

UAnim_TribalCharacterAnimInstance::UAnim_TribalCharacterAnimInstance()
{
    TribalType = EAnim_TribalType::Hunter;
    AnimationState = EAnim_TribalState::Idle;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bIsInCombat = false;
    bIsCrafting = false;
    bIsPerformingRitual = false;
    bIsGathering = false;
    
    bShouldPlayWisdomGesture = false;
    bShouldPlayBattleCry = false;
    bShouldPlayHealingRitual = false;
    
    TribalSystem = nullptr;
    OwningCharacter = nullptr;
}

void UAnim_TribalCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get references to character and tribal system
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        TribalSystem = OwningCharacter->FindComponentByClass<UAnim_TribalCharacterSystem>();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal Character AnimInstance initialized"));
}

void UAnim_TribalCharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter)
    {
        return;
    }
    
    // Update animation properties
    UpdateMovementProperties();
    UpdateTribalAnimationProperties();
    UpdateCulturalProperties();
}

void UAnim_TribalCharacterAnimInstance::UpdateTribalAnimationProperties()
{
    if (TribalSystem)
    {
        TribalType = TribalSystem->TribalType;
        AnimationState = TribalSystem->CurrentState;
        bIsInCombat = TribalSystem->bIsInCombat;
        bIsCrafting = TribalSystem->bIsCrafting;
        bIsPerformingRitual = TribalSystem->bIsPerformingRitual;
    }
}

void UAnim_TribalCharacterAnimInstance::UpdateMovementProperties()
{
    if (OwningCharacter)
    {
        if (UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            Speed = Velocity.Size();
            bIsMoving = Speed > 1.0f;
            
            // Calculate movement direction relative to character forward
            if (bIsMoving)
            {
                FVector ForwardVector = OwningCharacter->GetActorForwardVector();
                FVector NormalizedVelocity = Velocity.GetSafeNormal();
                Direction = FVector::DotProduct(ForwardVector, NormalizedVelocity);
            }
            else
            {
                Direction = 0.0f;
            }
        }
    }
}

void UAnim_TribalCharacterAnimInstance::UpdateCulturalProperties()
{
    // Update cultural animation triggers based on tribal type and state
    bShouldPlayWisdomGesture = (TribalType == EAnim_TribalType::Elder && AnimationState == EAnim_TribalState::Ritual);
    bShouldPlayBattleCry = (TribalType == EAnim_TribalType::Warrior && AnimationState == EAnim_TribalState::Combat);
    bShouldPlayHealingRitual = (TribalType == EAnim_TribalType::Healer && AnimationState == EAnim_TribalState::Ritual);
    
    // Set gathering flag for appropriate activities
    bIsGathering = (AnimationState == EAnim_TribalState::Gathering);
}