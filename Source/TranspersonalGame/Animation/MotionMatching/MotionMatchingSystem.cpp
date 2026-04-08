#include "MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/PoseSearchSchema.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UMotionMatchingSystem::UMotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Configuração padrão
    QueryUpdateFrequency = 30.0f;
    bEnableDebugDrawing = false;
    
    // Inicializar perfil padrão
    CharacterProfile.ProfileName = "DefaultPaleontologist";
    CharacterProfile.MovementSpeedMultiplier = 1.0f;
    CharacterProfile.TurnRateMultiplier = 1.0f;
    CharacterProfile.BaseAnxietyLevel = 0.3f; // Paleontologista naturalmente cauteloso
    CharacterProfile.PosturalTension = 0.4f; // Sempre alerta no mundo jurássico
    CharacterProfile.QuickBlendTime = 0.1f;
    CharacterProfile.NormalBlendTime = 0.3f;
    CharacterProfile.SlowBlendTime = 0.8f;
    
    // Estado inicial
    CurrentPersonality = ECharacterPersonality::Cautious;
    CurrentTension = 0.3f;
    CurrentFatigue = 0.0f;
}

void UMotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache de componentes essenciais
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    }
    
    // Inicializar query com valores padrão
    CurrentQuery.CurrentState = EMotionMatchingState::Idle;
    CurrentQuery.Personality = CurrentPersonality;
    CurrentQuery.FearLevel = CharacterProfile.BaseAnxietyLevel;
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem: Initialized for character %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UMotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update com frequência controlada para performance
    TimeSinceLastQuery += DeltaTime;
    if (TimeSinceLastQuery >= (1.0f / QueryUpdateFrequency))
    {
        UpdateMotionMatching(DeltaTime);
        TimeSinceLastQuery = 0.0f;
    }
    
    // Update de sistemas secundários
    UpdatePersonalityModifiers(DeltaTime);
    
    if (bEnableDebugDrawing)
    {
        DrawDebugInfo();
    }
}

void UMotionMatchingSystem::UpdateMotionMatching(float DeltaTime)
{
    // Guardar query anterior
    PreviousQuery = CurrentQuery;
    
    // Build novo query baseado no estado atual
    CurrentQuery = BuildCurrentQuery();
    
    // Determinar se precisamos de transição
    bool bNeedsTransition = (CurrentQuery.CurrentState != PreviousQuery.CurrentState) ||
                           (FVector::Dist(CurrentQuery.DesiredVelocity, PreviousQuery.DesiredVelocity) > 50.0f) ||
                           (FMath::Abs(CurrentQuery.FearLevel - PreviousQuery.FearLevel) > 0.2f);
    
    if (bNeedsTransition && !bIsInTransition)
    {
        // Iniciar nova transição
        bIsInTransition = true;
        TransitionDuration = CalculateOptimalBlendTime(PreviousQuery, CurrentQuery);
        TransitionTimeRemaining = TransitionDuration;
        
        UE_LOG(LogTemp, Verbose, TEXT("MotionMatching: Starting transition from %s to %s (%.2fs)"),
               *UEnum::GetValueAsString(PreviousQuery.CurrentState),
               *UEnum::GetValueAsString(CurrentQuery.CurrentState),
               TransitionDuration);
    }
    
    // Update transition timer
    if (bIsInTransition)
    {
        TransitionTimeRemaining -= DeltaTime;
        if (TransitionTimeRemaining <= 0.0f)
        {
            bIsInTransition = false;
            TransitionTimeRemaining = 0.0f;
        }
    }
    
    // Update character state
    UpdateCharacterState(DeltaTime);
}

FMotionMatchingQuery UMotionMatchingSystem::BuildCurrentQuery()
{
    FMotionMatchingQuery NewQuery;
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return NewQuery;
    }
    
    // Trajectory data
    NewQuery.CurrentVelocity = MovementComponent->Velocity;
    NewQuery.DesiredVelocity = MovementComponent->GetLastInputVector() * MovementComponent->GetMaxSpeed();
    
    // Predict future state
    NewQuery.FuturePosition = PredictFuturePosition(0.5f); // 500ms no futuro
    NewQuery.FutureRotation = PredictFutureRotation(0.5f);
    
    // Character state
    NewQuery.CurrentState = DetermineCurrentState();
    NewQuery.Personality = CurrentPersonality;
    
    // Environmental factors
    UpdateEnvironmentalFactors();
    NewQuery.GroundSlope = CurrentQuery.GroundSlope; // Mantém valor anterior se não atualizado
    NewQuery.bIsOnUnstableGround = CurrentQuery.bIsOnUnstableGround;
    NewQuery.bNearPredator = CurrentQuery.bNearPredator;
    
    // Emotional state
    NewQuery.FearLevel = FMath::Clamp(CurrentQuery.FearLevel, 0.0f, 1.0f);
    NewQuery.Tension = CurrentTension;
    NewQuery.Fatigue = CurrentFatigue;
    
    // Weight distribution (baseado na velocidade e direção)
    FVector VelNormalized = NewQuery.CurrentVelocity.GetSafeNormal();
    NewQuery.WeightShift = FMath::Clamp(VelNormalized.Size() * GetPersonalityMovementModifier(), 0.0f, 1.0f);
    
    return NewQuery;
}

