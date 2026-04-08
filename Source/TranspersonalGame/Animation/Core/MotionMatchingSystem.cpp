#include "MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMotionMatchingSystem::UMotionMatchingSystem()
{
    // Configuração inicial
    BlendTime = 0.2f;
    FearLevel = 0.0f;
    TrustLevel = 0.0f;
    AlertnessLevel = 0.5f;
    
    Nervousness = 0.5f;
    Confidence = 0.5f;
    Curiosity = 0.5f;
    
    LastDatabaseSwitch = 0.0f;
    
    // Inicializar históricos
    FearHistory.Reserve(10);
    SpeedHistory.Reserve(10);
}

void UMotionMatchingSystem::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Configurar schema padrão se não estiver definido
    if (!PoseSearchSchema)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: PoseSearchSchema não definido!"));
    }
}

void UMotionMatchingSystem::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (APawn* Pawn = TryGetPawnOwner())
    {
        // Atualizar parâmetros de movimento
        if (ACharacter* Character = Cast<ACharacter>(Pawn))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                Velocity = MovementComp->Velocity;
                Speed = Velocity.Size();
                Direction = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, Velocity).Yaw;
                
                bIsMoving = Speed > 1.0f;
                bIsCrouching = MovementComp->IsCrouching();
                bIsRunning = Speed > 300.0f; // Threshold para corrida
            }
        }
        
        // Atualizar sistemas emocionais
        UpdateEmotionalBlending(DeltaTimeX);
        SmoothParameters(DeltaTimeX);
    }
}

void UMotionMatchingSystem::SetEmotionalState(float InFear, float InTrust, float InAlertness)
{
    FearLevel = FMath::Clamp(InFear, 0.0f, 1.0f);
    TrustLevel = FMath::Clamp(InTrust, 0.0f, 1.0f);
    AlertnessLevel = FMath::Clamp(InAlertness, 0.0f, 1.0f);
    
    // Trigger Blueprint event
    OnEmotionalStateChanged(FearLevel, TrustLevel, AlertnessLevel);
}

void UMotionMatchingSystem::UpdateMovementParameters(const FVector& InVelocity, bool bInCrouching, bool bInRunning)
{
    Velocity = InVelocity;
    Speed = Velocity.Size();
    bIsCrouching = bInCrouching;
    bIsRunning = bInRunning;
    bIsMoving = Speed > 1.0f;
    
    if (bIsMoving)
    {
        Direction = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, Velocity).Yaw;
    }
}

UPoseSearchDatabase* UMotionMatchingSystem::SelectDatabase()
{
    if (MotionDatabases.Num() == 0)
    {
        return nullptr;
    }
    
    // Lógica de seleção baseada no estado emocional
    int32 DatabaseIndex = 0;
    
    // Base: movimento normal
    if (FearLevel < 0.3f && TrustLevel < 0.3f)
    {
        DatabaseIndex = 0; // Neutral locomotion
    }
    // Medo alto: movimentos nervosos, olhares constantes
    else if (FearLevel > 0.7f)
    {
        DatabaseIndex = FMath::Min(1, MotionDatabases.Num() - 1); // Fearful locomotion
    }
    // Confiança alta: movimentos mais relaxados
    else if (TrustLevel > 0.7f)
    {
        DatabaseIndex = FMath::Min(2, MotionDatabases.Num() - 1); // Confident locomotion
    }
    // Estado de alerta: movimentos cautelosos
    else if (AlertnessLevel > 0.8f)
    {
        DatabaseIndex = FMath::Min(3, MotionDatabases.Num() - 1); // Alert locomotion
    }
    
    DatabaseIndex = FMath::Clamp(DatabaseIndex, 0, MotionDatabases.Num() - 1);
    
    // Cache para otimização
    if (CurrentDatabase != MotionDatabases[DatabaseIndex])
    {
        CurrentDatabase = MotionDatabases[DatabaseIndex];
        LastDatabaseSwitch = GetWorld()->GetTimeSeconds();
    }
    
    return CurrentDatabase;
}

void UMotionMatchingSystem::UpdateEmotionalBlending(float DeltaTime)
{
    // Adicionar ao histórico para suavização
    FearHistory.Add(FearLevel);
    if (FearHistory.Num() > 10)
    {
        FearHistory.RemoveAt(0);
    }
    
    SpeedHistory.Add(Speed);
    if (SpeedHistory.Num() > 10)
    {
        SpeedHistory.RemoveAt(0);
    }
    
    // Calcular médias suavizadas
    float SmoothedFear = 0.0f;
    for (float Fear : FearHistory)
    {
        SmoothedFear += Fear;
    }
    SmoothedFear /= FearHistory.Num();
    
    // Ajustar blend time baseado no estado emocional
    BlendTime = CalculateBlendTime();
}

void UMotionMatchingSystem::SmoothParameters(float DeltaTime)
{
    // Suavizar transições baseado na personalidade
    float SmoothingFactor = 1.0f - (Nervousness * 0.3f); // Personagens nervosos têm transições mais bruscas
    
    // Aplicar suavização aos parâmetros críticos
    // (Implementação específica dependeria dos parâmetros do Motion Matching)
}

float UMotionMatchingSystem::CalculateBlendTime() const
{
    float BaseBlendTime = 0.2f;
    
    // Medo alto = transições mais rápidas (reações instintivas)
    if (FearLevel > 0.7f)
    {
        BaseBlendTime *= 0.5f;
    }
    // Confiança alta = transições mais suaves
    else if (TrustLevel > 0.7f)
    {
        BaseBlendTime *= 1.5f;
    }
    
    // Nervosismo afeta a suavidade
    BaseBlendTime *= (1.0f + Nervousness * 0.5f);
    
    return FMath::Clamp(BaseBlendTime, 0.05f, 0.8f);
}