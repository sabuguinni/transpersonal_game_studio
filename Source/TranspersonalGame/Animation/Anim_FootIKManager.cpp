#include "Anim_FootIKManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UAnim_FootIKManager::UAnim_FootIKManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Configurações padrão
    bEnableFootIK = true;
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    FootHeight = 5.0f;
    MaxHipAdjustment = 30.0f;
    bShowDebugTraces = false;

    // Nomes dos bones padrão (Manny skeleton)
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    HipBoneName = FName("pelvis");

    // Inicializar dados
    IKData = FAnim_FootIKData();
    TargetIKData = FAnim_FootIKData();
    PreviousIKData = FAnim_FootIKData();

    // Configurar trace object types
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
}

void UAnim_FootIKManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_FootIKManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableFootIK && OwnerMeshComponent && OwnerCharacter)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_FootIKManager::InitializeComponent()
{
    // Obter referências do owner
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMeshComponent = OwnerCharacter->GetMesh();
        
        // Configurar actors para ignorar no trace
        ActorsToIgnore.Empty();
        ActorsToIgnore.Add(OwnerCharacter);
    }
}

void UAnim_FootIKManager::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !OwnerCharacter)
    {
        return;
    }

    // Verificar se o personagem está no chão
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp || MovementComp->IsFalling())
    {
        // Se estiver no ar, gradualmente desabilitar IK
        TargetIKData.LeftFootIKAlpha = FMath::FInterpTo(TargetIKData.LeftFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        TargetIKData.RightFootIKAlpha = FMath::FInterpTo(TargetIKData.RightFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        TargetIKData.HipOffset = FMath::FInterpTo(TargetIKData.HipOffset, 0.0f, DeltaTime, IKInterpSpeed);
        
        InterpolateIKData(DeltaTime);
        return;
    }

    // Atualizar cada pé
    UpdateSingleFoot(LeftFootBoneName, TargetIKData.LeftFootLocation, TargetIKData.LeftFootRotation, TargetIKData.LeftFootIKAlpha, DeltaTime);
    UpdateSingleFoot(RightFootBoneName, TargetIKData.RightFootLocation, TargetIKData.RightFootRotation, TargetIKData.RightFootIKAlpha, DeltaTime);

    // Calcular offset do hip
    CalculateHipOffset(DeltaTime);

    // Interpolar dados finais
    InterpolateIKData(DeltaTime);

    // Debug visual
    if (bShowDebugTraces)
    {
        DrawDebugInfo();
    }
}

void UAnim_FootIKManager::UpdateSingleFoot(FName BoneName, FVector& FootLocation, FRotator& FootRotation, float& IKAlpha, float DeltaTime)
{
    // Obter posição mundial do pé
    FVector FootWorldLocation = GetFootWorldLocation(BoneName);
    if (FootWorldLocation.IsZero())
    {
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }

    // Realizar trace para baixo
    FVector HitLocation;
    FVector HitNormal;
    bool bHit = PerformFootTrace(FootWorldLocation, HitLocation, HitNormal);

    if (bHit)
    {
        // Calcular nova posição e rotação do pé
        FVector NewFootLocation = HitLocation + (HitNormal * FootHeight);
        FRotator CurrentFootRotation = OwnerMeshComponent->GetBoneTransform(BoneName).GetRotation().Rotator();
        FRotator NewFootRotation = CalculateFootRotation(HitNormal, CurrentFootRotation);

        // Converter para espaço local do componente
        FTransform ComponentTransform = OwnerMeshComponent->GetComponentTransform();
        FootLocation = ComponentTransform.InverseTransformPosition(NewFootLocation);
        FootRotation = ComponentTransform.InverseTransformRotation(NewFootRotation.Quaternion()).Rotator();

        // Ativar IK
        IKAlpha = FMath::FInterpTo(IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        // Desativar IK gradualmente
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
}

bool UAnim_FootIKManager::PerformFootTrace(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }

    // Configurar trace
    FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, TraceDistance);

    FHitResult HitResult;
    bool bHit = UKismetSystemLibrary::LineTraceMultiForObjects(
        GetWorld(),
        TraceStart,
        TraceEnd,
        TraceObjectTypes,
        false,
        ActorsToIgnore,
        bShowDebugTraces ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
        HitResult,
        true
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }

    return false;
}

FVector UAnim_FootIKManager::GetFootWorldLocation(FName BoneName) const
{
    if (!OwnerMeshComponent)
    {
        return FVector::ZeroVector;
    }

    // Verificar se o bone existe
    int32 BoneIndex = OwnerMeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        return FVector::ZeroVector;
    }

    // Obter transform do bone em world space
    FTransform BoneTransform = OwnerMeshComponent->GetBoneTransform(BoneName);
    return BoneTransform.GetLocation();
}