UPoseSearchDatabase* UMotionMatchingSystem::SelectBestDatabase(const FMotionMatchingQuery& Query)
{
    // Seleção de database baseada no estado e personalidade
    switch (Query.CurrentState)
    {
        case EMotionMatchingState::Idle:
            return CharacterProfile.IdleDatabase;
            
        case EMotionMatchingState::Walking:
        case EMotionMatchingState::Running:
        case EMotionMatchingState::Sneaking:
            return CharacterProfile.LocomotionDatabase;
            
        case EMotionMatchingState::Climbing:
        case EMotionMatchingState::Swimming:
        case EMotionMatchingState::Interaction:
            return CharacterProfile.ActionDatabase;
            
        default:
            return CharacterProfile.LocomotionDatabase; // Fallback seguro
    }
}

void UMotionMatchingSystem::SetCharacterProfile(const FCharacterAnimationProfile& NewProfile)
{
    CharacterProfile = NewProfile;
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Character profile changed to %s"), *NewProfile.ProfileName);
}

void UMotionMatchingSystem::SetPersonality(ECharacterPersonality NewPersonality)
{
    if (CurrentPersonality != NewPersonality)
    {
        CurrentPersonality = NewPersonality;
        CurrentQuery.Personality = NewPersonality;
        
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Personality changed to %s"), 
               *UEnum::GetValueAsString(NewPersonality));
    }
}

void UMotionMatchingSystem::UpdateEnvironmentalContext(float GroundSlope, bool bUnstableGround, bool bNearThreat)
{
    CurrentQuery.GroundSlope = GroundSlope;
    CurrentQuery.bIsOnUnstableGround = bUnstableGround;
    CurrentQuery.bNearPredator = bNearThreat;
    
    // Ajustar fear level baseado no contexto
    if (bNearThreat)
    {
        SetFearLevel(FMath::Min(CurrentQuery.FearLevel + 0.3f, 1.0f));
    }
    else if (bUnstableGround)
    {
        SetFearLevel(FMath::Min(CurrentQuery.FearLevel + 0.1f, 1.0f));
    }
}

void UMotionMatchingSystem::SetFearLevel(float NewFearLevel)
{
    float ClampedFear = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    CurrentQuery.FearLevel = ClampedFear;
    
    // Fear afeta tension automaticamente
    AddTension(ClampedFear * 0.5f, 3.0f);
}

void UMotionMatchingSystem::AddTension(float TensionAmount, float Duration)
{
    CurrentTension = FMath::Clamp(CurrentTension + TensionAmount, 0.0f, 1.0f);
    TensionDecayTimer = Duration;
}

void UMotionMatchingSystem::AddFatigue(float FatigueAmount)
{
    CurrentFatigue = FMath::Clamp(CurrentFatigue + FatigueAmount, 0.0f, 1.0f);
}

float UMotionMatchingSystem::GetCurrentBlendTime() const
{
    if (bIsInTransition)
    {
        return TransitionDuration;
    }
    
    // Blend time baseado no estado e personalidade
    float BaseBlendTime = CharacterProfile.NormalBlendTime;
    
    // Modificadores baseados em tension e fear
    float TensionModifier = 1.0f - (CurrentTension * 0.3f); // Mais tenso = transições mais rápidas
    float FearModifier = 1.0f - (CurrentQuery.FearLevel * 0.4f); // Mais medo = reações mais rápidas
    
    return BaseBlendTime * TensionModifier * FearModifier * GetPersonalityBlendModifier();
}

// Private Methods Implementation

void UMotionMatchingSystem::UpdateCharacterState(float DeltaTime)
{
    // Atualizar fatigue baseado na atividade
    if (MovementComponent && MovementComponent->Velocity.Size() > 200.0f) // Running
    {
        AddFatigue(DeltaTime * 0.1f);
    }
    else if (MovementComponent && MovementComponent->Velocity.Size() > 50.0f) // Walking
    {
        AddFatigue(DeltaTime * 0.02f);
    }
    else // Resting
    {
        CurrentFatigue = FMath::Max(CurrentFatigue - (DeltaTime * 0.05f), 0.0f);
    }
}

