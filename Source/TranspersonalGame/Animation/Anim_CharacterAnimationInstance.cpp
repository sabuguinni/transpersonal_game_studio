#include "Anim_CharacterAnimationInstance.h"
#include "TranspersonalGame/Characters/TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimationInstance::UAnim_CharacterAnimationInstance()
{
    // Inicializar valores padrão
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    MovementState = EAnim_MovementState::Idle;
    SurvivalState = EAnim_SurvivalState::Healthy;
    TurnRate = 0.0f;
    
    StaminaLevel = 1.0f;
    HealthLevel = 1.0f;
    HungerLevel = 1.0f;
    ThirstLevel = 1.0f;
    FearLevel = 0.0f;
    
    IdleWalkBlend = 0.0f;
    WalkRunBlend = 0.0f;
    AnimationSpeedMultiplier = 1.0f;
    
    PreviousSpeed = 0.0f;
    PreviousDirection = 0.0f;
    
    TranspersonalCharacter = nullptr;
    CharacterMovement = nullptr;
}

void UAnim_CharacterAnimationInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Obter referência ao personagem
    APawn* OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        TranspersonalCharacter = Cast<ATranspersonalCharacter>(OwningPawn);
        if (TranspersonalCharacter)
        {
            CharacterMovement = TranspersonalCharacter->GetCharacterMovement();
            UE_LOG(LogTemp, Log, TEXT("Animation Instance inicializada para %s"), *TranspersonalCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Pawn não é TranspersonalCharacter: %s"), *OwningPawn->GetClass()->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Sem Pawn owner para Animation Instance"));
    }
}

void UAnim_CharacterAnimationInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!TranspersonalCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Actualizar todas as propriedades
    UpdateMovementProperties(DeltaTimeX);
    UpdateSurvivalProperties();
    UpdateAnimationBlending(DeltaTimeX);
}

void UAnim_CharacterAnimationInstance::UpdateMovementProperties(float DeltaTime)
{
    if (!TranspersonalCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Calcular velocidade actual
    FVector Velocity = CharacterMovement->Velocity;
    float CurrentSpeed = Velocity.Size();
    
    // Suavizar mudanças de velocidade
    Speed = FMath::FInterpTo(PreviousSpeed, CurrentSpeed, DeltaTime, MovementSmoothingSpeed);
    PreviousSpeed = Speed;
    
    // Calcular direcção do movimento
    float CurrentDirection = CalculateDirection();
    Direction = FMath::FInterpAngle(PreviousDirection, CurrentDirection, DeltaTime, MovementSmoothingSpeed);
    PreviousDirection = Direction;
    
    // Verificar se está no ar
    bIsInAir = CharacterMovement->IsFalling();
    
    // Verificar se está agachado
    bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calcular taxa de rotação
    FRotator CurrentRotation = TranspersonalCharacter->GetActorRotation();
    static FRotator PreviousRotation = CurrentRotation;
    float RotationDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(PreviousRotation.Yaw, CurrentRotation.Yaw));
    TurnRate = RotationDelta / DeltaTime;
    PreviousRotation = CurrentRotation;
    
    // Determinar estado de movimento
    MovementState = DetermineMovementState();
}

void UAnim_CharacterAnimationInstance::UpdateSurvivalProperties()
{
    if (!TranspersonalCharacter)
    {
        return;
    }
    
    // Obter stats de sobrevivência do personagem
    StaminaLevel = TranspersonalCharacter->GetStamina() / 100.0f;
    HealthLevel = TranspersonalCharacter->GetHealth() / 100.0f;
    HungerLevel = TranspersonalCharacter->GetHunger() / 100.0f;
    ThirstLevel = TranspersonalCharacter->GetThirst() / 100.0f;
    FearLevel = TranspersonalCharacter->GetFear() / 100.0f;
    
    // Determinar estado de sobrevivência
    SurvivalState = DetermineSurvivalState();
}

