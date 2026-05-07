#include "Anim_DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_DinosaurAnimInstance::UAnim_DinosaurAnimInstance()
{
    // Configurações padrão
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    ChargeSpeedThreshold = 600.0f;
    AggressionSensitivity = 0.5f;
    TerritorialRadius = 1000.0f;
    
    // Variáveis de controle
    LastDirection = 0.0f;
    TimeInCurrentBehavior = 0.0f;
    LastAggressionCheck = 0.0f;
    ThreatResponseCooldown = 0.0f;
    bWasMoving = false;
    LastKnownThreatLocation = FVector::ZeroVector;
    
    // Assets de animação (serão configurados via Blueprint)
    CarnivorousLocomotionBlendSpace = nullptr;
    HerbivorousLocomotionBlendSpace = nullptr;
    FlyingLocomotionBlendSpace = nullptr;
    AttackMontage = nullptr;
    RoarMontage = nullptr;
    FeedingMontage = nullptr;
    DrinkingMontage = nullptr;
    AlertMontage = nullptr;
    DeathMontage = nullptr;
}

void UAnim_DinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerPawn = TryGetPawnOwner();
    
    if (OwnerPawn)
    {
        // Configurar espécie baseada no nome do pawn ou tag
        FString PawnName = OwnerPawn->GetName();
        if (PawnName.Contains(TEXT("TRex")))
        {
            SetSpecies(EAnim_DinosaurSpecies::TRex);
        }
        else if (PawnName.Contains(TEXT("Raptor")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Raptor);
        }
        else if (PawnName.Contains(TEXT("Triceratops")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Triceratops);
        }
        else if (PawnName.Contains(TEXT("Brachiosaurus")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Brachiosaurus);
        }
        else if (PawnName.Contains(TEXT("Stegosaurus")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Stegosaurus);
        }
        else if (PawnName.Contains(TEXT("Pteranodon")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Pteranodon);
        }
        else if (PawnName.Contains(TEXT("Ankylosaurus")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Ankylosaurus);
        }
        else if (PawnName.Contains(TEXT("Compsognathus")))
        {
            SetSpecies(EAnim_DinosaurSpecies::Compsognathus);
        }
        
        ConfigureSpeciesSettings();
    }
}

void UAnim_DinosaurAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerPawn)
    {
        return;
    }
    
    UpdateMovementData(DeltaTimeX);
    UpdateBehaviorState();
    UpdateEnvironmentalAwareness();
    UpdateSpeciesSpecificBehavior();
    
    // Atualizar timers
    TimeInCurrentBehavior += DeltaTimeX;
    LastAggressionCheck += DeltaTimeX;
    
    if (ThreatResponseCooldown > 0.0f)
    {
        ThreatResponseCooldown -= DeltaTimeX;
    }
}

void UAnim_DinosaurAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwnerPawn)
    {
        return;
    }
    
    // Obter velocidade atual
    FVector Velocity = OwnerPawn->GetVelocity();
    AnimData.Speed = Velocity.Size();
    
    // Calcular direção de movimento
    if (AnimData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerPawn->GetActorForwardVector();
        FVector MovementDirection = Velocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, MovementDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, MovementDirection).Z;
        
        AnimData.Direction = FMath::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
        
        // Suavizar mudanças de direção
        AnimData.Direction = FMath::FInterpTo(LastDirection, AnimData.Direction, DeltaTime, 5.0f);
        LastDirection = AnimData.Direction;
        
        AnimData.bIsMoving = true;
        bWasMoving = true;
    }
    else
    {
        AnimData.bIsMoving = false;
        if (bWasMoving)
        {
            // Resetar timer quando para de se mover
            TimeInCurrentBehavior = 0.0f;
            bWasMoving = false;
        }
    }
    
    // Atualizar estado de movimento
    AnimData.MovementState = CalculateMovementState();
}

void UAnim_DinosaurAnimInstance::UpdateBehaviorState()
{
    EAnim_DinosaurBehaviorState NewBehaviorState = CalculateBehaviorState();
    
    if (NewBehaviorState != AnimData.BehaviorState)
    {
        AnimData.BehaviorState = NewBehaviorState;
        TimeInCurrentBehavior = 0.0f;
        
        // Reproduzir montage apropriado para o novo comportamento
        PlayBehaviorMontage(NewBehaviorState);
    }
}