FRotator UAnim_FootIKManager::CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation) const
{
    // Calcular rotação baseada na normal da superfície
    FVector ForwardVector = CurrentRotation.Vector();
    FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();

    return UKismetMathLibrary::MakeRotFromXZ(ForwardVector, HitNormal);
}

void UAnim_FootIKManager::CalculateHipOffset(float DeltaTime)
{
    // Calcular offset baseado na diferença de altura entre os pés
    float LeftFootHeight = TargetIKData.LeftFootLocation.Z;
    float RightFootHeight = TargetIKData.RightFootLocation.Z;
    
    // Usar o pé mais baixo como referência
    float LowestFootHeight = FMath::Min(LeftFootHeight, RightFootHeight);
    
    // Aplicar offset no hip apenas se necessário
    float DesiredHipOffset = FMath::Clamp(LowestFootHeight, -MaxHipAdjustment, 0.0f);
    TargetIKData.HipOffset = FMath::FInterpTo(TargetIKData.HipOffset, DesiredHipOffset, DeltaTime, IKInterpSpeed);
}

void UAnim_FootIKManager::InterpolateIKData(float DeltaTime)
{
    // Interpolar suavemente todos os dados de IK
    IKData.LeftFootLocation = FMath::VInterpTo(IKData.LeftFootLocation, TargetIKData.LeftFootLocation, DeltaTime, IKInterpSpeed);
    IKData.RightFootLocation = FMath::VInterpTo(IKData.RightFootLocation, TargetIKData.RightFootLocation, DeltaTime, IKInterpSpeed);
    
    IKData.LeftFootRotation = FMath::RInterpTo(IKData.LeftFootRotation, TargetIKData.LeftFootRotation, DeltaTime, IKInterpSpeed);
    IKData.RightFootRotation = FMath::RInterpTo(IKData.RightFootRotation, TargetIKData.RightFootRotation, DeltaTime, IKInterpSpeed);
    
    IKData.LeftFootIKAlpha = FMath::FInterpTo(IKData.LeftFootIKAlpha, TargetIKData.LeftFootIKAlpha, DeltaTime, IKInterpSpeed);
    IKData.RightFootIKAlpha = FMath::FInterpTo(IKData.RightFootIKAlpha, TargetIKData.RightFootIKAlpha, DeltaTime, IKInterpSpeed);
    
    IKData.HipOffset = FMath::FInterpTo(IKData.HipOffset, TargetIKData.HipOffset, DeltaTime, IKInterpSpeed);
}

void UAnim_FootIKManager::DrawDebugInfo() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }

    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Debug info text
    FString DebugText = FString::Printf(
        TEXT("Foot IK Debug:\nLeft Alpha: %.2f\nRight Alpha: %.2f\nHip Offset: %.2f"),
        IKData.LeftFootIKAlpha,
        IKData.RightFootIKAlpha,
        IKData.HipOffset
    );
    
    DrawDebugString(GetWorld(), CharacterLocation + FVector(0, 0, 100), DebugText, nullptr, FColor::Yellow, 0.0f);
    
    // Debug foot positions
    FVector LeftFootWorld = OwnerMeshComponent->GetComponentTransform().TransformPosition(IKData.LeftFootLocation);
    FVector RightFootWorld = OwnerMeshComponent->GetComponentTransform().TransformPosition(IKData.RightFootLocation);
    
    DrawDebugSphere(GetWorld(), LeftFootWorld, 5.0f, 8, FColor::Red, false, 0.0f);
    DrawDebugSphere(GetWorld(), RightFootWorld, 5.0f, 8, FColor::Blue, false, 0.0f);
}

void UAnim_FootIKManager::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
    
    if (!bEnable)
    {
        // Reset IK data
        TargetIKData = FAnim_FootIKData();
    }
}

void UAnim_FootIKManager::SetFootIKSettings(float NewTraceDistance, float NewInterpSpeed, float NewFootHeight)
{
    TraceDistance = FMath::Max(NewTraceDistance, 10.0f);
    IKInterpSpeed = FMath::Max(NewInterpSpeed, 1.0f);
    FootHeight = FMath::Max(NewFootHeight, 0.0f);
}