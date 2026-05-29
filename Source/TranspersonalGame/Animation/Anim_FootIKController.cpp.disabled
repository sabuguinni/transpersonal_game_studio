#include "Anim_FootIKController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_FootIKController::UAnim_FootIKController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Configuração padrão
    bEnableFootIK = true;
    TraceDistance = 150.0f;
    IKInterpSpeed = 15.0f;
    CapsuleHalfHeight = 88.0f;
    
    // Nomes dos sockets padrão do Mannequin UE5
    LeftFootSocketName = TEXT("foot_l");
    RightFootSocketName = TEXT("foot_r");
    PelvisSocketName = TEXT("pelvis");

    // Inicializar dados
    TargetPelvisOffset = 0.0f;
    CurrentPelvisOffset = 0.0f;
    TargetLeftFootOffset = FVector::ZeroVector;
    TargetRightFootOffset = FVector::ZeroVector;
    CurrentLeftFootOffset = FVector::ZeroVector;
    CurrentRightFootOffset = FVector::ZeroVector;
    TargetLeftFootRotation = FRotator::ZeroRotator;
    TargetRightFootRotation = FRotator::ZeroRotator;
    CurrentLeftFootRotation = FRotator::ZeroRotator;
    CurrentRightFootRotation = FRotator::ZeroRotator;

    OwnerMeshComponent = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_FootIKController::BeginPlay()
{
    Super::BeginPlay();

    // Cache dos componentes do owner
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMeshComponent = OwnerCharacter->GetMesh();
        
        // Ajustar CapsuleHalfHeight baseado no capsule component do character
        if (OwnerCharacter->GetCapsuleComponent())
        {
            CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        }
    }
}

void UAnim_FootIKController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableFootIK && OwnerMeshComponent && OwnerCharacter)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_FootIKController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !OwnerCharacter)
    {
        return;
    }

    // Verificar se o personagem está no chão
    if (!OwnerCharacter->GetCharacterMovement() || OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        // Se está caindo, desabilitar IK gradualmente
        FootIKData.IKAlpha = FMath::FInterpTo(FootIKData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }

    // Habilitar IK quando no chão
    FootIKData.IKAlpha = FMath::FInterpTo(FootIKData.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);

    // Calcular offsets dos pés
    float LeftFootRotation = 0.0f;
    float RightFootRotation = 0.0f;
    
    float LeftFootOffset = GetFootOffsetFromGround(LeftFootSocketName, LeftFootRotation);
    float RightFootOffset = GetFootOffsetFromGround(RightFootSocketName, RightFootRotation);

    // Atualizar targets
    TargetLeftFootOffset = FVector(0.0f, 0.0f, LeftFootOffset);
    TargetRightFootOffset = FVector(0.0f, 0.0f, RightFootOffset);
    TargetLeftFootRotation = FRotator(0.0f, 0.0f, LeftFootRotation);
    TargetRightFootRotation = FRotator(0.0f, 0.0f, RightFootRotation);

    // Atualizar offset da pelvis
    UpdatePelvisOffset(LeftFootOffset, RightFootOffset);

    // Interpolar para valores suaves
    CurrentLeftFootOffset = FMath::VInterpTo(CurrentLeftFootOffset, TargetLeftFootOffset, DeltaTime, IKInterpSpeed);
    CurrentRightFootOffset = FMath::VInterpTo(CurrentRightFootOffset, TargetRightFootOffset, DeltaTime, IKInterpSpeed);
    CurrentLeftFootRotation = FMath::RInterpTo(CurrentLeftFootRotation, TargetLeftFootRotation, DeltaTime, IKInterpSpeed);
    CurrentRightFootRotation = FMath::RInterpTo(CurrentRightFootRotation, TargetRightFootRotation, DeltaTime, IKInterpSpeed);
    CurrentPelvisOffset = FMath::FInterpTo(CurrentPelvisOffset, TargetPelvisOffset, DeltaTime, IKInterpSpeed);

    // Atualizar dados finais
    FootIKData.LeftFootOffset = CurrentLeftFootOffset;
    FootIKData.RightFootOffset = CurrentRightFootOffset;
    FootIKData.LeftFootRotation = CurrentLeftFootRotation;
    FootIKData.RightFootRotation = CurrentRightFootRotation;
    FootIKData.PelvisOffset = CurrentPelvisOffset;
}

float UAnim_FootIKController::GetFootOffsetFromGround(const FName& SocketName, float& OutRotation)
{
    if (!OwnerMeshComponent)
    {
        OutRotation = 0.0f;
        return 0.0f;
    }

    // Obter posição do socket do pé
    FVector SocketLocation = OwnerMeshComponent->GetSocketLocation(SocketName);
    
    // Ajustar para posição relativa ao chão
    FVector StartTrace = SocketLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight);
    
    float HitDistance = 0.0f;
    FRotator SurfaceRotation = FRotator::ZeroRotator;
    FVector HitLocation = PerformFootTrace(StartTrace, HitDistance, SurfaceRotation);

    // Calcular offset necessário
    float FootOffset = (TraceDistance - HitDistance) - CapsuleHalfHeight;
    
    // Calcular rotação do pé baseada na superfície
    OutRotation = SurfaceRotation.Roll;

    return FootOffset;
}

FVector UAnim_FootIKController::PerformFootTrace(const FVector& StartLocation, float& OutHitDistance, FRotator& OutSurfaceRotation)
{
    if (!GetWorld())
    {
        OutHitDistance = TraceDistance;
        OutSurfaceRotation = FRotator::ZeroRotator;
        return StartLocation;
    }

    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutHitDistance = HitResult.Distance;
        
        // Calcular rotação baseada na normal da superfície
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        OutSurfaceRotation = FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
        
        return HitResult.Location;
    }
    else
    {
        OutHitDistance = TraceDistance;
        OutSurfaceRotation = FRotator::ZeroRotator;
        return EndLocation;
    }
}

void UAnim_FootIKController::UpdatePelvisOffset(float LeftFootOffset, float RightFootOffset)
{
    // Usar o menor offset (mais negativo) para ajustar a pelvis
    // Isso garante que ambos os pés toquem o chão
    float MinOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    // Só ajustar pelvis se houver offset negativo (pé precisa descer)
    if (MinOffset < 0.0f)
    {
        TargetPelvisOffset = MinOffset;
    }
    else
    {
        TargetPelvisOffset = 0.0f;
    }
}

void UAnim_FootIKController::ResetFootIK()
{
    FootIKData = FAnim_FootIKData();
    
    TargetPelvisOffset = 0.0f;
    CurrentPelvisOffset = 0.0f;
    TargetLeftFootOffset = FVector::ZeroVector;
    TargetRightFootOffset = FVector::ZeroVector;
    CurrentLeftFootOffset = FVector::ZeroVector;
    CurrentRightFootOffset = FVector::ZeroVector;
    TargetLeftFootRotation = FRotator::ZeroRotator;
    TargetRightFootRotation = FRotator::ZeroRotator;
    CurrentLeftFootRotation = FRotator::ZeroRotator;
    CurrentRightFootRotation = FRotator::ZeroRotator;
}