void UAnim_DinosaurAnimInstance::UpdateEnvironmentalAwareness()
{
    if (!OwnerPawn || !OwnerPawn->GetWorld())
    {
        return;
    }
    
    FVector PawnLocation = OwnerPawn->GetActorLocation();
    
    // Verificar proximidade com água (simplificado)
    // TODO: Implementar detecção real de água quando o sistema de água estiver pronto
    AnimData.bNearWater = false;
    
    // Verificar proximidade com comida (simplificado)
    // TODO: Implementar detecção de plantas/presas quando o sistema estiver pronto
    AnimData.bNearFood = false;
    
    // Verificar proximidade com ameaças (simplificado)
    // TODO: Implementar detecção de jogadores/outros dinossauros
    AnimData.bNearThreat = false;
    
    // Atualizar nível de alerta baseado no ambiente
    if (AnimData.bNearThreat)
    {
        AnimData.AlertnessLevel = FMath::Min(1.0f, AnimData.AlertnessLevel + 0.1f);
    }
    else
    {
        AnimData.AlertnessLevel = FMath::Max(0.0f, AnimData.AlertnessLevel - 0.05f);
    }
}

void UAnim_DinosaurAnimInstance::UpdateSpeciesSpecificBehavior()
{
    switch (AnimData.Species)
    {
        case EAnim_DinosaurSpecies::TRex:
            // T-Rex: Comportamento de predador alfa
            if (AnimData.HungerLevel > 0.7f && AnimData.bNearFood)
            {
                AnimData.AggressionLevel = FMath::Min(1.0f, AnimData.AggressionLevel + 0.02f);
            }
            break;
            
        case EAnim_DinosaurSpecies::Raptor:
            // Raptor: Comportamento de caça em grupo
            if (AnimData.bNearThreat)
            {
                AnimData.AggressionLevel = FMath::Min(1.0f, AnimData.AggressionLevel + 0.05f);
            }
            break;
            
        case EAnim_DinosaurSpecies::Triceratops:
            // Triceratops: Comportamento defensivo
            if (AnimData.bNearThreat)
            {
                AnimData.AggressionLevel = FMath::Min(0.6f, AnimData.AggressionLevel + 0.03f);
            }
            break;
            
        case EAnim_DinosaurSpecies::Brachiosaurus:
            // Brachiosaurus: Comportamento pacífico
            AnimData.AggressionLevel = FMath::Max(0.0f, AnimData.AggressionLevel - 0.01f);
            break;
            
        default:
            break;
    }
    
    // Reduzir agressão gradualmente se não houver estímulos
    if (!AnimData.bNearThreat && !AnimData.bNearFood)
    {
        AnimData.AggressionLevel = FMath::Max(0.0f, AnimData.AggressionLevel - 0.01f);
    }
}

EAnim_DinosaurMovementState UAnim_DinosaurAnimInstance::CalculateMovementState() const
{
    if (!AnimData.bIsMoving)
    {
        // Determinar tipo de idle baseado no comportamento
        switch (AnimData.BehaviorState)
        {
            case EAnim_DinosaurBehaviorState::Feeding:
                return EAnim_DinosaurMovementState::Grazing;
            case EAnim_DinosaurBehaviorState::Passive:
                if (TimeInCurrentBehavior > 30.0f) // Dormir após 30 segundos parado
                {
                    return EAnim_DinosaurMovementState::Sleeping;
                }
                break;
            default:
                break;
        }
        
        return EAnim_DinosaurMovementState::Idle;
    }
    
    // Estados baseados na velocidade e comportamento
    if (AnimData.Speed >= ChargeSpeedThreshold && AnimData.BehaviorState == EAnim_DinosaurBehaviorState::Aggressive)
    {
        return EAnim_DinosaurMovementState::Charging;
    }
    else if (AnimData.Speed >= RunSpeedThreshold)
    {
        return EAnim_DinosaurMovementState::Running;
    }
    else if (AnimData.Speed >= WalkSpeedThreshold)
    {
        if (AnimData.BehaviorState == EAnim_DinosaurBehaviorState::Hunting)
        {
            return EAnim_DinosaurMovementState::Stalking;
        }
        return EAnim_DinosaurMovementState::Walking;
    }
    
    return EAnim_DinosaurMovementState::Idle;
}

EAnim_DinosaurBehaviorState UAnim_DinosaurAnimInstance::CalculateBehaviorState() const
{
    // Prioridade: Ameaça > Fome > Estado padrão
    
    if (AnimData.bNearThreat)
    {
        if (AnimData.AggressionLevel > 0.7f)
        {
            return EAnim_DinosaurBehaviorState::Aggressive;
        }
        else if (AnimData.Species == EAnim_DinosaurSpecies::Compsognathus || 
                 AnimData.Species == EAnim_DinosaurSpecies::Pteranodon)
        {
            return EAnim_DinosaurBehaviorState::Fleeing;
        }
        else
        {
            return EAnim_DinosaurBehaviorState::Alert;
        }
    }
    
    if (AnimData.HungerLevel > 0.6f)
    {
        if (AnimData.bNearFood)
        {
            return EAnim_DinosaurBehaviorState::Feeding;
        }
        else if (AnimData.Species == EAnim_DinosaurSpecies::TRex || 
                 AnimData.Species == EAnim_DinosaurSpecies::Raptor)
        {
            return EAnim_DinosaurBehaviorState::Hunting;
        }
    }
    
    if (AnimData.bNearWater && AnimData.HungerLevel < 0.3f)
    {
        return EAnim_DinosaurBehaviorState::Passive; // Bebendo
    }
    
    return EAnim_DinosaurBehaviorState::Passive;
}

