#include "Anim_SurvivalStateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_SurvivalStateManager::UAnim_SurvivalStateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configuração padrão
    StateTransitionSpeed = 2.0f;
    InjuredThreshold = 30.0f;
    ExhaustedThreshold = 20.0f;
    FearThreshold = 70.0f;
    
    // Inicializar dados de animação
    CurrentAnimData = FAnim_SurvivalData();
    CurrentAnimData.CurrentState = EAnim_SurvivalState::Idle;
    CurrentAnimData.HealthPercentage = 100.0f;
    CurrentAnimData.StaminaPercentage = 100.0f;
}

void UAnim_SurvivalStateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Obter referências para componentes do personagem
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalStateManager: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalStateManager: Owner is not a Character!"));
    }
}

void UAnim_SurvivalStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Actualizar dados de movimento
    UpdateMovementData();
    
    // Actualizar lógica de estados de sobrevivência
    UpdateSurvivalStateLogic();
    
    // Analisar condições do terreno
    AnalyzeTerrainConditions();
    
    // Processar transições suaves entre estados
    SmoothStateTransition(DeltaTime);
}

void UAnim_SurvivalStateManager::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calcular velocidade e direcção
    FVector Velocity = MovementComponent->Velocity;
    CurrentAnimData.Speed = Velocity.Size();
    CurrentAnimData.bIsMoving = CurrentAnimData.Speed > 5.0f;
    
    // Calcular direcção relativa ao personagem
    if (CurrentAnimData.bIsMoving && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        CurrentAnimData.Direction = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    }
    else
    {
        CurrentAnimData.Direction = 0.0f;
    }
    
    // Estados de movimento especiais
    CurrentAnimData.bIsFalling = MovementComponent->IsFalling();
    CurrentAnimData.bIsJumping = MovementComponent->IsMovingOnGround() == false && 
                                 Velocity.Z > 0.0f;
}

void UAnim_SurvivalStateManager::UpdateSurvivalStateLogic()
{
    // Determinar o estado óptimo baseado nas condições actuais
    EAnim_SurvivalState OptimalState = DetermineOptimalState();
    
    // Definir estado alvo se for diferente do actual
    if (OptimalState != CurrentAnimData.CurrentState)
    {
        TargetState = OptimalState;
        PreviousState = CurrentAnimData.CurrentState;
        StateTransitionTimer = 0.0f;
    }
}

EAnim_SurvivalState UAnim_SurvivalStateManager::DetermineOptimalState() const
{
    // Prioridade 1: Estados críticos de sobrevivência
    if (CurrentAnimData.HealthPercentage <= InjuredThreshold)
    {
        return EAnim_SurvivalState::Injured;
    }
    
    if (CurrentAnimData.StaminaPercentage <= ExhaustedThreshold)
    {
        return EAnim_SurvivalState::Exhausted;
    }
    
    if (CurrentAnimData.FearLevel >= FearThreshold || CurrentAnimData.bIsInDanger)
    {
        return EAnim_SurvivalState::Fearful;
    }
    
    // Prioridade 2: Estados de movimento
    if (CurrentAnimData.bIsFalling || CurrentAnimData.bIsJumping)
    {
        return CurrentAnimData.CurrentState; // Manter estado actual durante salto/queda
    }
    
    if (CurrentAnimData.bIsHidden)
    {
        return EAnim_SurvivalState::Hiding;
    }
    
    // Prioridade 3: Estados baseados em velocidade
    if (CurrentAnimData.Speed > 300.0f) // Corrida
    {
        return EAnim_SurvivalState::Running;
    }
    else if (CurrentAnimData.Speed > 50.0f) // Caminhada
    {
        return EAnim_SurvivalState::Walking;
    }
    
    // Estado padrão
    return EAnim_SurvivalState::Idle;
}

void UAnim_SurvivalStateManager::SmoothStateTransition(float DeltaTime)
{
    if (TargetState != CurrentAnimData.CurrentState)
    {
        StateTransitionTimer += DeltaTime * StateTransitionSpeed;
        
        if (StateTransitionTimer >= 1.0f)
        {
            // Completar transição
            CurrentAnimData.CurrentState = TargetState;
            StateTransitionTimer = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("SurvivalStateManager: Transitioned to state %d"), 
                   (int32)CurrentAnimData.CurrentState);
        }
    }
}