void UAnim_CharacterAnimationInstance::UpdateAnimationBlending(float DeltaTime)
{
    // Calcular blend entre idle e walk
    if (Speed <= MinMovementSpeed)
    {
        IdleWalkBlend = FMath::FInterpTo(IdleWalkBlend, 0.0f, DeltaTime, MovementSmoothingSpeed);
    }
    else
    {
        float WalkBlendValue = FMath::Clamp(Speed / WalkToRunThreshold, 0.0f, 1.0f);
        IdleWalkBlend = FMath::FInterpTo(IdleWalkBlend, WalkBlendValue, DeltaTime, MovementSmoothingSpeed);
    }
    
    // Calcular blend entre walk e run
    if (Speed <= WalkToRunThreshold)
    {
        WalkRunBlend = FMath::FInterpTo(WalkRunBlend, 0.0f, DeltaTime, MovementSmoothingSpeed);
    }
    else
    {
        float RunBlendValue = FMath::Clamp((Speed - WalkToRunThreshold) / WalkToRunThreshold, 0.0f, 1.0f);
        WalkRunBlend = FMath::FInterpTo(WalkRunBlend, RunBlendValue, DeltaTime, MovementSmoothingSpeed);
    }
    
    // Calcular multiplicador de velocidade baseado em stamina
    float StaminaMultiplier = FMath::Lerp(0.5f, 1.0f, StaminaLevel);
    float HealthMultiplier = FMath::Lerp(0.3f, 1.0f, HealthLevel);
    AnimationSpeedMultiplier = StaminaMultiplier * HealthMultiplier;
    
    // Reduzir velocidade se com medo
    if (FearLevel > 0.5f)
    {
        AnimationSpeedMultiplier *= FMath::Lerp(1.0f, 0.7f, (FearLevel - 0.5f) * 2.0f);
    }
}

EAnim_MovementState UAnim_CharacterAnimationInstance::DetermineMovementState() const
{
    if (bIsInAir)
    {
        if (CharacterMovement && CharacterMovement->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (CharacterMovement && CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    if (Speed <= MinMovementSpeed)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed <= WalkToRunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

EAnim_SurvivalState UAnim_CharacterAnimationInstance::DetermineSurvivalState() const
{
    // Prioridade: Dying > Injured > Fearful > Thirsty > Hungry > Tired > Healthy
    
    if (HealthLevel <= 0.1f)
    {
        return EAnim_SurvivalState::Dying;
    }
    
    if (HealthLevel <= 0.3f)
    {
        return EAnim_SurvivalState::Injured;
    }
    
    if (FearLevel >= 0.7f)
    {
        return EAnim_SurvivalState::Fearful;
    }
    
    if (ThirstLevel <= 0.2f)
    {
        return EAnim_SurvivalState::Thirsty;
    }
    
    if (HungerLevel <= 0.2f)
    {
        return EAnim_SurvivalState::Hungry;
    }
    
    if (StaminaLevel <= 0.3f)
    {
        return EAnim_SurvivalState::Tired;
    }
    
    return EAnim_SurvivalState::Healthy;
}

float UAnim_CharacterAnimationInstance::CalculateDirection() const
{
    if (!TranspersonalCharacter || !CharacterMovement)
    {
        return 0.0f;
    }
    
    FVector Velocity = CharacterMovement->Velocity;
    if (Velocity.SizeSquared() < FMath::Square(MinMovementSpeed))
    {
        return 0.0f;
    }
    
    // Normalizar velocidade
    Velocity.Normalize();
    
    // Obter forward vector do personagem
    FVector ForwardVector = TranspersonalCharacter->GetActorForwardVector();
    FVector RightVector = TranspersonalCharacter->GetActorRightVector();
    
    // Calcular ângulo relativo
    float ForwardDot = FVector::DotProduct(Velocity, ForwardVector);
    float RightDot = FVector::DotProduct(Velocity, RightVector);
    
    // Converter para ângulo em graus
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    
    // Normalizar para 0-360
    if (Angle < 0.0f)
    {
        Angle += 360.0f;
    }
    
    return Angle;
}