void UAnim_DinosaurAnimInstance::PlayBehaviorMontage(EAnim_DinosaurBehaviorState BehaviorType)
{
    UAnimMontage* MontageToPlay = GetMontageForBehavior(BehaviorType);
    
    if (MontageToPlay && !Montage_IsPlaying(MontageToPlay))
    {
        Montage_Play(MontageToPlay, 1.0f);
    }
}

void UAnim_DinosaurAnimInstance::StopBehaviorMontage()
{
    if (IsAnyMontagePlaying())
    {
        Montage_StopGroupByName(0.2f, TEXT("DefaultGroup"));
    }
}

void UAnim_DinosaurAnimInstance::SetSpecies(EAnim_DinosaurSpecies NewSpecies)
{
    AnimData.Species = NewSpecies;
    ConfigureSpeciesSettings();
}

void UAnim_DinosaurAnimInstance::SetAggressionLevel(float NewAggression)
{
    AnimData.AggressionLevel = FMath::Clamp(NewAggression, 0.0f, 1.0f);
}

void UAnim_DinosaurAnimInstance::SetHungerLevel(float NewHunger)
{
    AnimData.HungerLevel = FMath::Clamp(NewHunger, 0.0f, 1.0f);
}

void UAnim_DinosaurAnimInstance::TriggerThreatResponse()
{
    if (ThreatResponseCooldown <= 0.0f)
    {
        AnimData.bNearThreat = true;
        AnimData.AlertnessLevel = 1.0f;
        ThreatResponseCooldown = 5.0f; // Cooldown de 5 segundos
        
        // Reproduzir som de alerta se disponível
        if (AlertMontage)
        {
            Montage_Play(AlertMontage, 1.0f);
        }
    }
}

UAnimMontage* UAnim_DinosaurAnimInstance::GetMontageForBehavior(EAnim_DinosaurBehaviorState BehaviorType) const
{
    switch (BehaviorType)
    {
        case EAnim_DinosaurBehaviorState::Aggressive:
            return AttackMontage;
        case EAnim_DinosaurBehaviorState::Alert:
            return AlertMontage;
        case EAnim_DinosaurBehaviorState::Feeding:
            return FeedingMontage;
        case EAnim_DinosaurBehaviorState::Territorial:
            return RoarMontage;
        default:
            return nullptr;
    }
}

UBlendSpace* UAnim_DinosaurAnimInstance::GetBlendSpaceForSpecies() const
{
    switch (AnimData.Species)
    {
        case EAnim_DinosaurSpecies::TRex:
        case EAnim_DinosaurSpecies::Raptor:
        case EAnim_DinosaurSpecies::Compsognathus:
            return CarnivorousLocomotionBlendSpace;
            
        case EAnim_DinosaurSpecies::Triceratops:
        case EAnim_DinosaurSpecies::Brachiosaurus:
        case EAnim_DinosaurSpecies::Stegosaurus:
        case EAnim_DinosaurSpecies::Ankylosaurus:
            return HerbivorousLocomotionBlendSpace;
            
        case EAnim_DinosaurSpecies::Pteranodon:
            return FlyingLocomotionBlendSpace;
            
        default:
            return CarnivorousLocomotionBlendSpace;
    }
}

void UAnim_DinosaurAnimInstance::ConfigureSpeciesSettings()
{
    switch (AnimData.Species)
    {
        case EAnim_DinosaurSpecies::TRex:
            WalkSpeedThreshold = 150.0f;
            RunSpeedThreshold = 400.0f;
            ChargeSpeedThreshold = 800.0f;
            AggressionSensitivity = 0.8f;
            TerritorialRadius = 2000.0f;
            break;
            
        case EAnim_DinosaurSpecies::Raptor:
            WalkSpeedThreshold = 100.0f;
            RunSpeedThreshold = 500.0f;
            ChargeSpeedThreshold = 900.0f;
            AggressionSensitivity = 0.9f;
            TerritorialRadius = 1500.0f;
            break;
            
        case EAnim_DinosaurSpecies::Brachiosaurus:
            WalkSpeedThreshold = 80.0f;
            RunSpeedThreshold = 200.0f;
            ChargeSpeedThreshold = 300.0f;
            AggressionSensitivity = 0.2f;
            TerritorialRadius = 3000.0f;
            break;
            
        case EAnim_DinosaurSpecies::Compsognathus:
            WalkSpeedThreshold = 200.0f;
            RunSpeedThreshold = 600.0f;
            ChargeSpeedThreshold = 1000.0f;
            AggressionSensitivity = 0.3f;
            TerritorialRadius = 500.0f;
            break;
            
        default:
            // Configurações padrão já definidas no construtor
            break;
    }
}