void UAnim_SurvivalStateManager::AnalyzeTerrainConditions()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Raycast para baixo para detectar tipo de terreno
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    UWorld* World = GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
                                                  ECC_Visibility, QueryParams))
    {
        // Calcular inclinação do terreno
        FVector Normal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
        CurrentAnimData.TerrainSlope = SlopeAngle;
        
        // Determinar tipo de terreno baseado na inclinação e material
        if (SlopeAngle > 30.0f)
        {
            CurrentAnimData.TerrainType = EAnim_TerrainType::Steep;
        }
        else if (SlopeAngle > 15.0f)
        {
            CurrentAnimData.TerrainType = EAnim_TerrainType::Uneven;
        }
        else
        {
            CurrentAnimData.TerrainType = EAnim_TerrainType::Flat;
        }
        
        // Debug visual (apenas em desenvolvimento)
        #if WITH_EDITOR
        DrawDebugLine(World, StartLocation, HitResult.Location, FColor::Green, false, 0.1f);
        #endif
    }
}

void UAnim_SurvivalStateManager::SetSurvivalState(EAnim_SurvivalState NewState)
{
    if (NewState != CurrentAnimData.CurrentState)
    {
        PreviousState = CurrentAnimData.CurrentState;
        TargetState = NewState;
        StateTransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalStateManager: Manual state change to %d"), (int32)NewState);
    }
}

void UAnim_SurvivalStateManager::UpdateSurvivalStats(float Health, float Stamina, float Fear, float Hunger, float Thirst)
{
    CurrentAnimData.HealthPercentage = FMath::Clamp(Health, 0.0f, 100.0f);
    CurrentAnimData.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 100.0f);
    CurrentAnimData.FearLevel = FMath::Clamp(Fear, 0.0f, 100.0f);
    CurrentAnimData.HungerLevel = FMath::Clamp(Hunger, 0.0f, 100.0f);
    CurrentAnimData.ThirstLevel = FMath::Clamp(Thirst, 0.0f, 100.0f);
}

void UAnim_SurvivalStateManager::SetTerrainInfo(EAnim_TerrainType Terrain, float Slope)
{
    CurrentAnimData.TerrainType = Terrain;
    CurrentAnimData.TerrainSlope = FMath::Clamp(Slope, 0.0f, 90.0f);
}

void UAnim_SurvivalStateManager::SetDangerState(bool bInDanger)
{
    CurrentAnimData.bIsInDanger = bInDanger;
    
    if (bInDanger)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalStateManager: Character is in danger!"));
    }
}

bool UAnim_SurvivalStateManager::ShouldPlayInjuredAnimation() const
{
    return CurrentAnimData.HealthPercentage <= InjuredThreshold;
}

bool UAnim_SurvivalStateManager::ShouldPlayExhaustedAnimation() const
{
    return CurrentAnimData.StaminaPercentage <= ExhaustedThreshold;
}

bool UAnim_SurvivalStateManager::ShouldPlayFearfulAnimation() const
{
    return CurrentAnimData.FearLevel >= FearThreshold || CurrentAnimData.bIsInDanger;
}

float UAnim_SurvivalStateManager::GetMovementSpeedModifier() const
{
    float Modifier = 1.0f;
    
    // Reduzir velocidade baseado na saúde
    if (CurrentAnimData.HealthPercentage < 50.0f)
    {
        Modifier *= (CurrentAnimData.HealthPercentage / 50.0f) * 0.5f + 0.5f;
    }
    
    // Reduzir velocidade baseado na stamina
    if (CurrentAnimData.StaminaPercentage < 30.0f)
    {
        Modifier *= (CurrentAnimData.StaminaPercentage / 30.0f) * 0.3f + 0.7f;
    }
    
    // Modificar baseado no terreno
    switch (CurrentAnimData.TerrainType)
    {
        case EAnim_TerrainType::Muddy:
        case EAnim_TerrainType::Sandy:
            Modifier *= 0.8f;
            break;
        case EAnim_TerrainType::Rocky:
        case EAnim_TerrainType::Steep:
            Modifier *= 0.7f;
            break;
        case EAnim_TerrainType::Slippery:
            Modifier *= 0.6f;
            break;
        default:
            break;
    }
    
    return FMath::Clamp(Modifier, 0.1f, 1.5f);
}

float UAnim_SurvivalStateManager::GetAnimationPlayRate() const
{
    float PlayRate = 1.0f;
    
    // Acelerar animações quando com medo
    if (CurrentAnimData.FearLevel > 50.0f)
    {
        PlayRate *= 1.0f + (CurrentAnimData.FearLevel / 100.0f) * 0.3f;
    }
    
    // Desacelerar quando exausto ou ferido
    if (CurrentAnimData.StaminaPercentage < 30.0f || CurrentAnimData.HealthPercentage < 30.0f)
    {
        PlayRate *= 0.8f;
    }
    
    return FMath::Clamp(PlayRate, 0.5f, 1.5f);
}