void UMotionMatchingSystem::UpdatePersonalityModifiers(float DeltaTime)
{
    // Decay de tension ao longo do tempo
    if (TensionDecayTimer > 0.0f)
    {
        TensionDecayTimer -= DeltaTime;
        if (TensionDecayTimer <= 0.0f)
        {
            CurrentTension = FMath::Max(CurrentTension - (DeltaTime * 0.2f), CharacterProfile.PosturalTension);
        }
    }
    
    // Decay natural de fear level
    if (CurrentQuery.FearLevel > CharacterProfile.BaseAnxietyLevel)
    {
        CurrentQuery.FearLevel = FMath::Max(CurrentQuery.FearLevel - (DeltaTime * 0.1f), CharacterProfile.BaseAnxietyLevel);
    }
}

void UMotionMatchingSystem::UpdateEnvironmentalFactors()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Line trace para detectar slope do terreno
    FVector Start = OwnerCharacter->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        FVector Normal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector));
        CurrentQuery.GroundSlope = FMath::RadiansToDegrees(SlopeAngle);
        
        // Terreno instável se slope > 30 graus
        CurrentQuery.bIsOnUnstableGround = CurrentQuery.GroundSlope > 30.0f;
    }
}

EMotionMatchingState UMotionMatchingSystem::DetermineCurrentState()
{
    if (!MovementComponent)
    {
        return EMotionMatchingState::Idle;
    }
    
    float Speed = MovementComponent->Velocity.Size();
    bool bIsMoving = Speed > 10.0f;
    bool bIsFalling = MovementComponent->IsFalling();
    bool bIsSwimming = MovementComponent->IsSwimming();
    
    // Estados baseados em movimento
    if (bIsFalling)
    {
        return EMotionMatchingState::Falling;
    }
    
    if (bIsSwimming)
    {
        return EMotionMatchingState::Swimming;
    }
    
    if (!bIsMoving)
    {
        // Estados idle baseados em fear level
        if (CurrentQuery.FearLevel > 0.7f)
        {
            return EMotionMatchingState::Fear;
        }
        else if (CurrentQuery.FearLevel > 0.4f)
        {
            return EMotionMatchingState::Caution;
        }
        else if (CurrentFatigue > 0.7f)
        {
            return EMotionMatchingState::Exhaustion;
        }
        
        return EMotionMatchingState::Idle;
    }
    
    // Estados de movimento baseados em velocidade e personalidade
    float WalkThreshold = 150.0f * GetPersonalityMovementModifier();
    float RunThreshold = 300.0f * GetPersonalityMovementModifier();
    
    if (Speed < WalkThreshold)
    {
        // Movimento cauteloso se fear level alto
        return (CurrentQuery.FearLevel > 0.5f) ? EMotionMatchingState::Sneaking : EMotionMatchingState::Walking;
    }
    else if (Speed < RunThreshold)
    {
        return EMotionMatchingState::Walking;
    }
    else
    {
        return EMotionMatchingState::Running;
    }
}

float UMotionMatchingSystem::CalculateOptimalBlendTime(const FMotionMatchingQuery& FromQuery, const FMotionMatchingQuery& ToQuery)
{
    // Blend time baseado na diferença entre estados
    float BaseTime = CharacterProfile.NormalBlendTime;
    
    // Transições rápidas para estados de emergência
    if (ToQuery.CurrentState == EMotionMatchingState::Fear || 
        ToQuery.CurrentState == EMotionMatchingState::Falling)
    {
        return CharacterProfile.QuickBlendTime;
    }
    
    // Transições lentas para estados de repouso
    if (ToQuery.CurrentState == EMotionMatchingState::Idle ||
        ToQuery.CurrentState == EMotionMatchingState::Exhaustion)
    {
        return CharacterProfile.SlowBlendTime;
    }
    
    // Modificadores baseados na personalidade
    return BaseTime * GetPersonalityBlendModifier();
}

void UMotionMatchingSystem::DrawDebugInfo()
{
    if (!OwnerCharacter || !GetWorld())
    {
        return;
    }
    
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    
    // Debug text com informações do sistema
    FString DebugText = FString::Printf(TEXT("Motion Matching Debug\n"));
    DebugText += FString::Printf(TEXT("State: %s\n"), *UEnum::GetValueAsString(CurrentQuery.CurrentState));
    DebugText += FString::Printf(TEXT("Personality: %s\n"), *UEnum::GetValueAsString(CurrentPersonality));
    DebugText += FString::Printf(TEXT("Fear: %.2f | Tension: %.2f | Fatigue: %.2f\n"), 
                                CurrentQuery.FearLevel, CurrentTension, CurrentFatigue);
    DebugText += FString::Printf(TEXT("Speed: %.1f | Slope: %.1f°\n"), 
                                CurrentQuery.CurrentVelocity.Size(), CurrentQuery.GroundSlope);
    DebugText += FString::Printf(TEXT("Transition: %s (%.2fs)\n"), 
                                bIsInTransition ? TEXT("YES") : TEXT("NO"), TransitionTimeRemaining);
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 250), DebugText, nullptr, FColor::Yellow, 0.0f);
    
    // Debug trajectory
    FVector FuturePos = ActorLocation + (CurrentQuery.DesiredVelocity * 0.5f);
    DrawDebugLine(GetWorld(), ActorLocation, FuturePos, FColor::Green, false, 0.1f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), FuturePos, 20.0f, 8, FColor::Green, false, 0.1f);
}

FVector UMotionMatchingSystem::PredictFuturePosition(float TimeAhead) const
{
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    FVector CurrentPos = OwnerCharacter->GetActorLocation();
    FVector Velocity = CurrentQuery.DesiredVelocity;
    
    // Aplicar modificadores de personalidade
    Velocity *= GetPersonalityMovementModifier();
    
    return CurrentPos + (Velocity * TimeAhead);
}

FRotator UMotionMatchingSystem::PredictFutureRotation(float TimeAhead) const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return FRotator::ZeroRotator;
    }
    
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    
    // Se estamos a mover, prever rotação baseada na direção do movimento
    if (CurrentQuery.DesiredVelocity.Size() > 10.0f)
    {
        FRotator DesiredRotation = CurrentQuery.DesiredVelocity.Rotation();
        float TurnRate = CharacterProfile.TurnRateMultiplier * 180.0f; // graus por segundo
        
        return FMath::RInterpTo(CurrentRotation, DesiredRotation, TimeAhead, TurnRate);
    }
    
    return CurrentRotation;
}

float UMotionMatchingSystem::GetPersonalityMovementModifier() const
{
    switch (CurrentPersonality)
    {
        case ECharacterPersonality::Confident:
            return 1.2f; // Movimentos mais rápidos e decididos
        case ECharacterPersonality::Nervous:
            return 0.8f; // Movimentos mais hesitantes
        case ECharacterPersonality::Tired:
            return 0.6f; // Movimentos mais lentos
        case ECharacterPersonality::Injured:
            return 0.5f; // Movimentos limitados
        case ECharacterPersonality::Predator:
            return 1.1f; // Movimentos controlados mas eficientes
        case ECharacterPersonality::Prey:
            return 0.9f; // Movimentos cautelosos
        case ECharacterPersonality::Cautious:
        default:
            return 1.0f; // Velocidade normal
    }
}

float UMotionMatchingSystem::GetPersonalityTensionModifier() const
{
    switch (CurrentPersonality)
    {
        case ECharacterPersonality::Confident:
            return 0.7f; // Menos tensão natural
        case ECharacterPersonality::Nervous:
            return 1.5f; // Mais tensão natural
        case ECharacterPersonality::Tired:
            return 0.8f; // Menos energia para tensão
        case ECharacterPersonality::Predator:
            return 1.2f; // Tensão controlada
        case ECharacterPersonality::Prey:
            return 1.4f; // Sempre alerta
        case ECharacterPersonality::Cautious:
        default:
            return 1.0f; // Tensão normal
    }
}

float UMotionMatchingSystem::GetPersonalityBlendModifier() const
{
    switch (CurrentPersonality)
    {
        case ECharacterPersonality::Confident:
            return 0.8f; // Transições mais rápidas e decisivas
        case ECharacterPersonality::Nervous:
            return 0.6f; // Reações rápidas
        case ECharacterPersonality::Tired:
            return 1.3f; // Transições mais lentas
        case ECharacterPersonality::Injured:
            return 1.5f; // Movimentos mais cuidadosos
        case ECharacterPersonality::Predator:
            return 0.9f; // Movimentos controlados
        case ECharacterPersonality::Prey:
            return 0.7f; // Reações rápidas de sobrevivência
        case ECharacterPersonality::Cautious:
        default:
            return 1.0f; // Timing normal
    }
}